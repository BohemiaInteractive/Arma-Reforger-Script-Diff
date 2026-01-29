class SCR_GroupSubMenuPlayerlist : SCR_GroupSubMenuBase
{
	protected SCR_InputButtonComponent m_VolunteerForGroupLeaderButton;
	protected SCR_InputButtonComponent m_ViewProfileButton;
	protected SCR_PlayerControllerGroupComponent m_PlayerGroupController;
	protected Widget m_wTaskIcon;
	
	protected int m_iLastSelectedPlayerId;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		GetGame().GetInputManager().ActivateContext("GroupMenuContext");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		m_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_PlayerGroupController)
			return;
		m_wTaskIcon = menuRoot.FindAnyWidget("m_wTaskIcon");

		CreateAddGroupButton();
		CreateJoinGroupButton();
		CreateRemoveGroupButton();
		CreateAcceptInviteButton();
		CreateGroupSettingsButton();
		CreateVolunteerForGroupLeaderButton();
		CreateViewProfileButton();
		SetupNameChangeButton();
		SetupPrivateChecker();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();
	
		UpdateViewProfileButton(true);
		
		//todo:mku this is a temporary solution because of how playerlist is implemented right now
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
		
		SCR_AIGroup.GetOnPlayerAdded().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(UpdateGroupSettingsButtons);
		SCR_AIGroup.GetOnPrivateGroupChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomNameChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnFlagSelected().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomDescriptionChanged().Insert(UpdateGroupsMenu);
		SCR_GroupTileButton.GetOnGroupTileClicked().Insert(UpdateGroupsMenu);
		SCR_GroupTileButton.GetOnPlayerTileFocus().Insert(OnPlayerTileFocus);
		SCR_GroupTileButton.GetOnPlayerTileFocusLost().Insert(OnPlayerTileFocusLost);
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
	override void OnTabHide()
	{
		super.OnTabHide();
	
		SCR_GroupTileButton.GetOnPlayerTileFocus().Remove(OnPlayerTileFocus);
		SCR_GroupTileButton.GetOnPlayerTileFocusLost().Remove(OnPlayerTileFocusLost);
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
		SCR_AIGroup.GetOnPlayerLeaderChanged().Remove(UpdateGroupSettingsButtons);
		SCR_AIGroup.GetOnCustomNameChanged().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnFlagSelected().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomDescriptionChanged().Remove(UpdateGroupsMenu);
		m_PlayerGroupController.GetOnInviteReceived().Remove(SetAcceptButtonStatus);
		
		//todo:mku this is a temporary solution because of how playerlist is implemented right now
		OverlayWidget header = OverlayWidget.Cast(m_wMenuRoot.FindAnyWidget("SortHeader"));
		if (header)
			header.SetVisible(true);
		ScrollLayoutWidget scrollWidget = ScrollLayoutWidget.Cast(m_wMenuRoot.FindAnyWidget("ScrollLayout0"));
		if (scrollWidget)
			scrollWidget.SetVisible(true);
		HorizontalLayoutWidget footerLeft = HorizontalLayoutWidget.Cast(m_wMenuRoot.FindAnyWidget("FooterLeft"));
		if (footerLeft)
			footerLeft.SetVisible(true);
		
		SCR_GroupTileButton.GetOnPlayerTileFocus().Remove(OnPlayerTileFocus);
		SCR_GroupTileButton.GetOnPlayerTileFocusLost().Remove(OnPlayerTileFocusLost);
	}

	//------------------------------------------------------------------------------------------------
	override protected void UpdateGroups(SCR_PlayerControllerGroupComponent playerGroupController)
	{
		super.UpdateGroups(playerGroupController);

		if (playerGroupController == m_PlayerGroupController)
			UpdateGroupSettingsButtons();
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
		else
		{
			if (!m_PlayerGroupController)
				return;

			//we reset the selected group so the menu goes to players actual group, in this case newly created one
			m_PlayerGroupController.SetSelectedGroupID(-1);
			m_PlayerGroupController.RequestCreateGroup();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinSelectedGroup()
	{
		if (!m_PlayerGroupController)
			return;	
		m_PlayerGroupController.RequestJoinGroup(m_PlayerGroupController.GetSelectedGroupID());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveSelectedGroup()
	{
		if (!m_PlayerGroupController)
			return;

		m_PlayerGroupController.RequestRemoveGroup(m_PlayerGroupController.GetSelectedGroupID());
	}

	//------------------------------------------------------------------------------------------------
	void AcceptInvite()
	{
		m_PlayerGroupController.AcceptInvite();
		SetAcceptButtonStatus();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAcceptButtonStatus()
	{
		if (!m_AcceptInviteButton)
			return;
		
		if (m_PlayerGroupController.GetGroupInviteID() == -1)
		{
			m_AcceptInviteButton.SetEnabled(false);
		}
		else
		{
			SCR_AIGroup group = m_GroupManager.FindGroup(m_PlayerGroupController.GetGroupInviteID());
			
			if (!group)
			{
				m_AcceptInviteButton.SetEnabled(false);
				m_PlayerGroupController.SetGroupInviteID(-1);
				return;	
			}		
			m_AcceptInviteButton.SetEnabled(true);	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateAddGroupButton()
	{
		m_AddGroupButton = CreateNavigationButton("MenuAddGroup", CREATE_GROUP, true);
		if (!m_AddGroupButton)
			return;
		m_AddGroupButton.GetRootWidget().SetZOrder(0);
		m_AddGroupButton.m_OnActivated.Insert(CreateNewGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateJoinGroupButton()
	{
		m_JoinGroupButton = CreateNavigationButton("MenuJoinGroup", JOIN_GROUP, true);
		if (!m_JoinGroupButton)
			return;
		m_JoinGroupButton.GetRootWidget().SetZOrder(0);
		m_JoinGroupButton.m_OnActivated.Insert(JoinSelectedGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateRemoveGroupButton()
	{
		m_RemoveGroupButton = CreateNavigationButton("MenuRemoveGroup", REMOVE_GROUP, true);
		if (!m_RemoveGroupButton)
			return;

		m_RemoveGroupButton.GetRootWidget().SetZOrder(0);
		m_RemoveGroupButton.m_OnActivated.Insert(RemoveSelectedGroup);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateAcceptInviteButton()
	{
		m_AcceptInviteButton = CreateNavigationButton("GroupAcceptInvite", ACCEPT_INVITE, true);
		if (!m_AcceptInviteButton)
			return;
		m_AcceptInviteButton.GetRootWidget().SetZOrder(0);
		m_AcceptInviteButton.m_OnActivated.Insert(AcceptInvite);
		m_PlayerGroupController.GetOnInviteReceived().Insert(SetAcceptButtonStatus);
		SetAcceptButtonStatus();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateGroupSettingsButton()
	{
		m_GroupSettingsButton = CreateNavigationButton("MenuSettingsGroup", "#AR-Player_Groups_Settings", true, false);
		if (!m_GroupSettingsButton)
			return;
		m_GroupSettingsButton.GetRootWidget().SetZOrder(0);
		m_GroupSettingsButton.m_OnActivated.Insert(OpenGroupSettingsDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateVolunteerForGroupLeaderButton()
	{
		m_VolunteerForGroupLeaderButton = CreateNavigationButton("MenuVolunteerLeaderGroup", "#AR-Player_Groups_Leader_Volunteer", true, false);
		if (!m_VolunteerForGroupLeaderButton)
			return;
		m_VolunteerForGroupLeaderButton.GetRootWidget().SetZOrder(0);
		m_VolunteerForGroupLeaderButton.m_OnActivated.Insert(OpenVolunteerForGroupLeaderDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateViewProfileButton()
	{
		m_ViewProfileButton = CreateNavigationButton("MenuViewProfile", "", true);
		if (!m_ViewProfileButton)
			return;
		
		// Dynamically add the component to update the button label dpending on platform. TODO: allow sub menus to create different layouts of buttons
		SCR_ViewProfileButtonComponent handler = new SCR_ViewProfileButtonComponent();
		if (!handler)
			return;

		m_ViewProfileButton.GetRootWidget().AddHandler(handler);
		handler.Init();
		
		UpdateViewProfileButton(true);
		
		m_ViewProfileButton.GetRootWidget().SetZOrder(0);
		m_ViewProfileButton.m_OnActivated.Insert(OnViewProfile);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateViewProfileButton(bool forceHidden = false)
	{
		if (!m_ViewProfileButton)
			return;

		SetNavigationButtonVisibile(m_ViewProfileButton, !forceHidden && GetGame().GetPlayerManager().IsUserProfileAvailable(m_iLastSelectedPlayerId));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerTileFocus(int id)
	{
		m_iLastSelectedPlayerId = id;
		UpdateViewProfileButton();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerTileFocusLost(int id)
	{
		UpdateViewProfileButton(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnViewProfile()
	{
		GetGame().GetPlayerManager().ShowUserProfile(m_iLastSelectedPlayerId);
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
		playerComponent.RequestPrivateGroupChange(playerComponent.GetPlayerID() , !playerGroup.IsPrivate());
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
	//! Displays or hides Volunteer button and Group Settings button based on the player role in group
	protected void UpdateGroupSettingsButtons()
	{
		if (!m_PlayerGroupController || !m_GroupManager)
			return;

		int groupId = m_PlayerGroupController.GetGroupID();

		// Selected group is not player's group, hide both buttons
		if (groupId != m_PlayerGroupController.GetSelectedGroupID())
		{
			m_GroupSettingsButton.SetVisible(false, false);
			m_VolunteerForGroupLeaderButton.SetVisible(false, false);
			return;
		}

		SCR_AIGroup group = m_GroupManager.FindGroup(groupId);
		if (!group)
			return;

		bool isGroupLeader = group.GetLeaderID() == m_PlayerGroupController.GetPlayerID();

		// Group leader can see Group Settings button, other group members can see Volunteer button
		m_GroupSettingsButton.SetVisible(isGroupLeader, false);
		m_VolunteerForGroupLeaderButton.SetVisible(!isGroupLeader, false);

		if (isGroupLeader)
			return;

		SCR_VotingManagerComponent manager = SCR_VotingManagerComponent.GetInstance();
		if (!manager)
			return;

		// Volunteer button is enabled only when player does not have voting cooldown
		if (manager.GetCurrentVoteCooldownForLocalPlayer(EVotingType.GROUP_LEADER) > 0)
			m_VolunteerForGroupLeaderButton.SetEnabled(false);
		else
			m_VolunteerForGroupLeaderButton.SetEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPrivateCheckerClicked()
	{
		SCR_AIGroup group = m_GroupManager.FindGroup(m_PlayerGroupController.GetGroupID());
		if (!group)
			return;
		
		m_PlayerGroupController.RequestPrivateGroupChange(m_PlayerGroupController.GetPlayerID() , !group.IsPrivate());
	}
};
