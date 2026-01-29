//#define DISABLE_HUD_MANAGER

//------------------------------------------------------------------------------------------------
// (i): The enum values needs to be power of 2
enum EHudLayers
{
	BACKGROUND = 1, 	// Only background textures like Screen effects
	LOW = 2, 			// Read-only information, like weapon info
	MEDIUM = 4,			//
	HIGH = 8,			// Dialogue-like elements like weapon switching
	OVERLAY = 16,		// Interactive elements that should always be on top
	ALWAYS_TOP = 32
}

[ComponentEditorProps(icon: HYBRID_COMPONENT_ICON)]
class SCR_HUDManagerComponentClass : HUDManagerComponentClass
{
}

void OnScreenBrightnessChanged(float opacity, float sceneBrightness);			
typedef func OnScreenBrightnessChanged;

class SCR_HUDManagerComponent : HUDManagerComponent
{
	[Attribute()]
	protected ref array<ref SCR_HUDManagerHandler> m_aHandlers;
	protected ref array<SCR_HUDManagerHandler> m_aUpdatableHandlers = {};

	[Attribute()]
	string m_sDefaultBackgroundLayer;
	[Attribute()]
	string m_sDefaultForegroundLayer;

	[Attribute("0 0 1 1", UIWidgets.GraphDialog, params: "0.1 0.6 0 0.4", desc: "Adaptive opacity (y) based on screen brightness (x).", category: "AdaptiveOpacity")]
	private ref Curve m_AdaptiveOpacityCurve;	
	
	[Attribute("SCR_InterfaceSettings", desc: "Classname of the InterfaceSettings menu")]
	protected string m_sInterfaceSettingsClassName;
	
	private ref map<EHudLayers, Widget> m_aLayerWidgets = new map<EHudLayers, Widget>;
	private Widget m_wRoot;
	private Widget m_wRootTop;

	protected BaseWorld m_World;

	private bool m_MenuOpen;

	protected ref ScriptInvokerBase<OnScreenBrightnessChanged> m_OnSceneBrightnessChanged = new ScriptInvokerBase<OnScreenBrightnessChanged>();

	protected float m_fUpdateTime = -1;
	const float UPDATE_DELAY = 500;

	const float ADAPTIVE_OPACITY_BRIGHTNESS_THROTTLE = 0.001;
	const float ADAPTIVE_OPACITY_THROTTLE = 0.01;					// threshold opacity change to fire the AdaptiveOpacity invoker

	protected float m_fSceneBrightness = -1;
	protected float m_fOpacity = -1;

	protected IEntity m_Owner;

	protected ref array<SCR_HUDManagerHandler> m_aHandlersToRemoveFromUpdate = {};

	#ifndef DISABLE_HUD_MANAGER

