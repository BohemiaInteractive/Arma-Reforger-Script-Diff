//! Role selection menu for setting up player faction, group and loadout.
class SCR_RoleSelectionMenu : SCR_DeployMenuBase
{
	protected SCR_DeployMenuHandler m_MenuHandler;

	protected SCR_FactionRequestUIComponent m_FactionRequestUIHandler;
	protected SCR_LoadoutRequestUIComponent m_LoadoutRequestUIHandler;
	protected SCR_GroupRequestUIComponent m_GroupRequestUIHandler;

	protected SCR_FactionManager m_FactionManager;
	protected SCR_FactionPlayerList m_FactionPlayerList;
	protected SCR_GroupPlayerList m_GroupPlayerList;

	protected SCR_BaseGameMode m_GameMode;
	protected SCR_PlayerFactionAffiliationComponent m_PlyFactionAffilComp;
	protected SCR_PlayerLoadoutComponent m_PlyLoadoutComp;

	protected SCR_InputButtonComponent m_ContinueButton;
	protected SCR_InputButtonComponent m_GroupOpenButton;
	protected SCR_InputButtonComponent m_ViewProfileButton;

	protected Widget m_wPersistentFaction;
	protected TextWidget m_wScenarioTimeElapsed;
	protected TextWidget m_wPlayerCount;
	protected TextWidget m_wServerName;
	
	protected int m_iMaxPlayerCount;	
	protected float m_fTimer = 0;
	
	protected int m_iLastFocusedEntryPlayerId;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		m_MapEntity = SCR_MapEntity.GetMapInstance();
		m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		m_MenuHandler = SCR_DeployMenuHandler.Cast(GetRootWidget().FindHandler(SCR_DeployMenuHandler));

		m_wPersistentFaction = GetRootWidget().FindAnyWidget("PermanentFaction");
		if (m_wPersistentFaction)
			m_wPersistentFaction.SetVisible(!SCR_RespawnSystemComponent.GetInstance().IsFactionChangeAllowed());

		m_wScenarioTimeElapsed = TextWidget.Cast(GetRootWidget().FindAnyWidget("TimeElapsed"));
		m_wServerName = TextWidget.Cast(GetRootWidget().FindAnyWidget("ServerName"));
		if (m_wServerName)
		{
			SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
			if (header)
			m_wServerName.SetText(header.m_sName);
		}
		
		ServerInfo serverInfo = GetGame().GetServerInfo();
		if (serverInfo)
			m_iMaxPlayerCount = serverInfo.GetPlayerLimit();
		
		if (!m_MenuHandler)
		{
			Print("SCR_DeployMenuHandler is missing on root widget of " + this, LogLevel.ERROR);
			return;
		}

