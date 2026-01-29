[EntityEditorProps(category: "GameScripted/Groups", description: "This component should be attached to player controller and is used by groups to send requests to server.")]
class SCR_PlayerControllerGroupComponentClass : ScriptComponentClass
{
}

class SCR_PlayerControllerGroupComponent : ScriptComponent
{
	protected int m_iGroupID = -1;
	// Map with playerID and list of groups the player was invited to
	protected ref map<int, ref array<int>> m_mPlayerInvitesToGroups;
	protected ref ScriptInvoker<int, int> m_OnInviteReceived;
	protected ref ScriptInvoker<int> m_OnInviteAccepted;
	protected ref ScriptInvoker<int> m_OnInviteCancelled;
	protected ref ScriptInvoker<int> m_OnGroupChanged;
	protected ref ScriptInvokerInt m_OnSetSelectedGroupID;

	protected int m_iUISelectedGroupID = -1;
	protected int m_iGroupInviteID = -1;
	protected int m_iGroupInviteFromPlayerID = -1;
	protected int m_iPreviousGroupID = -1;
	protected string m_sGroupInviteFromPlayerName; //Player name is saved to get the name of the one who invited even if that player left the server	
	
	protected static const ref Color DEFAULT_COLOR = new Color(0, 0, 0, 0.4);
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return
	static SCR_PlayerControllerGroupComponent GetPlayerControllerComponent(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController)
			return null;
		
		return SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_PlayerControllerGroupComponent GetLocalPlayerControllerGroupComponent()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return null;
		
