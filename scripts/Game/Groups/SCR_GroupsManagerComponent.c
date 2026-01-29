typedef set<Faction> FactionHolder;

[EntityEditorProps(category: "GameScripted/Groups", description: "Player groups manager, attach to game mode entity!.")]
class SCR_GroupsManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_GroupsManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute()]
	protected ResourceName m_sDefaultGroupPrefab;

	[Attribute("1000")]
	protected int m_iPlayableGroupFrequencyOffset;

	[Attribute("38000")]
	protected int m_iPlayableGroupFrequencyMin;

	[Attribute("54000")]
	protected int m_iPlayableGroupFrequencyMax;

	[Attribute("1", desc:"If checked, the added agent will tune the radio to the group frequency")]
	protected bool m_bTuneAgentsRadioToGroupFrequency;

	[Attribute("1", desc:"If checked, the player will tune the radio to the group frequency when is spawn")]
	protected bool m_bTunePlayersRadioToGroupFrequency;

	[Attribute("Flags", UIWidgets.ResourcePickerThumbnail, "Flag icon of this particular group.", params: "edds")]
	private ref array<ResourceName> m_aGroupFlags;

	[Attribute(defvalue: "1", desc: "When true group menu will be normally accesible, when false group menu will be disabled, but NOT the group functionality")]
	protected bool m_bAllowGroupMenu;

	//this is changed only localy and doesnt replicate
	protected bool m_bConfirmedByPlayer;

	protected bool m_bNewGroupsAllowed = true;
	protected bool m_bCanPlayersChangeAttributes = true;

	protected static SCR_GroupsManagerComponent s_Instance;

#ifdef DEBUG_GROUPS
	static Widget s_wDebugLayout;
