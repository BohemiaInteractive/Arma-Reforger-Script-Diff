class SCR_MainMenuTileComponent: SCR_TileBaseComponent
{	
	protected const int THUMBNAIL_LOCALSCALE = 1920;
	
	protected MissionWorkshopItem m_Item;
	protected ref SCR_MissionHeader m_Header;

	protected TextWidget m_wName;
	protected TextWidget m_wDescription;
	protected Widget m_wTextContent;
	protected Widget m_DisabledTileShadow;
	protected ImageWidget m_wBackground;
	
	protected ImageWidget m_wNewIcon;
	protected ImageWidget m_wRecentIcon;
	protected ImageWidget m_wFavoriteIcon;

	protected SCR_InputButtonComponent m_Play;
	protected SCR_InputButtonComponent m_Continue;
	protected SCR_InputButtonComponent m_Restart;
	
	protected bool m_bFocused;
	protected bool m_bDisabled;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wName = TextWidget.Cast(w.FindAnyWidget("Name"));
		m_wDescription = TextWidget.Cast(w.FindAnyWidget("Description"));
		m_wDescription.SetVisible(false);
		
		m_wTextContent = w.FindAnyWidget("TextSizeLayout");		
		
		m_wBackground = ImageWidget.Cast(w.FindAnyWidget("Shading"));
		m_wBackground.SetVisible(false);
		
		m_wNewIcon = ImageWidget.Cast(w.FindAnyWidget("NewIcon"));
		m_wNewIcon.SetVisible(false);
		
		m_wRecentIcon = ImageWidget.Cast(w.FindAnyWidget("RecentIcon"));
		m_wRecentIcon.SetVisible(false);
		
		m_wFavoriteIcon = ImageWidget.Cast(w.FindAnyWidget("FavoriteIcon"));
		m_wFavoriteIcon.SetVisible(false);
		
		m_DisabledTileShadow = w.FindAnyWidget("m_DisabledTileShadow");
		m_DisabledTileShadow.SetVisible(false);

		m_Play = SCR_InputButtonComponent.GetInputButtonComponent(SCR_ScenarioUICommon.BUTTON_PLAY, w);
		if (m_Play)
			m_Play.m_OnActivated.Insert(OnPlay);

		m_Continue = SCR_InputButtonComponent.GetInputButtonComponent(SCR_ScenarioUICommon.BUTTON_CONTINUE, w);
		if (m_Continue)
			m_Continue.m_OnActivated.Insert(OnContinue);

		m_Restart = SCR_InputButtonComponent.GetInputButtonComponent(SCR_ScenarioUICommon.BUTTON_RESTART, w);
		if (m_Restart)
			m_Restart.m_OnActivated.Insert(OnRestart);

		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		bool isMouseInteraction = GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE;
		
		UpdateButtonInputVisibility(m_Play.GetRootWidget(), !isMouseInteraction);
		UpdateButtonInputVisibility(m_Continue.GetRootWidget(), !isMouseInteraction);
		UpdateButtonInputVisibility(m_Restart.GetRootWidget(), !isMouseInteraction);

		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceUserChanged);
	}

	// React on switching between input methods
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceUserChanged(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		if (m_bDisabled || newDevice == EInputDeviceType.TRACK_IR || newDevice == EInputDeviceType.JOYSTICK)
			return;

		bool isMouseInteraction = newDevice == EInputDeviceType.MOUSE;
		
		UpdateButtonInputVisibility(m_Play.GetRootWidget(), !isMouseInteraction);
		UpdateButtonInputVisibility(m_Continue.GetRootWidget(), !isMouseInteraction);
		UpdateButtonInputVisibility(m_Restart.GetRootWidget(), !isMouseInteraction);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateButtonInputVisibility(notnull Widget root, bool visibility)
	{
		Widget buttonWidget = root.FindAnyWidget("InputButtonDisplayRoot");
		if (!buttonWidget)
			return;
		
		buttonWidget.SetEnabled(visibility);
		buttonWidget.SetVisible(visibility);
	}
	
	//------------------------------------------------------------------------------------------------
	void DisableTile()
	{
		m_bDisabled = true;
		
		m_DisabledTileShadow.SetVisible(true);
		
		m_Play.SetEnabled(false);
		m_Continue.SetEnabled(false);
		m_Restart.SetEnabled(false);
		
		SCR_ButtonImageComponent comp = SCR_ButtonImageComponent.Cast(m_wRoot.FindHandler(SCR_ButtonImageComponent));
		if (comp)
			 comp.SetImageSaturation(!m_bDisabled);
		
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		OnPlayOrContinue();

		return super.OnClick(w, x, y, button);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if (m_bDisabled)
			return super.OnFocus(w, x, y);
		
		SCR_ScenarioUICommon.UpdateInputButtons(m_Item, {m_Play, m_Continue, m_Restart});

		m_bFocused = true;
		float position[4] = {20.0, 0.0, 20.0, 25.0};
		
		AnimateWidget.Padding(m_wTextContent, position, 5.0);
		m_wDescription.SetVisible(true);
		m_wBackground.SetVisible(true);		
		
		return super.OnFocus(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		SCR_ScenarioUICommon.UpdateInputButtons(m_Item, {m_Play, m_Continue, m_Restart}, false);

		m_bFocused = false;
		float position[4] = {20.0, 0.0, 20.0, -70.0};
		
		AnimateWidget.Padding(m_wTextContent, position, 5.0);
		m_wDescription.SetVisible(false);
		m_wBackground.SetVisible(false);
		
		return super.OnFocusLost(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] item
	void ShowMission(notnull MissionWorkshopItem item, bool showRecommended = false)
	{
		m_Item = item;
		if (!m_Item)
			return;

		if (m_wName)
			m_wName.SetText(m_Item.Name());

		if (m_wDescription)
			m_wDescription.SetText(m_Item.Description());

		// Set image through SCR_ButtonImageComponent
		m_Header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(m_Item.Id()));

		SCR_ButtonImageComponent comp = SCR_ButtonImageComponent.Cast(m_wRoot.FindHandler(SCR_ButtonImageComponent));
		if (comp)
		{
			ResourceName texture = GetTexture();
			if (!texture.IsEmpty())
			{
				comp.SetImage(texture);
				comp.SetImageSaturation(!m_bDisabled);
			}
			
		}

		SCR_ScenarioUICommon.UpdateInputButtons(m_Item, {m_Play, m_Continue, m_Restart}, m_bFocused);
		
		if (item.IsFavorite())
			m_wFavoriteIcon.SetVisible(true);
		
		if (item.GetTimeSinceLastPlay() >= 0)
			m_wRecentIcon.SetVisible(true);
		
		if (showRecommended)
			m_wNewIcon.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	protected ResourceName GetTexture()
	{
		if (!m_Header && m_Item)
			return m_Item.Thumbnail().GetLocalScale(THUMBNAIL_LOCALSCALE).Path();

		if (!m_Header.m_sPreviewImage.IsEmpty())
			return m_Header.m_sPreviewImage;

		if (!m_Header.m_sIcon.IsEmpty())
			return m_Header.m_sIcon;

		return m_Header.m_sLoadingScreen;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayOrContinue()
	{
		if (m_bDisabled || !m_Item || !SCR_ScenarioUICommon.CanPlay(m_Item))
			return;
		
		if (m_Continue.IsEnabled())
			OnContinue();
		else
			OnPlay();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlay()
	{
		if (m_bDisabled || !m_Item || !SCR_ScenarioUICommon.CanPlay(m_Item))
			return;

		SCR_ScenarioUICommon.TryPlayScenario(m_Item);
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.MainMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnContinue()
	{
		if (m_bDisabled)
			return;
		
		SCR_ScenarioUICommon.LoadSave(m_Item, m_Header, ChimeraMenuPreset.MainMenu);
		SCR_ScenarioUICommon.TryPlayScenario(m_Item);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestart()
	{
		if (m_bDisabled || !m_Item || !SCR_ScenarioUICommon.CanPlay(m_Item))
			return;

		SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateDialog(SCR_ScenarioUICommon.DIALOG_RESTART);
		dialog.m_OnConfirm.Insert(OnRestartConfirmed);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestartConfirmed()
	{
		SCR_ScenarioUICommon.TryPlayScenario(m_Item);
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.MainMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		SCR_ScenarioUICommon.UpdateInputButtons(m_Item, {m_Play, m_Continue, m_Restart}, m_bFocused);
	}
}