		PlayerController pc = GetGame().GetPlayerController();
		m_PlyFactionAffilComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!m_PlyFactionAffilComp)
		{
			Print("Cannot find player faction affiliation component!", LogLevel.ERROR);
		}

		m_PlyLoadoutComp = SCR_PlayerLoadoutComponent.Cast(pc.FindComponent(SCR_PlayerLoadoutComponent));
		if (!m_PlyLoadoutComp)
		{
			Print("Cannot find player loadout component!", LogLevel.ERROR);
		}

		m_wPlayerCount = TextWidget.Cast(GetRootWidget().FindAnyWidget("PlayerCount"));
		m_FactionPlayerList = m_MenuHandler.GetFactionPlayerList();

		UpdateElapsedTime();
		UpdatePlayerCount();

		FindRequestHandlers();
		HookEvents();

		m_ContinueButton = SCR_InputButtonComponent.GetInputButtonComponent("CloseButton", GetRootWidget());
		if (m_ContinueButton)
		{
			m_ContinueButton.m_OnActivated.Insert(Close);
			m_ContinueButton.SetEnabled(CanContinue());
		}

		m_GroupOpenButton = SCR_InputButtonComponent.GetInputButtonComponent("GroupMenu", GetRootWidget());
		if (m_GroupOpenButton)
		{
			m_GroupOpenButton.m_OnActivated.Insert(OpenGroupMenu);
			m_GroupOpenButton.SetEnabled(CanOpenGroupMenu());
		}		

		m_ViewProfileButton = SCR_InputButtonComponent.GetInputButtonComponent("ViewProfile", GetRootWidget());
		if (m_ViewProfileButton)
		{
			UpdateViewProfileButton(true);
			m_ViewProfileButton.m_OnActivated.Insert(OnViewProfile);
		}
		
		Widget chat = GetRootWidget().FindAnyWidget("ChatPanel");
		if (chat)
			m_ChatPanel = SCR_ChatPanel.Cast(chat.FindHandler(SCR_ChatPanel));

		m_ChatButton = SCR_InputButtonComponent.GetInputButtonComponent("ChatButton", GetRootWidget());
		if (m_ChatButton)
			m_ChatButton.m_OnActivated.Insert(OnChatToggle);

		m_PauseButton = SCR_InputButtonComponent.GetInputButtonComponent("PauseButton", GetRootWidget());
		if (m_PauseButton)
			m_PauseButton.m_OnActivated.Insert(OnPauseMenu);
		InitMapPlain();
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		gameMode.PauseGame(true, SCR_EPauseReason.MENU);
	}
	
	override void OnMenuOpened()
	{
		super.OnMenuOpened();
		
		if (m_FactionRequestUIHandler)
		{
			SCR_FactionButton btn = SCR_FactionButton.Cast(m_FactionRequestUIHandler.GetFirstValidButton());
			if (btn)
				btn.SetFocused();
		}
	}

	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		GetGame().GetInputManager().ActivateContext("DeployMenuContext");
		
		if (m_ChatPanel)
			m_ChatPanel.OnUpdateChat(tDelta);

		m_fTimer -= tDelta;
		if (m_fTimer < 0)
			UpdateElapsedTime();
	}
	
	override void OnMenuFocusGained()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.AutoInit();

		GetGame().GetInputManager().AddActionListener("ShowScoreboard", EActionTrigger.DOWN, OpenPlayerList);
		
		super.OnMenuFocusGained();
	}

	override void OnMenuFocusLost()
	{
		GetGame().GetInputManager().RemoveActionListener("ShowScoreboard", EActionTrigger.DOWN, OpenPlayerList);
		
		super.OnMenuFocusLost();
	}

	//! Find request handlers based on the layout configuration.
	protected void FindRequestHandlers()
	{
		m_FactionRequestUIHandler = m_MenuHandler.GetFactionRequestHandler();
		m_LoadoutRequestUIHandler = m_MenuHandler.GetLoadoutRequestHandler();
		m_GroupRequestUIHandler = m_MenuHandler.GetGroupRequestHandler();
	}

	//! Initialize event listeners.
	protected void HookEvents()
	{
		m_PlyFactionAffilComp.GetOnPlayerFactionRequestInvoker_O().Insert(OnPlayerFactionRequest);
		m_PlyFactionAffilComp.GetOnPlayerFactionResponseInvoker_O().Insert(OnPlayerFactionResponse);

		m_PlyLoadoutComp.GetOnPlayerLoadoutRequestInvoker_O().Insert(OnPlayerLoadoutRequest);
		m_PlyLoadoutComp.GetOnPlayerLoadoutResponseInvoker_O().Insert(OnPlayerLoadoutResponse);

		m_FactionRequestUIHandler.GetOnButtonFocused().Insert(ShowFactionPlayerList);
		m_FactionRequestUIHandler.GetOnFactionRequested().Insert(OnPlayerFactionResponse);

		m_LoadoutRequestUIHandler.GetOnButtonFocused().Insert(ShowLoadoutList);
		m_LoadoutRequestUIHandler.GetOnMouseLeft().Insert(OnMouseLeft);
		m_LoadoutRequestUIHandler.GetOnPlayerEntryFocused().Insert(OnPlayerEntryFocused);
		m_LoadoutRequestUIHandler.GetOnPlayerEntryFocusLost().Insert(OnPlayerEntryFocusLost);

		m_GroupRequestUIHandler.GetOnMouseLeft().Insert(OnMouseLeft);
		m_GroupRequestUIHandler.GetOnButtonFocused().Insert(ShowLoadoutList);

		m_GroupRequestUIHandler.GetOnPlayerGroupJoined().Insert(OnPlayerGroupJoined);
		m_GroupRequestUIHandler.GetOnLocalPlayerGroupJoined().Insert(OnLocalGroupJoined);
	}

	//! Update elapsed time.
	protected void UpdateElapsedTime()
	{
		m_fTimer = 1;
		string timeElapsed = SCR_FormatHelper.FormatTime(m_GameMode.GetElapsedTime());

		if (m_wScenarioTimeElapsed)
			m_wScenarioTimeElapsed.SetText(timeElapsed);

		UpdatePlayerCount();
	}

	//! Initialize empty map in the background.
	protected void InitMapPlain()
	{
		const ResourceName conf = "{A786DD4868598F15}Configs/Map/MapPlain.conf"; // TODO: check for good const usage
		m_MapEntity.OpenMap(m_MapEntity.SetupMapConfig(EMapEntityMode.PLAIN, conf, GetRootWidget()));
	}
	
	/*
		Determine which widget should be shown on the right side of the role selection screen
		when user hovers their mouse cursor off a button on the left side of the screen.
	*/
	protected void OnMouseLeft()
	{
		bool hasFaction = (m_PlyFactionAffilComp.GetAffiliatedFaction() != null);
		bool hasGroup = true;
		
		if (!hasFaction)
		{
			SCR_FactionButton btn = SCR_FactionButton.Cast(m_FactionRequestUIHandler.GetFirstValidButton());
			ShowFactionPlayerList(btn.GetFaction());
			return;
		}

		ShowLoadoutList();
	}

	//! Show player list for a faction.
	protected void ShowFactionPlayerList(Faction faction = null)
	{
		if (m_FactionPlayerList)
		{
			m_FactionPlayerList.SetFaction(faction);
			m_FactionPlayerList.ShowPlayerList(true);
		}	
		m_LoadoutRequestUIHandler.ShowLoadoutSelector(false);

	}

	//! Shows the loadout selector on the right side of role selection screen.
	protected void ShowLoadoutList()
	{
		if (m_FactionPlayerList)
			m_FactionPlayerList.ShowPlayerList(false);
		
		SCR_PlayerControllerGroupComponent plyGroupComp = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!plyGroupComp)
			return;

		// can show after the player is added to the group
		if (plyGroupComp.GetGroupID() < 0)
			return;

		m_LoadoutRequestUIHandler.ShowLoadoutSelector(true);
	}
	
	//! Shows the Player List selector on the right side of role selection screen
	protected void ShowGroupPlayerList()
	{
		if (m_FactionPlayerList)
			m_FactionPlayerList.ShowPlayerList(false);
		
		m_LoadoutRequestUIHandler.ShowLoadoutSelector(false);
	}

	//! Updates the session's player count.
	protected void UpdatePlayerCount()
	{
		if (m_iMaxPlayerCount > 0)
			m_wPlayerCount.SetTextFormat("%1/%2", GetGame().GetPlayerManager().GetPlayerCount().ToString(), m_iMaxPlayerCount);
		else
			m_wPlayerCount.SetText(GetGame().GetPlayerManager().GetPlayerCount().ToString());
	}	

	//! Check if role selection screen can be closed.
	protected bool CanContinue()
	{
		if (!m_PlyFactionAffilComp || !m_PlyLoadoutComp)
			return false;
		
		bool canContinue = m_PlyFactionAffilComp.GetAffiliatedFaction() && m_PlyLoadoutComp.GetLoadout();
		if (m_GroupRequestUIHandler && m_GroupRequestUIHandler.IsEnabled())
		{
			canContinue = canContinue && (m_GroupRequestUIHandler.GetPlayerGroup() != null);
		}

		return canContinue;
	}
	
	//! Check if group menu can be opened.
	protected bool CanOpenGroupMenu()
	{
		return m_PlyFactionAffilComp && m_PlyFactionAffilComp.GetAffiliatedFaction();
	}

	protected void OpenGroupMenu()
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager || !groupManager.IsGroupMenuAllowed())
			return;
		
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.GroupMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerFactionRequest(SCR_PlayerFactionAffiliationComponent component, int factionIndex)
	{
	}

	//! Callback when player receives a response to afaction request.
	protected void OnPlayerFactionResponse(SCR_PlayerFactionAffiliationComponent component, int factionIndex, bool response)
	{
		if (response)
		{
			m_FactionRequestUIHandler.OnPlayerFactionAssigned(component.GetAffiliatedFaction());
			if (factionIndex == -1)
			{
				m_GroupRequestUIHandler.SetExpanded(false);
				m_FactionRequestUIHandler.SetExpanded(false);
			}
			else
			{
				// If groups are enabled, update them first, otherwise just show players assigned to the faction
				if (m_GroupRequestUIHandler && m_GroupRequestUIHandler.IsEnabled())
				{
					Widget list = m_FactionRequestUIHandler.GetFactionButton(factionIndex).GetList();
					m_GroupRequestUIHandler.SetListWidget(list);
					m_GroupRequestUIHandler.ShowAvailableGroups(component.GetAffiliatedFaction());
					m_LoadoutRequestUIHandler.ShowAvailableLoadouts(component.GetAffiliatedFaction());
				}
				else
				{
					Widget list = m_FactionRequestUIHandler.GetFactionButton(factionIndex).GetGridList();
					m_LoadoutRequestUIHandler.SetListWidget(list);
					m_LoadoutRequestUIHandler.ShowAvailableLoadouts(component.GetAffiliatedFaction());

					array<int> players = {};
					GetGame().GetPlayerManager().GetPlayers(players);
					foreach (int pid : players)
					{
						if (SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(pid)) != component.GetAffiliatedFaction())
							players.RemoveItem(pid);
					}

					m_LoadoutRequestUIHandler.ShowPlayerLoadouts(players);
				}	
			}
	
			m_GroupOpenButton.SetEnabled(CanOpenGroupMenu());
			m_ContinueButton.SetEnabled(CanContinue());
			if (m_wPersistentFaction)
				m_wPersistentFaction.SetVisible(false);
		}

		m_FactionRequestUIHandler.Unlock();
	}

	//! Callback when a player joins a group.
	protected void OnPlayerGroupJoined(SCR_AIGroup group, int pid = -1)
	{
		if (!group || group.GetGroupID() != m_GroupRequestUIHandler.GetShownGroupId())
			return;

		SCR_GroupButton groupBtn = m_GroupRequestUIHandler.GetGroupButton(group);
		if (!groupBtn)
			return;

		Widget list = m_GroupRequestUIHandler.GetGroupButton(group).GetList();
		array<int> players = group.GetPlayerIDs();

		m_LoadoutRequestUIHandler.SetListWidget(list);
		m_LoadoutRequestUIHandler.ShowPlayerLoadouts(players, group.GetMaxMembers());
	}

	//! Callback when local player joins a group.
	protected void OnLocalGroupJoined(SCR_AIGroup group)
	{
		if (!group)
		{
			m_LoadoutRequestUIHandler.SetListVisible(false);
			return;
		}
		
		OnPlayerGroupJoined(group);

		m_ContinueButton.SetEnabled(CanContinue());

		m_LoadoutRequestUIHandler.ShowAvailableLoadouts(group.GetFaction());

		ShowLoadoutList();
	}
	
	protected void OnPlayerLoadoutRequest(SCR_PlayerLoadoutComponent component, int loadoutIndex)
	{
	}

	//! Callback when local player receives a response to a loadout request.
	protected void OnPlayerLoadoutResponse(SCR_PlayerLoadoutComponent component, int loadoutIndex, bool response)
	{
		if (response)
		{
			m_LoadoutRequestUIHandler.OnPlayerLoadoutAssigned(component);
			m_LoadoutRequestUIHandler.SetSelected(component);
			m_LoadoutRequestUIHandler.RefreshLoadoutPreview();			
		}

		m_ContinueButton.SetEnabled(CanContinue());
		m_LoadoutRequestUIHandler.Unlock();
	}

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

		SCR_ChatPanelManager.GetInstance().ToggleChatPanel(m_ChatPanel);
	}
	
	protected void OnPauseMenu()
	{
		UpdateViewProfileButton(true);
		
		MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PauseMenu, 0, true, false);

		PauseMenuUI pauseMenu = PauseMenuUI.Cast(menu);
		if (pauseMenu)
		{
			pauseMenu.FadeBackground(true, true);
			pauseMenu.DisableSettings();
		}
	}

	override void OnMenuClose()
	{
		super.OnMenuClose();

		if (m_MapEntity && m_MapEntity.IsOpen())
			m_MapEntity.CloseMap();
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		gameMode.PauseGame(false, SCR_EPauseReason.MENU);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerEntryFocused(int id)
	{
		m_iLastFocusedEntryPlayerId = id;
		UpdateViewProfileButton();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerEntryFocusLost(Widget w)
	{
		UpdateViewProfileButton(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnViewProfile()
	{
		GetGame().GetPlayerManager().ShowUserProfile(m_iLastFocusedEntryPlayerId);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateViewProfileButton(bool forceHidden = false)
	{
		if (!m_ViewProfileButton)
			return;

		m_ViewProfileButton.SetVisible(!forceHidden && GetGame().GetPlayerManager().IsUserProfileAvailable(m_iLastFocusedEntryPlayerId), false);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Open role selection menu.
	\return the menu instance
	*/
	static SCR_RoleSelectionMenu OpenRoleSelectionMenu()
	{
		if (!GetRoleSelectionMenu())
			GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.RoleSelectionDialog);
		
		return GetRoleSelectionMenu();
	}
	
	//! Close role selection menu.
	static void CloseRoleSelectionMenu()
	{
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.RoleSelectionDialog);
	}

	/*!
	Get opened role selection menu.
	\return the menu instance
	*/
	static SCR_RoleSelectionMenu GetRoleSelectionMenu()
	{
		return SCR_RoleSelectionMenu.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.RoleSelectionDialog));
	}	
};
