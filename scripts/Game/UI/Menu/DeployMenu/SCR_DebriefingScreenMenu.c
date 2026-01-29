//------------------------------------------------------------------------------------------------
class SCR_DebriefingScreenMenu : SCR_WelcomeScreenMenu
{
	protected static const string RESTART_TIMER_WIDGET_NAME = "RestartTimerHolder";
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		m_PlayerManager = GetGame().GetPlayerManager();
		m_MapEntity = SCR_MapEntity.GetMapInstance();

		SCR_DebriefingScreenComponent debriefingScreen = SCR_DebriefingScreenComponent.Cast(m_GameMode.FindComponent(SCR_DebriefingScreenComponent));
		if (!debriefingScreen)
		{
			Close();
			return;
		}
		
		m_GameMasterButton = SCR_InputButtonComponent.GetInputButtonComponent("Editor", GetRootWidget());
		if (m_GameMasterButton)
		{
		    if (m_GameMode.GetState() != SCR_EGameModeState.GAME)
		    {
		         m_GameMasterButton.SetVisible(false);
		    }
		    else 
		    {
		    	SCR_EditorManagerEntity editor = SCR_EditorManagerEntity.GetInstance();
				if (editor)	
		    		m_GameMasterButton.SetVisible(editor && !editor.IsLimited());
				else
					m_GameMasterButton.SetVisible(false);
		    }
		}

		// Check is the restarting counter is in the layout and make it active
		Widget restartCounterWidget = GetRootWidget().FindAnyWidget(RESTART_TIMER_WIDGET_NAME);
		if (restartCounterWidget)
			restartCounterWidget.SetVisible(true);
		
		RichTextWidget scenarioTitle = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("DeploySetup"));
		if (scenarioTitle)
			//scenarioTitle.SetText(debriefingScreen.GetHeaderTitle());
			scenarioTitle.SetText("#AR-DebriefingScreen_Debriefing");

		/*RichTextWidget scenarioSubtitle = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("HeaderSubtitle"));
		if (scenarioSubtitle)
			scenarioSubtitle.SetText(debriefingScreen.GetHeaderSubtitle());*/

		ServerInfo serverInfo = GetGame().GetServerInfo();
		if (serverInfo)
			m_iMaxPlayerCount = serverInfo.GetPlayerLimit();

		m_wPlayerCount = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("PlayerCount"));
		UpdatePlayerCount(0);

		m_wScenarioTimeElapsed = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("TimeElapsed"));

		Widget continueBtn = GetRootWidget().FindAnyWidget("CloseButton");
		if (continueBtn)
		{
			SCR_InputButtonComponent continueButton = SCR_InputButtonComponent.Cast(continueBtn.FindHandler(SCR_InputButtonComponent));
			if (continueButton)
				continueButton.m_OnActivated.Insert(ReturnToMenu);
			
			RichTextWidget continueBtnBackToMenu = RichTextWidget.Cast(continueBtn.FindAnyWidget("Text"));
			if (continueBtnBackToMenu)
				continueBtnBackToMenu.SetText("#AR-PauseMenu_Exit");
		}
		
		Widget pauseMenuBtn = GetRootWidget().FindAnyWidget("PauseButton");
		if (pauseMenuBtn)
		{
			SCR_InputButtonComponent pauseMenuButton = SCR_InputButtonComponent.Cast(pauseMenuBtn.FindHandler(SCR_InputButtonComponent));
			if (pauseMenuButton)
				pauseMenuButton.m_OnActivated.Insert(OpenPauseMenu);
		}
		
		Widget chat = GetRootWidget().FindAnyWidget("ChatPanel");
		if (chat)
			m_ChatPanel = SCR_ChatPanel.Cast(chat.FindHandler(SCR_ChatPanel));

		m_ChatButton = SCR_InputButtonComponent.GetInputButtonComponent("ChatButton", GetRootWidget());
		if (m_ChatButton)
			m_ChatButton.m_OnActivated.Insert(OnChatToggle);

		UpdateElapsedTime();
		SCR_DeployMenuBaseScreenLayout baseLayout = debriefingScreen.GetBaseLayout();
		if (!baseLayout)
			return;

		baseLayout.InitContent(this);

		//InitMap();
	}
	
	//------------------------------------------------------------------------------------------------
	void ReturnToMenu()
	{
		SCR_ConfigurableDialogUi dlg = SCR_CommonDialogs.CreateDialog("scenario_exit");
		if (!dlg)
			return;
		
		dlg.m_OnConfirm.Insert(BackToMainMenuPopupConfirm);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void BackToMainMenuPopupConfirm()
	{
		GetRootWidget().RemoveFromHierarchy();
		GameStateTransitions.RequestGameplayEndTransition();
	}
	
	//------------------------------------------------------------------------------------------------
	void OpenDebriefingScreenMenu()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.DebriefingScreenMenu);
	}

	//------------------------------------------------------------------------------------------------
	void CloseDebriefingScreenMenu()
	{
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.DebriefingScreenMenu);
	}

	//------------------------------------------------------------------------------------------------
	//! Opens pause menu
	override protected void OpenPauseMenu()
	{
		MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PauseMenu, 0, true, false);

		PauseMenuUI pauseMenu = PauseMenuUI.Cast(menu);
		if (pauseMenu)
		{
			pauseMenu.FadeBackground(true, true);
			pauseMenu.DisableSettings();
			pauseMenu.DisableGameMaster();
			pauseMenu.DisableArmaVision();
		}
	}
};
