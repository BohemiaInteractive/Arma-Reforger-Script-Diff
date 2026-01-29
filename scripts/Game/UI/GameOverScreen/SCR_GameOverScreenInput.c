class GameOverScreenInput: ChimeraMenuBase
{	
	protected InputManager m_InputManager = GetGame().GetInputManager();
	protected Widget m_BackButton;
	protected Widget m_ChatButton;
	protected SCR_GameOverScreenUIComponent m_GameOverScreenUIComponent;
	protected SCR_ChatPanel m_ChatPanel;
	
	override void OnMenuOpen() 
	{
		Widget widgetRoot = GetRootWidget();

		//~ Find Back button
		m_BackButton = widgetRoot.FindAnyWidget(UIConstants.BUTTON_BACK);
		SCR_InputButtonComponent comp = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, widgetRoot);
		if (comp)
			comp.m_OnActivated.Insert(ReturnToMainMenu);
		
		//~ Find Chat button
		m_ChatButton = widgetRoot.FindAnyWidget("ChatButton");
		comp = SCR_InputButtonComponent.GetInputButtonComponent("ChatButton", widgetRoot);
		if (comp)
			comp.m_OnActivated.Insert(OnChatToggle);
		
		//~ Find chat
		Widget wChatPanel = GetRootWidget().FindAnyWidget("ChatPanel");
		if (wChatPanel)
			m_ChatPanel = SCR_ChatPanel.Cast(wChatPanel.FindHandler(SCR_ChatPanel));
		
		//~ Find GameOverScreen UI
		m_GameOverScreenUIComponent = SCR_GameOverScreenUIComponent.Cast(widgetRoot.FindHandler(SCR_GameOverScreenUIComponent));
		
		// Hide unnecessary HUD elements. This should hide chat as well.
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (hudManager)
			hudManager.SetVisibleLayers(hudManager.GetVisibleLayers() & ~(EHudLayers.HIGH | EHudLayers.ALWAYS_TOP));
		
		GetGame().GetInputManager().AddActionListener("ShowScoreboard", EActionTrigger.DOWN, ShowPlayerList);
	}
	
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		// Show back the previously hidden HUD elements.
		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
		{
			SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
			if (hudManager)
				hudManager.SetVisibleLayers(hudManager.GetVisibleLayers() | EHudLayers.HIGH | EHudLayers.ALWAYS_TOP);
		}
		GetGame().GetInputManager().RemoveActionListener("ShowScoreboard", EActionTrigger.DOWN, ShowPlayerList);
	}
	
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		if (m_ChatPanel)
			m_ChatPanel.OnUpdateChat(tDelta);
	}
	
	protected void OnChatToggle()
	{
		if (!m_ChatPanel || m_ChatPanel.IsOpen() || (m_ChatButton && (!m_ChatButton.IsVisible() || m_ChatButton.GetOpacity() != 1)))
			return;
		
		SCR_ChatPanelManager.GetInstance().ToggleChatPanel(m_ChatPanel);
	}
	
	protected void BackToMainMenuPopupComfirm()
	{
		Close();
		GameStateTransitions.RequestGameplayEndTransition();
	}
	
	protected void OnBackToMainMenu()
	{
		SCR_ConfigurableDialogUi dlg = SCR_CommonDialogs.CreateDialog("scenario_exit");
		if (!dlg)
			return;
		
		dlg.m_OnConfirm.Insert(BackToMainMenuPopupComfirm);
	}
	
	void ReturnToMainMenu()
	{
		if (m_BackButton && (!m_BackButton.IsVisible() || m_BackButton.GetOpacity() != 1))
			return;
		
		OnBackToMainMenu();
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowPlayerList()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PlayerListMenu, 0, true, false);
	}
};