		return SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetGroupID()
	{
		return m_iGroupID;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void RequestCreateGroup()
	{
		Rpc(RPC_AskCreateGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Request server to create new group with specific groupRole
	//! \param[in] groupRole
	//! \param[in] isPrivate
	//! \param[in] name
	//! \param[in] description
	//! \param[in] joinGroup
	//! \param[in] deleteIfNoPlayer
	void RequestCreateGroupWithData(SCR_EGroupRole groupRole, bool isPrivate, string name, string description, bool joinGroup = true, bool deleteIfNoPlayer = true)
	{
		Rpc(RpcAsk_CreateGroupWithData, groupRole, isPrivate, name, description, joinGroup, deleteIfNoPlayer);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	void RequestKickPlayer(int playerID)
	{
		Rpc(RPC_AskKickPlayer, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	void RequestPromoteLeader(int playerID)
	{
		Rpc(RPC_AskPromoteLeader, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] isPrivate
	void RequestPrivateGroupChange(int playerID, bool isPrivate)
	{
		Rpc(RPC_ChangePrivateGroup, playerID, isPrivate);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] groupID
	void PlayerRequestToJoinPrivateGroup(int playerID, RplId groupID)
	{		
		Rpc(RPC_PlayerRequestToJoinPrivateGroup, playerID, groupID);	
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	void ClearAllRequesters(RplId groupID)
	{		
		Rpc(RPC_ClearAllRequesters, groupID);	
		RPC_ClearAllRequesters(groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetPlayerID()
	{
		PlayerController playerController = PlayerController.Cast(GetOwner());
		if (!playerController)
			return -1;
		
		return playerController.GetPlayerId();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] group
	//! \return
	bool CanPlayerJoinGroup(int playerID, notnull SCR_AIGroup group)
	{
		/*
			Assumed issue:
				SCR_FactionManager have cache mapping playerIds to factions.
				This cache may not be in sysnc with the FactionAffiliation, because of different replication hieararchies and methods
			Fix:
				We do not use cache, but instead ask directly SCR_PlayerFactionAffiliationComponent.
				This should work on client asking for its own PlayerId or server for any IDs.
		*/
		
		// Flipping comment for quick switch of implementation
		//*
		Faction groupFaction = group.GetFaction();
		if (!groupFaction)
		{
			#ifdef DEPLOY_MENU_DEBUG
				Print(string.Format("SCR_PlayerControllerGroupComponent.CanPlayerJoinGroup(%1, %2) - No group faction", playerID, group), LogLevel.ERROR);
			#endif
			return false;
		}
		
		Faction playerFaction = SCR_FactionManager.SGetPlayerFaction(playerID);
		if (playerFaction != groupFaction)
		{
			#ifdef DEPLOY_MENU_DEBUG
			Print(string.Format("SCR_PlayerControllerGroupComponent.CanPlayerJoinGroup(%1, %2) - Faction mis-match! See, below:", playerID, group), LogLevel.ERROR);
			Print(playerFaction, LogLevel.NORMAL);
			if (playerFaction)
				Print(playerFaction.GetFactionKey(), LogLevel.NORMAL);

			Print(groupFaction, LogLevel.NORMAL);
			if (groupFaction)
				Print(groupFaction.GetFactionKey(), LogLevel.NORMAL);
			#endif
			return false;
		}
		
		/*/
		// First we check the player is in the faction of the group
		Faction playerFaction;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
		{
			// TODO (langepau): Remove temporary debug logging when respawn issue is fixed.
			playerFaction = factionManager.GetPlayerFaction(playerID);
			Faction groupFaction = group.GetFaction();
			if (playerFaction != groupFaction)
			{
				#ifdef DEPLOY_MENU_DEBUG
				Print(string.Format("SCR_PlayerControllerGroupComponent.CanPlayerJoinGroup(%1, %2) - Faction mis-match! See, below:", playerID, group), LogLevel.ERROR);
				Print(playerFaction, LogLevel.NORMAL);
				if (playerFaction)
					Print(playerFaction.GetFactionKey(), LogLevel.NORMAL);

				Print(groupFaction, LogLevel.NORMAL);
				if (groupFaction)
					Print(groupFaction.GetFactionKey(), LogLevel.NORMAL);
				#endif

				return false;
			}
		}
		else
		{
			#ifdef DEPLOY_MENU_DEBUG
			Print(string.Format("SCR_PlayerControllerGroupComponent.CanPlayerJoinGroup(%1, %2) - No SCR_FactionManager!", playerID, group), LogLevel.ERROR);
			#endif
		}
		//*/

		// Groups manager doesn't exist, no point in continuing, cannot join
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
		{	
			#ifdef DEPLOY_MENU_DEBUG
			Print(string.Format("SCR_PlayerControllerGroupComponent.CanPlayerJoinGroup(%1, %2) - No SCR_GroupsManagerComponent!", playerID, group), LogLevel.ERROR);
			#endif
			return false;
		}

		// Cannot join a full group
		if (group.IsFull())
			return false;

		// Cannot join the group we are in already
		if (groupsManager.GetPlayerGroup(playerID) == group)
		{
			#ifdef DEPLOY_MENU_DEBUG
			Print(string.Format("SCR_PlayerControllerGroupComponent.CanPlayerJoinGroup(%1, %2) - Already in group!", playerID, group), LogLevel.ERROR);
			#endif
			return false;
		}
		
		SCR_Faction scrPlayerFaction = SCR_Faction.Cast(playerFaction);
		if (scrPlayerFaction)
		{
			// Commander can only join group with Commander role
			SCR_EGroupRole groupRole = group.GetGroupRole();
			if (scrPlayerFaction.GetCommanderId() == playerID && groupRole != SCR_EGroupRole.COMMANDER)
				return false;

			// Check if player has required rank for group loadouts
			SCR_GroupRolePresetConfig groupPreset = groupsManager.FindGroupRolePresetConfig(scrPlayerFaction, groupRole);
			if (groupPreset)
				return groupsManager.HasPlayerRequiredRank(groupPreset, playerID, true);
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Can player remove group
	//! \param[in] playerID
	//! \param[in] group
	//! \return true if player can remove a group
	bool CanPlayerRemoveGroup(int playerID, notnull SCR_AIGroup group)
	{
		// First we check the player is in the faction of the group
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
		{
			Faction playerFaction = factionManager.GetPlayerFaction(playerID);
			if (!playerFaction)
				return false;

			Faction groupFaction = group.GetFaction();
			if (playerFaction != groupFaction)
				return false;

			if (SCR_FactionCommanderHandlerComponent.GetInstance())
			{
				// commander group cannot be removed
				if (group.GetGroupRole() == SCR_EGroupRole.COMMANDER)
					return false;

				// check if player is faction commander, only faction commander can remove group
				SCR_Faction scrFaction = SCR_Faction.Cast(playerFaction);
				if (!scrFaction || !group.IsCreatedByCommander() || scrFaction.GetCommanderId() != playerID)
					return false;
			}
		}

		// Groups manager doesn't exist
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;

		// Cannot remove, there is still players
		if (group.GetPlayerCount() > 0)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] group
	//! \return
	bool IsPlayerLeader(int playerID, notnull SCR_AIGroup group)
	{
		return playerID == group.GetLeaderID();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsPlayerLeaderOwnGroup()
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return false;

		//get controller from owner, because if this is used on server we cannot get local players player controller :wesmart:
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetOwner());
		if (!controller)
			return false;
		
		int playerID = controller.GetPlayerId();
		SCR_AIGroup playerGroup = groupManager.GetPlayerGroup(playerID);
		if (!playerGroup)
			return false;
		
		return IsPlayerLeader(playerID, playerGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \return
	bool CanInvitePlayer(int playerID)
	{
		// Our group id is not valid -> cannot invite anyone
		if (m_iGroupID < 0)
			return false;
		
		// Groups manager doesn't exist, no point in continuing, cannot invite
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;
		
		// We get our group
		SCR_AIGroup group = groupsManager.GetPlayerGroup(GetPlayerID());
		if (!group)
			return false;
		
		// Check if the player can join us
		if (!CanPlayerJoinGroup(playerID, group))
			return false;
		
		// Already invited this player, cannot invite again
		if (WasAlreadyInvited(playerID))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \return
	bool WasAlreadyInvited(int playerID)
	{
		// The map is not initialised -> didn't invite anyone yet
		if (!m_mPlayerInvitesToGroups)
			return false;
		
		// We didn't invite this player to any group yet
		if (!m_mPlayerInvitesToGroups.Contains(playerID))
			return false;
		
		// If our group is in the array of invites for this player, we return true (already invited)
		// Otherwise we return false (wasn't invited yet)
		return m_mPlayerInvitesToGroups.Get(playerID).Contains(m_iGroupID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] accept
	void AcceptJoinPrivateGroup(int playerID, bool accept)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		
		SCR_AIGroup group = groupManager.FindGroup(GetGroupID());
		if (!group)
			return;
			
		group.GetOnJoinPrivateGroupConfirm().Invoke(DEFAULT_COLOR); // Saphyr TODO: temporary before definition from art dept.
		
		if (accept)
			Rpc(RPC_ConfirmJoinPrivateGroup,playerID, group.GetGroupID());
		else
			Rpc(RPC_CancelJoinPrivateGroup, playerID, group.GetGroupID());
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	void InvitePlayer(int playerID)
	{
		// When group id is not valid, return
		if (m_iGroupID < 0)
			return;
		
		// Init map if not initialised yet
		if (!m_mPlayerInvitesToGroups)
			m_mPlayerInvitesToGroups = new map<int, ref array<int>>();
		
		// Init array of groups the playerID was invited to if not initialised yet
		if (!m_mPlayerInvitesToGroups.Contains(playerID))
			m_mPlayerInvitesToGroups.Insert(playerID, {});
		
		// We already invited this player to our group, don't invite again
		if (m_mPlayerInvitesToGroups.Get(playerID).Contains(m_iGroupID))
			return;
		
		// We didn't invite this player to our group yet
		// Get an array of all the groups the local player invited the other player to
		array<int> invitedGroups = m_mPlayerInvitesToGroups.Get(playerID);
		
		// Invite the player and log the invitation
		Rpc(RPC_AskInvitePlayer, playerID);
		invitedGroups.Insert(m_iGroupID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] fromPlayerID
	void InviteThisPlayer(int groupID, int fromPlayerID)
	{
		Rpc(RPC_DoInvitePlayer, groupID, fromPlayerID)
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void AcceptInvite()
	{
		if (m_iGroupInviteID >= 0)
		{
			SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
			if (!groupManager)
				return;
			
			SCR_AIGroup group = groupManager.FindGroup(m_iGroupInviteID);
			if (!group)
				return;
			
			group.RemoveRequester(GetPlayerID());
			
			RequestJoinGroup(m_iGroupInviteID);
			m_iGroupInviteID = -1;
			if (m_OnInviteAccepted)
				m_OnInviteAccepted.Invoke();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnInviteReceived()
	{
		if (!m_OnInviteReceived)
			m_OnInviteReceived = new ScriptInvoker<int, int>();

		return m_OnInviteReceived;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	void OnGroupDeleted(SCR_AIGroup group)
	{
		if (!group)
			return; 
		
		//in case of the selected group being deleted, remove it from selected
		if (group.GetGroupID() == GetSelectedGroupID())
			SetSelectedGroupID(-1);
		
		if (group.GetGroupID() == m_iGroupInviteID)
		{
			//delete the invite if the target group has been deleted
			m_iGroupInviteID = -1;
			if (m_OnInviteCancelled)
				m_OnInviteCancelled.Invoke();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] groupID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_PlayerRequestToJoinPrivateGroup(int playerID, RplId groupID)
	{	
		SCR_AIGroup group = SCR_AIGroup.Cast(Replication.FindItem(groupID));
		if (!group)
			return;
		
		group.AddRequester(playerID);		
		SCR_NotificationsComponent.SendToPlayer(group.GetLeaderID(), ENotification.GROUPS_REQUEST_JOIN_PRIVATE_GROUP, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] groupID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_ConfirmJoinPrivateGroup(int playerID, int groupID)
	{	
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
			
		SCR_AIGroup group = groupManager.FindGroup(groupID);
		if (!group)
			return;
		
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController)
			return;
		
		SCR_PlayerControllerGroupComponent playerComponent = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		
		playerComponent.RequestJoinGroup(group.GetGroupID());	
					
		group.RemoveRequester(playerID);	
		
		SCR_NotificationsComponent.SendToPlayer(playerID, ENotification.GROUPS_REQUEST_ACCEPTED);	
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] groupID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_CancelJoinPrivateGroup(int playerID, int groupID)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		SCR_AIGroup group = groupManager.FindGroup(groupID);
		if (!group)
			return;
		
		group.RemoveRequester(playerID);		
		group.AddDeniedRequester(playerID);	
		
		SCR_NotificationsComponent.SendToPlayer(playerID, ENotification.GROUPS_REQUEST_DENIED);	
	}	
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] fromPlayerID
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RPC_DoInvitePlayer(int groupID, int fromPlayerID)
	{
		m_iGroupInviteID = groupID;
		m_iGroupInviteFromPlayerID = fromPlayerID;
		
		//Save player name so it can be obtained even if the player left
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (playerManager)
			m_sGroupInviteFromPlayerName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(fromPlayerID);
		
		if (m_OnInviteReceived)
			m_OnInviteReceived.Invoke(groupID, fromPlayerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskInvitePlayer(int playerID)
	{
		PlayerController invitedPlayer = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!invitedPlayer)
			return;
		
		SCR_PlayerControllerGroupComponent invitedPlayerGroupComponent = SCR_PlayerControllerGroupComponent.Cast(invitedPlayer.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!invitedPlayerGroupComponent)
			return;
		
		SocialComponent socialComp = SocialComponent.Cast(invitedPlayer.FindComponent(SocialComponent));
		if (socialComp)
		{
			PlayerController inviterComponent = PlayerController.Cast(GetOwner());
			if (inviterComponent && socialComp.IsRestricted(inviterComponent.GetPlayerId(), EUserInteraction.Invitation))
				return;
		}

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup group = groupsManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		SCR_GroupRolePresetConfig groupPreset = groupsManager.FindGroupRolePresetConfig(group.GetFaction(), group.GetGroupRole());
		if (groupPreset && !groupsManager.HasPlayerRequiredRank(groupPreset, playerID, true))
		{
			// Show "Insufficient rank" notification to client
			SCR_NotificationsComponent.SendToPlayer(GetPlayerID(), ENotification.GROUPS_INVITE_CANCELLED_INSUFFICIENT_RANK);
			return;
		}

		invitedPlayerGroupComponent.InviteThisPlayer(m_iGroupID, GetPlayerID());
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskCreateGroup()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		Faction faction = factionManager.GetPlayerFaction(GetPlayerID());
		if (!faction)
			return;
		
		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if(!scrFaction)
			return;
		
		// We check if there is any empty group already for our faction
		if (groupsManager.TryFindEmptyGroup(faction))
			return;
		
		// We check if other then predefined group can be created
		if(scrFaction.GetCanCreateOnlyPredefinedGroups())
			return;
		
		// No empty group found, we allow creation of new group		
		SCR_AIGroup newGroup = groupsManager.CreateNewPlayableGroup(faction);
		
		// No new group was created, return
		if (!newGroup)
			return;
		
		// New group sucessfully created
		// The player should be automatically added/moved to it
		RPC_AskJoinGroup(newGroup.GetGroupID());
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_CreateGroupWithData(SCR_EGroupRole groupRole, bool isPrivate, string name, string description, bool joinGroup, bool deleteIfNoPlayer)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;

		int playerId = GetPlayerID();
		Faction faction = factionManager.GetPlayerFaction(playerId);
		if (!faction)
			return;

		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if (!scrFaction)
			return;

		// No empty group found, we allow creation of new group
		SCR_AIGroup newGroup = groupsManager.CreateNewPlayableGroup(faction, groupRole);

		// No new group was created, return
		if (!newGroup)
			return;

		array<SCR_GroupRolePresetConfig> groupRolePresetConfigs = {};
		scrFaction.GetGroupRolePresetConfigs(groupRolePresetConfigs);
		foreach (SCR_GroupRolePresetConfig preset : groupRolePresetConfigs)
		{
			if (preset && preset.GetGroupRole() == groupRole)
			{
				preset.SetupGroup(newGroup); // set defaults
				preset.SetupGroupFlag(newGroup, scrFaction);

				break;
			}
		}

		newGroup.SetPrivate(isPrivate);
		newGroup.SetCanDeleteIfNoPlayer(deleteIfNoPlayer);

		/*
		To set the name and description of the group, the calling of the SetCustomNameAndDescription function must be delayed to the next frame,
		because the Rplcomponent on the new group is not set yet and the Rpc SetCustomName and SetCustomDescription are not called on the clients.
		RplSave and RplLoad won't help in this case, because customName and description pass through asynchronous profanity filter,
		so it takes a while for m_sCustomName and m_sCustomDescription to filter and set.
		*/
		GetGame().GetCallqueue().Call(SetCustomNameAndDescription, newGroup, name, description, playerId);

		// New group sucessfully created
		// The player should be automatically added/moved to it
		if (joinGroup)
			RPC_AskJoinGroup(newGroup.GetGroupID());

		// check if author is commander
		if (scrFaction.GetCommanderId() == playerId)
			newGroup.SetCreatedByCommander(true);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskKickPlayer(int playerID)
	{
		SCR_GroupsManagerComponent groupsManager;
		SCR_PlayerControllerGroupComponent playerGroupController;
		SCR_AIGroup group;
		if (!InitiateComponents(playerID, groupsManager, playerGroupController, group))
			return;
		
		//requesting player is not leader of the targets group, do nothing
		if (!group.IsPlayerLeader(GetPlayerID()))
			return;
		
		SCR_AIGroup newGroup = groupsManager.GetFirstNotFullForFaction(group.GetFaction(), group, true);
		if (!newGroup)
			newGroup = groupsManager.CreateNewPlayableGroup(group.GetFaction());
				
		if (!newGroup)
			return;
		playerGroupController.RequestJoinGroup(newGroup.GetGroupID());
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskPromoteLeader(int playerID)
	{
		SCR_GroupsManagerComponent groupsManager;
		SCR_PlayerControllerGroupComponent playerGroupController;
		SCR_AIGroup group;
		if (!InitiateComponents(playerID, groupsManager, playerGroupController, group))
			return;
		
		if (!group.IsPlayerLeader(GetPlayerID()))
			return;
		
		groupsManager.SetGroupLeader(group.GetGroupID(), playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] isPrivate
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_ChangePrivateGroup(int playerID, bool isPrivate)
	{
		SCR_GroupsManagerComponent groupsManager;
		SCR_PlayerControllerGroupComponent playerGroupController;
		SCR_AIGroup group;
		if (!InitiateComponents(playerID, groupsManager, playerGroupController, group))
			return;
		
		groupsManager.SetPrivateGroup(group.GetGroupID(), isPrivate);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	void RequestJoinGroup(int groupID)
	{
		Rpc(RPC_AskJoinGroup, groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RPC_DoChangeGroupID(int groupID)
	{
		m_iGroupID = groupID;
		if (groupID == m_iGroupInviteID)
		{
			//reset the invite if player manually joined the group he is invited into
			m_iGroupInviteID = -1;
			if (m_OnInviteCancelled)
				m_OnInviteCancelled.Invoke();
		}
		if (m_OnGroupChanged)
			GetGame().GetCallqueue().CallLater(OnGroupChangedDelayed, 0, false, groupID);
	}

	//------------------------------------------------------------------------------------------------	
	protected void OnGroupChangedDelayed(int groupId)
	{
		m_OnGroupChanged.Invoke(groupId);	
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskJoinGroup(int groupID)
	{
		// Trying to join the same group, reject.
		if (groupID == m_iGroupID)
			return;
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		int groupIDAfter;
		if (m_iGroupID != -1)
			groupIDAfter = groupsManager.MovePlayerToGroup(GetPlayerID(), m_iGroupID, groupID);
		else
			groupIDAfter = groupsManager.AddPlayerToGroup(groupID, GetPlayerID());
		
		if (groupIDAfter != m_iGroupID)
		{
			m_iPreviousGroupID = m_iGroupID;
			m_iGroupID = groupIDAfter;
			Rpc(RPC_DoChangeGroupID, groupIDAfter);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove group
	//! \param[in] groupID
	void RequestRemoveGroup(int groupID)
	{
		Rpc(RpcAsk_RemoveGroup, groupID);
	}

	//------------------------------------------------------------------------------------------------
	//! Ask the server to remove a group
	//! \param[in] groupID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_RemoveGroup(int groupID)
	{
		// Trying to remove the my group, reject.
		if (groupID == m_iGroupID || m_iGroupID == -1)
			return;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group || group.GetPlayerCount() > 0)
			return;

		groupsManager.DeleteGroupDelayed(group);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[out] groupsManager
	//! \param[out] playerGroupController
	//! \param[out] group
	//! \return
	bool InitiateComponents(int playerID, out SCR_GroupsManagerComponent groupsManager, out SCR_PlayerControllerGroupComponent playerGroupController , out SCR_AIGroup group)
	{
		groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;
		
		playerGroupController = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(playerID);
		if (!playerGroupController)
			return false;
		
		group = groupsManager.GetPlayerGroup(playerID);
		if (!group)
			return false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetSelectedGroupID()
	{
		return m_iUISelectedGroupID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnInviteAccepted()
	{
		if (!m_OnInviteAccepted)
			m_OnInviteAccepted =  new ScriptInvoker<int>();

		return m_OnInviteAccepted;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnGroupChanged()
	{
		if (!m_OnGroupChanged)
			m_OnGroupChanged =  new ScriptInvoker<int>();

		return m_OnGroupChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnInviteCancelled()
	{
		if (!m_OnInviteCancelled)
			m_OnInviteCancelled =  new ScriptInvoker<int>();

		return m_OnInviteCancelled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerInt GetOnSetSelectedGroupID()
	{
		if (!m_OnSetSelectedGroupID)
			m_OnSetSelectedGroupID = new ScriptInvokerInt();

		return m_OnSetSelectedGroupID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetGroupInviteID()
	{
		return m_iGroupInviteID;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] value
	void SetGroupInviteID(int value)
	{
		m_iGroupInviteID = value;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetGroupInviteFromPlayerID()
	{
		return m_iGroupInviteFromPlayerID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return previous group id
	int GetPreviousGroupID()
	{
		return m_iPreviousGroupID;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetGroupInviteFromPlayerName()
	{
		return m_sGroupInviteFromPlayerName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] groupID
	void SetSelectedGroupID(int groupID)
	{
		m_iUISelectedGroupID = groupID;

		if (m_OnSetSelectedGroupID)
			m_OnSetSelectedGroupID.Invoke(groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetCustomNameAndDescription(SCR_AIGroup group, string name, string description, int playerId)
	{
		group.SetCustomName(name, playerId);
		group.SetCustomDescription(description, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] desc
	void RequestSetCustomGroupDescription(int groupID, string desc)
	{
		SCR_AIGroup.DeLocalizeText(desc);
		Rpc(RPC_AskSetCustomDescription, groupID, desc, SCR_PlayerController.GetLocalPlayerId());
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] desc
	//! \param[in] authorID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetCustomDescription(int groupID, string desc, int authorID)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		group.SetCustomDescription(desc, authorID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] maxMembers
	void RequestSetGroupMaxMembers(int groupID, int maxMembers)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		if (group.GetMaxMembers() == maxMembers || maxMembers < 0)
			return;
		
		Rpc(RPC_AskSetGroupMaxMembers, groupID, maxMembers);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] maxMembers
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetGroupMaxMembers(int groupID, int maxMembers)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		if (group.GetMaxMembers() == maxMembers || maxMembers < 0)
			return;
		
		group.SetMaxMembers(maxMembers);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets custom frequency (KHz) for a group. Can set frequency that is already claimed.
	//! Claims set frequency if not already claimed.
	//! Frequency set by this method will not be used by automatically created groups.
	//! \param[in] groupID
	//! \param[in] frequency
	void RequestSetCustomFrequency(int groupID, int frequency)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		if (frequency < 0 || group.GetRadioFrequency() == frequency)
			return;
		
		Rpc(RPC_AskSetFrequency, groupID, frequency);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] isAllowed
	void RequestSetNewGroupsAllowed(bool isAllowed)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager || isAllowed == groupsManager.GetNewGroupsAllowed())
			return;
		
		Rpc(RPC_AskSetNewGroupsAllowed, isAllowed);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] isAllowed
	void RequestSetCanPlayersChangeAttributes(bool isAllowed)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager || isAllowed == groupsManager.GetNewGroupsAllowed())
			return;
		
		Rpc(RPC_AskSetCanPlayersChangeAttributes, isAllowed);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] name
	void RequestSetCustomGroupName(int groupID, string name)
	{
		SCR_AIGroup.DeLocalizeText(name);
		Rpc(RPC_AskSetCustomName, groupID, name, SCR_PlayerController.GetLocalPlayerId());
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] name
	//! \param[in] authorID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetCustomName(int groupID, string name, int authorID)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		group.SetCustomName(name, authorID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] isAllowed
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetNewGroupsAllowed(bool isAllowed)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager || isAllowed == groupsManager.GetNewGroupsAllowed())
			return;
		
		groupsManager.SetNewGroupsAllowed(isAllowed);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] isAllowed
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetCanPlayersChangeAttributes(bool isAllowed)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager || isAllowed == groupsManager.GetNewGroupsAllowed())
			return;
		
		groupsManager.SetCanPlayersChangeAttributes(isAllowed);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] frequency
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetFrequency(int groupID, int frequency)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		if (frequency < 0 || group.GetRadioFrequency() == frequency)
			return;
		
		SCR_Faction groupFaction = SCR_Faction.Cast(group.GetFaction());
		if (!groupFaction)
			return;
		
		int formerFrequency = group.GetRadioFrequency();
		int foundGroupsWithFrequency = 0;
		
		//no null check here because the array will always at least contain the group that was passed as parameter into this method
		array<SCR_AIGroup> existingGroups = groupsManager.GetPlayableGroupsByFaction(groupFaction);
		
		foreach (SCR_AIGroup checkedGroup: existingGroups)
		{
			if (checkedGroup.GetRadioFrequency() == formerFrequency)
				foundGroupsWithFrequency++;
		}
		
		//if there is only our group with this frequency or none, release it before changing our frequency
		if (foundGroupsWithFrequency <= 1)
			groupsManager.ReleaseFrequency(formerFrequency, groupFaction);
		
		//if the new frequency is unclaimed, claime it so newly created groups do not get it by default
		if (!groupsManager.IsFrequencyClaimed(frequency, groupFaction))
			groupsManager.ClaimFrequency(frequency, groupFaction);
		
		group.SetRadioFrequency(frequency);
	}
	 
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] flagIndex
	//! \param[in] isFromImageset
	void RequestSetGroupFlag(int groupID, int flagIndex, bool isFromImageset)
	{
		Rpc(RPC_AskSetGroupFlag, groupID, flagIndex, isFromImageset);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] flagIndex
	//! \param[in] isFromImageset
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetGroupFlag(int groupID, int flagIndex, bool isFromImageset)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.FindGroup(groupID);
		if (!group)
			return;
		
		group.SetGroupFlag(flagIndex, isFromImageset);		
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_ClearAllRequesters(RplId groupID)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(Replication.FindItem(groupID));
		if (!group)
			return;
		
		group.ClearRequesters();
		group.ClearDeniedRequester();	
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rplCompID
	void RequestCreateSlaveGroup(RplId rplCompID)
	{
		Rpc(RPC_AskCreateSlaveGroup, rplCompID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rplCompID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskCreateSlaveGroup(RplId rplCompID)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return;
		
		IEntity groupEntity = GetGame().SpawnEntityPrefab(Resource.Load(commandingManager.GetGroupPrefab()));
		if (!groupEntity)
			return;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(groupEntity);
		if (!group)
			return;
		
		//we dont delete slave groups when empty, we disable them
		group.SetDeleteWhenEmpty(false);
		
		RplComponent slaveRplComp = RplComponent.Cast(group.FindComponent(RplComponent));
		if (!slaveRplComp)
			return;
		
		groupManager.RequestSetGroupSlave(rplCompID, slaveRplComp.Id());
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] character
	//! \param[in] faction
	//! \return
	bool IsAICharacterInAnyGroup(SCR_ChimeraCharacter character, SCR_Faction faction)
	{
		//TODO: kuceramar: come up with better solution that doesnt include going through all groups
		//possible JIP problems due ot using SCR_CHimeraCharacter
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return false;

		array<SCR_AIGroup> groups = groupManager.GetPlayableGroupsByFaction(faction);
		
		foreach(SCR_AIGroup group : groups)
		{
			if (group.IsAIControlledCharacterMember(character))
				return true;
		}
		
		return false;
	}
		
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] character
	//! \param[in] playerID
	void RequestAddAIAgent(SCR_ChimeraCharacter character, int playerID)
	{
		RplComponent rplComp = RplComponent.Cast(character.FindComponent(RplComponent));
		if (!rplComp)
			return;
		
		Rpc(RPC_AskAddAIAgent, rplComp.Id(), playerID);
	} 
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] characterID
	//! \param[in] playerID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskAddAIAgent(RplId characterID, int playerID)
	{
		SCR_GroupsManagerComponent groupsManager;
		SCR_PlayerControllerGroupComponent playerGroupController;
		SCR_AIGroup group;
		if (!InitiateComponents(playerID, groupsManager, playerGroupController, group))
			return;
		
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(characterID));
		if (!rplComp)
			return;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(rplComp.GetEntity());
		if (!character)
			return;

		if (!group.IsPlayerLeader(playerID))
			return;
		 
		AddAIToSlaveGroup(character, group);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Should be only called on the server
	//! \param[in] controlledEntity
	//! \param[in] group
	void AddAIToSlaveGroup(notnull IEntity controlledEntity, SCR_AIGroup group)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		SCR_AIGroup slaveGroup = group.GetSlave();
		if (!slaveGroup)
			return;
		
		if (!slaveGroup.IsAIActivated())
			slaveGroup.ActivateAI();
		
		slaveGroup.AddAgentFromControlledEntity(controlledEntity);
		
		RplId groupCompID, characterCompID;
		RplComponent rplComp = RplComponent.Cast(slaveGroup.FindComponent(RplComponent));
		if (!rplComp)
			return;
		
		groupCompID = rplComp.Id();
		rplComp = RplComponent.Cast(controlledEntity.FindComponent(RplComponent)); 
		characterCompID = rplComp.Id();
		
		groupManager.AskAddAiMemberToGroup(groupCompID, characterCompID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] characterID
	//! \param[in] playerID
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskRemoveAIAgent(RplId characterID, int playerID)
	{
		SCR_GroupsManagerComponent groupsManager;
		SCR_PlayerControllerGroupComponent playerGroupController;
		SCR_AIGroup group;
		if (!InitiateComponents(playerID, groupsManager, playerGroupController, group))
			return;
		
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(characterID));
		if (!rplComp)
			return;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(rplComp.GetEntity());
		if (!character)
			return;		
		
		if (!group.IsPlayerLeader(playerID))
			return;
		
		RemoveAiFromSlaveGroup(character, group);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Should be only called on the server
	//! \param[in] controlledEntity
	//! \param[in] group
	void RemoveAiFromSlaveGroup(notnull IEntity controlledEntity, SCR_AIGroup group)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		SCR_AIGroup slaveGroup = group.GetSlave();
		if (!slaveGroup)
			return;
		
		//deactivate the group since we are removing last AI member
		if (slaveGroup.GetAgentsCount() == 1)
			slaveGroup.Deactivate();
		
		slaveGroup.RemoveAgentFromControlledEntity(controlledEntity);
		
		RplId groupCompID, characterCompID;
		RplComponent rplComp = RplComponent.Cast(slaveGroup.FindComponent(RplComponent));
		groupCompID = rplComp.Id();
		rplComp = RplComponent.Cast(controlledEntity.FindComponent(RplComponent)); 
		if (!rplComp)
			return;
		
		characterCompID = rplComp.Id();
		
		groupManager.AskRemoveAiMemberFromGroup(groupCompID, characterCompID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] character
	//! \param[in] playerID
	void RequestRemoveAgent(SCR_ChimeraCharacter character, int playerID)
	{
		RplComponent rplComp = RplComponent.Cast(character.FindComponent(RplComponent));
		if (!rplComp)
			return;
		
		Rpc(RPC_AskRemoveAIAgent, rplComp.Id(), playerID);
	}

	//------------------------------------------------------------------------------------------------
	void RequestGroupLeaderVote(int playerID)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController)
			return;

		SCR_VoterComponent votingComponent = SCR_VoterComponent.Cast(playerController.FindComponent(SCR_VoterComponent));
		if (!votingComponent)
			return;

		votingComponent.Vote(EVotingType.GROUP_LEADER, playerID);
	}

	//------------------------------------------------------------------------------------------------
	void SetGroupLeader(int playerID)
	{
		Rpc(RpcDo_SetGroupLeader, playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcDo_SetGroupLeader(int playerID)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		SCR_AIGroup playerGroup = groupsManager.GetPlayerGroup(playerID);
		if (!playerGroup)
			return;

		if (playerGroup.GetLeaderID() == playerID)
			return;

		playerGroup.SetGroupLeader(playerID);
	}

	//------------------------------------------------------------------------------------------------
	void SetRallyPoint(notnull SCR_MilitaryBaseComponent base, bool force)
	{
		Rpc(RpcAsk_SetRallyPoint, GetPlayerID(), base.GetCallsign(), force);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetRallyPoint(int playerID, int callsignId, bool force)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		SCR_AIGroup playerGroup = groupsManager.GetPlayerGroup(playerID);
		if (!playerGroup)
			return;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (!campaign)
			return;

		SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
		if (!baseManager)
			return;

		SCR_MilitaryBaseComponent base = baseManager.FindBaseByCallsign(callsignId);
		if (!base)
			return;

		playerGroup.SetRallyPoint(base, force);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveRallyPoint()
	{
		Rpc(RpcAsk_RemoveRallyPoint, GetPlayerID());
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RemoveRallyPoint(int playerID)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		SCR_AIGroup playerGroup = groupsManager.GetPlayerGroup(playerID);
		if (!playerGroup)
			return;

		playerGroup.RemoveRallyPoint();
	}

	//------------------------------------------------------------------------------------------------
	void SetTransportUnitSourceBase(notnull SCR_TransportUnitComponent transportUnit, SCR_CampaignMilitaryBaseComponent base)
	{
		RplId rplIdTransportUnitComponent = Replication.FindItemId(transportUnit);
		if (!rplIdTransportUnitComponent.IsValid())
			return;

		int baseCallsign = SCR_CampaignMilitaryBaseComponent.INVALID_BASE_CALLSIGN;
		if (base)
			baseCallsign = base.GetCallsign();

		Rpc(RpcDo_AskSetTransportUnitSourceBase, baseCallsign, rplIdTransportUnitComponent);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcDo_AskSetTransportUnitSourceBase(int baseCallsign, RplId rplIdTransportUnitComponent)
	{
		if (!rplIdTransportUnitComponent.IsValid())
			return;
		
		SCR_TransportUnitComponent transportUnit = SCR_TransportUnitComponent.Cast(Replication.FindItem(rplIdTransportUnitComponent));
		if (!transportUnit)
			return;

		SCR_CampaignMilitaryBaseComponent base = SCR_GameModeCampaign.GetInstance().GetBaseManager().FindBaseByCallsign(baseCallsign);
		transportUnit.SetSourceBase(base);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

#ifdef ENABLE_DIAG
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_GROUPS, "Groups", "GameCode");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_GROUPS_ENABLE_DIAG, "", "Enable groups diag", "Groups");
		ConnectToDiagSystem(owner);
#endif
		groupsManager.GetOnPlayableGroupRemoved().Insert(OnGroupDeleted);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
#ifdef ENABLE_DIAG
		DisconnectFromDiagSystem(owner);
#endif
		
		super.OnDelete(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_AIGroup GetPlayersGroup()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return null;
		
		return groupsManager.FindGroup(m_iGroupID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetActualGroupFrequency()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return 0;
		
		SCR_AIGroup group = groupsManager.FindGroup(m_iGroupID);
		if (!group)
			return 0;
		
		return group.GetRadioFrequency();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{		
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_GROUPS_ENABLE_DIAG))
			return;
		
		DbgUI.Begin("Groups");
				
		int playerID = SCR_PlayerController.GetLocalPlayerId();		
		Faction faction;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			faction = factionManager.GetPlayerFaction(playerID);
		
		if (!faction)
		{
			DbgUI.Text("Groups do not support factionless players now!!");
			DbgUI.End();
			return;
		}
		
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		
		DbgUI.Text("Your Faction:" + faction.GetFactionKey());
		DbgUI.Spacer(2);
		
		DbgUI.Text("Playable groups of your faction:");		
		ListGroupsFromFaction(faction);
		DbgUI.Spacer(6);
		
		DbgUI.Text("Your group:");
		SCR_AIGroup group = groupsManager.GetPlayerGroup(playerID);
		if (group)
		{
			DbgUI.Text("." + group.ToString());
			Print(group.ToString(), LogLevel.NORMAL);
		}
		DbgUI.Spacer(2);
		
		if (DbgUI.Button("Create and join group for my faction"))
			CreateAndJoinGroup(faction);
		
		DbgUI.Spacer(2);
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] faction
	void ListGroupsFromFaction(Faction faction)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		array<SCR_AIGroup> groups = groupsManager.GetPlayableGroupsByFaction(faction);
		if (!groups)
		{
			DbgUI.Text("No groups for your faction!!");
			return;
		}

		foreach(SCR_AIGroup group : groups)
		{
			DbgUI.Text(group.ToString());			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] faction
	void CreateAndJoinGroup(Faction faction)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		SCR_AIGroup group = groupsManager.GetFirstNotFullForFaction(faction, null, true);
		if (group)
			RequestJoinGroup(group.GetGroupID());
		else
			RequestCreateGroup(); //requestCreateGroup automatically puts player to the newly created group
	}
}