	//------------------------------------------------------------------------------------------------
	int GetInfoDisplayCount()
	{
		return m_aHUDElements.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetOwner()
	{
		return m_Owner;
	}

	//------------------------------------------------------------------------------------------------
	SCR_HUDManagerHandler FindHandler(typename handlerType)
	{
		foreach (SCR_HUDManagerHandler handler : m_aHandlers)
		{
			if (handler.Type() == handlerType)
				return handler;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void RemoveHandlerFromUpdatableHandlers(notnull SCR_HUDManagerHandler handler)
	{
		m_aHandlersToRemoveFromUpdate.Insert(handler);
	}

	//------------------------------------------------------------------------------------------------
	Widget GetHUDRootWidget()
	{
		return m_wRoot;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnInit(IEntity owner)
	{
		if (!GetGame().GetWorldEntity())
			return;
		m_Owner = owner;

		foreach (SCR_HUDManagerHandler handler : m_aHandlers)
		{
			handler.OnInit(this);

			if (handler.CanUpdate())
				m_aUpdatableHandlers.Insert(handler);
		}

		foreach (SCR_HUDManagerHandler handler : m_aHandlers)
		{
			handler.OnStart(this);
		}

		ArmaReforgerScripted game = GetGame();
		if (game && !game.GetHUDManager())
		{
			game.SetHUDManager(this);
			CreateHUDLayers();
		}

		m_World = GetGame().GetWorld();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnUpdate(IEntity owner)
	{
		if (!m_World)
			return;

		if (!m_aHandlersToRemoveFromUpdate.IsEmpty())
		{
			foreach (SCR_HUDManagerHandler handler : m_aHandlersToRemoveFromUpdate)
			{
				m_aUpdatableHandlers.RemoveItem(handler);
				handler.SetCanUpdate(false);
			}

			m_aHandlersToRemoveFromUpdate.Clear();
		}

		foreach (SCR_HUDManagerHandler handler : m_aUpdatableHandlers)
		{
			handler.OnUpdate(this);
		}

		float time = m_World.GetWorldTime();

		if (time < m_fUpdateTime)
			return;

		m_fUpdateTime = time + UPDATE_DELAY;

		//------------------------------------------------------------------------------------------------
		// Adaptive opacity
		//------------------------------------------------------------------------------------------------

		// We want to enforce full opacity for brightness > 0.1 (midday, good weather)
		float sceneBrightness = Math.Clamp(m_World.GetCameraSceneMiddleBrightness(0), 0, 0.1);

		// Throttle the opacity update if brightness change is not bigger than n%
		if (Math.AbsFloat(m_fSceneBrightness - sceneBrightness) < ADAPTIVE_OPACITY_BRIGHTNESS_THROTTLE)
			return;

		// Curve based opacity interpolation
		float opacity = LegacyCurve.Curve(ECurveType.CurveProperty2D, sceneBrightness, m_AdaptiveOpacityCurve)[1];
		opacity = Math.Clamp(Math.Round(opacity * 100) / 100, 0, 1);
		
		// Invoke the opacity update only if change is greater than n%
		if (Math.AbsFloat(m_fOpacity - opacity) >= ADAPTIVE_OPACITY_THROTTLE)
		{
			//PrintFormat("sceneBrightness: %1 -> opacity %2", sceneBrightness, opacity);

			m_fSceneBrightness = sceneBrightness;
			m_fOpacity = opacity;

			if (m_OnSceneBrightnessChanged)
				m_OnSceneBrightnessChanged.Invoke(opacity, sceneBrightness);
		}
	}

	//------------------------------------------------------------------------------------------------
	float GetAdaptiveOpacity()
	{
		return m_fOpacity;
	}

	//------------------------------------------------------------------------------------------------
	float GetSceneBrightness()
	{
		return m_fSceneBrightness;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetInterfaceSettingsClass()
	{
		return m_sInterfaceSettingsClassName;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<OnScreenBrightnessChanged> GetSceneBrightnessChangedInvoker()
	{
		return m_OnSceneBrightnessChanged;
	}

	//------------------------------------------------------------------------------------------------
	protected void ~SCR_HUDManagerComponent()
	{
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
	}

	#endif

	//------------------------------------------------------------------------------------------------
	protected void CreateHUDLayers()
	{
		m_wRoot = GetGame().GetWorkspace().CreateWidget(WidgetType.FrameWidgetTypeID, WidgetFlags.VISIBLE, Color.FromInt(Color.WHITE), 0);
		m_wRootTop = GetGame().GetWorkspace().CreateWidget(WidgetType.FrameWidgetTypeID, WidgetFlags.VISIBLE, Color.FromInt(Color.WHITE), 0);
		m_wRootTop.SetZOrder(100); //set high to be always on top, even above MenuManager layouts

		if (m_wRoot && m_wRootTop)
		{
			InitRootWidget(m_wRoot, "SCR_HUDManagerComponent.m_wRoot");
			InitRootWidget(m_wRootTop, "SCR_HUDManagerComponent.m_wRootTop");

			array<int> bitValues = new array<int>;
			int bitCount = SCR_Enum.GetEnumValues(EHudLayers, bitValues);

			for (int i = 0; i < bitCount; i++)
			{
				Widget frame;
				Widget parent = m_wRoot;
				if (bitValues[i] == EHudLayers.ALWAYS_TOP)
					parent = m_wRootTop;

				frame = GetGame().GetWorkspace().CreateWidget(WidgetType.FrameWidgetTypeID, WidgetFlags.VISIBLE, Color.FromInt(Color.WHITE), 0, parent);
				FrameSlot.SetAnchorMin(frame, 0, 0);
				FrameSlot.SetAnchorMax(frame, 1, 1);
				FrameSlot.SetOffsets(frame, 0, 0, 0, 0);
				frame.SetFlags(WidgetFlags.IGNORE_CURSOR);
				frame.SetName(string.Format("SCR_HudManagerComponent: %1", typename.EnumToString(EHudLayers, bitValues[i])));

				m_aLayerWidgets.Insert(bitValues[i], frame);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Return hud component of given type
	SCR_InfoDisplay FindInfoDisplay(typename type)
	{
		foreach (SCR_InfoDisplay display : m_aHUDElements)
		{
			if (display.Type() == type)
				return display;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Find layout by resorce name
	SCR_InfoDisplay FindInfoDisplayByResourceName(ResourceName path)
	{
		foreach (SCR_InfoDisplay display : m_aHUDElements)
		{
			if (display.m_LayoutPath == path)
				return display;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Find layout by resorce name
	Widget FindLayoutByResourceName(ResourceName path)
	{
		foreach (SCR_InfoDisplay display : m_aHUDElements)
		{
			if (display.m_LayoutPath == path)
				return display.GetRootWidget();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitRootWidget(Widget root, string name)
	{
		FrameSlot.SetAnchorMin(root, 0, 0);
		FrameSlot.SetAnchorMax(root, 1, 1);
		FrameSlot.SetOffsets(root, 0, 0, 0, 0);
		root.SetFlags(WidgetFlags.IGNORE_CURSOR);
		root.SetName(name);
	}

	//------------------------------------------------------------------------------------------------
	Widget CreateLayout(ResourceName path, EHudLayers layer, int zOrder = 0)
	{
		#ifndef DISABLE_HUD_MANAGER

		if (path == string.Empty)
			return null;

		Widget parent;
		if (m_aLayerWidgets.Find(layer, parent) && parent)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(path, parent);
			if (w)
			{
				FrameSlot.SetAnchorMin(w, 0, 0);
				FrameSlot.SetAnchorMax(w, 1, 1);
				FrameSlot.SetOffsets(w, 0, 0, 0, 0);
				w.SetFlags(WidgetFlags.IGNORE_CURSOR);
				if (zOrder != 0)
					w.SetZOrder(zOrder);

				return w;
			}
		}

		#endif

		return null;
	}

	SCR_HUDElement CreateFreeElement(ResourceName path, string parentName)
	{
		SCR_HUDManagerLayoutHandler layoutHandler = SCR_HUDManagerLayoutHandler.Cast(FindHandler(SCR_HUDManagerLayoutHandler));
		if (!layoutHandler)
			return null;

		SCR_HUDLayout owningLayout;
		Widget parentWidget = layoutHandler.FindWidgetByNameFromAnyLayout(parentName, owningLayout);
		if (!parentWidget)
			return null;

		if (parentWidget.FindHandler(SCR_HUDSlotUIComponent))
		{
			Print(parentName + " is a HUD Slot! Use InfoDisplay instead!", LogLevel.ERROR);
			return null;
		}

		Widget elementWidget = GetGame().GetWorkspace().CreateWidgets(path, parentWidget);

		SCR_HUDElement hudElement = new SCR_HUDElement();
		hudElement.SetWidget(elementWidget);
		hudElement.SetParentWidgetName(parentName);
		hudElement.SetParentLayout(owningLayout);
		owningLayout.AddHudElement(hudElement);

		return hudElement;
	}

	/*!
	Set visibility of HUD.
	\param isVisible True if visible
	*/
	void SetVisible(bool isVisible)
	{
		m_wRoot.SetVisible(isVisible);
		m_wRootTop.SetVisible(isVisible);
	}
	/*!
	Check if HUD is visible.
	\return True if visible
	*/
	bool IsVisible()
	{
		return m_wRoot.IsVisible();
	}

	/*!
	Set which HUD layers should be visible.
	\param layers Enum flag containing all layers to be shown. Use -1 to show all layers
	*/
	void SetVisibleLayers(EHudLayers layers = -1)
	{
		if (layers == -1)
		{
			//--- Show all
			foreach (EHudLayers layer, Widget w : m_aLayerWidgets)
			{
				w.SetVisible(true);
			}
		}
		else
		{
			//--- Show custom
			foreach (EHudLayers layer, Widget w : m_aLayerWidgets)
			{
				w.SetVisible(layers & layer);
			}
		}
	}
	/*!
	Get which HUD layers are visible.
	Returns visibility status of individual layer; it can still not be shown if the whole HUD is hidden (IsVisible()).
	\param layers Enum flag containing all visible layers
	*/
	EHudLayers GetVisibleLayers()
	{
		EHudLayers layers;
		foreach (EHudLayers layer, Widget w : m_aLayerWidgets)
		{
			if (w.IsVisible()) layers = layers | layer;
		}
		return layers;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_HUDManagerComponent GetHUDManager()
	{
		#ifndef DISABLE_HUD_MANAGER

		ArmaReforgerScripted game = GetGame();

		if (game)
			return game.GetHUDManager();

		#endif

		return null;
	}

	#ifdef WORKBENCH
	// The two methods below allow us to open a context menu over the SCR_HUDManagerComponent
	// It adds a 'Add actions to AvailableActionsDisplay info' entry
	// After selection a dialog with all actions is shown from which the user can select some
	// AvailableActionsDisplay is then filled with entries of selected actions
	// Entry is only added if we have a SCR_AvailableActionsDisplay object in the hud manager
	// Multiple SCR_AvailableActionsDisplay are not supported

	//------------------------------------------------------------------------------------------------
	const int CONTEXT_GENERATE_AVAILABLE_INPUT_ACTION = 0;
	//------------------------------------------------------------------------------------------------
	override array<ref WB_UIMenuItem> _WB_GetContextMenuItems(IEntity owner)
	{
		// Prep array
		array<ref WB_UIMenuItem> items = new array<ref WB_UIMenuItem>();

		// Find available actions display
		array<BaseInfoDisplay> elements = new array<BaseInfoDisplay>();
		GetInfoDisplays(elements);
		foreach (auto element : elements)
		{
			// Create item option
			if (SCR_AvailableActionsDisplay.Cast(element))
			{
				ref WB_UIMenuItem actionsContextItem = new WB_UIMenuItem("Add actions to AvailableActionsDisplay info", CONTEXT_GENERATE_AVAILABLE_INPUT_ACTION);
				items.Insert(actionsContextItem);
				break;
			}
		}
		// Return arr
		return items;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnContextMenu(IEntity owner, int id)
	{
		if (id == CONTEXT_GENERATE_AVAILABLE_INPUT_ACTION)
		{
			SCR_AvailableActionsDisplay target;

			// Find available actions display
			array<BaseInfoDisplay> elements = new array<BaseInfoDisplay>();
			GetInfoDisplays(elements);
			foreach (auto element : elements)
			{
				if (SCR_AvailableActionsDisplay.Cast(element))
				{
					target = SCR_AvailableActionsDisplay.Cast(element);
					break;
				}
			}

			if (!target)
				return;

			GenericEntity genericOwner = GenericEntity.Cast(owner);
			if (!genericOwner)
				return;

			// Get all actions
			InputManager mgr = GetGame().GetInputManager();
			array<string> actions = {};
			if (mgr)
			{
				int cnt = mgr.GetActionCount();
				for (int i = 0; i < cnt; i++)
				{
					string act = mgr.GetActionName(i);
					actions.Insert(act);
				}
			}

			// Get wb we api
			WorldEditorAPI api = genericOwner._WB_GetEditorAPI();
			if (!api)
				return;

			// Pass them into a dialog
			// TODO: Make this not use cpp and make it more reasonable in general
			array<int> selection = new array<int>();
			api.ShowItemListDialog("Actions Selection", "Select actions from the actions manager to create entry for.", 340, 480, actions, selection, 0);

			// Parent entity source
			IEntitySource entitySource = api.EntityToSource(owner);
			if (!entitySource)
				return;

			// Hud manager source
			BaseContainer componentContainer;
			int cmpCount = entitySource.GetComponentCount();
			for (int i = 0; i < cmpCount; i++)
			{
				IEntityComponentSource componentSource = entitySource.GetComponent(i);
				if (componentSource.GetClassName() == "SCR_HUDManagerComponent")
				{
					componentContainer = componentSource.ToBaseContainer();
					break;
				}
			}

			if (!componentContainer)
				return;

			// InfoDisplays array of hud manager
			BaseContainerList infoDisplays = componentContainer.GetObjectArray("InfoDisplays");
			if (!infoDisplays)
				return;

			// SCR_AvailableActionsDisplay object
			BaseContainer displaySource;
			for (int i = 0; i < infoDisplays.Count(); i++)
			{
				BaseContainer container = infoDisplays.Get(i);
				if (container.GetClassName() == "SCR_AvailableActionsDisplay")
				{
					displaySource = container;
					break;
				}
			}

			if (!displaySource)
				return;

			api.BeginEntityAction("Create action container");

			const string list = "m_aActions";
			// Create object for each action
			foreach (auto sel : selection)
			{
				string selectedActionName = actions[sel];
				if (selectedActionName == string.Empty)
					continue;

				const int objIndex = 0;
				// Create object, push it into array
				api.CreateObjectArrayVariableMember(displaySource, null, list, "SCR_AvailableActionContext", objIndex);
				// Retrieve the source
				BaseContainerList itemsList = displaySource.GetObjectArray(list);
				BaseContainer thisContainer = itemsList.Get(objIndex);
				thisContainer.Set("m_sAction", selectedActionName);
				thisContainer.Set("m_sName", selectedActionName);
			}

			api.EndEntityAction();
			return;
		}
	}
	#endif
}
