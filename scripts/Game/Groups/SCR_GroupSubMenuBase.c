class SCR_GroupSubMenuBase : SCR_SubMenuBase
{
	[Attribute("6", params: "1 100 1")]
	protected int m_iMaxColumnNumber;
	
	[Attribute("{17CCACBDA8CF5E32}UI/layouts/Menus/GroupSlection/GroupButton.layout")]
	protected ResourceName m_ButtonLayout;
	
	[Attribute("#AR-PauseMenu_Continue", UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sContinueButtonText;
	
	protected const string CREATE_GROUP = "#AR_DeployMenu_AddNewGroup";
	protected const string JOIN_GROUP = "#AR-DeployMenu_JoinGroup";
	protected const string ACCEPT_INVITE = "#AR-DeployMenu_AcceptInvite";
	protected const string DECLINE_INVITE = "#AR-Group_RefuseJoinPrivateGroup";
	protected const string REMOVE_GROUP = "#AR-DeployMenu_RemoveGroup";
	protected const string SETTINGS_GROUP = "#AR-Player_Groups_Settings";
	
	protected SCR_GroupsManagerComponent m_GroupManager;
	protected SCR_PlayerControllerGroupComponent m_PlayerGroupController;
	
	protected Widget m_wGridWidget;
	protected SCR_InputButtonComponent m_AddGroupButton;
	protected SCR_InputButtonComponent m_JoinGroupButton;
	protected SCR_InputButtonComponent m_AcceptInviteButton;
	protected SCR_InputButtonComponent m_DeclineInviteButton;
	protected SCR_InputButtonComponent m_GroupSettingsButton;
	protected SCR_InputButtonComponent m_RemoveGroupButton;

	//------------------------------------------------------------------------------------------------
 	protected void InitGroups(SCR_PlayerControllerGroupComponent playerGroupController)
	{
		if (!m_wGridWidget)
			return;
		
		GetGame().GetCallqueue().Call(UpdateGroups, playerGroupController);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateGroups(SCR_PlayerControllerGroupComponent playerGroupController)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		SCR_Faction playerFaction = SCR_Faction.Cast(factionManager.GetLocalPlayerFaction());
		if (!playerFaction)
			return;
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		if (!playerGroupController)
			return;
		
		//no need to check playerFaction for null, because groups are not enabled for players without faction
		m_AddGroupButton.SetEnabled(groupManager.CanCreateNewGroup(playerFaction));
		Widget children = m_wGridWidget.GetChildren();
		while (children)
		{
			m_wGridWidget.RemoveChild(children);
			children = m_wGridWidget.GetChildren();
		}
		
		// Group Settings Button
		SetNavigationButtonVisible(m_GroupSettingsButton, playerGroupController.IsPlayerLeaderOwnGroup() && groupManager.CanPlayersChangeAttributes());
		
		array<SCR_AIGroup> playableGroups = groupManager.GetSortedPlayableGroupsByFaction(playerFaction);
		if (!playableGroups)
			return;
		
		int selectedGroupID = playerGroupController.GetSelectedGroupID();
		
		if (playableGroups.IsIndexValid(selectedGroupID) && m_wMenuRoot)
		{
			ImageWidget privateIcon = ImageWidget.Cast(m_wMenuRoot.FindAnyWidget("PrivateIconDetail"));
			if (privateIcon)
				privateIcon.SetVisible(playableGroups[selectedGroupID].IsPrivate());
		}
		
		int groupCount = playableGroups.Count();
		
		for (int i = 0; i < groupCount; i++)
		{
			Widget groupTile = GetGame().GetWorkspace().CreateWidgets(m_ButtonLayout, m_wGridWidget);	
			if (!groupTile)
				continue;
					
			ButtonWidget buttonWidget = ButtonWidget.Cast(groupTile.FindAnyWidget("Button"));
			if (!buttonWidget)
				continue;
			
			SCR_GroupTileButton buttonComponent = SCR_GroupTileButton.Cast(buttonWidget.FindHandler(SCR_GroupTileButton));
			if (buttonComponent)
			{
				buttonComponent.SetGroupID(playableGroups[i].GetGroupID());
				buttonComponent.SetGroupFaction(playerFaction);
				buttonComponent.SetJoinGroupButton(m_JoinGroupButton);
				buttonComponent.SetRemoveGroupButton(m_RemoveGroupButton);
				buttonComponent.InitiateGroupTile();

				if (MustRefreshList(selectedGroupID, playerGroupController.GetGroupID(), playableGroups[i].GetGroupID(), i))
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool MustRefreshList(int selectedGroupId, int playerGroupId, int otherGroupId, int groupIterator)
	{
		bool noSelection = selectedGroupId < 0;
		bool isPlayerGroup = otherGroupId == playerGroupId;

		return (playerGroupId == -1 && groupIterator == 0 && noSelection)
			|| (noSelection && isPlayerGroup)
			|| (selectedGroupId == otherGroupId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		m_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_PlayerGroupController)
			return;

		CreateAcceptInviteButton();
		CreateDeclineInviteButton();
		CreateAddGroupButton();
		CreateJoinGroupButton();
		CreateRemoveGroupButton();
		CreateGroupSettingsButton();
		SetupNameChangeButton();
		SetupPrivateChecker();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		//todo: This is a temporary solution because of how playerlist is implemented right now
		OverlayWidget header = OverlayWidget.Cast(m_wMenuRoot.FindAnyWidget("SortHeader"));
		if (header)
			header.SetVisible(false);

		ScrollLayoutWidget scrollWidget = ScrollLayoutWidget.Cast(m_wMenuRoot.FindAnyWidget("ScrollLayout0"));
		if (scrollWidget)
			scrollWidget.SetVisible(false);

		HorizontalLayoutWidget footerLeft = HorizontalLayoutWidget.Cast(m_wMenuRoot.FindAnyWidget("FooterLeft"));
		if (footerLeft)
			footerLeft.SetVisible(false);

		UpdateGroupsMenu();

		if (m_GroupManager)
		{
			m_GroupManager.GetOnPlayableGroupRemoved().Insert(UpdateGroupsMenu);
			m_GroupManager.GetOnPlayableGroupCreated().Insert(UpdateGroupsMenu);
		}

		if (m_PlayerGroupController)
		{
			m_PlayerGroupController.GetOnInviteReceived().Insert(SetAcceptButtonStatus);
			m_PlayerGroupController.GetOnSetSelectedGroupID().Insert(SetSelectedGroupButtonStatus);
		}

		SCR_AIGroup.GetOnPlayerAdded().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPrivateGroupChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomNameChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnFlagSelected().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomDescriptionChanged().Insert(UpdateGroupsMenu);
		SCR_GroupTileButton.GetOnGroupTileClicked().Insert(UpdateGroupsMenu);
		SetAcceptButtonStatus();
	}

	//------------------------------------------------------------------------------------------------
	void UpdateGroupsMenu()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController || !m_wMenuRoot)
			return;

		SetAcceptButtonStatus();

		m_wGridWidget = m_wMenuRoot.FindAnyWidget("GroupList");
		InitGroups(m_PlayerGroupController);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabRemove()
	{
		super.OnTabRemove();

		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (m_GroupManager)
		{
			m_GroupManager.GetOnPlayableGroupRemoved().Remove(UpdateGroupsMenu);
			m_GroupManager.GetOnPlayableGroupCreated().Remove(UpdateGroupsMenu);
		}

		SCR_AIGroup.GetOnPlayerAdded().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerRemoved().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPrivateGroupChanged().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomNameChanged().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnFlagSelected().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomDescriptionChanged().Remove(UpdateGroupsMenu);
		SCR_GroupTileButton.GetOnGroupTileClicked().Remove(UpdateGroupsMenu);

		if (m_PlayerGroupController)
		{
			m_PlayerGroupController.GetOnInviteReceived().Remove(SetAcceptButtonStatus);
			m_PlayerGroupController.GetOnSetSelectedGroupID().Remove(SetSelectedGroupButtonStatus);
		}

		//todo: This is a temporary solution because of how playerlist is implemented right now
		OverlayWidget header = OverlayWidget.Cast(m_wMenuRoot.FindAnyWidget("SortHeader"));
		if (header)
			header.SetVisible(true);

		ScrollLayoutWidget scrollWidget = ScrollLayoutWidget.Cast(m_wMenuRoot.FindAnyWidget("ScrollLayout0"));
		if (scrollWidget)
			scrollWidget.SetVisible(true);

		HorizontalLayoutWidget footerLeft = HorizontalLayoutWidget.Cast(m_wMenuRoot.FindAnyWidget("FooterLeft"));
		if (footerLeft)
			footerLeft.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateNewGroup()
	{
		SCR_Faction scrFaction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		if (!scrFaction)
			return;

		// checks if the group role config is set, if it's set it will open the CreateGroupSettingsDialog
		// otherwise it will create a new group without a role
		if (scrFaction.IsGroupRolesConfigured())
		{
			GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.CreateGroupSettingsDialog);
		}
		else if (m_PlayerGroupController)
		{
			//we reset the selected group so the menu goes to players actual group, in this case newly created one
			m_PlayerGroupController.SetSelectedGroupID(-1);
			m_PlayerGroupController.RequestCreateGroup();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinSelectedGroup()
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup group = groupManager.FindGroup(m_PlayerGroupController.GetSelectedGroupID());

		if (!group)
			return;

		if (group.IsPrivate())
		{
			m_PlayerGroupController.PlayerRequestToJoinPrivateGroup(m_PlayerGroupController.GetPlayerID(), Replication.FindItemId(group));
			SCR_NotificationsComponent.SendToPlayer(m_PlayerGroupController.GetPlayerID(), ENotification.GROUPS_REQUEST_SENT, group.GetGroupID());
		}
		else
		{
			m_PlayerGroupController.RequestJoinGroup(m_PlayerGroupController.GetSelectedGroupID());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveSelectedGroup()
	{
		m_PlayerGroupController.RequestRemoveGroup(m_PlayerGroupController.GetSelectedGroupID());
	}

	//------------------------------------------------------------------------------------------------
	void AcceptInvite()
	{
		m_PlayerGroupController.AcceptInvite();
		int SelectedGroupID = m_PlayerGroupController.GetSelectedGroupID();
		SetSelectedGroupButtonStatus(SelectedGroupID);
	}

	//------------------------------------------------------------------------------------------------
	void DeclineInvite()
	{
		m_PlayerGroupController.DeclineInvite();
		int SelectedGroupID = m_PlayerGroupController.GetSelectedGroupID();
		SetSelectedGroupButtonStatus(SelectedGroupID);
	}

	//------------------------------------------------------------------------------------------------
	void SetAcceptButtonStatus()
	{
		if (!m_AcceptInviteButton || !m_DeclineInviteButton)
			return;
		
		int selectedGroupId = m_PlayerGroupController.GetSelectedGroupID();

		if (!m_PlayerGroupController.HasInviteFromGroup(selectedGroupId))
		{
			m_AcceptInviteButton.SetVisible(false, false);
			m_DeclineInviteButton.SetVisible(false, false);
		}
		else
		{
			SCR_AIGroup group = m_GroupManager.FindGroup(selectedGroupId);

			if (!group)
			{
				m_AcceptInviteButton.SetVisible(false, false);
				m_DeclineInviteButton.SetVisible(false, false);
				m_PlayerGroupController.RemoveGroupInviteId(selectedGroupId);
				return;
			}

			m_AcceptInviteButton.SetVisible(true, false);
			m_DeclineInviteButton.SetVisible(true, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetSelectedGroupButtonStatus(int selectedGroupId)
	{
		bool canShow = selectedGroupId > -1 && !m_PlayerGroupController.HasInviteFromGroup(selectedGroupId);

		m_JoinGroupButton.SetVisible(canShow, false);
		m_AddGroupButton.SetVisible(canShow, false);

		SetAcceptButtonStatus();
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateAddGroupButton()
	{
		m_AddGroupButton = CreateNavigationButton("MenuAddGroup", CREATE_GROUP, true);
		if (!m_AddGroupButton)
			return;

		m_AddGroupButton.m_OnActivated.Insert(CreateNewGroup);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateJoinGroupButton()
	{
		m_JoinGroupButton = CreateNavigationButton("MenuJoinGroup", JOIN_GROUP, true);
		if (!m_JoinGroupButton)
			return;

		m_JoinGroupButton.m_OnActivated.Clear();
		m_JoinGroupButton.m_OnActivated.Insert(JoinSelectedGroup);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateRemoveGroupButton()
	{
		m_RemoveGroupButton = CreateNavigationButton("MenuRemoveGroup", REMOVE_GROUP, true);
		if (!m_RemoveGroupButton)
			return;

		m_RemoveGroupButton.m_OnActivated.Insert(RemoveSelectedGroup);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateAcceptInviteButton()
	{
		m_AcceptInviteButton = CreateNavigationButton("GroupAcceptInvite", ACCEPT_INVITE, true);
		if (!m_AcceptInviteButton)
			return;

		m_AcceptInviteButton.m_OnActivated.Insert(AcceptInvite);
		SetAcceptButtonStatus();
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateDeclineInviteButton()
	{
		m_DeclineInviteButton = CreateNavigationButton("GroupDeclineInvite", DECLINE_INVITE, true);
		if (!m_DeclineInviteButton)
			return;

		m_DeclineInviteButton.m_OnActivated.Insert(DeclineInvite);
		SetAcceptButtonStatus();
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateGroupSettingsButton()
	{
		m_GroupSettingsButton = CreateNavigationButton("MenuSettingsGroup", SETTINGS_GROUP, true);
		if (!m_GroupSettingsButton)
			return;

		m_GroupSettingsButton.m_OnActivated.Insert(OpenGroupSettingsDialog);
	}

	//------------------------------------------------------------------------------------------------
	protected void ChangeGroupPublicState()
	{
		SCR_PlayerControllerGroupComponent playerComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerComponent)
			return;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		SCR_AIGroup playerGroup = groupsManager.FindGroup(playerComponent.GetGroupID());
		playerComponent.RequestPrivateGroupChange(playerComponent.GetPlayerID(), !playerGroup.IsPrivate());
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenVolunteerForGroupLeaderDialog()
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.VolunteerForGroupLeaderDialog);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenGroupSettingsDialog()
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.GroupSettingsDialog);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupNameChangeButton()
	{
		ButtonWidget nameChangeButton = ButtonWidget.Cast(GetRootWidget().FindAnyWidget("ChangeNameButton"));
		if (!nameChangeButton)
			return;

		SCR_ButtonImageComponent buttonComp = SCR_ButtonImageComponent.Cast(nameChangeButton.FindHandler(SCR_ButtonImageComponent));
		if (!buttonComp)
			return;

		buttonComp.m_OnClicked.Insert(OpenGroupSettingsDialog);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupPrivateChecker()
	{
		ButtonWidget privateChecker = ButtonWidget.Cast(GetRootWidget().FindAnyWidget("PrivateChecker"));
		if (!privateChecker)
			return;

		SCR_ButtonCheckerComponent buttonComp = SCR_ButtonCheckerComponent.Cast(privateChecker.FindHandler(SCR_ButtonCheckerComponent));
		if (!buttonComp)
			return;

		buttonComp.m_OnClicked.Insert(OnPrivateCheckerClicked);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPrivateCheckerClicked()
	{
		SCR_AIGroup group = m_GroupManager.FindGroup(m_PlayerGroupController.GetGroupID());
		if (!group)
			return;

		m_PlayerGroupController.RequestPrivateGroupChange(m_PlayerGroupController.GetPlayerID(), !group.IsPrivate());
	}
}
