class SCR_GMMenuTileComponent : SCR_TileBaseComponent
{
	protected MissionWorkshopItem m_Item;
	protected ref SCR_MissionHeader m_Header;

	protected TextWidget m_wName;
	protected TextWidget m_wDescription;

	protected SCR_InputButtonComponent m_Play;
	protected SCR_InputButtonComponent m_Continue;
	protected SCR_InputButtonComponent m_Restart;
	protected SCR_InputButtonComponent m_FindServer;
	
	protected bool m_bFocused;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wName = TextWidget.Cast(w.FindAnyWidget("Name"));
		m_wDescription = TextWidget.Cast(w.FindAnyWidget("Description"));

		m_Play = SCR_InputButtonComponent.GetInputButtonComponent(SCR_ScenarioUICommon.BUTTON_PLAY, w);
		if (m_Play)
			m_Play.m_OnActivated.Insert(OnPlay);

		m_Continue = SCR_InputButtonComponent.GetInputButtonComponent(SCR_ScenarioUICommon.BUTTON_CONTINUE, w);
		if (m_Continue)
			m_Continue.m_OnActivated.Insert(OnContinue);

		m_FindServer = SCR_InputButtonComponent.GetInputButtonComponent(SCR_ScenarioUICommon.BUTTON_FIND_SERVERS, w);
		if (m_FindServer)
			m_FindServer.m_OnActivated.Insert(OnFindServers);

		m_Restart = SCR_InputButtonComponent.GetInputButtonComponent(SCR_ScenarioUICommon.BUTTON_RESTART, w);
		if (m_Restart)
			m_Restart.m_OnActivated.Insert(OnRestart);

		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		OnPlay();

		return super.OnClick(w, x, y, button);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		SCR_ScenarioUICommon.UpdateInputButtons(m_Item, {m_Play, m_Continue, m_Restart, m_FindServer});

		m_bFocused = true;
		
		return super.OnFocus(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		SCR_ScenarioUICommon.UpdateInputButtons(m_Item, {m_Play, m_Continue, m_Restart, m_FindServer}, false);

		m_bFocused = false;
		
		return super.OnFocusLost(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] item
	void ShowMission(notnull MissionWorkshopItem item)
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
				comp.SetImage(texture);
		}

		SCR_ScenarioUICommon.UpdateInputButtons(m_Item, {m_Play, m_Continue, m_Restart, m_FindServer}, m_bFocused);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	protected ResourceName GetTexture()
	{
		if (!m_Header && m_Item)
			return m_Item.Thumbnail().GetLocalScale(1920).Path();

		if (!m_Header.m_sPreviewImage.IsEmpty())
			return m_Header.m_sPreviewImage;

		if (!m_Header.m_sIcon.IsEmpty())
			return m_Header.m_sIcon;

		return m_Header.m_sLoadingScreen;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlay()
	{
		if (!m_Item || !SCR_ScenarioUICommon.CanPlay(m_Item))
			return;

		SCR_ScenarioUICommon.TryPlayScenario(m_Item);
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.EditorSelectionMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnContinue()
	{
		SCR_ScenarioUICommon.LoadSave(m_Item, m_Header, ChimeraMenuPreset.EditorSelectionMenu);
		SCR_ScenarioUICommon.TryPlayScenario(m_Item);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestart()
	{
		if (!m_Item || !SCR_ScenarioUICommon.CanPlay(m_Item))
			return;

		SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateDialog(SCR_ScenarioUICommon.DIALOG_RESTART);
		dialog.m_OnConfirm.Insert(OnRestartConfirmed);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestartConfirmed()
	{
		SCR_ScenarioUICommon.TryPlayScenario(m_Item);
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.EditorSelectionMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFindServers()
	{
		if (!m_Item || !SCR_ScenarioUICommon.CanJoin(m_Item))
			return;

		ServerBrowserMenuUI.TryOpenServerBrowserWithScenarioFilter(m_Item);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		SCR_ScenarioUICommon.UpdateInputButtons(m_Item, {m_Play, m_Continue, m_Restart, m_FindServer}, m_bFocused);
	}
}