#endif

	protected ref ScriptInvoker m_OnPlayableGroupCreated = new ScriptInvoker();
	protected ref ScriptInvoker m_OnPlayableGroupRemoved = new ScriptInvoker();
	protected ref ScriptInvoker m_OnNewGroupsAllowedChanged = new ScriptInvoker();
	protected ref ScriptInvoker m_OnCanPlayersChangeAttributeChanged = new ScriptInvoker();
	protected int m_iLatestGroupID = 1000; // Lower ids reserved for pre-defined groups
	protected ref map<Faction, ref array<SCR_AIGroup>> m_mPlayableGroups = new map<Faction, ref array<SCR_AIGroup>>();
	protected ref map<Faction, int> m_mPlayableGroupFrequencies = new map<Faction, int>();
	protected ref map<int, ref FactionHolder> m_mUsedFrequenciesMap = new map<int, ref FactionHolder>();
	protected ref array<SCR_AIGroup> m_aDeletionQueue = {};
	protected int m_iMovingPlayerToGroupID = -1;

	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_GroupsManagerComponent GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] targetArray
	void GetGroupFlags(notnull array<ResourceName> targetArray)
	{
		targetArray.Copy(m_aGroupFlags);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnPlayableGroupCreated()
	{
		return m_OnPlayableGroupCreated;
	}

	protected bool IsProxy()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rplComponent)
			return false;

		return rplComponent.IsProxy();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] previousGroupID
	//! \param[in] newGroupID
	//! \return
	int MovePlayerToGroup(int playerID, int previousGroupID, int newGroupID)
	{
		m_iMovingPlayerToGroupID = newGroupID;
		SCR_AIGroup previousGroup = FindGroup(previousGroupID);
		if (previousGroup)
			previousGroup.RemovePlayer(playerID);

		SCR_AIGroup newGroup = FindGroup(newGroupID);
		if (newGroup)
		{
			if (newGroup.IsFull())
			{
				m_iMovingPlayerToGroupID = -1;
				return -1;
			}

			newGroup.AddPlayer(playerID);
			m_iMovingPlayerToGroupID = -1;
			return newGroupID;
		}
		else
		{
			m_iMovingPlayerToGroupID = -1;
			return -1;
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] playerID
	void ClearRequests(int groupID, int playerID)
	{
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerGroupController)
			return;

		SCR_AIGroup group = FindGroup(groupID);
		if (!group)
			return;

		RplId groupRplID = Replication.FindId(group);

		array<int> requesterIDs = {};
		group.GetRequesterIDs(requesterIDs);

		for (int i = 0, count = requesterIDs.Count(); i < count; i++)
		{
			if (!group.IsPlayerInGroup(playerID))
				SCR_NotificationsComponent.SendToPlayer(requesterIDs[i], ENotification.GROUPS_REQUEST_CANCELLED);
		}

		playerGroupController.ClearAllRequesters(groupRplID);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] playerID
	//! \return
	int AddPlayerToGroup(int groupID, int playerID)
	{
		SCR_AIGroup group = FindGroup(groupID);
		if (!group)
			return -1;

		if (group.IsFull())
			return -1;

		group.AddPlayer(playerID);
		return groupID;
	}

	//------------------------------------------------------------------------------------------------
	//! Setup preset groups once
	protected void CreatePredefinedGroups()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);
		
		bool isCommanderRoleEnabled = true;
		SCR_GameModeCampaign gameModeCampaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (gameModeCampaign)
			isCommanderRoleEnabled = gameModeCampaign.GetCommanderRoleEnabled();

		int presetGroupId = 1;
		foreach (Faction faction : factions)
		{
			SCR_Faction scrFaction = SCR_Faction.Cast(faction);
			if (!scrFaction)
				return;

			array<ref SCR_GroupPreset> groups = {};
			scrFaction.GetPredefinedGroups(groups);

			array<SCR_GroupRolePresetConfig> groupRolePresetConfigs = {};
			scrFaction.GetGroupRolePresetConfigs(groupRolePresetConfigs);

			const array<SCR_AIGroup> existingGroups = GetPlayableGroupsByFaction(scrFaction);

			SCR_AIGroup newGroup;
			foreach (SCR_GroupPreset gr : groups)
			{
				if (!isCommanderRoleEnabled && gr.GetGroupRole() == SCR_EGroupRole.COMMANDER)
					continue;

				// Pre defined group was already loaded from e.g. save-game, so no need to create it
				bool found;
				if (existingGroups)
				{
					foreach (SCR_AIGroup group : existingGroups)
					{
						if (group.IsPredefinedGroup() && (group.GetGroupRole() == gr.GetGroupRole()))
						{
							group.SetGroupID(presetGroupId++);
							found = true;
							break;
						}
					}
					
					if (found)
						continue;
				}

				newGroup = CreateNewPlayableGroup(faction, gr.GetGroupRole());
				if (!newGroup)
					continue;

				newGroup.SetGroupID(presetGroupId++);
				newGroup.SetPredefinedGroup(true);
				newGroup.SetCanDeleteIfNoPlayer(false);

				// setup group by groupRolePresetConfig if is configured
				bool isConfigFound;
				foreach (SCR_GroupRolePresetConfig config : groupRolePresetConfigs)
				{
					if (config.GetGroupRole() == gr.GetGroupRole())
					{
						config.SetupGroupWithOverride(newGroup, gr);
						config.SetupGroupFlag(newGroup, scrFaction);
						isConfigFound = true;
						break;
					}
				}

				if (!isConfigFound)
				{
					// setup group only by predefined preset
					gr.SetupGroup(newGroup); // set defaults
					gr.SetupGroupFlag(newGroup, scrFaction);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] groupID
	//! \param[in] playerID
	//------------------------------------------------------------------------------------------------
	void SetGroupLeader(int groupID, int playerID)
	{
		SCR_AIGroup group = FindGroup(groupID);
		if (!group)
			return;
		group.SetGroupLeader(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! called on server only
	void SetNewGroupsAllowed(bool isAllowed)
	{
		if (isAllowed == m_bNewGroupsAllowed)
			return;

		RPC_DoSetNewGroupsAllowed(isAllowed);
		Rpc(RPC_DoSetNewGroupsAllowed, isAllowed);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] isAllowed
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetNewGroupsAllowed(bool isAllowed)
	{
		if (isAllowed == m_bNewGroupsAllowed)
			return;

		m_bNewGroupsAllowed = isAllowed;
		m_OnNewGroupsAllowedChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	//! called on server only
	void SetCanPlayersChangeAttributes(bool isAllowed)
	{
		if (isAllowed == m_bCanPlayersChangeAttributes)
			return;

		RPC_SetCanPlayersChangeAttributes(isAllowed);
		Rpc(RPC_SetCanPlayersChangeAttributes, isAllowed);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] isAllowed
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetCanPlayersChangeAttributes(bool isAllowed)
	{
		if (isAllowed == m_bCanPlayersChangeAttributes)
			return;

		m_bCanPlayersChangeAttributes = isAllowed;
		m_OnCanPlayersChangeAttributeChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] groupID
	//! \param[in] isPrivate
	//------------------------------------------------------------------------------------------------
	void SetPrivateGroup(int groupID, bool isPrivate)
	{
		SCR_AIGroup group = FindGroup(groupID);
		if (!group)
			return;
		group.SetPrivate(isPrivate);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \return
	//------------------------------------------------------------------------------------------------
	SCR_AIGroup FindGroup(int groupID)
	{
		array<SCR_AIGroup> groups;
		GetAllPlayableGroups(groups);

		for (int i = groups.Count() - 1; i >= 0; i--)
		{
			if (groups[i].GetGroupID() == groupID)
				return groups[i];
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnPlayableGroupRemoved()
	{
		return m_OnPlayableGroupRemoved;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnNewGroupsAllowedChanged()
	{
		return m_OnNewGroupsAllowedChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCanPlayersChangeAttributeChanged()
	{
		return m_OnCanPlayersChangeAttributeChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return
	//------------------------------------------------------------------------------------------------
	// Returns group by player id
	SCR_AIGroup GetPlayerGroup(int playerID)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return null;

		Faction faction = factionManager.GetPlayerFaction(playerID);
		if (!faction)
			return null;

		array<SCR_AIGroup> playableGroups = GetPlayableGroupsByFaction(faction);
		if (!playableGroups)
			return null;

		for (int i = playableGroups.Count() - 1; i >= 0; i--)
		{
			if (playableGroups[i] && playableGroups[i].IsPlayerInGroup(playerID))
				return playableGroups[i];
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \return
	//------------------------------------------------------------------------------------------------
	bool IsPlayerInAnyGroup(int playerID)
	{
		array<SCR_AIGroup> playableGroups;
		GetAllPlayableGroups(playableGroups);
		for (int i = playableGroups.Count() - 1; i >= 0; i--)
		{
			if (playableGroups[i].IsPlayerInGroup(playerID))
				return true;
		}

		return false;
	}

#ifdef DEBUG_GROUPS

	//------------------------------------------------------------------------------------------------
	void UpdateDebugUI()
	{
		if (!s_wDebugLayout)
			s_wDebugLayout = GetGame().GetWorkspace().CreateWidgets("{661C199F3D59BB24}UI/layouts/Debug/Groups.layout");

		VerticalLayoutWidget groupsLayout = VerticalLayoutWidget.Cast(s_wDebugLayout.FindAnyWidget("GroupsLayout"));
		if (!groupsLayout)
			return;

		//Clear entries
		while (groupsLayout.GetChildren())
		{
			groupsLayout.GetChildren().RemoveFromHierarchy();
		}

		array<SCR_AIGroup> playableGroups = GetAllPlayableGroups();
		for (int i = playableGroups.Count() - 1; i >= 0; i--)
		{
			Widget groupEntry = GetGame().GetWorkspace().CreateWidgets("{898A1945FD29FC71}UI/layouts/Debug/GroupsGroupEntry.layout", groupsLayout);
			RichTextWidget groupName = RichTextWidget.Cast(groupEntry.FindAnyWidget("GroupName"));
			if (groupName)
				groupName.SetText(playableGroups[i].GetGroupID().ToString() + ": " + playableGroups[i].GetCallsignSingleString());

			array<int> playerIDs = playableGroups[i].GetPlayerIDs();
			int playersCount = playerIDs.Count();

			for (int j = 0; j < playersCount; j++)
			{
				HorizontalLayoutWidget characterEntry = HorizontalLayoutWidget.Cast(groupEntry.GetWorkspace().CreateWidgets("{952C36C11AF74465}UI/layouts/Debug/GroupsCharacterEntry.layout", groupEntry));
				TextWidget playerNameWidget = TextWidget.Cast(characterEntry.FindAnyWidget("CharacterName"));
				if (!playerNameWidget)
					continue;

				playerNameWidget.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerIDs[j]));
			}
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \return
	array<SCR_AIGroup> GetPlayableGroupsByFaction(Faction faction)
	{
		return m_mPlayableGroups.Get(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \return Sorted playable group by faction, sorted by faction and groupID
	array<SCR_AIGroup> GetSortedPlayableGroupsByFaction(Faction faction)
	{
		array<SCR_AIGroup> playableGroupsOrdered = {};
		array<SCR_AIGroup> playableGroups = m_mPlayableGroups.Get(faction);
		if (!playableGroups)
			return playableGroupsOrdered;

		int groupCount = playableGroups.Count();

		array<int> sortedGroupIds = {};
		for (int i = 0; i < groupCount; i++)
		{
			sortedGroupIds.Insert(playableGroups[i].GetGroupID());
		}

		sortedGroupIds.Sort();

		for (int i = 0; i < groupCount; i++)
		{
			playableGroupsOrdered.Insert(FindGroup(sortedGroupIds[i]));
		}

		return playableGroupsOrdered;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] outAllGroups
	void GetAllPlayableGroups(out array<SCR_AIGroup> outAllGroups)
	{
		array<SCR_AIGroup> allGroups = {};
		array<SCR_AIGroup> currentGroups = {};
		for (int i = m_mPlayableGroups.Count() - 1; i >= 0; i--)
		{
			currentGroups = m_mPlayableGroups.GetElement(i);

			for (int j = currentGroups.Count() - 1; j >= 0; j--)
			{
				if (currentGroups[j])
					allGroups.Insert(currentGroups[j]);
			}
		}

		outAllGroups = allGroups;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] groupRole
	//! \param[in] faction
	//! \return Returns required character rank for given group role
	SCR_ECharacterRank GetRequiredRank(SCR_EGroupRole groupRole, notnull SCR_Faction faction)
	{
		array<SCR_GroupRolePresetConfig> groupRolePresetConfigs = {};
		faction.GetGroupRolePresetConfigs(groupRolePresetConfigs);

		foreach (SCR_GroupRolePresetConfig preset : groupRolePresetConfigs)
		{
			if (preset.GetGroupRole() == groupRole)
				return preset.GetRequiredRank();
		}

		return SCR_ECharacterRank.PRIVATE;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \param[in] creatableByPlayerFilter With this filter only returns groups that can be created by the player
	//! \return group roles from config
	array<SCR_EGroupRole> GetConfiguredGroupRoles(notnull Faction faction, bool creatableByPlayerFilter = false)
	{
		array<SCR_EGroupRole> availableGroupRoles = {};

		// get group roles from the config in predefined groups, a group that is not set in the config will not be visible.
		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if (!scrFaction)
			return availableGroupRoles;

		array<SCR_GroupRolePresetConfig> groupRolePresetConfigs = {};
		scrFaction.GetGroupRolePresetConfigs(groupRolePresetConfigs);

		foreach (SCR_GroupRolePresetConfig preset : groupRolePresetConfigs)
		{
			if (!creatableByPlayerFilter || creatableByPlayerFilter && preset.CanBeCreatedByPlayer())
				availableGroupRoles.Insert(preset.GetGroupRole());
		}

		return availableGroupRoles;
	}

	//------------------------------------------------------------------------------------------------
	//! \return array of SCR_EGroupRole, available group roles
	array<SCR_EGroupRole> GetAvailableGroupRoles(notnull Faction faction)
	{
		array<SCR_EGroupRole> availableGroupRoles = {};

		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if (!scrFaction)
			return availableGroupRoles;

		PlayerController pc = GetGame().GetPlayerController();
		int playerId;
		if (pc)
			playerId = pc.GetPlayerId();

		bool isLocalPlayerCommander = SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander();

		array<SCR_GroupRolePresetConfig> availableGroupRolePresetConfigs = {};
		scrFaction.GetGroupRolePresetConfigs(availableGroupRolePresetConfigs);
		foreach (SCR_GroupRolePresetConfig preset : availableGroupRolePresetConfigs)
		{
			if (!preset.CanBeCreatedByPlayer())
				continue;

			if (isLocalPlayerCommander || (!isLocalPlayerCommander && HasPlayerRequiredRank(preset, playerId, false)))
				availableGroupRoles.Insert(preset.GetGroupRole());
		}

		array<SCR_AIGroup> playableGroups = GetPlayableGroupsByFaction(faction);

		// faction commander can create all group roles
		if (isLocalPlayerCommander)
			return availableGroupRoles;

		for (int i = availableGroupRoles.Count() - 1; i >= 0; i--)
		{
			foreach (SCR_AIGroup group : playableGroups)
			{
				if (!group)
					continue;

				if (group.GetGroupRole() != availableGroupRoles[i])
					continue;

				if (group.IsPrivate())
					continue;

				// remove group role if is in group more members than half of it's max group size
				if (group.GetPlayerCount() > (group.GetMaxMembers() * 0.5))
					continue;

				availableGroupRoles.RemoveOrdered(i);
				break;
			}
		}

		return availableGroupRoles;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] groupRole
	//! \param[in] faction
	//! \return
	bool CanCreateGroupWithLocalPlayerRank(SCR_EGroupRole groupRole, notnull Faction faction)
	{
		PlayerController pc = GetGame().GetPlayerController();
		int playerId;
		if (pc)
			playerId = pc.GetPlayerId();

		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if (!scrFaction)
			return true;

		array<SCR_GroupRolePresetConfig> availableGroupRolePresetConfigs = {};
		scrFaction.GetGroupRolePresetConfigs(availableGroupRolePresetConfigs);
		foreach (SCR_GroupRolePresetConfig preset : availableGroupRolePresetConfigs)
		{
			if (preset.GetGroupRole() != groupRole || !preset.CanBeCreatedByPlayer())
				continue;

			if (HasPlayerRequiredRank(preset, playerId, false))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] groupRole
	//! \param[in] faction
	//! \return
	bool AreAllGroupsMajorityFull(SCR_EGroupRole groupRole, notnull Faction faction)
	{
		array<SCR_AIGroup> playableGroups = GetPlayableGroupsByFaction(faction);
		foreach (SCR_AIGroup group : playableGroups)
		{
			if (!group || group.IsPrivate() || group.GetGroupRole() != groupRole)
				continue;

			if (group.GetPlayerCount() <= (group.GetMaxMembers() * 0.5))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void AssignGroupFrequency(notnull SCR_AIGroup group)
	{
		int frequency = 0;
		Faction groupFaction = group.GetFaction();

		frequency = GetFreeFrequency(groupFaction);
		if (frequency == -1)
			return;

		ClaimFrequency(frequency, groupFaction);
		group.SetRadioFrequency(frequency);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] group
	void DeleteGroupDelayed(SCR_AIGroup group)
	{
		if (m_aDeletionQueue.Contains(group))
			return;

		if (m_aDeletionQueue.Count() == 0)
			GetGame().GetCallqueue().Call(DeleteGroups);

		m_aDeletionQueue.Insert(group);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	//! \param[in] playerID
	void OnGroupPlayerRemoved(SCR_AIGroup group, int playerID)
	{
		// This script should only run on the server
		if (IsProxy())
			return;

		// Is empty?
		if (group.GetPlayerCount() > 0)
			return;

		//Can this group exist empty?
		if (!group.GetDeleteIfNoPlayer())
		{
			if (group.IsPrivacyChangeable() && group.IsPrivate())
				group.SetPrivate(false);

			return;
		}

		// Yes, can we delete it?
		array<SCR_AIGroup> playableGroups = GetPlayableGroupsByFaction(group.GetFaction());
		if (!playableGroups)
			return;

		DeleteGroupDelayed(group);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	//! \param[in] playerID
	void OnGroupPlayerAdded(SCR_AIGroup group, int playerID)
	{
		if (IsProxy())
			return;

		// Is group full?
		if (group.GetPlayerCount() < group.GetMaxMembers())
			return; // Group not full, we don't have to make any new groups

		// Group was full, we need to see if we have any other not full group
		Faction faction = group.GetFaction();
		if (!faction)
			return;

		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if (!scrFaction)
			return;

		array<SCR_AIGroup> groups = GetPlayableGroupsByFaction(faction);
		if (!groups)
			return;

		// No groups found for faction?
		int groupsCount = groups.Count();
		if (groupsCount == 0 && !scrFaction.GetCanCreateOnlyPredefinedGroups())
		{
			// Anyway we create a new one
			CreateNewPlayableGroup(faction);
			return;
		}

		for (int i = groupsCount - 1; i >= 0; i--)
		{
			// We are checking for full groups if at least one group is not full, we return, we don't have to create new one
			if (groups[i].GetPlayerCount() < group.GetMaxMembers())
				return;
		}

		// All the groups were full, create new one
		if (!scrFaction.GetCanCreateOnlyPredefinedGroups() && scrFaction.IsEnabledAutoGroupCreationWhenFull())
			CreateNewPlayableGroup(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] Group role preset
	//! \param[in] playerId
	//! \param[in] ignoreGroupRequiredRank
	//! \return whether player has required rank of group role preset (this is ignored if ignoreGroupRequiredRank is true) and required rank for any of the group role preset loadouts
	bool HasPlayerRequiredRank(SCR_GroupRolePresetConfig preset, int playerId, bool ignoreGroupRequiredRank)
	{
		if (playerId == 0)
			return true;

		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!pc)
			return true;

		SCR_PlayerXPHandlerComponent xpHandler = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));
		if (!xpHandler)
			return true;

		SCR_ECharacterRank playerRank = xpHandler.GetPlayerRankByXP();
		SCR_ECharacterRank requiredRank = preset.GetRequiredRank();

		// Check if player has required rank of the group role preset
		if (!ignoreGroupRequiredRank && playerRank < requiredRank)
			return false;

		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return true;

		// Check if player has required rank for at least one of the group preset loadout
		requiredRank = SCR_ECharacterRank.INVALID;
		array<ResourceName> loadouts = preset.GetLoadouts();
		Resource resource;
		SCR_EntityCatalogEntry entry;
		SCR_EntityCatalogLoadoutData data;
		foreach (ResourceName loadoutResource : loadouts)
		{
			resource = Resource.Load(loadoutResource);
			if (!resource)
				continue;

			entry = entityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.CHARACTER, resource.GetResource().GetResourceName());
			if (!entry)
				continue;

			data = SCR_EntityCatalogLoadoutData.Cast(entry.GetEntityDataOfType(SCR_EntityCatalogLoadoutData));
			if (!data)
				continue;

			requiredRank = Math.Min(requiredRank, data.GetRequiredRank());
		}

		return playerRank >= requiredRank;
	}

	//------------------------------------------------------------------------------------------------
	SCR_GroupRolePresetConfig FindGroupRolePresetConfig(notnull Faction faction, SCR_EGroupRole role)
	{
		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if (!scrFaction)
			return null;

		array<SCR_GroupRolePresetConfig> groupRolePresetConfigs = {};
		scrFaction.GetGroupRolePresetConfigs(groupRolePresetConfigs);
		SCR_GroupRolePresetConfig groupPreset;

		foreach (SCR_GroupRolePresetConfig preset : groupRolePresetConfigs)
		{
			if (preset.GetGroupRole() == role)
			{
				groupPreset = preset;
				break;
			}
		}

		return groupPreset;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a gadget of EGadgetType.RADIO
	private BaseRadioComponent GetCommunicationDevice(notnull IEntity controlledEntity)
	{
		SCR_GadgetManagerComponent gagdetManager = SCR_GadgetManagerComponent.Cast(controlledEntity.FindComponent(SCR_GadgetManagerComponent));
		if (!gagdetManager)
			return null;

		IEntity radio = gagdetManager.GetGadgetByType(EGadgetType.RADIO); //variable purpose = debug
		if (!radio)
			return null;

		return BaseRadioComponent.Cast(radio.FindComponent(BaseRadioComponent));
	}

	//------------------------------------------------------------------------------------------------
	private void TuneAgentsRadio(AIAgent agentEntity)
	{
		if (!agentEntity)
			return;

		SCR_AIGroup group = SCR_AIGroup.Cast(agentEntity.GetParentGroup());
		if (!group)
			return;

		BaseRadioComponent radio = GetCommunicationDevice(agentEntity.GetControlledEntity());

		if (!radio)
			return;

		if (m_bTuneAgentsRadioToGroupFrequency)
		{
			BaseTransceiver tsv = radio.GetTransceiver(0);
			if (tsv)
				tsv.SetFrequency(group.GetRadioFrequency());
		}

		// Set radio active channel to group default radio channel
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(agentEntity.GetControlledEntity());
		if (playerId <= 0)
			return;

		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!pc)
			return;

		SCR_VONController vonController = SCR_VONController.Cast(pc.FindComponent(SCR_VONController));
		if (!vonController)
			return;

		vonController.SetActiveChannel(group.GetDefaultActiveRadioChannel());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] child
	void OnGroupAgentAdded(AIAgent child)
	{
		GetGame().GetCallqueue().CallLater(TuneAgentsRadio, 1, false, child);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	//! \param[in] child
	void OnGroupAgentRemoved(SCR_AIGroup group, AIAgent child)
	{
		return; //For now we don't delete empty groups

		if (!group)
			return;

		// Is group empty?
		if (group.GetAgentsCount() > 0)
			return;

		// Group is empty, can we delete it?
		Faction faction = group.GetFaction();
		if (!faction)
			return;

		array<SCR_AIGroup> groups = GetPlayableGroupsByFaction(faction);
		if (!groups)
			return;

		// No groups found for faction?
		int groupsCount = groups.Count();
		if (groupsCount == 0)
			return;

		// If we find any other group, which is empty, we delete this group
		for (int i = groupsCount - 1; i >= 0; i--)
		{
			if (groups[i] == group)
				continue;

			if (groups[i].GetAgentsCount() == 0)
			{
				DeleteAndUnregisterGroup(group);
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] group
	void DeleteAndUnregisterGroup(notnull SCR_AIGroup group)
	{
		m_OnPlayableGroupRemoved.Invoke(group);
		UnregisterGroup(group);
		delete group;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] group
	void UnregisterGroup(notnull SCR_AIGroup group)
	{
		Faction faction = group.GetFaction();
		if (!faction)
			return;

		array<SCR_AIGroup> groups = GetPlayableGroupsByFaction(faction);
		if (!groups)
			return;

		int frequency = group.GetRadioFrequency();
		int foundGroupsWithFrequency = 0;
		array<SCR_AIGroup> existingGroups = {};

		existingGroups = GetPlayableGroupsByFaction(faction);
		if (existingGroups)
		{
			foreach (SCR_AIGroup checkedGroup : existingGroups)
			{
				if (checkedGroup && checkedGroup.GetRadioFrequency() == frequency)
					foundGroupsWithFrequency++;
			}
		}

		//if there is only our group with this frequency or none, release it before changing our frequency
		if (foundGroupsWithFrequency <= 1)
			ReleaseFrequency(frequency, faction);

		SCR_AIGroup slaveGroup = group.GetSlave();

		//in case the slave group doesnt have any AIs in it, delete
		//mourTodo: handle what the AIs should do in case their master group is deleted
		if (slaveGroup && slaveGroup.GetAgentsCount() <= 0)
			delete slaveGroup;

		if (groups.Find(group) >= 0)
			groups.RemoveItem(group);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] group
	void RegisterGroup(SCR_AIGroup group)
	{
		array<SCR_AIGroup> groups;
		if (!m_mPlayableGroups.Find(group.GetFaction(), groups))
		{
			// No array found, let's make one
			groups = {};
			m_mPlayableGroups.Insert(group.GetFaction(), groups);
		}

		if (groups.Find(group) < 0)
			groups.Insert(group);

		group.GetOnAgentAdded().Insert(OnGroupAgentAdded);
		group.GetOnAgentRemoved().Insert(OnGroupAgentRemoved);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupID
	//! \param[in] factionIndex
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetGroupFaction(RplId groupID, int factionIndex)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		Faction faction = factionManager.GetFactionByIndex(factionIndex);
		if (!faction)
			return;

		SCR_AIGroup group = SCR_AIGroup.Cast(Replication.FindItem(groupID));
		if (group)
			group.SetFaction(faction);
	}

	//------------------------------------------------------------------------------------------------
	void AssignGroupID(SCR_AIGroup group)
	{
		group.SetGroupID(m_iLatestGroupID);
		m_iLatestGroupID++;
	}

	//------------------------------------------------------------------------------------------------
	//! Is called only on the server (authority)
	void OnPlayerFactionChanged(notnull FactionAffiliationComponent owner, Faction previousFaction, Faction newFaction)
	{
		if (!newFaction)
			return;

		SCR_Faction scrFaction = SCR_Faction.Cast(newFaction);
		if (!scrFaction)
			return;
		if (scrFaction.GetCanCreateOnlyPredefinedGroups())
			return;

		SCR_AIGroup newPlayerGroup = GetFirstNotFullForFaction(newFaction);
		if (!newPlayerGroup && scrFaction.IsEnabledAutoGroupCreationWhenFull())
		{
			newPlayerGroup = CreateNewPlayableGroup(newFaction);

			// Automatically created ones without any ai players in them later are irrlevant for saving
			if (newPlayerGroup)
				newPlayerGroup.SetPredefinedGroup(true);
		}

		//group creation can fail
		if (!newPlayerGroup || !owner)
			return;

		PlayerController controller = PlayerController.Cast(owner.GetOwner());
		if (!controller)
			return;

		SCR_PlayerControllerGroupComponent groupComp = SCR_PlayerControllerGroupComponent.Cast(controller.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!groupComp)
			return;

		SCR_AIGroup oldGroup = FindGroup(groupComp.GetGroupID());
		if (!oldGroup)
			return;

		oldGroup.RemovePlayer(controller.GetPlayerId());
	}

	//------------------------------------------------------------------------------------------------
	//! Called on clients (proxies) to notice a playable group has been created
	void OnGroupCreated(SCR_AIGroup group)
	{
		m_OnPlayableGroupCreated.Invoke(group);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] faction
	//! \return
	SCR_AIGroup TryFindEmptyGroup(notnull Faction faction)
	{
		array<SCR_AIGroup> factionGroups = GetPlayableGroupsByFaction(faction);
		if (!factionGroups)
			return null;

		for (int i = factionGroups.Count() - 1; i >= 0; i--)
		{
			if (!factionGroups[i])
				return null;

			if (factionGroups[i].GetPlayerCount() == 0)
				return factionGroups[i];
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] faction
	//! \param[in] groupRole
	//! \return
	SCR_AIGroup CreateNewPlayableGroup(Faction faction, SCR_EGroupRole groupRole = SCR_EGroupRole.NONE)
	{
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rplComp)
			return null;
		// TO DO: Commented out coz of initial replication

		//bool isMaster = rplComp.IsMaster();
		//if (!rplComp.IsMaster()) // TO DO: Commented out coz of initial replication
		// 	return null;

		Resource groupResource = Resource.Load(m_sDefaultGroupPrefab);
		if (!groupResource.IsValid())
			return null;

		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(groupResource, GetOwner().GetWorld()));
		if (!group)
			return null;

		group.DeactivateAI();

		group.SetFaction(faction);
		RegisterGroup(group);
		AssignGroupFrequency(group);
		AssignGroupID(group);
		group.SetGroupRole(groupRole);

		//if there is commanding present, we create the slave group for AIs at the creation of the group
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (commandingManager)
		{
			IEntity groupEntity = GetGame().SpawnEntityPrefab(Resource.Load(commandingManager.GetGroupPrefab()));
			if (!groupEntity)
				return null;

			SCR_AIGroup slaveGroup = SCR_AIGroup.Cast(groupEntity);
			if (!slaveGroup)
				return null;

			slaveGroup.DeactivateAI();

			RplComponent RplComp = RplComponent.Cast(slaveGroup.FindComponent(RplComponent));
			if (!RplComp)
				return null;

			RplId slaveGroupRplID = RplComp.Id();

			RplComp = RplComponent.Cast(group.FindComponent(RplComponent));
			if (!RplComp)
				return null;

			RequestSetGroupSlave(RplComp.Id(), slaveGroupRplID);
		}

		m_OnPlayableGroupCreated.Invoke(group);

#ifdef DEBUG_GROUPS
		GetGame().GetCallqueue().CallLater(UpdateDebugUI, 1, false);
#endif

		return group;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \param[in] ownGroup
	//! \param[in] respectPrivate
	//! \return
	SCR_AIGroup GetFirstNotFullForFaction(notnull Faction faction, SCR_AIGroup ownGroup = null, bool respectPrivate = false)
	{
		SCR_AIGroup group;
		array<SCR_AIGroup> factionGroups = GetPlayableGroupsByFaction(faction);
		if (!factionGroups)
			return group;

		for (int i = 0, count = factionGroups.Count(); i < count; i++)
		{
			if (!factionGroups[i].IsFull() && factionGroups[i] != ownGroup && (!respectPrivate || !factionGroups[i].IsPrivate()))
			{
				group = factionGroups[i];
				break;
			}
		}

		return group;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] newGroupFaction
	//! \return
	bool CanCreateNewGroup(notnull Faction newGroupFaction)
	{
		if (!m_bNewGroupsAllowed)
			return false;

		SCR_Faction scrFaction = SCR_Faction.Cast(newGroupFaction);
		if (scrFaction.GetCanCreateOnlyPredefinedGroups())
			return false;

		if (GetFreeFrequency(newGroupFaction) == -1)
			return false;

		if (scrFaction.IsGroupRolesConfigured())
		{
			array<SCR_EGroupRole> availableGroupRoles = GetAvailableGroupRoles(newGroupFaction);
			if (availableGroupRoles.IsEmpty())
				return false;
		}
		else
		{
			SCR_AIGroup playerGroup = GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());

			// disable creation of new group if player is the last in his group as there is no reason to create new one
			if (playerGroup && playerGroup.GetPlayerCount() == 1)
				return false;

			if (TryFindEmptyGroup(newGroupFaction))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool CanPlayersChangeAttributes()
	{
		return m_bCanPlayersChangeAttributes;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] frequencyFaction
	//! \return
	int GetFreeFrequency(Faction frequencyFaction)
	{
		FactionHolder usedForFactions = new FactionHolder();

		int factionHQFrequency; // Don't assign this frequency to any of the groups
		SCR_Faction militaryFaction = SCR_Faction.Cast(frequencyFaction);
		if (militaryFaction)
			factionHQFrequency = militaryFaction.GetFactionRadioFrequency();

		int minFrequencyAvailable = m_iPlayableGroupFrequencyMin;

		while (minFrequencyAvailable <= m_iPlayableGroupFrequencyMax)
		{
			if (minFrequencyAvailable == factionHQFrequency)
			{
				minFrequencyAvailable += m_iPlayableGroupFrequencyOffset;
				continue; // We cannot assign this frequency to any group, it is used by the factions HQ
			}

			if (!m_mUsedFrequenciesMap.Find(minFrequencyAvailable, usedForFactions))
				break; // No assigned frequencies for this faction yet

			if (usedForFactions.Find(frequencyFaction) == -1)
				break; // Unused frequency found

			minFrequencyAvailable += m_iPlayableGroupFrequencyOffset;
		}

		if (minFrequencyAvailable <= m_iPlayableGroupFrequencyMax)
			return minFrequencyAvailable;

		Print("Ran out of frequencies for groups", LogLevel.WARNING);
		return -1;

	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] frequency
	//! \param[in] faction
	void ClaimFrequency(int frequency, Faction faction)
	{
		FactionHolder usedForFactions = new FactionHolder();
		FactionHolder factions = new FactionHolder();

		if (!m_mUsedFrequenciesMap.Find(frequency, usedForFactions))
		{
			factions.Insert(faction);
			m_mUsedFrequenciesMap.Insert(frequency, factions);
			return;
		}
		if (usedForFactions.Find(faction) == -1)
		{
			usedForFactions.Insert(faction);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] frequency
	//! \param[in] faction
	//! \return
	bool IsFrequencyClaimed(int frequency, Faction faction)
	{
		FactionHolder usedForFactions = new FactionHolder();
		if (!m_mUsedFrequenciesMap.Find(frequency, usedForFactions))
			return false;

		if (usedForFactions.Find(faction))
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] frequency
	//! \param[in] faction
	void ReleaseFrequency(int frequency, Faction faction)
	{
		if (!faction || !frequency)
			return;

		FactionHolder factions = new FactionHolder();
		if (m_mUsedFrequenciesMap.Find(frequency, factions))
		{
			if (factions.Count() <= 1 && factions.Find(faction) != -1)
				m_mUsedFrequenciesMap.Remove(frequency);
			else
			{
				int factionIdx = factions.Find(faction);
				if (factionIdx >= 0 && factionIdx < factions.Count())
					factions.Remove(factionIdx);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] compID
	//! \param[in] slaveID
	void RequestSetGroupSlave(RplId compID, RplId slaveID)
	{
		RPC_DoSetGroupSlave(compID, slaveID);
		//Call next method later to be sure that SCR_AIGroup was replicated to the client succesfully (temporary solution)
		GetGame().GetCallqueue().CallLater(RpcWrapper, 2000, false, compID, slaveID);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] compID
	//! \param[in] slaveID
	void RpcWrapper(RplId compID, RplId slaveID)
	{
		Rpc(RPC_DoSetGroupSlave, compID, slaveID);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] masterGroupID
	//! \param[in] slaveGroupID
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetGroupSlave(RplId masterGroupID, RplId slaveGroupID)
	{
		SCR_AIGroup masterGroup, group;
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(masterGroupID));
		if (!rplComp)
			return;

		masterGroup = SCR_AIGroup.Cast(rplComp.GetEntity());
		if (!masterGroup)
			return;

		rplComp = RplComponent.Cast(Replication.FindItem(slaveGroupID));
		if (!rplComp)
			return;

		group = SCR_AIGroup.Cast(rplComp.GetEntity());
		if (!group)
			return;

		masterGroup.SetSlave(group);
		group.GetOnAgentRemoved().Insert(OnAIMemberRemoved);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupRplCompID
	//! \param[in] aiCharacterComponentID
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoRemoveAIMemberFromGroup(RplId groupRplCompID, RplId aiCharacterComponentID)
	{
		SCR_ChimeraCharacter AIMember;
		array<SCR_ChimeraCharacter> AIMembers;
		GetAIMembers(groupRplCompID, aiCharacterComponentID, AIMembers, AIMember);
		if (!AIMembers || !AIMember)
			return;

		AIMember.SetRecruited(false);
		AIMembers.RemoveItem(AIMember);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] groupRplCompID
	//! \param[in] aiCharacterComponentID
	//! \param[out] members
	//! \param[out] AIMember
	void GetAIMembers(RplId groupRplCompID, RplId aiCharacterComponentID, out array<SCR_ChimeraCharacter> members, out SCR_ChimeraCharacter AIMember)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(aiCharacterComponentID));
		if (!rplComp)
			return;

		AIMember = SCR_ChimeraCharacter.Cast(rplComp.GetEntity());
		if (!AIMember)
			return;

		rplComp = RplComponent.Cast(Replication.FindItem(groupRplCompID));
		if (!rplComp)
			return;

		SCR_AIGroup group = SCR_AIGroup.Cast(rplComp.GetEntity());
		if (!group)
			return;

		members = group.GetAIMembers();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupRplCompID
	//! \param[in] aiCharacterComponentID
	void AskRemoveAiMemberFromGroup(RplId groupRplCompID, RplId aiCharacterComponentID)
	{
		RPC_DoRemoveAIMemberFromGroup(groupRplCompID, aiCharacterComponentID);
		Rpc(RPC_DoRemoveAIMemberFromGroup, groupRplCompID, aiCharacterComponentID);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupRplCompID
	//! \param[in] aiCharacterComponentID
	void AskAddAiMemberToGroup(RplId groupRplCompID, RplId aiCharacterComponentID)
	{
		RPC_DoAddAIMemberToGroup(groupRplCompID, aiCharacterComponentID);
		Rpc(RPC_DoAddAIMemberToGroup, groupRplCompID, aiCharacterComponentID);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupRplCompID
	//! \param[in] aiCharacterComponentID
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoAddAIMemberToGroup(RplId groupRplCompID, RplId aiCharacterComponentID)
	{
		SCR_ChimeraCharacter AIMember;
		array<SCR_ChimeraCharacter> AIMembers;
		GetAIMembers(groupRplCompID, aiCharacterComponentID, AIMembers, AIMember);
		if (!AIMembers || !AIMember)
			return;

		AIMember.SetRecruited(true);
		AIMembers.Insert(AIMember);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	//! \param[in] agent
	void OnAIMemberRemoved(SCR_AIGroup group, AIAgent agent)
	{
		if (!group || !agent)
			return;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(agent.GetControlledEntity());
		if (!character)
			return;

		RplId groupCompRplID, characterCompRplID;
		RplComponent rplComp = RplComponent.Cast(group.FindComponent(RplComponent));
		if (!rplComp)
			return;

		groupCompRplID = rplComp.Id();

		rplComp = RplComponent.Cast(character.FindComponent(RplComponent));
		if (!rplComp)
			return;

		characterCompRplID = rplComp.Id();
		AskRemoveAiMemberFromGroup(groupCompRplID, characterCompRplID);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetConfirmedByPlayer()
	{
		return m_bConfirmedByPlayer;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsGroupMenuAllowed()
	{
		return m_bAllowGroupMenu;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetNewGroupsAllowed()
	{
		return m_bNewGroupsAllowed;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] isConfirmed
	void SetConfirmedByPlayer(bool isConfirmed)
	{
		m_bConfirmedByPlayer = isConfirmed;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void DeleteGroups()
	{
		for (int i = m_aDeletionQueue.Count() - 1; i >= 0; i--)
		{
			DeleteAndUnregisterGroup(m_aDeletionQueue[i]);
			m_aDeletionQueue.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerId
	//! \param[in] player
	void TunePlayersFrequency(int playerId, IEntity player)
	{
		PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!controller)
			return;

		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(player);
		if (!gadgetManager)
			return;

		IEntity radio = gadgetManager.GetGadgetByType(EGadgetType.RADIO);
		if (!radio)
			return;

		BaseRadioComponent radioComponent = BaseRadioComponent.Cast(radio.FindComponent(BaseRadioComponent));
		if (!radioComponent)
			return;

		SCR_PlayerControllerGroupComponent groupComponent = SCR_PlayerControllerGroupComponent.Cast(controller.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!groupComponent || groupComponent.GetActualGroupFrequency() == 0)
			return;

		BaseTransceiver transceiver = radioComponent.GetTransceiver(0);
		if (!transceiver)
			return;

		transceiver.SetFrequency(groupComponent.GetActualGroupFrequency());
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerRegistered(int playerId)
	{
		if (!m_pGameMode.IsMaster())
			return;

		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		SCR_PlayerFactionAffiliationComponent playerFactionAffiliation = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (playerFactionAffiliation)
			playerFactionAffiliation.GetOnFactionChanged().Insert(OnPlayerFactionChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		// TODO@AS: Ensure authority only
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (playerController)
		{
			SCR_PlayerFactionAffiliationComponent playerFactionAffiliation = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));
			if (playerFactionAffiliation)
				playerFactionAffiliation.GetOnFactionChanged().Remove(OnPlayerFactionChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] reconnectData
	void OnPlayerReconnected(SCR_ReconnectData reconnectData)
	{
		int playerID = reconnectData.m_iPlayerId;
		SCR_AIGroup group = GetPlayerGroup(playerID);
		if (group)
			return;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		Faction faction = factionManager.GetPlayerFaction(playerID);
		if (!faction)
			return;

		group = GetFirstNotFullForFaction(faction);
		if (group)
		{
			AddPlayerToGroup(group.GetGroupID(), playerID);
			return;
		}

		group = CreateNewPlayableGroup(faction);
		if (!group)
			return;

		AddPlayerToGroup(group.GetGroupID(), playerID);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_AIGroup.GetOnPlayerAdded().Insert(OnGroupPlayerAdded);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(OnGroupPlayerRemoved);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(ClearRequests);

		SCR_BaseGameMode gameMode = GetGameMode();

		if (m_bTunePlayersRadioToGroupFrequency)
			gameMode.GetOnPlayerSpawned().Insert(TunePlayersFrequency);

		SCR_ReconnectComponent reconnectComp = SCR_ReconnectComponent.Cast(gameMode.FindComponent(SCR_ReconnectComponent));
		if (reconnectComp)
			reconnectComp.GetOnReconnect().Insert(OnPlayerReconnected);

		m_bConfirmedByPlayer = false;

		if (IsProxy())
			return;

		// call later because the SCR_MapMarkerManagerComponent is not yet initted, which creates dynamic markers via SCR_MapMarkerEntrySquadLeader
		// Also call later so save game data is applied if needed
		gameMode.GetOnGameStart().Insert(CreatePredefinedGroups);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_GroupsManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (!s_Instance)
			s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_GroupsManagerComponent()
	{
#ifdef DEBUG_GROUPS
		if (s_wDebugLayout)
			s_wDebugLayout.RemoveFromHierarchy();
#endif
	}
}
