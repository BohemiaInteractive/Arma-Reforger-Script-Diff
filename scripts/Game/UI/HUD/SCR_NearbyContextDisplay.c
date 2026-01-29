//#define DEBUG_NEARBY_CONTEXT_DISPLAY

class SCR_NearbyContextDisplay : SCR_InfoDisplayExtended
{
	[Attribute("5", UIWidgets.Slider, "Maximum amount of individual cached Widgets per kind.")]
	protected int m_iMaxPrecachedWidgets;

	[Attribute("{BF5FA7B21D658280}UI/layouts/HUD/InteractionSystem/ContextBasicInteractionBlip.layout", UIWidgets.ResourceNamePicker, "Layout", "layout", category: "NearbyContextDisplay")]
	ResourceName m_sIconLayoutPath;

	[Attribute("", UIWidgets.ResourceNamePicker, "edds", "edds")]
	protected ResourceName m_sDefaultIconImage;

	[Attribute("", desc: "SCR_NearbyContextCachingConfig Config that hold all the Widgets that will be cached.", params:"conf class=SCR_NearbyContextCachingConfig")]
	protected ResourceName m_sCachingConfigResource;

	[Attribute("0.2", desc: "Minimum Opacity of the Blip Widget when calculating Opacity based on distance to Camera.")]
	protected float m_fMinWidgetAlpha;

	[Attribute("", desc: "each ID must be unique")]
	protected ref array<ref SCR_NearbyContextColorsComponentInteract> m_aNearbyWidgetColors;

	protected ref SCR_NearbyContextCachingConfig m_CachingConfig;

	//! Interaction handler attached to parent entity.
	protected SCR_InteractionHandlerComponent m_InteractionHandlerComponent;

	protected ChimeraCharacter m_Character;

	//! Array of widgets that are filled and re-used as deemed neccessary.
	private ref array<Widget> m_aWidgets = {};
	//! Currently visible widgets.
	private int m_iVisibleWidgets;
	//! Original widget width.
	private float m_fOriginalSizeX;
	//! Original widget height.
	private float m_fOriginalSizeY;
	//! Distance in meters in which a context can be interacted with. Defined in 'InteractionHandlerComponent' on DefaultPlayerController entity.
	protected float m_fInteractionDistance;
	//! Distnace in meters in which contexts will be collected and visible to indicate a possible interaction. Defined in 'InteractionHandlerComponent' on DefaultPlayerController entity.
	protected float m_fContextCollectDistance;

	protected bool m_bIsInitialized;

	protected string m_sDefaultMapKey;
	protected ref array<Widget> m_aCurrentlyUsedWidgets = {};

	protected ref array<SCR_NearbyContextCachingData> m_aConfigLayouts = {};

	//! Holds all the different cached Widgets with the Layout GUID as key
	protected ref map<string, ref array<Widget>> m_mCachedWidgets = new map<string, ref array<Widget>>();

	//! Holds all the different cached id with the enum as key
	protected ref map<SCR_ENearbyInteractionContextColors, ref SCR_NearbyContextColorsComponentInteract> m_mCachedActionColors;

	protected ref TraceParam m_RaycastParam = new TraceParam();

