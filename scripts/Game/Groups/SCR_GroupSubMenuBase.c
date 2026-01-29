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
	protected const string REMOVE_GROUP = "#AR-DeployMenu_RemoveGroup";
	
	protected SCR_GroupsManagerComponent m_GroupManager;
	
	protected Widget m_wGridWidget;
	protected SCR_InputButtonComponent m_AddGroupButton;
	protected SCR_InputButtonComponent m_JoinGroupButton;
	protected SCR_InputButtonComponent m_AcceptInviteButton;
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
		SetNavigationButtonVisibile(m_GroupSettingsButton, playerGroupController.IsPlayerLeaderOwnGroup() && groupManager.CanPlayersChangeAttributes());
		
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
				if (playerGroupController.GetGroupID() == -1 && i == 0 && playerGroupController.GetSelectedGroupID() < 0)
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
				if (selectedGroupID < 0 && playableGroups[i].GetGroupID() == playerGroupController.GetGroupID() )
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
				if (selectedGroupID == playableGroups[i].GetGroupID())
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
			}
		}
	}
}
