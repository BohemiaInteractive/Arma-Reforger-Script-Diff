class SCR_ActionMenuInteractionDisplay : SCR_BaseInteractionDisplay
{
	[Attribute("{C80DEB58AA948E59}UI/layouts/HUD/InteractionSystem/ActionMenuElement.layout", UIWidgets.ResourceNamePicker, params: "layout")]
	ResourceName m_sElementLayout;

	[Attribute("{1D3B10902C8F8FEA}UI/layouts/HUD/InteractionSystem/ActionMenuSnackBar.layout", UIWidgets.ResourceNamePicker, params: "layout")]
	ResourceName m_sSnackBarLayout;

	[Attribute(UIColors.GetColorAttribute(GUIColors.ENABLED), UIWidgets.ColorPicker)]
	protected ref Color m_EnabledColor;

	[Attribute(UIColors.GetColorAttribute(GUIColors.DISABLED), UIWidgets.ColorPicker)]
	protected ref Color m_DisabledColor;

	[Attribute(UIColors.GetColorAttribute(GUIColors.DEFAULT_GLOW), UIWidgets.ColorPicker)]
	protected ref Color m_DisabledColorGlow;

	[Attribute(UIColors.GetColorAttribute(UIColors.CONTRAST_COLOR), UIWidgets.ColorPicker)]
	protected ref Color m_PerformingColor;

	[Attribute("0", UIWidgets.CheckBox, "Fixed placement")]
	protected bool m_bFixedPlacement;

	[Attribute("25.0", UIWidgets.Slider, params: "0 100 0.1")]
	protected float m_fScrollAnimationSpeed;

	[Attribute("0.1", UIWidgets.Slider, params: "0 10 0.05")]
	protected float m_fAutoExpandTime;

	[Attribute("0.00", UIWidgets.Slider, "Action list element fade-in expand delay (in seconds).", params: "0 1 0.001")]
	protected float m_fFadeInOffset;

	[Attribute("0.6", UIWidgets.Slider, params: "0.3 10 0.2")]
	protected float m_fWaitTimeAdditionalInfo;

	// Non-selected element atributes
	[Attribute("40", UIWidgets.Slider, params: "0.05 50 0.05")]
	protected float m_fHeightElement;

	// Transition speed in seconds between the icon and the button
	[Attribute("0.3", UIWidgets.Slider, params: "0.3 10 0.2")]
	protected float m_fButtonFadeInSpeed;

	[Attribute("", desc: "each ID must be unique")]
	protected ref array<ref SCR_NearbyContextColorsComponentInteract> m_aNearbyWidgetColors;

	protected ref SCR_ActionMenuLayoutWidgets m_Widgets = new SCR_ActionMenuLayoutWidgets();
	protected ref SCR_SnackBarController m_SnackBar;

	protected ref array<ref SCR_ActionMenuElementComponent> m_aActionMenuElements = {};
	protected ref array<ref SCR_ActionMenuElementComponent> m_aActiveActionMenuElements = {};

	protected InteractionHandlerComponent m_InteractionHandlerComponent;

	protected BaseUserAction m_PrevAction;
	protected UserActionContext m_PrevContext;

	// Data containing info from interaction handler
	protected ref ActionDisplayData m_LastData;

	protected bool m_bIsSingleEntryMenu;

	protected int m_iCurrentScroll;
	protected float m_fCurrentScrollAnimation;

	protected float m_fExpandTimer;					// Timer for auto-expand feature
	protected float m_fExpandedTimer;				// Time elapsed from menu expand, used for sequential fade in of elements
	protected float m_fSnackBarAppearTimer;			// Time for Snack bar to appear
	protected float m_fSnackBarOpacityTimer;		// Time for Snack bar to change opacity

	protected bool m_bShouldDisplaySnackBar;

	protected float m_fTimeSpentAdditionalInfo;
	
	protected bool m_bForceUpdate;
	protected bool m_bForceResetScroll
	protected bool m_bProgressReachedMax;
	protected bool m_bIsActionEnabled;
	
	protected int m_iContextCount;
	protected ref TStringArray m_aCurrentContextNames;
	
	//! Holds all the different cached id with the enum as key
	protected ref map<SCR_ENearbyInteractionContextColors, ref SCR_NearbyContextColorsComponentInteract> m_mCachedActionColors;
	
	protected float m_fTotalHeightElements;
	
	protected const float ANCHOR_X = 0.48;
	protected const float ANCHOR_Y = 0.94;

	protected const float SAFEZONE_PADDING = 15.0;

	protected const float WIDGET_DEFAULT_OPACITY = 1.0;
	protected const float WIDGET_HIDDEN_OPACITY = 0.75;

	protected const vector SCREEN_POS_MIN = "0.15 0.15 0";
	protected const vector SCREEN_POS_MAX = "0.85 0.85 0";
	
	protected const string SETTING_HINT_ACTION = "m_bShowInteractionHint";

	//------------------------------------------------------------------------------------------------
	//! Creates and initializes root widget to be slotted with actions
	protected void Create()
	{
		if (!m_wRoot)
		{
			Print("ActionsMenuDisplay is missing root layout!", LogLevel.ERROR);
			return;
		}

		if (!m_bFixedPlacement)
		{
			FrameSlot.SetAnchorMin(m_wRoot, 0, 0);
			FrameSlot.SetAnchorMax(m_wRoot, 0, 0);
			FrameSlot.SetAlignment(m_wRoot, 0, 0);
		}
		else
		{
			FrameSlot.SetAnchorMin(m_wRoot, ANCHOR_X, ANCHOR_Y);
			FrameSlot.SetAnchorMax(m_wRoot, ANCHOR_X, ANCHOR_Y);
			FrameSlot.SetAlignment(m_wRoot, 0, 0);
		}

		m_Widgets.Init(m_wRoot);

		if (!m_Widgets.m_wActionMenu)
		{
			Debug.Error2("SCR_ActionMenuInteractionDisplay", "m_wActionMenu is NULL");
			return;
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

		m_Widgets.m_wOutlineOverlay.SetVisible(false);
		m_Widgets.m_wOutlineProgress.SetMaskProgress(0);
		m_Widgets.m_wOutlineProgressGlow.SetMaskProgress(0);
		m_Widgets.m_IconOverlayComponent.SetColorsData(m_mCachedActionColors);

		Widget snackBarWidget = m_wRoot.GetWorkspace().CreateWidgets(m_sSnackBarLayout);

		m_SnackBar = SCR_SnackBarController.Cast(snackBarWidget.FindHandler(SCR_SnackBarController));
		
		m_aCurrentContextNames = {};

		// Initialize action menu state and data
		Collapse();
	}

	//------------------------------------------------------------------------------------------------
	protected void Destroy()
	{
		if (m_aActionMenuElements)
			m_aActionMenuElements.Clear();

		if (m_SnackBar)
			m_SnackBar.SetVisibility(false);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Update(ActionsTuple actionsData, BaseUserAction selectedAction, UserActionContext currentContext, float fTimeSlice)
	{
		if (!currentContext && m_PrevContext)
		{
			m_PrevAction = null;
			m_PrevContext = null;
			return false;
		}

		// Detect context change (different set of actions)
		if (currentContext != m_PrevContext)
		{
			m_PrevContext = currentContext;
			m_bForceResetScroll = true;
			m_bForceUpdate = true;

			Collapse();
		} // Detect if we changed action
		else if (m_PrevAction != selectedAction)
		{
			m_bForceUpdate = true;
		}

		int actionsCount = actionsData.param1.Count();
		if (!actionsData.param1 || actionsCount < 1)
			return false;

		UpdateAnimateActions(actionsData, selectedAction, currentContext, actionsCount < 2, fTimeSlice);

		if (m_bForceUpdate)
			return UpdateDisplayActions(actionsData, selectedAction, currentContext, actionsCount);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool UpdateAnimateActions(ActionsTuple actionsData, BaseUserAction selectedAction, UserActionContext currentContext, bool isSingleAction, float fTimeSlice)
	{
		if (isSingleAction)
		{
			Collapse();
		}
		else
		{
			RunExpandTimer(fTimeSlice);
			RunExpandedTimer(fTimeSlice);
		}

		m_iCurrentScroll = -1;
		bool scrollingDown;
		foreach (int i, BaseUserAction currAction : actionsData.param1)
		{
			if (currAction == m_PrevAction)
				scrollingDown = true;

			if (currAction == selectedAction)
			{
				m_iCurrentScroll = i;
				break;
			}
		}

		// Set and animate scroll
		SetScroll(m_iCurrentScroll, fTimeSlice);
		SetPaggingEffect(m_iCurrentScroll, scrollingDown);

		if (m_fTimeSpentAdditionalInfo > -1)
			m_fTimeSpentAdditionalInfo += fTimeSlice;

		if (m_fTimeSpentAdditionalInfo > m_fWaitTimeAdditionalInfo)
		{
			m_fTimeSpentAdditionalInfo = -1;
			if (m_bShouldDisplaySnackBar)
			{
				m_bShouldDisplaySnackBar = false;
				
				if (selectedAction.GetCannotPerformReason() != string.Empty)
				{
					m_SnackBar.SetText(selectedAction.GetCannotPerformReason());
					m_SnackBar.AnimateShow();
				}
			}
			else
			{
				bool shouldShowInteractionHint = true;
				
				//check if the hint is enabled in settings
				SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
				if (hudManager)
				{
					BaseContainer interfaceSettings = GetGame().GetGameUserSettings().GetModule(hudManager.GetInterfaceSettingsClass());
					if (interfaceSettings)
						interfaceSettings.Get(SETTING_HINT_ACTION, shouldShowInteractionHint);
				}
				
				//check setting value here
				AnimateWidget.Opacity(m_Widgets.m_wIcon, !shouldShowInteractionHint, GetButtonFadeInSpeed(), true);
				AnimateWidget.Opacity(m_Widgets.m_wIconGlow, !shouldShowInteractionHint, GetButtonFadeInSpeed(), true);
				AnimateWidget.Opacity(m_Widgets.m_wInputButtonDisplayRoot, shouldShowInteractionHint, GetButtonFadeInSpeed(), true);
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool UpdateDisplayActions(ActionsTuple actionsData, BaseUserAction selectedAction, UserActionContext currentContext, int actionsCount)
	{
		m_bForceUpdate = false;
		
		IEntity playerEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!playerEntity)
			return true;
		
		int iActiveWidgetsCount = m_aActionMenuElements.Count();
		int iActionsDelta = iActiveWidgetsCount - actionsCount;
		int iAbsDelta = Math.AbsInt(iActionsDelta);

		// Toggle selection / adjustment widgets
		bool isInProgress = selectedAction && selectedAction.IsInProgress();
		m_bIsActionEnabled = selectedAction.CanBePerformed(playerEntity);
		
		if (!isInProgress || !m_bIsActionEnabled)
		{
			AnimateWidget.StopAllAnimations(m_Widgets.m_wIcon);
			AnimateWidget.StopAllAnimations(m_Widgets.m_wIconGlow);
			AnimateWidget.StopAllAnimations(m_Widgets.m_wInputButtonDisplayRoot);
			
			m_fTimeSpentAdditionalInfo = 0;
			m_Widgets.m_wIcon.SetVisible(true);
			m_Widgets.m_wIcon.SetOpacity(1.0);
			m_Widgets.m_wIconGlow.SetVisible(true);
			m_Widgets.m_wIconGlow.SetOpacity(1.0);
			m_Widgets.m_wInputButtonDisplayRoot.SetVisible(false);
			m_Widgets.m_wInputButtonDisplayRoot.SetOpacity(0.0);
		}

		// Add or remove widgets
		if (iActionsDelta < 0)
		{
			// Elements missing
			SCR_ActionMenuElementComponent actionMenuElement;
			Widget elementWidget;
			WorkspaceWidget rootWorkspace = m_wRoot.GetWorkspace();
			for (int i = 0; i < iAbsDelta; i++)
			{
				elementWidget = rootWorkspace.CreateWidgets(m_sElementLayout, m_Widgets.m_wActionMenu);
				if (!elementWidget)
					continue;
				
				LayoutSlot.SetHorizontalAlign(elementWidget, LayoutHorizontalAlign.Left);

				actionMenuElement = SCR_ActionMenuElementComponent.Cast(elementWidget.FindHandler(SCR_ActionMenuElementComponent));
				if (actionMenuElement)
					m_aActionMenuElements.Insert(actionMenuElement);
			}
		}
		else if (iActionsDelta > 0)
		{
			// Elements over
			SCR_ActionMenuElementComponent actionMenuElement;
			for (int i = iActiveWidgetsCount-1; i >= iActiveWidgetsCount-iAbsDelta; i--)
			{
				actionMenuElement = m_aActionMenuElements[i];
				actionMenuElement.SetVisible(false);
			}
		}

		if (m_iCurrentScroll < 0)
			return false;

		// Expand menu if we have more than 1 action
		if (actionsCount > 1)
			Expand();

		m_aActiveActionMenuElements.Clear();

		// Set widget data
		SCR_ActionUIInfo actionInfo;
		SCR_ActionMenuElementComponent actionMenuElement;
		foreach (int i, BaseUserAction currAction : actionsData.param1)
		{
			actionMenuElement = m_aActionMenuElements[i];

			// Setup menu element
			actionInfo = SCR_ActionUIInfo.Cast(currAction.GetUIInfo());
			bool isActionDisabled = currAction.CanBePerformed(playerEntity);
			bool succeeded = actionMenuElement.Set(currAction, currAction == selectedAction, isActionDisabled, actionInfo);
			
			if (!actionMenuElement.GetIconComponent().HasColorData())
				actionMenuElement.GetIconComponent().SetColorsData(m_mCachedActionColors);
			
			actionMenuElement.GetIconComponent().UpdateColors(currAction, true);
			actionMenuElement.UpdateColors();

			if (!succeeded)
			{
				actionMenuElement.SetVisible(false);
				continue;
			}

			// Set element visibility based on menu state
			if ((i == 0 || !m_bIsSingleEntryMenu) && (m_fExpandedTimer >= i * m_fFadeInOffset))
			{
				actionMenuElement.SetVisible(true);
				m_aActiveActionMenuElements.Insert(actionMenuElement);
			}
			else
				actionMenuElement.SetVisible(false);
		}
		
		m_fTotalHeightElements = (m_fHeightElement * (m_aActiveActionMenuElements.Count() - 1)) * 0.5;
		if (m_bForceResetScroll)
		{
			m_bForceResetScroll = false;
			SetScroll(0);
		}

		// Update visibility of the HOLD icons
		if (selectedAction.GetActionDuration() != 0)
		{
			if (!isInProgress)
			{
				m_Widgets.m_wOutlineOverlay.SetVisible(true);
				
				float duration = Math.AbsFloat(selectedAction.GetActionDuration());
				
				m_Widgets.m_ButtonOverlayComponent.SetHoldAction(true, duration);
			}
			else
			{
				m_Widgets.m_ButtonOverlayComponent.ForceSetHoldAction(true);
			}
		}
		else
		{
			m_Widgets.m_wOutlineOverlay.SetVisible(false);
			m_Widgets.m_ButtonOverlayComponent.SetHoldAction(false);
		}
		
		
		if (!isInProgress)
		{
			actionInfo = SCR_ActionUIInfo.Cast(selectedAction.GetUIInfo());
			m_Widgets.m_IconOverlayComponent.OnAssigned(actionInfo, selectedAction);
			
			if (!m_Widgets.m_IconOverlayComponent.HasColorData())
				m_Widgets.m_IconOverlayComponent.SetColorsData(m_mCachedActionColors);
			
			m_Widgets.m_IconOverlayComponent.UpdateColors(selectedAction, true);
			m_Widgets.m_wKeyBG.SetColor(m_Widgets.m_ButtonOverlayComponent.m_ActionDefault);
		}

		// Setups Snack Bar based on if the selected action can be performed
		if (m_bIsActionEnabled)
		{
			m_Widgets.m_wOutlineBackground.SetColor(m_EnabledColor);
			m_Widgets.m_wOutlineBackgroundGlow.SetVisible(true);
			
			m_bShouldDisplaySnackBar = false;

			if (!isInProgress)
			{
				m_Widgets.m_wIcon.SetColor(m_EnabledColor);
				m_Widgets.m_wIconGlow.SetColor(m_DisabledColorGlow);
			}
			
			m_SnackBar.AnimateHide();
		}
		else
		{
			m_Widgets.m_wOutlineBackground.SetColor(m_DisabledColor);
			m_Widgets.m_wOutlineBackgroundGlow.SetVisible(false);
			
			m_bShouldDisplaySnackBar = true;

			m_Widgets.m_wIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, UIConstants.ICON_INTERACT_DISABLED);
			m_Widgets.m_wIconGlow.LoadImageFromSet(0, UIConstants.ICONS_GLOW_IMAGE_SET, UIConstants.ICON_INTERACT_DISABLED);
			m_Widgets.m_wIcon.SetColor(m_DisabledColor);
			m_Widgets.m_wIconGlow.SetColor(m_DisabledColorGlow);
		}

		m_PrevAction = selectedAction;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Itterates through all shown actions and applies the correct padding & opacity
	//! \param[in] int current action
	protected void SetPaggingEffect(int currentAction, bool scrollingDown)
	{
		if (currentAction < 0)
			return;

		SCR_ActionMenuElementComponent element;
		int count = m_aActionMenuElements.Count();

		// If the current action is not the first action, itterate through every action before the current one first.
		for (int i = currentAction, index; i >= 0; i--)
		{
			if (i >= count)
				continue;

			element = m_aActionMenuElements[i];

			float fVerticalPos = currentAction - i;
			float timer = m_fCurrentScrollAnimation - currentAction;

			if (scrollingDown)
				fVerticalPos = fVerticalPos - Math.AbsFloat(timer);
			else
				fVerticalPos = fVerticalPos - (1.0 - timer);

			element.SetOpacity(Math.AbsFloat(fVerticalPos));
			index++;
		}

		// Itterate through all actions below (after) the current one and apply the correct padding.
		for (int i = currentAction + 1, index; i < count; i++)
		{
			float fVerticalPos = (i - currentAction);
			float timer = m_fCurrentScrollAnimation - currentAction;

			if (scrollingDown)
				fVerticalPos = fVerticalPos - Math.AbsFloat(timer);
			else
				fVerticalPos = fVerticalPos - (1.0 - timer);

			element = m_aActionMenuElements[i];
			element.SetOpacity(Math.AbsFloat(fVerticalPos));
			index++;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetSafeZone(vector worldPosition, vector screenPosition, vector min01, vector max01, out vector clampedPosition)
	{
		WorkspaceWidget workspace = m_Widgets.m_wActionMenu.GetWorkspace();
		int screenw = workspace.DPIUnscale(workspace.GetWidth());
		int screenh = workspace.DPIUnscale(workspace.GetHeight());

		vector min = {
			screenw * min01[0] + SAFEZONE_PADDING,
			screenh * min01[1] + SAFEZONE_PADDING,
			0.0
		};

		float menuw, menuh;
		m_Widgets.m_wActionMenu.GetScreenSize(menuw, menuh);
		menuw = workspace.DPIUnscale(menuw);
		menuh = workspace.DPIUnscale(menuh);

		// Maxs are offset by the menu width, so the menu never overflows
		vector max = {
			screenw * max01[0] - menuw - SAFEZONE_PADDING,
			screenh * max01[1] - menuh + SAFEZONE_PADDING,
			0.0
		};

		bool clamped = false;
		clampedPosition = screenPosition;

		// Bold assumption that can be done is that if this display is shown,
		// the main camera is used for collecting the actions
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (cameraManager)
		{
			CameraBase currentCamera = cameraManager.CurrentCamera();
			if (currentCamera)
			{
				vector camTM[4];
				currentCamera.GetWorldCameraTransform(camTM);
				vector toActionDir = (worldPosition - camTM[3]).Normalized();
				float d = vector.Dot(toActionDir, camTM[2]);
				const float threshold = 0.5;
				const float invThreshold = 1.0 / 0.5;
				// Action is "behind view"
				if (d < threshold)
				{
					float stickiness = 1.0;
					// Use this to blend towards "full backwards" smoothly
					if (d > 0.0)
						stickiness = Math.Clamp(1.0 - (d * invThreshold), 0.0, 1.0);

					clampedPosition[1] = Math.Lerp(clampedPosition[1], max[1], stickiness);
					clamped = true;
				}
			}
		}

		for (int i = 0; i < 2; ++i)
		{
			if (clampedPosition[i] < min[i])
			{
				clampedPosition[i] = min[i];
				clamped = true;
			}
			else if (clampedPosition[i] > max[i])
			{
				clampedPosition[i] = max[i];
				clamped = true;
			}
		}

		return clamped;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] worldPosition
	protected void SetPosition(vector worldPosition)
	{
		if (!m_bFixedPlacement)
		{
			vector vScreenPosition = GetGame().GetWorkspace().ProjWorldToScreen(worldPosition, GetGame().GetWorld());
			vector vClampedPosition;
			GetSafeZone(worldPosition, vScreenPosition, SCREEN_POS_MIN, SCREEN_POS_MAX, vClampedPosition);

			FrameSlot.SetPos(m_wRoot, vClampedPosition[0], vClampedPosition[1]);

			// If action menu is drawn over picture in picture, fade out its opacity
			// so it is not as intrusive and so it promotes the idea of it being "behind" the sights
			SCR_2DPIPSightsComponent pipSights = ArmaReforgerScripted.GetCurrentPIPSights();
			bool isPointInPIP = pipSights && pipSights.IsScreenPositionInSights(vClampedPosition);

			float alpha = WIDGET_DEFAULT_OPACITY;
			if (isPointInPIP)
				alpha = WIDGET_HIDDEN_OPACITY;

			m_wRoot.SetOpacity(alpha);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] currentScroll which element in the list are we on
	//! \param[in] timeSlice elasped time since the last frame
	protected void SetScroll(int currentScroll, float timeSlice = 0)
	{
		if (!m_Widgets.m_wActionMenu || currentScroll < 0)
			return;

		// Reset action list to initial position
		if (timeSlice == 0)
		{
			m_fCurrentScrollAnimation = 0.0;
			FrameSlot.SetPosY(m_Widgets.m_wActionMenu, m_fTotalHeightElements);
			
			return;
		}
		
		// Scroll the action list towards the target position (currentScroll)
		if (Math.AbsFloat(m_fCurrentScrollAnimation - currentScroll) > 0.001)
		{
			m_fCurrentScrollAnimation = Math.Lerp(m_fCurrentScrollAnimation, currentScroll, Math.Clamp(m_fScrollAnimationSpeed * timeSlice, 0.0, 1.0));
			
			float fVerticalPos = m_fTotalHeightElements - m_fHeightElement * m_fCurrentScrollAnimation;
			FrameSlot.SetPosY(m_Widgets.m_wActionMenu, fVerticalPos);
		}		
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] fTimeSlice
	protected void RunExpandTimer(float fTimeSlice)
	{
		if (m_bIsSingleEntryMenu)
		{
			m_fExpandTimer = 0.0;
			return;
		}

		// Expansion of additional actions
		m_fExpandTimer += fTimeSlice;

		if (m_fExpandTimer > m_fAutoExpandTime)
			Expand();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] fTimeSlice
	protected void RunExpandedTimer(float fTimeSlice)
	{
		if (m_bIsSingleEntryMenu)
		{
			m_fExpandedTimer = 0.0;
			return;
		}

		// Expansion of additional actions
		m_fExpandedTimer += fTimeSlice;
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void Expand()
	{
		m_bIsSingleEntryMenu = false;
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void Collapse()
	{
		m_bIsSingleEntryMenu = true;

		SetScroll(0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetButtonFadeInSpeed()
	{
		return 1 / m_fButtonFadeInSpeed;
	}

	#ifndef DISABLE_INTERACTIONS
	//------------------------------------------------------------------------------------------------
	//! Called when action starts being performed
	override void OnActionStart(IEntity pUser, BaseUserAction pPerformedAction)
	{
		m_Widgets.m_wOutlineProgress.SetMaskProgress(0);
		m_Widgets.m_wOutlineProgressGlow.SetMaskProgress(0);

		m_Widgets.m_ButtonOverlayComponent.OnButtonHold(1);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when action is being performed
	override void OnActionProgress(IEntity pUser, BaseUserAction pPerformedAction, float fProgress, float fDuration)
	{
		//~ Continues action
		if (fProgress > 0.0 && fDuration > 0.0)
		{
			m_fTimeSpentAdditionalInfo = 0;
			m_Widgets.m_wOutlineProgress.SetMaskProgress(fProgress / fDuration);
			m_Widgets.m_wOutlineProgressGlow.SetMaskProgress(fProgress / fDuration);
			m_Widgets.m_wKeyBG.SetColor(m_Widgets.m_ButtonOverlayComponent.m_ActionHovered);
			
			//~ Hackfix to track continuous actions looping back
			if (!m_bForceUpdate && m_bProgressReachedMax && fProgress < fDuration)
				m_bForceUpdate = true;
			
			m_bProgressReachedMax = fProgress >= fDuration;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called when an action is finished
	override void OnActionFinish(IEntity pUser, BaseUserAction pFinishedAction, ActionFinishReason eFinishReason)
	{
		m_Widgets.m_wOutlineProgress.SetMaskProgress(0);
		m_Widgets.m_wOutlineProgressGlow.SetMaskProgress(0);
		m_Widgets.m_ButtonOverlayComponent.OnButtonHold(0);
		m_Widgets.m_wKeyBG.SetColor(m_Widgets.m_ButtonOverlayComponent.m_ActionDefault);
		
		if (eFinishReason == ActionFinishReason.FINISHED)
			m_bForceUpdate = true;
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		// Need data to update
		if (!m_LastData || !m_LastData.pSelectedAction)
		{
			if (m_bShown)
				Show(false, UIConstants.FADE_RATE_FAST);

			return;
		}

		// Update ui
		bool succeeded = Update(m_LastData.pActionsData, m_LastData.pSelectedAction, m_LastData.pCurrentContext, timeSlice);

		// Toggle visibility
		if (!succeeded)
		{
			if (m_bShown)
				Show(false, UIConstants.FADE_RATE_FAST);

			return;
		}

		if (!m_bShown)
			Show(true, UIConstants.FADE_RATE_DEFAULT);

		// Update current widget context position
		UserActionContext lastContext = m_LastData.pCurrentContext;
		if (lastContext)
		{
			vector position = lastContext.GetOrigin(); // Context's world position
			if (lastContext.ShouldDisplayAtReferencePoint())
			{
				if (m_InteractionHandlerComponent)
					position = m_InteractionHandlerComponent.GetLastReferencePoint(timeSlice);//GetGame().GetWorld().GetTimestamp()); // Interaction reference point
			}
			SetPosition(position);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);
		Create();

		m_InteractionHandlerComponent = InteractionHandlerComponent.Cast(owner.FindComponent(InteractionHandlerComponent));

		Show(false, UIConstants.FADE_RATE_FAST);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		Destroy();
		m_LastData = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when display was hidden and is supposed to be shown. Called before first ShowActionsDisplay call
	override void ShowDisplay()
	{
		super.ShowDisplay();
		
		Show(true, UIConstants.FADE_RATE_DEFAULT);

	}

	//------------------------------------------------------------------------------------------------
	//! Called when display was open and is supposed to hide. Called after last ShowActionsDisplay call
	override void HideDisplay()
	{
		if (m_SnackBar)
			m_SnackBar.AnimateHide();

		m_LastData 		= null;
		m_PrevAction 	= null;
		m_PrevContext 	= null;
		
		if (m_Widgets)
		{
			m_Widgets.m_wOutlineProgress.SetMaskProgress(0);
			m_Widgets.m_wOutlineProgressGlow.SetMaskProgress(0);
			m_Widgets.m_ButtonOverlayComponent.OnButtonHold(0);
			m_Widgets.m_wKeyBG.SetColor(m_Widgets.m_ButtonOverlayComponent.m_ActionDefault);
		}

		Show(false, UIConstants.FADE_RATE_FAST);

		super.HideDisplay();
	}

	//------------------------------------------------------------------------------------------------
	//! Data provided by the interaction handler.
	override void SetDisplayData(ActionDisplayData data)
	{
		m_LastData = data;
		
		CompareNewDateWithOld(data);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Hackfix: We are trying to find clues if we need to update the UI based on the acton data
	protected void CompareNewDateWithOld(ActionDisplayData data)
	{		
		if (!data || !data.pActionsData)
			return;
		
		bool isEmpty;
		int count = data.pActionsData.param1.Count();
		if (m_iContextCount != count)
		{
			m_iContextCount = count;
			m_aCurrentContextNames = {};
			m_bForceResetScroll = true;
			isEmpty = true;
		}
		else if (count > 0 && data.pActionsData.param1.IsIndexValid(m_iCurrentScroll))
		{
			//~ Hackfix: if the player sprint, the action is disabled
			IEntity playerEntity = SCR_PlayerController.GetLocalControlledEntity();
			if (playerEntity && data.pActionsData.param1[m_iCurrentScroll])
			{
				bool canPerfom = data.pActionsData.param1[m_iCurrentScroll].CanBePerformed(playerEntity);
				if (m_bIsActionEnabled != canPerfom)
					m_bForceUpdate = true;
			}
		}		
		
		//~ Hackfix: We compare the old action names and try to find anything different
		foreach(int i, BaseUserAction currAction: data.pActionsData.param1)
		{
			string actionName = currAction.GetActionName();
			if (!isEmpty && actionName == m_aCurrentContextNames[i])
				continue;
			
			m_bForceUpdate = true;
			if (!isEmpty)
				m_aCurrentContextNames.InsertAt(actionName, i);
			else
				m_aCurrentContextNames.Insert(actionName);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Allows for external functions to request a UI update
	void ForceUpdateUI()
	{
		m_bForceUpdate = true;
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_ActionMenuInteractionDisplay()
	{
		Destroy();
	}
	#endif
}
