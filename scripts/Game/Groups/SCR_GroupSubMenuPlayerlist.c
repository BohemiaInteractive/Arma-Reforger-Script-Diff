class SCR_GroupSubMenuPlayerlist : SCR_GroupSubMenuBase
{
	protected SCR_InputButtonComponent m_VolunteerForGroupLeaderButton;
	protected SCR_InputButtonComponent m_ViewProfileButton;
	
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
		
		CreateVolunteerForGroupLeaderButton();
		CreateViewProfileButton();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();
	
		UpdateViewProfileButton(true);
		
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(UpdateGroupSettingsButtons);
		SCR_GroupTileButton.GetOnPlayerTileFocus().Insert(OnPlayerTileFocus);
		SCR_GroupTileButton.GetOnPlayerTileFocusLost().Insert(OnPlayerTileFocusLost);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();
	
		SCR_AIGroup.GetOnPlayerLeaderChanged().Remove(UpdateGroupSettingsButtons);
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
	protected void CreateVolunteerForGroupLeaderButton()
	{
		m_VolunteerForGroupLeaderButton = CreateNavigationButton("MenuVolunteerLeaderGroup", "#AR-Player_Groups_Leader_Volunteer", true, false);
		if (!m_VolunteerForGroupLeaderButton)
			return;

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
	protected void UpdateViewProfileButton(bool forceHidden = false)
	{
		if (!m_ViewProfileButton)
			return;

		SetNavigationButtonVisible(m_ViewProfileButton, !forceHidden && GetGame().GetPlayerManager().IsUserProfileAvailable(m_iLastSelectedPlayerId));
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

		// If volunteering for Group Leader is not enabled, disable and hide the button
		if (!m_GroupManager.IsGroupLeaderVolunteeringAllowed())
		{
			m_VolunteerForGroupLeaderButton.SetVisible(false, false);
			m_VolunteerForGroupLeaderButton.SetEnabled(false);
			return;
		}

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
}