	//------------------------------------------------------------------------------------------------
	//! Performs cleanup.
	void ~SCR_NearbyContextDisplay()
	{
		int widgetsCount = m_aWidgets.Count();
		for (int i = widgetsCount-1; i >= 0; i--)
		{
			if (m_aWidgets[i])
				m_aWidgets[i].RemoveFromHierarchy();
		}

		m_aWidgets.Clear();
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
		{
			Print("The SCR_NearbyContextDisplay root is NULL, the pre-cached SCR_NearbyContextDisplay widgets cannot be re-rooted!", LogLevel.ERROR);
			return;
		}

		m_bIsInitialized = true;

		// Re-root the blips to the info display root
		foreach (Widget w : m_aWidgets)
		{
			m_wRoot.AddChild(w);
		}

		m_sDefaultMapKey = m_sIconLayoutPath;

		m_RaycastParam.Flags = TraceFlags.WORLD | TraceFlags.ENTS;

		// For better performance the Cached Widgets are only checked ervery 10 seconds instead of on each update.
		GetGame().GetCallqueue().CallLater(ClearCachedWidgets, 10000, true);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayInit(IEntity owner)
	{
		m_InteractionHandlerComponent = SCR_InteractionHandlerComponent.Cast(owner.FindComponent(SCR_InteractionHandlerComponent));
		if (!m_InteractionHandlerComponent)
			Print("Interaction handler component could not be found! SCR_NearbyContextDisplay will not work.", LogLevel.WARNING);

		m_fInteractionDistance = m_InteractionHandlerComponent.GetVisibilityRange();
		m_fContextCollectDistance = m_InteractionHandlerComponent.GetNearbyCollectionRadius();

		if (m_sCachingConfigResource)
		{
			Resource rsc = BaseContainerTools.LoadContainer(m_sCachingConfigResource);
			if (rsc && rsc.IsValid())
			{
				BaseContainer container = rsc.GetResource().ToBaseContainer();
				if (container)
					m_CachingConfig = SCR_NearbyContextCachingConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
			}
		}

		if (!m_CachingConfig)
			return;

		m_CachingConfig.GetCachedLayouts(m_aConfigLayouts);

		// Precache all wanted widgets
		foreach (SCR_NearbyContextCachingData data : m_aConfigLayouts)
		{
			ResourceName layout;
			int precacheAmount;

			data.GetLayout(layout);
			data.GetPrecacheAmount(precacheAmount);

			if (!layout)
				continue;

			CacheWidget(layout, precacheAmount);
		}

		if (m_aNearbyWidgetColors)
		{
			m_mCachedActionColors = new map<SCR_ENearbyInteractionContextColors, ref SCR_NearbyContextColorsComponentInteract>();
			foreach (SCR_NearbyContextColorsComponentInteract colorset : m_aNearbyWidgetColors)
			{
				if (!m_mCachedActionColors.Contains(colorset.m_eId))
					m_mCachedActionColors.Insert(colorset.m_eId, colorset);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		if (!m_aCurrentlyUsedWidgets.IsEmpty())
		{
			foreach (Widget w : m_aCurrentlyUsedWidgets)
			{
				if (w)
					w.SetVisible(false);
			}

			m_aCurrentlyUsedWidgets.Clear();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		bool bCameraActive = m_CameraHandler && m_CameraHandler.IsCameraActive();
		// m_InteractionHandlerComponent.SetNearbyCollectionEnabled(true);
		if (!bCameraActive || !m_InteractionHandlerComponent.GetNearbyCollectionEnabled())
		{
			// if we are currently displaying something, clear it.
			if (!m_aCurrentlyUsedWidgets.IsEmpty())
			{
				foreach (Widget w : m_aCurrentlyUsedWidgets)
				{
					if (w)
						w.SetVisible(false);
				}

				m_aCurrentlyUsedWidgets.Clear();
			}

			return;
		}

		// Fetch nearby contexts
		array<UserActionContext> contexts = {};
		m_InteractionHandlerComponent.GetNearbyAvailableContextList(contexts);
		
		array<BaseUserAction> outActions = {};
		array<UserActionContext> additionalContexts = {};
		m_InteractionHandlerComponent.GetNearbyShowableContextList(additionalContexts);
		foreach(UserActionContext ctx: additionalContexts)
		{
			if (ctx.GetActionsList(outActions) > 0)
				if (SCR_NearbyContextWidgetComponentInteract.IsHealingAction(outActions[0]))
					contexts.Insert(ctx);
		}
		
		#ifdef DEBUG_ACTIONICONS
		additionalContexts = {};
		m_InteractionHandlerComponent.GetNearbyShowableContextList(additionalContexts);
		contexts.InsertAll(additionalContexts);
		#endif

		// Get required data
		BaseWorld world = owner.GetWorld();
		int cameraIndex = world.GetCurrentCameraId();
		vector mat[4];
		world.GetCamera(cameraIndex, mat);
		vector referencePos = mat[3];

		// Iterate through individual contexts,
		// validate that they are visible,
		// and update their widget representation
		const float threshold = 0.25;
		const float fovBase = 100; // whatever fits

		// Field of view and screen resolution is necessary to compute proper position and size of widgets
		float zoom = 1; // world.GetCameraVerticalFOV(cameraIndex) - missing crucial getter
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (cameraManager)
		{
			CameraBase camera = cameraManager.CurrentCamera();

			#ifdef DEBUG_NEARBY_CONTEXT_DISPLAY
			if (camera.GetProjType() == CameraType.NONE)
				PrintFormat("%1 [DisplayUpdate] None Projection", this);
			#endif

			if (camera && camera.GetProjType() != CameraType.NONE)
				zoom = fovBase / Math.Max(camera.GetVerticalFOV(), 1);
		}

		// Screen resolution is necessary to know how far away the widget is from screen center
		// or from cursor, if we ever allow player to use mouse cursor or eye tracking software to select the actions.
		float resX; float resY;
		GetGame().GetWorkspace().GetScreenSize(resX, resY);

		UserActionContext currentContext = m_InteractionHandlerComponent.GetCurrentContext();

		// If we draw something, hide it first
		if (!m_aCurrentlyUsedWidgets.IsEmpty())
		{
			foreach (Widget w : m_aCurrentlyUsedWidgets)
			{
				if (w)
					w.SetVisible(false);
			}

			m_aCurrentlyUsedWidgets.Clear();
		}

		bool isOverrideEnabled = m_InteractionHandlerComponent.GetManualCollectionOverride();
		SCR_NearbyContextColorsComponentInteract nearbyColors;
		SCR_HealSupportStationAction medAction;
		IEntity entAction;

		// Iterate through every available context and assign a Widget to it
		foreach (UserActionContext ctx : contexts)
		{
			// Do not draw currently select one
			if (currentContext == ctx)
				continue;

			vector position = ctx.GetOrigin();
			float posX, posY;
			GetWorldToScreenPosition(world, cameraIndex, position, posX, posY);
			
			if (ctx.GetActionsList(outActions) < 1)
				continue;
			
			entAction = outActions[0].GetOwner();
			if (!entAction)
				continue;
			
			#ifdef DEBUG_ACTIONICONS
			foreach(BaseUserAction baseAction: outActions)
			{
				SCR_ActionUIInfo actionUIInfo = SCR_ActionUIInfo.Cast(baseAction.GetUIInfo());
		
				if (!actionUIInfo)
					Print("DEBUG_ACTIONICONS:: No icons found for "+ baseAction);
			}
			#endif

			// Just ignore actions out of reach or out of screen, we will fade them out anyway
			if (IsOnScreen(resX, resY, posX, posY) && IsInLineOfSight(position, mat, threshold))
			{
				// Only do the raycast if there is no collection override and the context has UseRaycast enabled
				if (!isOverrideEnabled && ctx.ShouldCheckLineOfSight() && IsObstructed(position, referencePos, world, entAction))
					continue;

				//Get the widget array from map using the layout as key
				SCR_ActionContextUIInfo info = SCR_ActionContextUIInfo.Cast(ctx.GetUIInfo());
				Widget widget;
				array<Widget> mapWidgets;

				if (info)
				{
					string layout = info.GetLayout();
					string mapKey;

					// Get the GUID of the Layout as it's used as key in the map holding the array of different widgets
					if (layout)
						mapKey = layout;
					else
						mapKey = m_sDefaultMapKey;

					mapWidgets = m_mCachedWidgets.Get(mapKey);

					if (!mapWidgets || mapWidgets.IsEmpty())
						CacheWidget(mapKey, 1);

					mapWidgets = m_mCachedWidgets.Get(mapKey);

					foreach (Widget w : mapWidgets)
					{
						if (m_aCurrentlyUsedWidgets.Contains(w))
							continue;

						widget = w;
						m_aCurrentlyUsedWidgets.Insert(w);
						break;
					}

					//! If there is no widget available create a new one and use that
					if (!widget)
					{
						CacheWidget(mapKey, 1, true, widget);
						m_aCurrentlyUsedWidgets.Insert(widget);
					}

				}
				else
				{
					mapWidgets = m_mCachedWidgets.Get(m_sDefaultMapKey);

					if (!mapWidgets || mapWidgets.IsEmpty())
						CacheWidget(m_sDefaultMapKey, 1);

					mapWidgets = m_mCachedWidgets.Get(m_sDefaultMapKey);

					foreach (Widget w : mapWidgets)
					{
						if (m_aCurrentlyUsedWidgets.Contains(w))
							continue;

						widget = w;
						m_aCurrentlyUsedWidgets.Insert(w);
						break;
					}

					//! If there is no widget available create a new one and use that
					if (!widget)
					{
						CacheWidget(m_sDefaultMapKey, 1, true, widget);
						m_aCurrentlyUsedWidgets.Insert(widget);
					}
				}

				if (widget)
				{
					FrameSlot.SetPos(widget, posX, posY);

					bool visible = ctx.IsInVisibilityAngle(referencePos);
					float distanceLimit = m_fContextCollectDistance * zoom;
					float distanceSq = vector.DistanceSq(position, referencePos);
					float distance = Math.Sqrt(distanceSq);

					SetWidgetAlpha(widget, distance, distanceLimit);
					widget.SetZOrder(-(int)distance);

					SCR_NearbyContextWidgetComponentInteract widgetComp = SCR_NearbyContextWidgetComponentInteract.Cast(widget.FindHandler(SCR_NearbyContextWidgetComponentInteract));
					if (!widgetComp)
						continue;
					
					if (widgetComp.GetAssignedContext() != ctx)
					{
						widgetComp.OnAssigned(info, ctx);

						if (!widgetComp.HasColorData())
							widgetComp.SetColorsData(m_mCachedActionColors);
						
						widgetComp.UpdateColors(ctx);
					}

					widgetComp.ChangeVisibility(visible);
				}
			}
		}
		// Enable required amount of widgets
		SetVisibleWidgets();
	}

	//------------------------------------------------------------------------------------------------
	// Add layouts to cache
	//! Create a certain amount of given Widgets to cache
	//! \param[in] layout Layout that will be created and cached
	//! \param[in] int Amount of Widgets you want to cache
	//! \param[in] bool If true a Widget must be provided that will be used imideatly after caching
	//! \param[in] widget If returnWidget is true a Widget must be defined to where the newly created one should be used
	void CacheWidget(ResourceName layout, int amount, bool returnWidget = false, Widget widgetToReturn = null)
	{
		if (!layout)
			return;

		string key = layout;
		array<Widget> cachedWidgets = {};
		WorkspaceWidget workSpace = GetGame().GetWorkspace();
		Widget widgetToCache;

		if (m_mCachedWidgets.Contains(key))
		{
			cachedWidgets = m_mCachedWidgets.Get(key);
		}

		// Create all the Widgets and insert them into the map
		for (int i = 0; i < amount; i++)
		{
			widgetToCache = workSpace.CreateWidgets(layout, m_wRoot);

			if (widgetToCache)
			{
				cachedWidgets.Insert(widgetToCache);

				FrameSlot.SetAnchor(widgetToCache, 0, 0);
				FrameSlot.SetAlignment(widgetToCache, 0.5, 0.5);
				widgetToCache.SetVisible(false);

				// If the system is not fully initialized and the RootWidget does not exist yet we add them to the Widget array, so they can be reparented
				if (!m_bIsInitialized && !m_wRoot)
					m_aWidgets.Insert(widgetToCache);
			}

		}

		m_mCachedWidgets.Set(key, cachedWidgets);

		if (returnWidget)
			widgetToReturn = cachedWidgets[0];
	}

	//------------------------------------------------------------------------------------------------
	//! Check every Cached Widget array and check if the array holds more widgets then the cap allows
	//! If there are too many cached Widgets, delete Widgets until its below the cap.
	protected void ClearCachedWidgets()
	{
		int count;
		int maxCachedAmount = m_iMaxPrecachedWidgets;
		Widget cachedWidget;

		foreach (array<Widget> cachedWidgets : m_mCachedWidgets)
		{
			string mapKey = m_mCachedWidgets.GetKeyByValue(cachedWidgets);
			SCR_NearbyContextCachingData data = m_CachingConfig.GetDataFromLayout(mapKey);

			if (data)
				data.GetMaxCacheAmount(maxCachedAmount);

			count = cachedWidgets.Count();

			if (count <= maxCachedAmount)
				continue;

			for (int index = count - 1; index >= maxCachedAmount; index--)
			{
				cachedWidget = cachedWidgets[index];

				if (cachedWidget && !cachedWidget.IsVisible())
				{
					cachedWidget.RemoveFromHierarchy();
					cachedWidgets.Remove(index);
				}
			}

			m_mCachedWidgets.Set(mapKey, cachedWidgets);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Use a Raycast to check if the givin position is in line of sight
	//! \param[in] Position to which we cast the raycast
	//! \param[in] Position where the raycast should start
	//! \param[in] current world
	//! \param[in] Entity Owner of the Action
	//! \return true if raycast collided with something, false otherwise
	protected bool IsObstructed(vector contextPos, vector cameraPos, BaseWorld world, notnull IEntity entAction)
	{
		m_RaycastParam.Start = cameraPos;
		m_RaycastParam.End = contextPos;
		
		if (world.TraceMove(m_RaycastParam, IsCharacter) < m_InteractionHandlerComponent.GetRaycastThreshold())
		{
			if (!m_RaycastParam.TraceEnt)
				return true;
			
			IEntity parentEntityRay = m_RaycastParam.TraceEnt.GetRootParent();
			IEntity parentEntityAct = entAction.GetRootParent();
			
			if (parentEntityRay != parentEntityAct)
				return true;
		}
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method for TraceMove, to check if hit entity is a Character
	//! \param entity
	//! \return true if entity is a character, false otherwise
	protected static bool IsCharacter(notnull IEntity entity)
	{
		return ChimeraCharacter.Cast(entity) == null;
	}

	//------------------------------------------------------------------------------------------------
	//! Iterates through available precached widgets and leaves only provided count enabled.
	protected void SetVisibleWidgets()
	{
		if (!m_wRoot || m_aCurrentlyUsedWidgets.IsEmpty())
			return;

		foreach (Widget w : m_aCurrentlyUsedWidgets)
		{
			if (w)
				w.SetVisible(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Set the Widget alpha based on the distance of the Context it's attached to to the Camera
	//! \param[in] Widget to which the alpha will be applied
	//! \param[in] Distance from camera to context in world space
	//! \param[in] Max distance until the alpha will be 0
	protected void SetWidgetAlpha(Widget widget, float distance, float limit)
	{
		float alpha = 1;
		if (limit != 0 && distance >= m_fInteractionDistance)
			alpha = Math.Clamp(limit - distance, m_fMinWidgetAlpha, limit) / limit;

		// Apply opacity
		widget.SetOpacity(alpha);
	}

	//------------------------------------------------------------------------------------------------
	//! Recalculates worldPosition to screen space and applies it to widget.
	//! \param[in] worldPosition
	//! \param[in] int Camera thats being used
	//! \param[in] vector Position in world space
	//! \param[out] posX X Position on the screen
	//! \param[out] posY Y Position on the screen
	protected bool GetWorldToScreenPosition(BaseWorld world, int cameraIndex, vector worldPosition, out float posX, out float posY)
	{
		vector screenPosition = GetGame().GetWorkspace().ProjWorldToScreen(worldPosition, world, cameraIndex);
		posX = screenPosition[0];
		posY = screenPosition[1];

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Performs a dot product check against threshold whether point is in line of sight of provided transformation.
	//! \param[in] vector point Point to perform check for
	//! \param[in] vector[4] transform Reference matrix checked against
	//! \param[in] float threshold01 Dot product result is compared against this value, higher values result in more narrow arc.
	//! \return true if is in line of sight, flase otherwise
	protected bool IsInLineOfSight(vector point, vector transform[4], float val)
	{
		vector direction = point - transform[3];
		direction.Normalize();

		const float threshold = 0.25;
		if (vector.Dot(direction, transform[2]) > val)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks is the given screenposition is within the resolution and not outside of the screen
	//! \param[in] float ScreenresolutionX
	//! \param[in] float ScreenresolutionY
	//! \param[in] float X position on the screen
	//! \param[in] float Y position on the screen
	//! \return true if the position is on the screen. False if the position it outside of the screen
	protected bool IsOnScreen(float resX, float resY, float posX, float posY)
	{
		if (posX > resX || posY > resY || posX < 0 || posY < 0)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the distance from givin point to screen center by taking the X & Y position an calcualting the distance between them
	//! \param[in] float ScreenresolutionX
	//! \param[in] float ScreenresolutionY
	//! \param[in] float X position on the screen
	//! \param[in] float Y position on the screen
	//! \return Distance between posX & posY. 0 = Center. >0 further from center
	protected float GetDistanceFromScreenCenter(float resX, float resY, float posX, float posY)
	{
		float aspectRatio = resX / resY;

		// Get distance from center as 0,1
		// where 0 = center, 1 = edge (only for 1920 x 1080)
		// when screen is larger than default we increase the distance to larger than 1
		float xDistance = Math.AbsFloat(posX - resX * 0.5) / resX * aspectRatio;
		float yDistance = Math.AbsFloat(posY - resY * 0.5) / resY;

		// calculate last unknown length
		return Math.Sqrt(Math.Pow(xDistance, 2) + Math.Pow(yDistance, 2));
	}
}