//------------------------------------------------------------------------------------------------
//! Class that handles Welcome screen menu
class SCR_WelcomeScreenMenu : SCR_DeployMenuBase
{
	protected SCR_BaseGameMode m_GameMode;
	protected PlayerManager m_PlayerManager;
	protected RichTextWidget m_wPlayerCount;
	protected RichTextWidget m_wScenarioTimeElapsed;
	protected ref MapConfiguration m_MapConfig = new MapConfiguration();
	protected string m_sTimeElapsed;
	protected float m_fTimeSlice;
	protected int m_iMaxPlayerCount;

	//------------------------------------------------------------------------------------------------
	//! Initializes necessary parts of menu upon opening it
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		m_PlayerManager = GetGame().GetPlayerManager();
		m_MapEntity = SCR_MapEntity.GetMapInstance();

		SCR_WelcomeScreenComponent welcomeScreen = SCR_WelcomeScreenComponent.Cast(m_GameMode.FindComponent(SCR_WelcomeScreenComponent));
		if (!welcomeScreen)
		{
			Close();
			return;
		}

		/*RichTextWidget scenarioTitle = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("DeploySetup"));
		if (scenarioTitle)
			scenarioTitle.SetText(welcomeScreen.GetHeaderTitle());

		RichTextWidget scenarioSubtitle = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("HeaderSubtitle"));
		if (scenarioSubtitle)
			scenarioSubtitle.SetText(welcomeScreen.GetHeaderSubtitle()); */

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
				continueButton.m_OnActivated.Insert(CloseWelcomeScreenMenu);
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
		SCR_DeployMenuBaseScreenLayout baseLayout = welcomeScreen.GetBaseLayout();
		if (!baseLayout)
			return;

		baseLayout.InitContent(this);

		InitMap();

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		gameMode.PauseGame(true, SCR_EPauseReason.MENU);
	}

	//------------------------------------------------------------------------------------------------
	//! Mutes sounds once the menu is opened
	override void OnMenuOpened()
	{
		// Mute sounds
		// If menu is opened before loading screen is closed, wait for closing
		if (ArmaReforgerLoadingAnim.IsOpen())
			ArmaReforgerLoadingAnim.m_onExitLoadingScreen.Insert(MuteSounds);
		else
			MuteSounds();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		gameMode.PauseGame(false, SCR_EPauseReason.MENU);
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes map in the menu background
	protected void InitMap()
	{
		if (!m_MapEntity)
			return;
		const ResourceName conf = "{A786DD4868598F15}Configs/Map/MapPlain.conf"; // TODO: check for good const usage
		m_MapEntity.OpenMap(m_MapEntity.SetupMapConfig(EMapEntityMode.PLAIN, conf, GetRootWidget()));
	}

	//------------------------------------------------------------------------------------------------
	//! Adds action listeners when menu is focused
	override void OnMenuFocusGained()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.AutoInit();

		GetGame().GetInputManager().AddActionListener("ShowScoreboard", EActionTrigger.DOWN, OpenPlayerList);

		super.OnMenuFocusGained();
	}

	//------------------------------------------------------------------------------------------------
	//! Removes action listeners when menu loses focus
	override void OnMenuFocusLost()
	{
		GetGame().GetInputManager().RemoveActionListener("ShowScoreboard", EActionTrigger.DOWN, OpenPlayerList);

		super.OnMenuFocusLost();
	}

	//------------------------------------------------------------------------------------------------
	//! Opens pause menu
	protected void OpenPauseMenu()
	{
		MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PauseMenu, 0, true, false);

		PauseMenuUI pauseMenu = PauseMenuUI.Cast(menu);
		if (pauseMenu)
		{
			pauseMenu.FadeBackground(true, true);
			pauseMenu.DisableSettings();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Opens Welcome screen menu
	void OpenWelcomeScreenMenu()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.WelcomeScreenMenu);
	}

	//------------------------------------------------------------------------------------------------
	//! Closes Welcome screen menu
	void CloseWelcomeScreenMenu()
	{
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.WelcomeScreenMenu);

		PlayerController playerController = GetGame().GetPlayerController();
		SCR_PlayerDeployMenuHandlerComponent deployMenu = SCR_PlayerDeployMenuHandlerComponent.Cast(playerController.FindComponent(SCR_PlayerDeployMenuHandlerComponent));
		if (deployMenu)
			deployMenu.SetWelcomeClosed();
	}

	//------------------------------------------------------------------------------------------------
	//! Refreshes chat and elapsed time
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		GetGame().GetInputManager().ActivateContext("DeployMenuContext");
		GetGame().GetInputManager().ActivateContext("MenuWithEditorContext");

		if (m_ChatPanel)
			m_ChatPanel.OnUpdateChat(tDelta);

		m_fTimeSlice += tDelta;
		if (m_fTimeSlice > 1)
			UpdateElapsedTime();
	}

	//------------------------------------------------------------------------------------------------
	//! Toggles chat panel
	protected void OnChatToggle()
	{
		if (!m_ChatPanel)
		{
			Widget chat = GetRootWidget().FindAnyWidget("ChatPanel");
			if (chat)
				m_ChatPanel = SCR_ChatPanel.Cast(chat.FindHandler(SCR_ChatPanel));
		}

		if (!m_ChatPanel || m_ChatPanel.IsOpen())
			return;

		SCR_ChatPanelManager chatPanelManager = SCR_ChatPanelManager.GetInstance();

		if (!chatPanelManager)
			return;

		SCR_ChatPanelManager.GetInstance().ToggleChatPanel(m_ChatPanel);
	}

	//------------------------------------------------------------------------------------------------
	//! Updates elapsed time
	protected void UpdateElapsedTime()
	{
		m_fTimeSlice = 0;
		m_sTimeElapsed = SCR_FormatHelper.FormatTime(m_GameMode.GetElapsedTime());
		m_wScenarioTimeElapsed.SetText(m_sTimeElapsed);
		UpdatePlayerCount(0);
	}

	//------------------------------------------------------------------------------------------------
	//! Updates player count
	protected void UpdatePlayerCount(int playerID)
	{
		if (m_iMaxPlayerCount > 0)
			m_wPlayerCount.SetTextFormat("%1/%2", m_PlayerManager.GetPlayerCount().ToString(), m_iMaxPlayerCount);
		else
			m_wPlayerCount.SetText(m_PlayerManager.GetPlayerCount().ToString());
	}
}
