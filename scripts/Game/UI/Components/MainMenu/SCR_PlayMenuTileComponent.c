class SCR_PlayMenuTileComponent : SCR_TileBaseComponent
{
	[Attribute("0", UIWidgets.CheckBox, "Is this a big vertical tile? E.g. will use hi-res thumb picture..")]
	protected bool m_bBigTile;

	[Attribute("480", UIWidgets.Slider, "Estimated width of grid thumb picture, in a reference resolution.", "320 1920 1")]
	protected float m_fThumbnailWidth;

	MissionWorkshopItem m_Item;
	ref SCR_MissionHeader m_Header;

	protected Widget m_wContentGroup;
	protected Widget m_wFeatured;
	protected TextWidget m_wName;
	protected TextWidget m_wDescription;
	protected Widget m_wRecentlyPlayed;
	protected TextWidget m_wRecentlyPlayedText;

	// Mouse interact buttons
	protected Widget m_wMouseInteractButtons;

	bool m_bIsMouseInteraction;
	protected bool m_bFocused;
	
	// Button components
	protected SCR_ModularButtonComponent m_Play;
	protected SCR_ModularButtonComponent m_Continue;
	protected SCR_ModularButtonComponent m_FindServer;
	protected SCR_ModularButtonComponent m_Host;
	protected SCR_ModularButtonComponent m_Restart;
	
	// Warning
	protected SCR_SimpleWarningOverlayComponent m_WarningOverlay;
	
	// Tooltip
	protected SCR_ScriptedWidgetTooltip m_CurrentTooltip;

	// Script invokers for the mouse interact buttons
	protected ref ScriptInvokerString m_OnMouseInteractionButtonClicked;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		m_wName = TextWidget.Cast(w.FindAnyWidget("Name"));
		m_wDescription = TextWidget.Cast(w.FindAnyWidget("Description"));
		m_wContentGroup = w.FindAnyWidget("ContentGroup");

		m_wMouseInteractButtons = w.FindAnyWidget("MouseInteractButtons");
		Widget wPlay = w.FindAnyWidget(SCR_ScenarioUICommon.BUTTON_PLAY);
		Widget wContinue = w.FindAnyWidget(SCR_ScenarioUICommon.BUTTON_CONTINUE);
		Widget wRestart = w.FindAnyWidget(SCR_ScenarioUICommon.BUTTON_RESTART);
		Widget wHost = w.FindAnyWidget(SCR_ScenarioUICommon.BUTTON_HOST);
		Widget wFindServer = w.FindAnyWidget(SCR_ScenarioUICommon.BUTTON_FIND_SERVERS);

		m_Play = SCR_ModularButtonComponent.FindComponent(wPlay);
		if (m_Play)
			m_Play.m_OnClicked.Insert(OnPlay);

		m_Continue = SCR_ModularButtonComponent.FindComponent(wContinue);
		if (m_Continue)
			m_Continue.m_OnClicked.Insert(OnContinue);

		m_Restart = SCR_ModularButtonComponent.FindComponent(wRestart);
		if (m_Restart)
			m_Restart.m_OnClicked.Insert(OnRestart);

		m_Host = SCR_ModularButtonComponent.FindComponent(wHost);
		if (m_Host)
			m_Host.m_OnClicked.Insert(OnHost);

		m_FindServer = SCR_ModularButtonComponent.FindComponent(wFindServer);
		if (m_FindServer)
			m_FindServer.m_OnClicked.Insert(OnFindServers);
		
		if (GetGame().InPlayMode())
			Enable(false);

		InputManager inputManager = GetGame().GetInputManager();

		if (inputManager)
			m_bIsMouseInteraction = (inputManager.GetLastUsedInputDevice() == EInputDeviceType.MOUSE);

		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceUserChanged);
		
		m_WarningOverlay = SCR_SimpleWarningOverlayComponent.Cast(SCR_SimpleWarningOverlayComponent.FindComponentInHierarchy(m_wRoot));
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{	
		m_bFocused = true;
		UpdateModularButtons();

		// Tooltips
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);
		
		return super.OnFocus(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		m_bFocused = false;
		UpdateModularButtons();
	
		// Tooltips
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
		
		return super.OnFocusLost(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] item
	//! \param[in] contentType
	void Setup(notnull MissionWorkshopItem item, EPlayMenuContentType contentType)
	{
		m_Item = item;

		// Init header only if scenario is NOT coming from an addon
		if (!item.GetOwner())
			m_Header = SCR_MissionHeader.GetMissionHeader(item);

		m_wName.SetText(item.Name());
		m_wDescription.SetText(item.Description());

		UpdateModularButtons();
		UpdateWarning();
		
		// Set image through SCR_ButtonImageComponent
		SCR_ButtonImageComponent comp = SCR_ButtonImageComponent.Cast(m_wRoot.FindHandler(SCR_ButtonImageComponent));
		if (comp)
		{
			ResourceName texture = GetTexture();

			if (!texture.IsEmpty())
				comp.SetImage(texture, item.GetOwner() != null);
		}

		if (contentType == EPlayMenuContentType.RECENT)
		{
			int timeSinceLastPlayedSeconds = m_Item.GetTimeSinceLastPlay();
			string sLastPlayed = SCR_FormatHelper.GetTimeSinceEventImprecise(timeSinceLastPlayedSeconds);
		}
		
		if (m_wMouseInteractButtons)
			m_wMouseInteractButtons.SetVisible(false);

		Enable();
	}

	//------------------------------------------------------------------------------------------------
	protected void Enable(bool enable = true)
	{
		m_wRoot.SetEnabled(enable);
		m_wContentGroup.SetVisible(enable);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetTexture()
	{
		if (m_bBigTile && m_Header && !m_Header.m_sPreviewImage.IsEmpty())
			return m_Header.m_sPreviewImage;

		if (m_Header)
			return m_Header.m_sLoadingScreen;

		if (m_Item)
		{
			BackendImage image = m_Item.Thumbnail();

			// DEBUG problems with scenario addon thumbs
//			array<ImageScale> scales = {};
//			int i = image.GetScales(scales);

			// Get optimal width for the thumb
			float width = g_Game.GetWorkspace().DPIScale(m_fThumbnailWidth);
			ImageScale scale = image.GetLocalScale((int)width);
			if (scale)
				return scale.Path();
		}

		return string.Empty;
	}

	// React on switching between input methods
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceUserChanged(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		//PrintFormat("OnInputDeviceUserChanged | %1 -> %2", oldDevice, newDevice);

		if (newDevice == EInputDeviceType.TRACK_IR || newDevice == EInputDeviceType.JOYSTICK)
			return;

		m_bIsMouseInteraction = (newDevice == EInputDeviceType.MOUSE);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlay()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioUICommon.BUTTON_PLAY);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnContinue()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioUICommon.BUTTON_CONTINUE);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestart()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioUICommon.BUTTON_RESTART);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHost()
	{
		if (!SCR_ScenarioUICommon.CanHost(m_Item))
			return;
		
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioUICommon.BUTTON_HOST);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFindServers()
	{
		if (!SCR_ScenarioUICommon.CanJoin(m_Item))
			return;
		
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke(SCR_ScenarioUICommon.BUTTON_FIND_SERVERS);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateModularButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		m_CurrentTooltip = tooltip;
		SCR_ScenarioUICommon.UpdateMouseButtonTooltips(tooltip, m_Item);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateModularButtons()
	{
		SCR_ScenarioUICommon.UpdateMouseButtonTooltips(m_CurrentTooltip, m_Item);
		
		if (m_wMouseInteractButtons)
			m_wMouseInteractButtons.SetVisible(m_bFocused);

		SCR_ScenarioUICommon.UpdatePlayMouseButton(m_Play, m_Item, m_bFocused);
		SCR_ScenarioUICommon.UpdateContinueMouseButton(m_Continue, m_Item, m_bFocused);
		SCR_ScenarioUICommon.UpdateRestartMouseButton(m_Restart, m_Item, m_bFocused);
		SCR_ScenarioUICommon.UpdateHostMouseButton(m_Host, m_Item, m_bFocused);
		SCR_ScenarioUICommon.UpdateJoinMouseButton(m_FindServer, m_Item, m_bFocused);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateWarning()
	{
		if (!m_WarningOverlay)
			return;
		
		SCR_ERevisionAvailability availability = SCR_ScenarioUICommon.GetOwnerRevisionAvailability(m_Item);
		m_bIsInErrorState = availability != SCR_ERevisionAvailability.ERA_AVAILABLE;
		
		m_WarningOverlay.SetWarningVisible(m_bIsInErrorState, false);
		m_WarningOverlay.SetWarning(SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorMessage(availability), SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorTexture(availability));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerString GetOnMouseInteractionButtonClicked()
	{
		if (!m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked = new ScriptInvokerString();

		return m_OnMouseInteractionButtonClicked;
	}
}
