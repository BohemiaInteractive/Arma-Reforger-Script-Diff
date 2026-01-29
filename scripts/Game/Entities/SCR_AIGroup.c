//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----

void ScriptInvokerAIGroup(SCR_AIGroup group);
typedef func ScriptInvokerAIGroup;

void ScriptInvokerGroupRoleMethod(int groupID, SCR_EGroupRole groupRole);
typedef func ScriptInvokerGroupRoleMethod;
typedef ScriptInvokerBase<ScriptInvokerGroupRoleMethod> ScriptInvokerGroupRole;

[EntityEditorProps(category: "GameScripted/AI")]
class SCR_AIGroupClass : ChimeraAIGroupClass
{
	/*!
	Get group members and their offsets.
	\param entitySource Group's entity source
	\param[out] outPrefabs Array to be fillded with member prefabs
	\param[out] outOffsets Array to be fillded with local offsets
	\return Number of members
	*/
	static int GetMembers(IEntitySource entitySource, out array<ResourceName> outPrefabs, out array<vector> outOffsets)
	{
		//--- Not a group
		if (!entitySource || !entitySource.GetClassName().ToType().IsInherited(SCR_AIGroup)) return false;

		ArmaReforgerScripted game = GetGame();
		if (!game) return 0;

		AIWorld aiWorld = game.GetAIWorld();
		if (!aiWorld) return 0;

		//--- Get formation
		AIFormationDefinition formation;
		IEntityComponentSource componentSource;
		for (int i = 0, count = entitySource.GetComponentCount(); i < count; i++)
		{
			componentSource = entitySource.GetComponent(i);
			if (componentSource.GetClassName().ToType().IsInherited(AIFormationComponent))
			{
				string formationName;
				componentSource.Get("DefaultFormation", formationName);
				formation = aiWorld.GetFormation(formationName);
				break;
			}
		}
		if (!formation) return 0;

		//--- Get member prefabs
		entitySource.Get("m_aUnitPrefabSlots", outPrefabs);

		//--- Get offsets
		outOffsets.Clear();
		int count = outPrefabs.Count();
		for (int i = 0; i < count; i++)
		{
			outOffsets.Insert(formation.GetOffsetPosition(i));
		}

		return count;
	}
}

class SCR_AIGroup_DelayedSpawn
{
	bool snapToTerrain;
	int index;
	ResourceName resourceName;
	bool editMode;
}

void ScriptInvoker_AIGroupOnEmpty_Callback(AIGroup group);
typedef func ScriptInvoker_AIGroupOnEmpty_Callback;
typedef ScriptInvokerBase<ScriptInvoker_AIGroupOnEmpty_Callback> ScriptInvoker_AIGroupOnEmpty;

class SCR_AIGroup : ChimeraAIGroup
{
	[Attribute("", UIWidgets.EditBox, "Faction", category: "Group")]
	string m_faction;

	[Attribute(uiwidget: UIWidgets.ResourceAssignArray, desc: "Entities in group non-ai included", params: "et", category: "Group Members")]
	ref array<ResourceName> m_aUnitPrefabSlots;

	[Attribute(defvalue: "1", desc: "When true, group members will be spawned above terrain, offset by group's ATL height.\nWhen false, group members will be levelled horizontally with the group.", category: "Group Members")]
	private bool m_bSnapToTerrain;

	[Attribute("", UIWidgets.EditBox, "List of Waypoint names found in the level", category: "Group Waypoints")]
	ref array<string> m_aStaticWaypoints;

	[Attribute(defvalue: "", UIWidgets.Object, desc: "Waypoints that should be spawned from prefabs", category: "Group Waypoints")]
	ref array<ref SCR_WaypointPrefabLocation> m_aSpawnedWaypoints;

	[Attribute(defvalue: "", UIWidgets.EditBox, desc: "List of vehicles to use for movement", category: "Group Vehicles")]
	ref array<string> m_aStaticVehicles;

	[Attribute(defvalue: "1", desc: "When true, group members will be spawned durin OnInit, if false spawning must be called manually calling SpawnUnits()", category: "Group Members")]
	private bool m_bSpawnImmediately;

	[Attribute(defvalue: "0", UIWidgets.EditBox, desc: "Delay between spawns of individual members (ms)", category: "Group Members")]
	protected int m_fMemberSpawnDelay;

	[Attribute(defvalue: "1", UIWidgets.EditBox, desc: "When enabled, the group will be deleted when its last member dies or is deleted.\nThis will *not* delete the group when it starts empty.", category: "Group")]
	protected bool m_bDeleteWhenEmpty;

	protected static bool s_bIgnoreSnapToTerrain;
	protected static bool s_bIgnoreSpawning;

	protected ref array<IEntity> m_aSceneGroupUnitInstances;
	protected ref array<IEntity> m_aSceneWaypointInstances;
	protected ref array<BaseCompartmentSlot> m_aAllocatedCompartments = {};	// allocated components (on vehicles) this group wants to get in
	protected ref array<AISmartActionComponent> m_aAllocatedComponents = {};	// allocated smart action components this group wants to use

	protected int m_iMaxUnitsToSpawn = int.MAX;
	protected ref ScriptInvoker Event_OnInit;
	protected ref ScriptInvoker_AIGroupOnEmpty Event_OnEmpty;
	protected ref ScriptInvoker Event_OnAgentAdded;
	protected ref ScriptInvoker Event_OnAgentRemoved;
	protected ref ScriptInvoker Event_OnLeaderChanged;
	protected ref ScriptInvoker Event_OnCurrentWaypointChanged;
	protected ref ScriptInvoker Event_OnWaypointCompleted;
	protected ref ScriptInvoker Event_OnWaypointAdded;
	protected ref ScriptInvoker Event_OnWaypointRemoved;
	protected ref ScriptInvoker Event_OnWaypointToRemove;
	protected ref ScriptInvoker Event_OnFactionChanged;

	protected ref ScriptInvoker Event_OnGroupMemberStateChange;

	protected int m_iNumOfMembersToSpawn;

	protected ref array<int> m_aAgentIDQueue = {};

	//player groups variables
	[Attribute(category: "Player settings")]
	protected int m_iGroupRadioFrequency;

	[Attribute(category: "Player settings", params: "1 100 1")]
	protected int m_iMaxMembers;

	[Attribute("0", desc: "Can players join this group?", category: "Player settings")]
	protected bool m_bPlayable, m_bPrivate;

	[Attribute("1", desc: "Does this group get deleted when empty?", category: "Player settings")]
	protected bool m_bDeleteIfNoPlayer;

	protected int m_iGroupID = -1;
	protected int m_iLeaderID = -1;
	protected ref SCR_AIGroupUIInfo m_UiInfo;
	protected SCR_EGroupRole m_eGroupRole;
	protected bool m_bIsPrivacyChangeable = 1;
	protected bool m_bIsCreatedByCommander;
	protected bool m_bIsPredefinedGroup;

	//gamecode uses 0 as invalid playerID
	protected int m_iDescriptionAuthorID = 0;
	protected int m_iNameAuthorID = 0;
	protected int m_iDefaultActiveRadioChannel;

	protected string m_sCustomName = "";
	protected string m_sCustomDescription = "";
	protected ref array<int> m_aPlayerIDs = {};
	protected ref array<int> m_aDisconnectedPlayerIDs;
	protected static ref ScriptInvoker s_OnPlayerAdded = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnPlayerRemoved = new ScriptInvoker();
	protected static ref ScriptInvoker<int, int> s_OnPlayerLeaderChanged = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnPrivateGroupChanged = new ScriptInvoker();
	protected static ref ScriptInvoker<SCR_AIGroup> s_OnCustomNameChanged = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnFrequencyChanged = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnMaxMembersChanged = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnCustomDescChanged = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnFlagSelected = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnJoinPrivateGroupRequest = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnJoinPrivateGroupConfirm = new ScriptInvoker();
	protected static ref ScriptInvoker s_OnJoinPrivateGroupCancel = new ScriptInvoker();
	protected static ref ScriptInvokerGroupRole s_OnGroupRoleChanged;
	protected static ref ScriptInvokerBase<ScriptInvokerAIGroup> s_OnGroupRallyPointChanged;
	
	protected ref SCR_ScriptProfanityFilterRequestCallback m_ProfanityCallbackName;
	protected ref SCR_ScriptProfanityFilterRequestCallback m_ProfanityCallbackDesc;

	protected ref array<int> m_aRequesterIDs = {};
	protected ref array<int> m_aDeniedRequesters = {};

	[RplProp()]
	protected int m_iDeployedRadioCount = 0;

	protected int m_RallyPointId = -1;
	protected bool m_bForcedRallyPoint;

	//commanding variables
	protected SCR_AIGroup m_SlaveGroup;
	protected SCR_AIGroup m_MasterGroup;
	protected ref array<SCR_ChimeraCharacter> m_aAIMembers = {};
	protected SCR_AIGroupUtilityComponent m_GroupUtilityComponent;

	// entity spawn list
	protected ref array<ref SCR_AIGroup_DelayedSpawn> m_delayedSpawnList = {};
	protected ref ScriptInvokerBase<ScriptInvokerAIGroup> Event_OnAllDelayedEntitySpawned;

	protected SCR_ECharacterRank m_eRequiredRank;

	//------------------------------------------------------------------------------------------------
	//Returns number of AI's that still have to be spawned
	int GetSpawnQueueSize()
	{
		return m_delayedSpawnList.Count();
	}

	//------------------------------------------------------------------------------------------------
	SCR_AIGroupUtilityComponent GetGroupUtilityComponent()
	{
		return m_GroupUtilityComponent;
	}

	//------------------------------------------------------------------------------------------------
	int GetNumberOfMembersToSpawn()
	{
		return m_iNumOfMembersToSpawn;
	}

	//------------------------------------------------------------------------------------------------
	void SetNumberOfMembersToSpawn(int number)
	{
		m_iNumOfMembersToSpawn = number;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the group Rally Point to the input base
	//! \param[in] base to set rally point to
	//! \param[in] if rally point should be force set or not
	void SetRallyPoint(notnull SCR_MilitaryBaseComponent base, bool force = false)
	{
		if (!force && m_bForcedRallyPoint)
			return;

		int baseCallsign = base.GetCallsign();

		RpcDo_SetRallyPoint(baseCallsign, force);
		Rpc(RpcDo_SetRallyPoint, baseCallsign, force);
	}

	//------------------------------------------------------------------------------------------------
	//! Removes current group Rally Point
	void RemoveRallyPoint()
	{
		RpcDo_RemoveRallyPoint();
		Rpc(RpcDo_RemoveRallyPoint);
	}

	//------------------------------------------------------------------------------------------------
	//! \return Rally Point callsign ID
	int GetRallyPointId()
	{
		return m_RallyPointId;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Rally Point force state
	bool IsRallyPointForced()
	{
		return m_bForcedRallyPoint;
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		// No more entities in the list? Turn off frame events
		if (m_delayedSpawnList.IsEmpty())
		{
			EndDelayedSpawn();
			return;
		}

		// Spawn a singular AI entity this frame
		int spawnIndex = m_delayedSpawnList.Count() - 1;
		if (SpawnDelayedGroupMember(spawnIndex))
			m_delayedSpawnList.Remove(spawnIndex);

		// Notify all delayed spawning is done
		if (m_delayedSpawnList.IsEmpty() && Event_OnAllDelayedEntitySpawned)
			Event_OnAllDelayedEntitySpawned.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	/*
	Returns false when member couldn't be spawned but we should try again
	True otherwise
	*/
	bool SpawnDelayedGroupMember(int spawnIndex)
	{
		return SpawnGroupMember(
			m_delayedSpawnList.Get(spawnIndex).snapToTerrain,
			m_delayedSpawnList.Get(spawnIndex).index,
			m_delayedSpawnList.Get(spawnIndex).resourceName,
			m_delayedSpawnList.Get(spawnIndex).editMode,
			spawnIndex == 0 // isLast
		);
	}

	//------------------------------------------------------------------------------------------------
	void SpawnAllImmediately()
	{
		for (int spawnIndex = m_delayedSpawnList.Count() - 1; spawnIndex >= 0; spawnIndex--)
			SpawnDelayedGroupMember(spawnIndex);

		m_delayedSpawnList.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if group is in initialization state, for instance while still spawning its members
	bool IsInitializing()
	{
		return !m_delayedSpawnList.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	void BeginDelayedSpawn()
	{
		SetEventMask(EntityEvent.FRAME);
		DeactivateAI();

		// Allow this entity to frame/tick while in Game Master Mode
		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			world.RegisterEntityToBeUpdatedWhileGameIsPaused(this);
	}

	//------------------------------------------------------------------------------------------------
	void EndDelayedSpawn()
	{
		ClearEventMask(EntityEvent.FRAME);
		if (m_bDeleteWhenEmpty && GetAgentsCount() == 0)
			GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, 1, false, this);
		else if (!m_bPlayable && !m_MasterGroup)
			ActivateAI();

		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			world.UnregisterEntityToBeUpdatedWhileGameIsPaused(this);
	}

	//------------------------------------------------------------------------------------------------
	bool HasRequesterID(int id)
	{
		return m_aRequesterIDs.Contains(id);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveRequester(int playerID)
	{
		if (!m_aRequesterIDs.Contains(playerID))
			return;

		RPC_DoRemoveRequester(playerID);
		Rpc(RPC_DoRemoveRequester, playerID);
	}

	//------------------------------------------------------------------------------------------------
	int GetDeniedRequesters(out array<int> valueArray)
	{
		return valueArray.Copy(m_aDeniedRequesters);
	}

	//------------------------------------------------------------------------------------------------
	void AddDeniedRequester(int playerID)
	{
		if (m_aDeniedRequesters.Contains(playerID))
			return;

		RPC_DoAddDeniedRequester(playerID);
		Rpc(RPC_DoAddDeniedRequester, playerID);
	}

	//------------------------------------------------------------------------------------------------
	void ClearRequesters()
	{
		RPC_DoClearRequesterIDs();
		Rpc(RPC_DoClearRequesterIDs);
	}

	//------------------------------------------------------------------------------------------------
	void ClearDeniedRequester()
	{
		RPC_DoClearDeniedRequester();
		Rpc(RPC_DoClearDeniedRequester);
	}

	//------------------------------------------------------------------------------------------------
	int GetRequesterIDs(out array<int> valueArray)
	{
		return valueArray.Copy(m_aRequesterIDs);
	}

	//------------------------------------------------------------------------------------------------
	void AddRequester(int playerID)
	{
		if (m_aRequesterIDs.Contains(playerID))
			return;

		RPC_DoAddRequester(playerID);
		Rpc(RPC_DoAddRequester, playerID);
	}

	//------------------------------------------------------------------------------------------------
	void SetFlagIsFromImageSet(bool value)
	{
		if (!m_UiInfo)
			m_UiInfo = new SCR_AIGroupUIInfo();

		m_UiInfo.SetFlagIsFromImageSet(value);
	}

	//------------------------------------------------------------------------------------------------
	void SetCustomGroupFlag(ResourceName flag)
	{
		if (!m_UiInfo)
			m_UiInfo = new SCR_AIGroupUIInfo();

		m_UiInfo.SetGroupFlag(flag);
	}

	//------------------------------------------------------------------------------------------------
	bool GetFlagIsFromImageSet()
	{
		if (!m_UiInfo)
			m_UiInfo = new SCR_AIGroupUIInfo();

		bool flagIsFromSet = m_UiInfo.GetFlagIsFromImageSet();
		return flagIsFromSet;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetGroupFlag()
	{
		if (!m_UiInfo)
			m_UiInfo = new SCR_AIGroupUIInfo();

		ResourceName name = m_UiInfo.GetGroupFlag();
		return name;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnFlagSelected()
	{
		return s_OnFlagSelected;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnJoinPrivateGroupRequest()
	{
		return s_OnJoinPrivateGroupRequest;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnJoinPrivateGroupConfirm()
	{
		return s_OnJoinPrivateGroupConfirm;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnJoinPrivateGroupCancel()
	{
		return s_OnJoinPrivateGroupCancel;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerBase<ScriptInvokerAIGroup> GetOnGroupRallyPointChanged()
	{
		if (!s_OnGroupRallyPointChanged)
			s_OnGroupRallyPointChanged = new ScriptInvokerBase<ScriptInvokerAIGroup>(); 

		return s_OnGroupRallyPointChanged;
	}

	//------------------------------------------------------------------------------------------------
	bool IsPlayerInGroup(int playerID)
	{
		return m_aPlayerIDs.Contains(playerID);
	}

	//------------------------------------------------------------------------------------------------
	bool IsPlayerLeader(int playerID)
	{
		return playerID == m_iLeaderID;
	}

	//------------------------------------------------------------------------------------------------
	int GetLeaderID()
	{
		return m_iLeaderID;
	}

	//------------------------------------------------------------------------------------------------
	//! Recursively searches through the group hierarchy to find the first leader who is a player
	//! \return player ID that is used f.e. by PlayerManager, or -1 when there is no player leader in the group hierarchy
	int GetFirstPlayerLeaderID()
	{
		if (m_iLeaderID > 0)
			return m_iLeaderID;

		if (!m_MasterGroup)
			return -1;

		return m_MasterGroup.GetFirstPlayerLeaderID();
	}

	//------------------------------------------------------------------------------------------------
	int GetDefaultActiveRadioChannel()
	{
		return m_iDefaultActiveRadioChannel;
	}

	//------------------------------------------------------------------------------------------------
	void SetDefaultActiveRadioChannel(int id)
	{
		if (m_iDefaultActiveRadioChannel == id)
			return;

		m_iDefaultActiveRadioChannel = id;
	}

	//------------------------------------------------------------------------------------------------
	//! Called on the server (authority)
	void SetPrivate(bool isPrivate)
	{
		RPC_SetPrivate(isPrivate);
		Rpc(RPC_SetPrivate, isPrivate);
	}

	//------------------------------------------------------------------------------------------------
	bool IsPrivate()
	{
		return m_bPrivate;
	}

	//------------------------------------------------------------------------------------------------
	void SetPrivacyChangeable(bool IsPrivacyChangeable)
	{
		RPC_DoSetPrivacyChangeable(IsPrivacyChangeable);
		Rpc(RPC_DoSetPrivacyChangeable, IsPrivacyChangeable);
	}

	//------------------------------------------------------------------------------------------------
	bool IsPrivacyChangeable()
	{
		return m_bIsPrivacyChangeable;
	}

	//------------------------------------------------------------------------------------------------
	void SetGroupRole(SCR_EGroupRole groupRole)
	{
		RpcDo_SetGroupRole(groupRole);
		Rpc(RpcDo_SetGroupRole, groupRole);
	}

	//------------------------------------------------------------------------------------------------
	SCR_EGroupRole GetGroupRole()
	{
		return m_eGroupRole;
	}

	//------------------------------------------------------------------------------------------------
	//! \returns localized group role
	string GetGroupRoleName()
	{
		SCR_Faction scrFaction = SCR_Faction.Cast(GetFaction());
		if (!scrFaction)
			return string.Empty;

		array<SCR_GroupRolePresetConfig> groupRolePresets = {};
		scrFaction.GetGroupRolePresetConfigs(groupRolePresets);

		// find preset by group role
		foreach (SCR_GroupRolePresetConfig preset : groupRolePresets)
		{
			if (!preset || preset.GetGroupRole() != m_eGroupRole)
				continue;

			if (preset.GetGroupRoleName().IsEmpty())
				return SCR_Enum.GetEnumName(SCR_EGroupRole, preset.GetGroupRole());
			else
				return preset.GetGroupRoleName();
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if the group is set to be deleted when all players leave.
	bool GetDeleteIfNoPlayer()
	{
		return m_bDeleteIfNoPlayer;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets if the group should be deleted when all players leave.
	void SetCanDeleteIfNoPlayer(bool deleteEmpty)
	{
		m_bDeleteIfNoPlayer = deleteEmpty;
	}

	//------------------------------------------------------------------------------------------------
	bool IsFull()
	{
		return m_iMaxMembers <= m_aPlayerIDs.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool IsSlave()
	{
		if (!m_MasterGroup)
			return false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get the count of all players and AI in the group
	\param checkMasterAndSlaves If true then it will also check any attached Master and/or slave group
	\return Total group count of all AI and player member
	*/
	int GetPlayerAndAgentCount(bool checkMasterAndSlaves = false)
	{
		if (!checkMasterAndSlaves)
			return GetPlayerCount() + GetAgentsCount();

		//~ Return count of group and master and slave groups
		return GetPlayerCount(true) + GetAgentCountIncludingMasterAndSlaves();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	\return Get the count of all AI in the group as well as any attached Master and slave groups
	*/
	int GetAgentCountIncludingMasterAndSlaves()
	{
		int totalAgentCount = GetAgentsCount();

		SCR_AIGroup aiGroup = GetSlave();
		if (aiGroup)
			totalAgentCount += aiGroup.GetAgentsCount();

		aiGroup = GetMaster();
		if (aiGroup)
			totalAgentCount += aiGroup.GetAgentsCount();

		return totalAgentCount;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get the count of all players and AI in both the group itself as well as the slave/master group(s)
	\return Total group count of all AI and player member
	*/
	int GetTotalAgentCount()
	{
		int totalGroupCount = GetAgentsCount();

		SCR_AIGroup aiGroup = GetSlave();
		if (aiGroup)
			totalGroupCount += aiGroup.GetAgentsCount();

		aiGroup = GetMaster();
		if (aiGroup)
			totalGroupCount += aiGroup.GetAgentsCount();

		return totalGroupCount;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get the count of all players and AI in both the group itself as well as the slave/master group(s)
	\return Total group count of all AI and player member
	*/
	int GetTotalPlayerCount()
	{
		int totalGroupCount = GetPlayerCount();

		SCR_AIGroup aiGroup = GetSlave();
		if (aiGroup)
			totalGroupCount += aiGroup.GetPlayerCount();

		aiGroup = GetMaster();
		if (aiGroup)
			totalGroupCount += aiGroup.GetPlayerCount();

		return totalGroupCount;
	}

	//------------------------------------------------------------------------------------------------
	//! called on server only
	void SetGroupFlag(int flagIndex, bool isFromImageset)
	{
		if (!m_UiInfo)
			m_UiInfo = new SCR_AIGroupUIInfo();

		RPC_DoSetGroupFlag(flagIndex, isFromImageset);
		Rpc(RPC_DoSetGroupFlag, flagIndex, isFromImageset);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetGroupFlag(int flagIndex, bool isFromImageset)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_Faction scrFaction = SCR_Faction.Cast(GetFaction());
		if (!scrFaction)
			return;

		m_UiInfo.SetFlagIsFromImageSet(isFromImageset);

		if (isFromImageset)
		{
			if (flagIndex >= 0)
				m_UiInfo.SetGroupFlag(scrFaction.GetFlagName(flagIndex));
		}
		else
		{
			array<ResourceName> textures = {};
			scrFaction.GetGroupFlagTextures(textures);

			if (textures.IsIndexValid(flagIndex))
				m_UiInfo.SetGroupFlag(textures[flagIndex]);
		}

		s_OnFlagSelected.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	void SetCustomName(string name, int authorID)
	{
		RPC_DoSetCustomName(name, authorID);
		Rpc(RPC_DoSetCustomName, name, authorID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetCustomName(string name, int authorID)
	{
		if (name.IsEmpty())
		{
			m_sCustomName = name;
			m_iNameAuthorID = authorID;
			s_OnCustomNameChanged.Invoke(this);
			return;
		}

		array<string> textToFilter = {};

		m_ProfanityCallbackName = new SCR_ScriptProfanityFilterRequestCallback();
		textToFilter.Insert(name);
		m_ProfanityCallbackName.m_OnResult.Insert(OnNameFilteredCallback);



		if (!GetGame().GetPlatformService().FilterProfanityAsync(textToFilter, m_ProfanityCallbackName))
			OnNameFilteredCallback(textToFilter);

		m_iNameAuthorID = authorID;

	}

	void OnNameFilteredCallback(array<string> resultText)
	{
		if (GetGame().GetPlatformService().GetLocalPlatformKind() == PlatformKind.XBOX)
			SCR_ProfaneFilter.ReplaceProfanities(resultText.Get(0), m_sCustomName);
		else
			m_sCustomName = resultText.Get(0);

		s_OnCustomNameChanged.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	static bool DeLocalizeText(out string input)
	{
		if (input.IsEmpty())
			return false;
		string inputLC = input;
		inputLC.ToLower();

		string profanity = "#";

		int indexOf = inputLC.IndexOf(profanity);
		if (indexOf < 0)
			return false;

		int inputLength = input.Length();
		int profanityLength = profanity.Length();
		while (indexOf > -1)
		{
			if (indexOf > 0)
				input = input.Substring(0, indexOf) + input.Substring(indexOf + profanityLength, inputLength - profanityLength - indexOf);
			else
				input = input.Substring(profanityLength, inputLength - profanityLength);

			indexOf = inputLC.IndexOfFrom(indexOf + profanityLength, profanity);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! called on server only
	void SetCustomDescription(string desc, int authorID)
	{
		RPC_DoSetCustomDescription(desc, authorID);
		Rpc(RPC_DoSetCustomDescription, desc, authorID);
	}

	//------------------------------------------------------------------------------------------------
	void SetMaxGroupMembers(int value)
	{
		RPC_DoSetMaxGroupMembers(value);
		Rpc(RPC_DoSetMaxGroupMembers, value);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetMaxGroupMembers(int value)
	{
		m_iMaxMembers = value;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetCustomDescription(string desc, int authorID)
	{
		if (desc.IsEmpty())
		{
			m_sCustomDescription = desc;
			m_iDescriptionAuthorID = authorID;
			s_OnCustomDescChanged.Invoke(this);
			return;
		}

		array<string> textToFilter = {};

		m_ProfanityCallbackDesc = new SCR_ScriptProfanityFilterRequestCallback();
		textToFilter.Insert(desc);
		m_ProfanityCallbackDesc.m_OnResult.Insert(OnDescFilteredCallback);

		if (!GetGame().GetPlatformService().FilterProfanityAsync(textToFilter, m_ProfanityCallbackDesc))
			OnDescFilteredCallback(textToFilter);

		m_iDescriptionAuthorID = authorID;
	}

	void OnDescFilteredCallback(array<string> resultText)
	{
		if (GetGame().GetPlatformService().GetLocalPlatformKind() == PlatformKind.XBOX)
			SCR_ProfaneFilter.ReplaceProfanities(resultText.Get(0), m_sCustomDescription);
		else
			m_sCustomDescription = resultText.Get(0);

		s_OnCustomDescChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	string GetCustomDescription()
	{
		const PlayerController playerController = GetGame().GetPlayerController();
		if (playerController)
		{
			const SocialComponent socialComp = SocialComponent.Cast(playerController.FindComponent(SocialComponent));
			if (socialComp.IsRestricted(m_iDescriptionAuthorID, EUserInteraction.UserGeneratedContent))
				return string.Empty;
		}

		return m_sCustomDescription;
	}

	//------------------------------------------------------------------------------------------------
	string GetCustomName()
	{
		const PlayerController playerController = GetGame().GetPlayerController();
		if (playerController)
		{
			const SocialComponent socialComp = SocialComponent.Cast(playerController.FindComponent(SocialComponent));
			if (socialComp.IsRestricted(m_iNameAuthorID, EUserInteraction.UserGeneratedContent))
				return string.Empty;
		}

		return m_sCustomName;
	}

	//------------------------------------------------------------------------------------------------
	int GetDescriptionAuthorID()
	{
		return m_iDescriptionAuthorID;
	}

	//------------------------------------------------------------------------------------------------
	int GetNameAuthorID()
	{
		return m_iNameAuthorID;
	}

	//------------------------------------------------------------------------------------------------
	string GetCustomNameWithOriginal()
	{
		string company, platoon, squad, character, format;
		GetCallsigns(company, platoon, squad, character, format);
		string originalName = string.Format(format, company, platoon, squad, character);

		// added check for player controller, because playercontroler is not on dedicated server
		const PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return originalName;

		const SocialComponent socialComp = SocialComponent.Cast(playerController.FindComponent(SocialComponent));
		if (!socialComp)
			return originalName;

		if (m_sCustomName.IsEmpty() || socialComp.IsRestricted(m_iNameAuthorID, EUserInteraction.UserGeneratedContent))
			return originalName;

		return m_sCustomName + " ( " + originalName + " )";
	}

	//------------------------------------------------------------------------------------------------
	int GetGroupID()
	{
		return m_iGroupID;
	}

	//------------------------------------------------------------------------------------------------
	void SetGroupID(int id)
	{
		m_iGroupID = id;
	}

	//------------------------------------------------------------------------------------------------
	int GetMaxMembers()
	{
		return m_iMaxMembers;
	}

	//------------------------------------------------------------------------------------------------
	//! called on server only
	void SetMaxMembers(int maxMembers)
	{
		if (maxMembers == GetMaxMembers() || maxMembers < 0)
			return;

		RPC_DoSetMaxMembers(maxMembers);
		Rpc(RPC_DoSetMaxMembers, maxMembers);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetMaxMembers(int maxMembers)
	{
		m_iMaxMembers = maxMembers;
		s_OnMaxMembersChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	array<int> GetPlayerIDs()
	{
		return m_aPlayerIDs;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get the count of all players in the group
	\param checkMasterAndSlaves If true then it will also check any attached Master and/or slave groups
	\return Total count of players
	*/
	int GetPlayerCount(bool checkMasterAndSlaves = false)
	{
		if (!checkMasterAndSlaves)
			return m_aPlayerIDs.Count();

		int totalPlayerCount = GetPlayerCount();
		SCR_AIGroup aiGroup = GetSlave();
		if (aiGroup)
			totalPlayerCount += aiGroup.GetPlayerCount();

		aiGroup = GetMaster();
		if (aiGroup)
			totalPlayerCount += aiGroup.GetPlayerCount();

		return totalPlayerCount;
	}

	//------------------------------------------------------------------------------------------------
	//! \return required character rank
	SCR_ECharacterRank GetRequiredRank()
	{
		return m_eRequiredRank;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] rank
	void SetRequiredRank(SCR_ECharacterRank rank)
	{
		if (rank == m_eRequiredRank)
			return;

		RpcDo_SetRequiredRank(rank);
		Rpc(RpcDo_SetRequiredRank, rank);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_SetRequiredRank(SCR_ECharacterRank rank)
	{
		m_eRequiredRank = rank;
	}

	//------------------------------------------------------------------------------------------------
	bool IsCreatedByCommander()
	{
		return m_bIsCreatedByCommander;
	}

	//------------------------------------------------------------------------------------------------
	void SetCreatedByCommander(bool isCreatedByCommander)
	{
		RpcDo_SetCreatedByCommander(isCreatedByCommander);
		Rpc(RpcDo_SetCreatedByCommander, isCreatedByCommander);
	}
	//------------------------------------------------------------------------------------------------
	bool IsPredefinedGroup()
	{
		return m_bIsPredefinedGroup;
	}

	//------------------------------------------------------------------------------------------------
	void SetPredefinedGroup(bool predefined)
	{
		m_bIsPredefinedGroup = predefined;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_SetCreatedByCommander(bool isCreatedByCommander)
	{
		m_bIsCreatedByCommander = isCreatedByCommander;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMemberStateChange()
	{
		if (!Event_OnGroupMemberStateChange)
			Event_OnGroupMemberStateChange = new ScriptInvoker();

		return Event_OnGroupMemberStateChange;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnPlayerAdded()
	{
		return s_OnPlayerAdded;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnPlayerRemoved()
	{
		return s_OnPlayerRemoved;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnPlayerLeaderChanged()
	{
		return s_OnPlayerLeaderChanged;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnPrivateGroupChanged()
	{
		return s_OnPrivateGroupChanged;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnCustomNameChanged()
	{
		return s_OnCustomNameChanged;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnFrequencyChanged()
	{
		return s_OnFrequencyChanged;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnCustomDescriptionChanged()
	{
		return s_OnCustomDescChanged;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerGroupRole GetOnGroupRoleChanged()
	{
		if (!s_OnGroupRoleChanged)
			s_OnGroupRoleChanged = new ScriptInvokerGroupRole();

		return s_OnGroupRoleChanged;
	}

	//------------------------------------------------------------------------------------------------
	bool BelongedToGroup(int playerID)
	{
		return m_aDisconnectedPlayerIDs != null && m_aDisconnectedPlayerIDs.Contains(playerID);
	}

	//------------------------------------------------------------------------------------------------
	void AddAgentFromControlledEntity(notnull IEntity controlledEntity)
	{
		AIControlComponent aiControlComponent = AIControlComponent.Cast(controlledEntity.FindComponent(AIControlComponent));
		if (!aiControlComponent)
			return;

		AIAgent agent = aiControlComponent.GetAIAgent();
		if (!agent)
			return;

		AddAgent(agent);

		//we send notification to master group players
		if (IsSlave())
			NotificateGroupAIChange(controlledEntity, ENotification.GROUPS_AI_JOINED);

		OnGroupMemberStateChange();
	}

	//------------------------------------------------------------------------------------------------
	void RemoveAgentFromControlledEntity(notnull IEntity controlledEntity)
	{
		AIControlComponent aiControlComponent = AIControlComponent.Cast(controlledEntity.FindComponent(AIControlComponent));
		if (!aiControlComponent)
			return;

		AIAgent agent = aiControlComponent.GetAIAgent();
		if (!agent)
			return;
		RemoveAgent(agent);

		//we send notification to master group players
		if (IsSlave())
			NotificateGroupAIChange(controlledEntity, ENotification.GROUPS_AI_LEFT);

		OnGroupMemberStateChange();
	}

	//------------------------------------------------------------------------------------------------
	void NotificateGroupAIChange(IEntity controlledEntity, ENotification notificationType)
	{
		RplId rplId = -1;
		SCR_EditableEntityComponent editableEntityComp = SCR_EditableEntityComponent.Cast(controlledEntity.FindComponent(SCR_EditableEntityComponent));
		if (editableEntityComp)
			rplId = Replication.FindId(editableEntityComp);

		if (rplId.IsValid())
			SCR_NotificationsComponent.SendToGroup(GetMaster().GetGroupID(), notificationType, rplId);
	}

	//------------------------------------------------------------------------------------------------
	//! Should be only called on the server
	void OnPlayerDisconnected(int playerID)
	{
		// No need to null check m_aPlayerIDs, it always exists together with this entity
		int index = m_aPlayerIDs.Find(playerID);
		if (index < 0)
			return;

		// We have to check existence of m_aDisconnectedPlayerIDs, because we might be adding the first entry
		if (!m_aDisconnectedPlayerIDs)
			m_aDisconnectedPlayerIDs = {};

		RemovePlayer(playerID);
		m_aDisconnectedPlayerIDs.Insert(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Should be only called on the server
	void OnPlayerConnected(int playerID)
	{
		// No need to null check m_aDisconnectedPlayerIDs, this method is only called after entries are added to it
		int index = m_aDisconnectedPlayerIDs.Find(playerID);
		if (index < 0)
			return;

		m_aDisconnectedPlayerIDs.Remove(index);

		// Not full, we add the player, else bad luck, find a new group
		if (!IsFull())
			AddPlayer(playerID);

		// No more disconnected players from this group, let's stop listening
		if (m_aDisconnectedPlayerIDs.Count() == 0)
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode)
				gameMode.GetOnPlayerConnected().Remove(OnPlayerConnected);
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoOnGroupMemberStateChange()
	{
		if (Event_OnGroupMemberStateChange)
			GetGame().GetCallqueue().CallLater(Event_OnGroupMemberStateChange.Invoke, 1, false, null, null, null, null, null, null, null, null, null);
	}

	//------------------------------------------------------------------------------------------------
	//! Should only be called on the server (authority)
	void OnGroupMemberStateChange()
	{
		RPC_DoOnGroupMemberStateChange(); //Local call
		Rpc(RPC_DoOnGroupMemberStateChange); //Broadcast to clients
	}

	//------------------------------------------------------------------------------------------------
	void OnMemberDeath(notnull SCR_CharacterControllerComponent memberController, IEntity killerEntity, Instigator killer)
	{
		//This event is only called for members of the group, so it's safe to call QueueAddAgent automatically
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(memberController.GetCharacter());
		QueueAddAgent(playerID);
		RemovePlayerAgent(playerID);
		OnGroupMemberStateChange();
	}

	//------------------------------------------------------------------------------------------------
	void ListenToMemberDeath(notnull IEntity groupMember)
	{
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(groupMember.FindComponent(SCR_CharacterControllerComponent));
		if (!characterController)
			return;

		characterController.GetOnPlayerDeathWithParam().Insert(OnMemberDeath);
	}

	//------------------------------------------------------------------------------------------------
	void AddOnGadgetsLoadedListener(int playerID, notnull IEntity controlledEntity)
	{
		SCR_GadgetManagerComponent.GetOnGadgetInitDoneInvoker().Insert(OnControllableEntitySpawned);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerSpawned().Remove(AddOnGadgetsLoadedListener);
	}

	//------------------------------------------------------------------------------------------------
	void OnControllableEntitySpawned(IEntity controlledEntity, notnull SCR_GadgetManagerComponent gadgetManager)
	{
		if (!controlledEntity)
			return;

		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(controlledEntity);
		if (playerID == 0)
			return;

		int index = m_aAgentIDQueue.Find(playerID);
		if (index < 0)
			return;

		ListenToMemberDeath(controlledEntity);
		AddAgentFromControlledEntity(controlledEntity);
		m_aAgentIDQueue.Remove(index);

		if (!m_aAgentIDQueue.IsEmpty())
			return;

		SCR_GadgetManagerComponent.GetOnGadgetInitDoneInvoker().Remove(OnControllableEntitySpawned);
	}

	//------------------------------------------------------------------------------------------------
	void QueueAddAgent(int playerID)
	{
		// Avoiding duplicate entries
		m_aAgentIDQueue.Insert(playerID);
		if (m_aAgentIDQueue.Count() != 1)
			return;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		// Waiting for spawn of a controllable entity with correct ID
		gameMode.GetOnPlayerSpawned().Insert(AddOnGadgetsLoadedListener);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoAddPlayer(int playerID)
	{
		m_aPlayerIDs.Insert(playerID);
		s_OnPlayerAdded.Invoke(this, playerID);
	}

	//------------------------------------------------------------------------------------------------
	//!checks for a leader, if there is none, it will either use provided player as leader, or appoints next player in group as leader
	//! Called on the server (authority)
	void CheckForLeader(int playerID, bool noLeaderAllowed)
	{
		//check if leader is still in group, noleaderallowed is preparation for editor integration
		if (!IsPlayerInGroup(GetLeaderID()) && !noLeaderAllowed && playerID == -1)
		{
			if (m_aPlayerIDs.IsEmpty())
			{
				SetCustomName("", 0);
				SetCustomDescription("", 0);

				s_OnPlayerLeaderChanged.Invoke(m_iGroupID, -1);
				return;
			}
			SetGroupLeader(m_aPlayerIDs.Get(0));
			return;
		}

		//if we have a leader, do nothing
		if (GetLeaderID() != -1)
			return;

		//otherwise appoint provided player as a leader
		if (playerID != -1)
			SetGroupLeader(playerID);


	}

	//------------------------------------------------------------------------------------------------
	//! Called on the server (authority)
	void SetGroupLeader(int playerID)
	{
		//reset custom name and description on leader change
		if (GetNameAuthorID() != playerID)
			SetCustomName("", 0);

		if (GetDescriptionAuthorID() != playerID)
			SetCustomDescription("", 0);

		SCR_NotificationsComponent.SendToGroup(m_iGroupID, ENotification.GROUPS_PLAYER_PROMOTED_LEADER, playerID);
		RPC_SetLeaderID(playerID);
		Rpc(RPC_SetLeaderID, playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetLeaderID(int playerID)
	{
		if (playerID == m_iLeaderID)
			return;
		m_iLeaderID = playerID;
		s_OnPlayerLeaderChanged.Invoke(m_iGroupID, playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetPrivate(bool isPrivate)
	{
		m_bPrivate = isPrivate;
		s_OnPrivateGroupChanged.Invoke(m_iGroupID, isPrivate);

		if (!isPrivate)
		{
			ClearRequesters();
			ClearDeniedRequester();
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetPrivacyChangeable(bool IsPrivacyChangeable)
	{
		m_bIsPrivacyChangeable = IsPrivacyChangeable;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_SetGroupRole(SCR_EGroupRole groupRole)
	{
		m_eGroupRole = groupRole;

		if (s_OnGroupRoleChanged)
			s_OnGroupRoleChanged.Invoke(m_iGroupID, groupRole);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_SetRallyPoint(int callsignId, bool force)
	{
		if (m_RallyPointId == callsignId)
		{
			m_bForcedRallyPoint = force || m_bForcedRallyPoint;
			return;
		}

		m_RallyPointId = callsignId;
		m_bForcedRallyPoint = force;

		if (s_OnGroupRallyPointChanged)
			s_OnGroupRallyPointChanged.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_RemoveRallyPoint()
	{
		m_RallyPointId = -1;
		m_bForcedRallyPoint = false;

		if (s_OnGroupRallyPointChanged)
			s_OnGroupRallyPointChanged.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	//! Called on the server (authority)
	void AddPlayer(int playerID)
	{
		// Avoiding duplicate entries
		if (m_aPlayerIDs.Contains(playerID))
			return;

		SCR_NotificationsComponent.SendToGroup(m_iGroupID, ENotification.GROUPS_PLAYER_JOINED, playerID);
		RPC_DoAddPlayer(playerID);
		Rpc(RPC_DoAddPlayer, playerID);
		// Did this player re-connect?
		if (m_aDisconnectedPlayerIDs)
		{
			int index = m_aDisconnectedPlayerIDs.Find(playerID);
			if (index >= 0)
				m_aDisconnectedPlayerIDs.Remove(index);
		}

		// Start listening to disconnect events when we add the first player
		if (m_aPlayerIDs.Count() == 1)
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode)
				gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnected);
		}
		// Now we need the player character's agent
		IEntity controlledEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!controlledEntity)
			QueueAddAgent(playerID);
		else
			AddAgentFromControlledEntity(controlledEntity);

		GetGame().GetCallqueue().CallLater(CheckForLeader, 0, false, playerID, false);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoAddRequester(int playerID)
	{
		m_aRequesterIDs.Insert(playerID);
		s_OnJoinPrivateGroupRequest.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoRemoveRequester(int playerID)
	{
		m_aRequesterIDs.RemoveItem(playerID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoClearDeniedRequester()
	{
		m_aDeniedRequesters.Clear();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoClearRequesterIDs()
	{
		m_aRequesterIDs.Clear();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoAddDeniedRequester(int playerID)
	{
		m_aDeniedRequesters.Insert(playerID);
	}

	//------------------------------------------------------------------------------------------------

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoRemovePlayer(int playerID)
	{
		m_aPlayerIDs.RemoveItem(playerID);
		if (m_iLeaderID == playerID)
			m_iLeaderID = -1;

		s_OnPlayerRemoved.Invoke(this, playerID);
	}

	//------------------------------------------------------------------------------------------------
	void RemovePlayerAgent(int playerID)
	{
		IEntity controlledEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!controlledEntity)
			return;

		AIControlComponent aiControlComponent = AIControlComponent.Cast(controlledEntity.FindComponent(AIControlComponent));
		if (!aiControlComponent)
			return;

		AIAgent agent = aiControlComponent.GetAIAgent();
		if (!agent)
			return;

		RemoveAgent(agent);
	}

	//------------------------------------------------------------------------------------------------
	//! Called on the server (authority)
	void RemovePlayer(int playerID)
	{
		if (!m_aPlayerIDs.Contains(playerID))
			return;

		//if player is last in group it doesnt matter as the group will get deleted
		if (playerID == m_iLeaderID && GetPlayerCount() > 1)
		{
			SetCustomName("", 0);
			SetCustomDescription("", 0);
		}

		RPC_DoRemovePlayer(playerID);
		Rpc(RPC_DoRemovePlayer, playerID);
		CheckForLeader(-1, false);
		RemovePlayerAgent(playerID);
		SCR_NotificationsComponent.SendToGroup(m_iGroupID, ENotification.GROUPS_PLAYER_LEFT, playerID);
	}

	//------------------------------------------------------------------------------------------------
	void GetCallsigns(out string company, out string platoon, out string squad, out string character, out string format)
	{
		SCR_CallsignGroupComponent callsignComponent = SCR_CallsignGroupComponent.Cast(FindComponent(SCR_CallsignGroupComponent));
		if (!callsignComponent)
			return;

		callsignComponent.GetCallsignNames(company, platoon, squad, character, format);
	}

	//------------------------------------------------------------------------------------------------
	void SetRadioFrequency(int frequency)
	{
		if (frequency == GetRadioFrequency() || frequency <= 0)
			return;

		RPC_DoSetFrequency(frequency);
		Rpc(RPC_DoSetFrequency, frequency);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_DoSetFrequency(int frequency)
	{
		m_iGroupRadioFrequency = frequency;
		s_OnFrequencyChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	int GetRadioFrequency()
	{
		return m_iGroupRadioFrequency;
	}

	//------------------------------------------------------------------------------------------------
	bool IsPlayable()
	{
		return m_bPlayable;
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateUnitEntities(bool editMode, array<ResourceName> entityResourceNames)
	{
		if (!GetGame().GetAIWorld())
		{
			Print(string.Format("Cannot spawn team members of group %1, AIWorld is missing in the world!", this), LogLevel.WARNING);
			return;
		}

#ifdef WORKBENCH
		if (!editMode)
		{
			//--- Are AI components valid?
			AIFormationComponent AIFormation = AIFormationComponent.Cast(FindComponent(AIFormationComponent));

			if (!AIFormation)
				Print(string.Format("Group %1 does not have AIFormationComponent! Team members will not be spawned.", this), LogLevel.WARNING);
			else
			{
				AIFormationDefinition formationDefinition = AIFormation.GetFormation();
				if (!formationDefinition)
					Print(string.Format("Formation of group %1 not found in SCR_AIWorld! Team members will not be spawned.", this), LogLevel.WARNING);
			}
		}
#endif
		//--- Apply global override
		bool snapToTerrain = m_bSnapToTerrain;
		if (s_bIgnoreSnapToTerrain)
		{
			snapToTerrain = false;
			s_bIgnoreSnapToTerrain = false;
		}
		if (Replication.IsClient())
			return;

		//--- We are in WB, prepare array so previews can be deleted later
		if (editMode && !m_aSceneGroupUnitInstances)
			m_aSceneGroupUnitInstances = new array<IEntity>;

		m_iNumOfMembersToSpawn = Math.Min(entityResourceNames.Count(), m_iMaxUnitsToSpawn);
		//--- Create group members
		for (int i = m_iNumOfMembersToSpawn-1; i >= 0; i--)
		{
			// Spawn group across multiple frames
			SCR_AIGroup_DelayedSpawn delaySpawn = new SCR_AIGroup_DelayedSpawn();
			delaySpawn.snapToTerrain	= snapToTerrain;
			delaySpawn.index			= i;
			delaySpawn.resourceName		= entityResourceNames[i];
			delaySpawn.editMode			= editMode;

			m_delayedSpawnList.Insert(delaySpawn);
		}

		if (editMode)
		{
			//--- Edit mode has no game world, spawn immediately
			SpawnAllImmediately();
		}
		else
		{
			//--- Enable the frame event and frames when paused
			BeginDelayedSpawn();
		}

		//--- Call group init if it cannot be called by the last spawned entity
		if (m_iNumOfMembersToSpawn == 0 && Event_OnInit)
			Event_OnInit.Invoke(this);
	}
	/*
	Spawn single Group member.
	Returns false when action has to be delayed
	*/
	protected bool SpawnGroupMember(bool snapToTerrain, int index, ResourceName res, bool editMode, bool isLast)
	{
		if (!GetGame().GetAIWorld().CanLimitedAIBeAdded())
		{
			if (isLast && Event_OnInit)
				Event_OnInit.Invoke(this);

			//Event_OnLastGroupMemberSpawned.Invoke(this);

			return true;
		}
		BaseWorld world = GetWorld();
		AIFormationDefinition formationDefinition;
		AIFormationComponent formationComponent = AIFormationComponent.Cast(this.FindComponent(AIFormationComponent));
		if (formationComponent)
			formationDefinition = formationComponent.GetFormation();
		EntitySpawnParams spawnParams = new EntitySpawnParams;
		spawnParams.TransformMode = ETransformMode.WORLD;
		GetWorldTransform(spawnParams.Transform);
		vector pos = spawnParams.Transform[3];

		if (formationDefinition)
			pos = CoordToParent(formationDefinition.GetOffsetPosition(index));
		else
			pos = CoordToParent(Vector(index, 0, 0));

		if (snapToTerrain)
		{
			float surfaceY = world.GetSurfaceY(pos[0], pos[2]);
			pos[1] = surfaceY;
		}


		//Snap to the nearest navmesh point
		AIPathfindingComponent pathFindindingComponent = AIPathfindingComponent.Cast(this.FindComponent(AIPathfindingComponent));

		if (!editMode)
		{
			NavmeshWorldComponent navmesh = pathFindindingComponent.GetNavmeshComponent();
			if (navmesh)
			{
				if (navmesh.IsTileRequested(pos))
				{
					return false;
				}
				if (!navmesh.IsTileLoaded(pos))
				{
					navmesh.LoadTileIn(pos);
					return false;
				}
			}
		}

		if (pathFindindingComponent && pathFindindingComponent.GetClosestPositionOnNavmesh(pos, "10 10 10", pos))
		{
			float groundHeight = world.GetSurfaceY(pos[0], pos[2]);
			if (pos[1] < groundHeight)
				pos[1] = groundHeight;
			vector outWaterSurfacePoint;
			EWaterSurfaceType waterSurfaceType;
			vector transformWS[4];
			vector obbExtents;
			if (ChimeraWorldUtils.TryGetWaterSurface(GetWorld(), pos, outWaterSurfacePoint, waterSurfaceType, transformWS, obbExtents))
			{
				pos = outWaterSurfacePoint;
			}
		}

		spawnParams.Transform[3] = pos;

		IEntity member = GetGame().SpawnEntityPrefabEx(res, true, world, spawnParams);
		if (!member)
			return true;

		// Move in to vehicle
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(member.FindComponent(SCR_EditableEntityComponent));


		if (editMode)
			m_aSceneGroupUnitInstances.Insert(member);

		// Even same null-check is above, in some situations, member can get deleted and it would result in VME
		if (!member)
			return true;

		AddAIEntityToGroup(member);

		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(member.FindComponent(FactionAffiliationComponent));

		if (factionAffiliation)
			factionAffiliation.SetAffiliatedFactionByKey(m_faction);

		if (isLast && Event_OnInit)
			Event_OnInit.Invoke(this);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void SetWaypointParams(out AIWaypoint wp, SCR_WaypointPrefabLocation prefabParams)
	{
		if (SCR_TimedWaypoint.Cast(wp) && !float.AlmostEqual(prefabParams.m_WPTimeOverride, 0.0))
			SCR_TimedWaypoint.Cast(wp).SetHoldingTime(prefabParams.m_WPTimeOverride);
		if (!float.AlmostEqual(prefabParams.m_WPRadiusOverride, 0.0))
			wp.SetCompletionRadius(prefabParams.m_WPRadiusOverride);
		wp.SetName(prefabParams.m_WPInstanceName);
	}

	//------------------------------------------------------------------------------------------------
	void AddWaypointsDynamic(out array<IEntity> entityInstanceList, array<ref SCR_WaypointPrefabLocation> prefabs)
	{
		entityInstanceList = new array<IEntity>;
		EntitySpawnParams spawnParams = new EntitySpawnParams;
		spawnParams.TransformMode = ETransformMode.WORLD;
		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		for (int i = 0, length = prefabs.Count(); i < length; i++)
		{
			IEntity entity;
			AIWaypoint wp;
			mat[3] = prefabs[i].m_WPWorldLocation;
			spawnParams.Transform = mat;

#ifdef WORKBENCH //includes game mode run from WB
			WorldEditorAPI m_API = _WB_GetEditorAPI();
			if (m_API)
			{
				Print(prefabs[i].m_WPPrefabName);
				entity = GetGame().SpawnEntityPrefab(Resource.Load(prefabs[i].m_WPPrefabName), m_API.GetWorld(), spawnParams);
				entityInstanceList.Insert(entity);
				wp = AIWaypoint.Cast(entity);
			}
			else
			{
				entity = GetGame().SpawnEntityPrefab(Resource.Load(prefabs[i].m_WPPrefabName), GetGame().GetWorld(), spawnParams);
				wp = AIWaypoint.Cast(entity);
				AddWaypoint(wp);
			}
#else		// game run from build
			entity = GetGame().SpawnEntityPrefab(Resource.Load(prefabs[i].m_WPPrefabName), GetGame().GetWorld(), spawnParams);
			wp = AIWaypoint.Cast(entity);
			AddWaypoint(wp);
#endif
			if (wp)
				SetWaypointParams(wp, prefabs[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	void AddWaypointsStatic(array<string> aWaypointNames)
	{
		for (int i = 0, length = aWaypointNames.Count(); i < length; i++)
		{
#ifdef WORKBENCH
			WorldEditorAPI m_API = _WB_GetEditorAPI();
			if (m_API)
			{
				AddWaypoint(AIWaypoint.Cast(m_API.GetWorld().FindEntityByName(aWaypointNames[i])));
			}
			else
			{
				AddWaypoint(AIWaypoint.Cast(GetGame().GetWorld().FindEntityByName(aWaypointNames[i])));
			}
#else
			AddWaypoint(AIWaypoint.Cast(GetGame().GetWorld().FindEntityByName(aWaypointNames[i])));
#endif
		}
	}

	//------------------------------------------------------------------------------------------------
	void AddVehiclesStatic(array<string> aVehicleNames)
	{
		for (int i = 0, length = aVehicleNames.Count(); i < length; i++)
		{
			IEntity vehicle;

#ifdef WORKBENCH
			WorldEditorAPI m_API = _WB_GetEditorAPI();
			if (m_API)
			{
				vehicle = m_API.GetWorld().FindEntityByName(aVehicleNames[i]);
			}
			else
			{
				vehicle = GetGame().GetWorld().FindEntityByName(aVehicleNames[i]);
			}
#else
			vehicle = GetGame().GetWorld().FindEntityByName(aVehicleNames[i]);
#endif

			if (vehicle)
			{
				SCR_AIVehicleUsageComponent vehicleUsageComp = SCR_AIVehicleUsageComponent.FindOnNearestParent(vehicle, vehicle);
				if (vehicleUsageComp)
					m_GroupUtilityComponent.m_VehicleMgr.TryAddVehicle(vehicleUsageComp);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DestroyEntities(out array<IEntity> entityList)
	{
		if (!entityList)
			return;

		for (int i=0, length = entityList.Count(); i < length; i++)
		{
			if (AIWaypoint.Cast(entityList[i]))
				RemoveWaypointFromGroup(AIWaypoint.Cast(entityList[i]));
			else
				RemoveAIEntityFromGroup(entityList[i]);
			delete entityList[i];
		}
		entityList.Clear();
		entityList = null;
	}

	//------------------------------------------------------------------------------------------------
	void RemoveStaticWaypointRefs(array<string> aWaypointNames)
	{
		if (!aWaypointNames)
			return;

		for (int i=0, length = aWaypointNames.Count(); i < length; i++)
		{
#ifdef WORKBENCH
			WorldEditorAPI m_API = _WB_GetEditorAPI();
			if (m_API)
			{
				RemoveWaypointFromGroup(AIWaypoint.Cast(m_API.GetWorld().FindEntityByName(aWaypointNames[i])));
			}
			else
			{
				RemoveWaypointFromGroup(AIWaypoint.Cast(GetGame().GetWorld().FindEntityByName(aWaypointNames[i])));
			}
#else
			RemoveWaypointFromGroup(AIWaypoint.Cast(GetGame().GetWorld().FindEntityByName(aWaypointNames[i])));
#endif
		}
	}

	//------------------------------------------------------------------------------------------------
	void ClearRefs(out array<IEntity> entityList)
	{
		if (entityList)
			entityList.Clear();
		entityList = null;
	}

	//------------------------------------------------------------------------------------------------
	bool AddAIEntityToGroup(IEntity entity)
	{
		if (!entity) return false;

		AIControlComponent control = AIControlComponent.Cast(entity.FindComponent(AIControlComponent));
		if (!control) return false;

		AIAgent agent = control.GetControlAIAgent();
		if (!agent) return false;

		control.ActivateAI();

		if (!agent.GetParentGroup())
			AddAgent(agent); //--- Add to group only if some other system (e.g., component on the group member) wasn't faster

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool RemoveAIEntityFromGroup(IEntity entity)
	{
#ifdef WORKBENCH
		WorldEditorAPI m_API = _WB_GetEditorAPI();
		if (!m_API && entity && entity.FindComponent(AIControlComponent))
		{
			ref AIAgent agent = AIControlComponent.Cast(entity.FindComponent(AIControlComponent)).GetControlAIAgent();
			RemoveAgent(agent);
			return true;
		}
#else
		if (entity && entity.FindComponent(AIControlComponent))
		{
			ref AIAgent agent = AIControlComponent.Cast(entity.FindComponent(AIControlComponent)).GetControlAIAgent();
			RemoveAgent(agent);
			return true;
		}
#endif
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void AddWaypointToGroup(AIWaypoint waypoint)
	{
		if (waypoint)
		{
			AddWaypoint(waypoint);
		}
	}

	//------------------------------------------------------------------------------------------------
	void RemoveWaypointFromGroup(AIWaypoint waypoint)
	{
		if (waypoint)
		{
			RemoveWaypoint(waypoint)
		}
	}

	//------------------------------------------------------------------------------------------------
	void GetAllocatedCompartments(out array<BaseCompartmentSlot> allocatedCompartments)
	{
		allocatedCompartments = m_aAllocatedCompartments;
	}

	//------------------------------------------------------------------------------------------------
	void GetAllocatedSmartActions(out array<AISmartActionComponent> allocatedComponents)
	{
		allocatedComponents = m_aAllocatedComponents;
	}

	//------------------------------------------------------------------------------------------------
	void AllocateSmartActions(array<AISmartActionComponent> components)
	{
		if (!components || components.IsEmpty())
			return;

		foreach (AISmartActionComponent component : components)
		{
			if (m_aAllocatedComponents.Find(component) > -1)
			{
				Print("Trying to allocate same component twice!", LogLevel.WARNING);
				continue;
			};
			m_aAllocatedComponents.Insert(component);
			component.SetActionAccessible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	void AllocateCompartment(BaseCompartmentSlot compartment)
	{
		if (!compartment)
			return;
		if (m_aAllocatedCompartments.Find(compartment) > -1)
		{
			Print("Trying to allocate same compartment twice!", LogLevel.WARNING);
			return;
		};
		if (!compartment.IsCompartmentAccessible())
		{
			Print("Compartment already reserved!", LogLevel.WARNING);
			return;
		};
		compartment.SetCompartmentAccessible(false);
		m_aAllocatedCompartments.Insert(compartment);
	}

	//------------------------------------------------------------------------------------------------
	void ReleaseCompartment(BaseCompartmentSlot compartment)
	{
		int index = m_aAllocatedCompartments.Find(compartment);
		if (index > -1)
			m_aAllocatedCompartments.Remove(index);
		else
			Print("Trying to remove compartment that is not allocated!", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	void ReleaseCompartments()
	{
		foreach (BaseCompartmentSlot comp : m_aAllocatedCompartments)
		{
			if (comp)
				comp.SetCompartmentAccessible(true);
		}
		m_aAllocatedCompartments.Clear();
	}

	//------------------------------------------------------------------------------------------------
	void ReleaseSmartAction(AISmartActionComponent component)
	{
		int index = m_aAllocatedComponents.Find(component);
		if (index > -1)
			m_aAllocatedComponents.Remove(index);
		else
			Print("Trying to remove smart action that is not allocated!", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	void ReleaseSmartActions()
	{
		foreach (AISmartActionComponent comp : m_aAllocatedComponents)
		{
			if (comp)
				comp.SetActionAccessible(true);
		}
		m_aAllocatedComponents.Clear();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set name of group's faction (only if it's not set yet)
	\param factionKey, Faction key to set faction to
	\return True if the faction was set
	*/
	bool InitFactionKey(string factionKey)
	{
		if (m_faction != "") return false;
		m_faction = factionKey;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	If Faction is set change faction to given faction
	Server Only
	\param faction, Faction to set
	\return True if the faction was set
	*/
	bool SetFaction(Faction faction)
	{
		if (Replication.IsClient())
			return false;

		// If playable, we don't want to allow changing the faction.
		if (m_bPlayable && m_faction)
			return false;

		if (!faction)
			return false;

		m_faction = faction.GetFactionKey();

		array<AIAgent> agents = new array<AIAgent>;
		GetAgents(agents);
		array<IEntity> updatedVehicles = new array<IEntity>;
		//array<RplId> entitiesInVehiclesIds = new array<RplId>;
		IEntity vehicle;
		IEntity charEntity;

		foreach (AIAgent agent : agents)
		{
			charEntity = agent.GetControlledEntity();

			if (!charEntity)
				continue;

			FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(charEntity.FindComponent(FactionAffiliationComponent));
			if (factionAffiliation)
				factionAffiliation.SetAffiliatedFaction(faction);
		}

		//Send out event
		if (Event_OnFactionChanged)
			Event_OnFactionChanged.Invoke(faction);

		FactionManager factionManager = GetGame().GetFactionManager();
		int factionIndex = factionManager.GetFactionIndex(faction);
		if (factionManager)
			GetGame().GetCallqueue().CallLater(SetFactionDelayed, 1, false, factionIndex);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetFactionDelayed(int factionIndex)
	{
		Rpc(BroadCastSetFaction, factionIndex);
	}

	//------------------------------------------------------------------------------------------------
	//Send to update Editor UI
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void BroadCastSetFaction(int factionIndex)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		Faction faction = factionManager.GetFactionByIndex(factionIndex);
		m_faction = faction.GetFactionKey();
		if (Event_OnFactionChanged)
			Event_OnFactionChanged.Invoke(faction);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get name of group's faction.
	\return Faction name (search for it in FactionManager)
	*/
	string GetFactionName()
	{
		return m_faction;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get group's faction from FactionManager
	\return Faction (or null when faction is not found)
	*/
	Faction GetFaction()
	{
		// If it's a slave group, get faction from master group
		if (m_MasterGroup)
			return m_MasterGroup.GetFaction();

		// Master group:
		ArmaReforgerScripted game = GetGame();
		if (!game) return null;

		FactionManager factionManager = game.GetFactionManager();
		if (!factionManager) return null;

		return factionManager.GetFactionByKey(m_faction);
	}

	/*!
	Get group's faction index from FactionManager
	\return faction index
	*/
	int GetFactionIndex()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return -1;

		return factionManager.GetFactionIndex(GetFaction());
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Ignore snapping to terrain in the next spawned group.
	Used when some other system handles it and group's functionality would interfere with it.
	Has to be set before spawning a group, is reset to false afterwards.
	\param ignore True to ignore terrain snap
	*/
	static void IgnoreSnapToTerrain(bool ignore)
	{
		s_bIgnoreSnapToTerrain = ignore;
	}
	/*!
	Ignore spawning group members in the next spawned group.
	Used when some other system handles it and group's functionality would interfere with it.
	Has to be set before spawning a group, is reset to false afterwards.
	\param ignore True to ignore spawning
	*/
	static void IgnoreSpawning(bool ignore)
	{
		s_bIgnoreSpawning = ignore;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when all initial group members were spawned.
	Called only on server.
	Param is this aiGroup
	\return Script invoker
	*/
	ScriptInvoker GetOnInit()
	{
		if (!Event_OnInit)
			Event_OnInit = new ScriptInvoker();

		return Event_OnInit;
	}

	/*!
	Get event called when the group becomes empty.
	Called only on server.
	No invoker params are passed.
	\return Script invoker
	*/
	ScriptInvoker_AIGroupOnEmpty GetOnEmpty()
	{
		if (!Event_OnEmpty)
			Event_OnEmpty = new ScriptInvokerBase<ScriptInvoker_AIGroupOnEmpty_Callback>();

		return Event_OnEmpty;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get event called every time an agent is called to a group.
	Called only on server.
	Invoker params are: AIGroup group, AIAgent agent
	\return Script invoker
	*/
	ScriptInvoker GetOnAgentAdded()
	{
		if (!Event_OnAgentAdded)
			Event_OnAgentAdded = new ScriptInvoker();

		return Event_OnAgentAdded;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get event called every time an agent is removed from a group.
	Called only on server.
	Invoker params are: AIGroup group, AIAgent agent
	\return Script invoker
	*/
	ScriptInvoker GetOnAgentRemoved()
	{
		if (!Event_OnAgentRemoved)
			Event_OnAgentRemoved = new ScriptInvoker();

		return Event_OnAgentRemoved;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get event called every time a group leader changes.
	Called only on server.
	Invoker params are: AIGroup group, AIAgent currentLeader, AIAgent prevLeader
	\return Script invoker
	*/
	ScriptInvoker GetOnLeaderChanged()
	{
		if (!Event_OnLeaderChanged)
			Event_OnLeaderChanged = new ScriptInvoker();

		return Event_OnLeaderChanged;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when current waypoint of the group changes.
	Called only on server.
	Invoker params are: AIWaypoint currentWP, AIWaypoint prevWP
	\return Script invoker
	*/
	ScriptInvoker GetOnCurrentWaypointChanged()
	{
		if (!Event_OnCurrentWaypointChanged)
			Event_OnCurrentWaypointChanged = new ScriptInvoker();

		return Event_OnCurrentWaypointChanged;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when current waypoint of the group is completed.
	Called only on server.
	Invoker params are: AIWaypoint wp
	\return Script invoker
	*/
	ScriptInvoker GetOnWaypointCompleted()
	{
		if (!Event_OnWaypointCompleted)
			Event_OnWaypointCompleted = new ScriptInvoker();

		return Event_OnWaypointCompleted;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when a new waypoint is added to the group.
	Called only on server.
	Invoker params are: AIWaypoint wp
	\return Script invoker
	*/
	ScriptInvoker GetOnWaypointAdded()
	{
		if (!Event_OnWaypointAdded)
			Event_OnWaypointAdded = new ScriptInvoker();

		return Event_OnWaypointAdded;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when a waypoint is going to be removed from the group. Warning: the wp may be null - listener deletes it in EditableGroupComponent!
	Called only on server.
	Invoker params are: AIWaypoint wp
	\return Script invoker
	*/
	ScriptInvoker GetOnWaypointRemoved()
	{
		if (!Event_OnWaypointRemoved)
			Event_OnWaypointRemoved = new ScriptInvoker();

		return Event_OnWaypointRemoved;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when a waypoint is planned for removal but not removed from the group, yet.
	Called only on server.
	Invoker params are: AIWaypoint wp
	\return Script invoker
	*/
	ScriptInvoker GetOnWaypointToRemove()
	{
		if (!Event_OnWaypointToRemove)
			Event_OnWaypointToRemove = new ScriptInvoker();

		return Event_OnWaypointToRemove;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when faction of group is changed
	\return Script invoker
	*/
	ScriptInvoker GetOnFactionChanged()
	{
		if (!Event_OnFactionChanged)
			Event_OnFactionChanged = new ScriptInvoker();

		return Event_OnFactionChanged;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvokerAIGroup> GetOnAllDelayedEntitySpawned()
	{
		if (!Event_OnAllDelayedEntitySpawned)
			Event_OnAllDelayedEntitySpawned = new ScriptInvokerBase<ScriptInvokerAIGroup>();

		return Event_OnAllDelayedEntitySpawned;
	}

	//------------------------------------------------------------------------------------------------
	override void OnEmpty()
	{
		if (Event_OnEmpty)
			Event_OnEmpty.Invoke(this);

		//--- Delete after delay, doing it directly in this event would be unsafe
		if (m_bDeleteWhenEmpty)
			GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, 1, false, this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnAgentAdded(AIAgent child)
	{
		if (Event_OnAgentAdded)
			Event_OnAgentAdded.Invoke(child);

		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(child);
		if (agent)
		{
			agent.OnGroupWaypointChanged(GetCurrentWaypoint());
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnAgentRemoved(AIAgent child)
	{
		if (Event_OnAgentRemoved)
			Event_OnAgentRemoved.Invoke(this, child);
	}

	//------------------------------------------------------------------------------------------------
	override void OnLeaderChanged(AIAgent currentLeader, AIAgent prevLeader)
	{
		if (Event_OnLeaderChanged)
			Event_OnLeaderChanged.Invoke(currentLeader, prevLeader);

		if (currentLeader)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(currentLeader.GetControlledEntity());
			if (character)
			{
				SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
				if (controller)
					controller.m_OnLifeStateChanged.Insert(LeaderLifeStateChanged);
			}
		}

		if (!prevLeader)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(prevLeader.GetControlledEntity());
		if (character)
		{
			SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
			if (controller)
				controller.m_OnLifeStateChanged.Remove(LeaderLifeStateChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnCurrentWaypointChanged(AIWaypoint currentWP, AIWaypoint prevWP)
	{
		InvokeSubagentsOnWaypointChanged(currentWP);

		if (Event_OnCurrentWaypointChanged)
			Event_OnCurrentWaypointChanged.Invoke(currentWP, prevWP);
	}

	//------------------------------------------------------------------------------------------------
	override void OnWaypointCompleted(AIWaypoint wp)
	{
		InvokeSubagentsOnWaypointChanged(null);

		if (Event_OnWaypointCompleted)
			Event_OnWaypointCompleted.Invoke(wp);
	}

	//------------------------------------------------------------------------------------------------
	override void OnWaypointAdded(AIWaypoint wp)
	{
		if (Event_OnWaypointAdded)
			Event_OnWaypointAdded.Invoke(wp);
	}

	//------------------------------------------------------------------------------------------------
	override void OnWaypointRemoved(AIWaypoint wp, bool isCurrentWaypoint)
	{
		InvokeSubagentsOnWaypointChanged(null);

		if (Event_OnWaypointToRemove)
			Event_OnWaypointToRemove.Invoke(wp, isCurrentWaypoint); // listeners should not delete the waypoint

		if (Event_OnWaypointRemoved)
			Event_OnWaypointRemoved.Invoke(wp); // listeners can delete the waypoint
	}

	//------------------------------------------------------------------------------------------------
	//! Invokes OnGroupWaypointChanged on all soldiers
	//! newWaypoint can be null
	protected void InvokeSubagentsOnWaypointChanged(AIWaypoint newWaypoint)
	{
		array<AIAgent> agents = {};
		GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			SCR_ChimeraAIAgent _agent = SCR_ChimeraAIAgent.Cast(agent);
			if (_agent)
				_agent.OnGroupWaypointChanged(newWaypoint);
		}
	}

	//------------------------------------------------------------------------------------------------
#ifdef WORKBENCH
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "coords")
		{
			DestroyEntities(m_aSceneGroupUnitInstances);
			CreateUnitEntities(true, m_aUnitPrefabSlots);
		}
		return false;
	}
#endif

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_aAllocatedCompartments = new array<BaseCompartmentSlot>;
		m_GroupUtilityComponent = SCR_AIGroupUtilityComponent.Cast(this.FindComponent(SCR_AIGroupUtilityComponent));

		SCR_SpawnPoint.GetOnSpawnPointFinalizeSpawn().Insert(OnSpawnPointFinalizeSpawn);

		if (s_bIgnoreSpawning)
		{
			s_bIgnoreSpawning = false;

			//--- Instantly mark as initialized if no team members are to be spawned
			if (Event_OnInit)
				Event_OnInit.Invoke(this);
			
			return;
		}

		if (m_bSpawnImmediately || GetWorld().IsEditMode())
			SpawnUnits();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSpawnPointFinalizeSpawn(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		if (requestComponent.GetPlayerId() != m_iLeaderID)
			return;

		SCR_SpawnPointSpawnData spawnData = SCR_SpawnPointSpawnData.Cast(data);
		if (!spawnData)
			return;

		SCR_SpawnPoint spawnPoint = spawnData.GetSpawnPoint();
		if (!spawnPoint)
			return;

		IEntity baseEntity = spawnPoint.GetParent();
		if (!baseEntity)
			return;

		SCR_MilitaryBaseComponent baseComponent = SCR_MilitaryBaseComponent.Cast(baseEntity.FindComponent(SCR_MilitaryBaseComponent));
		if (!baseComponent)
			return;

		SetRallyPoint(baseComponent);
	}

	//------------------------------------------------------------------------------------------------
	bool GetSpawnImmediately()
	{
		return m_bSpawnImmediately;
	}

	//------------------------------------------------------------------------------------------------
	void SetSpawnImmediately(bool spawnImmediately)
	{
		m_bSpawnImmediately = spawnImmediately;
	}

	//------------------------------------------------------------------------------------------------
	void SetMaxUnitsToSpawn(int cnt)
	{
		m_iMaxUnitsToSpawn = cnt;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set delay (ms) between spawning of individual group members.
	Can be used only before they are spawned.
	\param memberSpawnDelay Delay
	*/
	void SetMemberSpawnDelay(int memberSpawnDelay)
	{
		m_fMemberSpawnDelay = memberSpawnDelay;
	}

	//------------------------------------------------------------------------------------------------
	void SpawnUnits()
	{
		if (SCR_Global.IsEditMode(this))
		{
			CreateUnitEntities(true, m_aUnitPrefabSlots);
			AddVehiclesStatic(m_aStaticVehicles);
			AddWaypointsStatic(m_aStaticWaypoints);
			AddWaypointsDynamic(m_aSceneWaypointInstances, m_aSpawnedWaypoints);
		}
		else
		{
			//--- Don't hardcode members array - it may change in run-time, and we don't want to delete members who meanwhile joined other group
			CreateUnitEntities(false, m_aUnitPrefabSlots);
			AddVehiclesStatic(m_aStaticVehicles);
			AddWaypointsStatic(m_aStaticWaypoints);
			AddWaypointsDynamic(null, m_aSpawnedWaypoints);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get slave subgroup that is used for AI characters when invited by players into this playable SCR_AIGroup.
	*/
	SCR_AIGroup GetSlave()
	{
		return m_SlaveGroup;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set slave subgroup that is used for AI characters when invited by players into this playable SCR_AIGroup.
	*/
	void SetSlave(SCR_AIGroup group)
	{
		m_SlaveGroup = group;
		group.SetMaster(this);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get master group that gives orders to this slave subgroup
	*/
	SCR_AIGroup GetMaster()
	{
		return m_MasterGroup;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set master group that gives orders to this slave subgroup
	*/
	void SetMaster(SCR_AIGroup group)
	{
		m_MasterGroup = group;
	}

		//------------------------------------------------------------------------------------------------
	array<SCR_ChimeraCharacter> GetAIMembers()
	{
		return m_aAIMembers;
	}

	//------------------------------------------------------------------------------------------------
	/*
	returns true if SCR_ChimeraCharacter is a member of slave subgroup that is linked to this playable SCR_AIGroup
	*/
	bool IsAIControlledCharacterMember(SCR_ChimeraCharacter character)
	{
		if (!character)
			return false;
		//if group doesnt have slave group for AIs, AI is automatically not a member
		if (!m_SlaveGroup)
			return false;
		return m_SlaveGroup.m_aAIMembers.Find(character) != -1;
	}

	//------------------------------------------------------------------------------------------------
	bool IsLoadoutInGroup(SCR_FactionPlayerLoadout loadout)
	{
		SCR_Faction scrFaction = SCR_Faction.Cast(GetFaction());
		if (!scrFaction)
			return false;

		array<SCR_GroupRolePresetConfig> groupRolePresets = {};
		scrFaction.GetGroupRolePresetConfigs(groupRolePresets);

		// find loadout in faction predefined groups
		foreach (SCR_GroupRolePresetConfig groupRolePreset : groupRolePresets)
		{
			if (groupRolePreset && groupRolePreset.GetGroupRole() == m_eGroupRole)
				return groupRolePreset.IsLoadoutInGroup(loadout);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!m_UiInfo)
		m_UiInfo = new SCR_AIGroupUIInfo();

		int factionIndex = -1;
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
			factionIndex = factionManager.GetFactionIndex(GetFaction());

		writer.WriteInt(factionIndex);
		writer.WriteInt(m_iGroupRadioFrequency);
		writer.WriteInt(m_iGroupID);

		int count = m_aPlayerIDs.Count();
		writer.WriteInt(count);
		for (int i = count - 1; i >= 0; i--)
		{
			writer.WriteInt(m_aPlayerIDs[i]);
		}
		writer.WriteInt(m_iLeaderID);
		writer.WriteBool(m_bPrivate);

		writer.WriteInt(m_iDescriptionAuthorID);
		writer.WriteInt(m_iNameAuthorID);

		writer.WriteString(m_sCustomDescription);
		writer.WriteString(m_sCustomName);

		writer.WriteString(m_UiInfo.GetGroupFlag());
		writer.WriteBool(m_UiInfo.GetFlagIsFromImageSet());

		RplId groupID;
		groupID = Replication.FindId(m_MasterGroup);
		writer.WriteRplId(groupID);
		groupID = Replication.FindId(m_SlaveGroup);
		writer.WriteRplId(groupID);

		writer.WriteInt(m_iMaxMembers);

		writer.WriteInt(m_eRequiredRank);

		writer.WriteInt(m_eGroupRole);
		writer.WriteBool(m_bIsPrivacyChangeable);
		writer.WriteBool(m_bIsCreatedByCommander);

		writer.WriteInt(m_RallyPointId);
		writer.WriteBool(m_bForcedRallyPoint);

		//do rpcs for players join/leave
		//add invokers for players join/leave

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int factionIndex;
		reader.ReadInt(factionIndex);
		if (factionIndex >= 0)
			BroadCastSetFaction(factionIndex);

		reader.ReadInt(m_iGroupRadioFrequency);
		reader.ReadInt(m_iGroupID);

		int count, playerID;
		reader.ReadInt(count);
		for (int i = count - 1; i >= 0; i--)
		{
			reader.ReadInt(playerID);
			m_aPlayerIDs.Insert(playerID);
		}

		if (m_bPlayable)
		{
			SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
			if (groupsManager)
			{
				groupsManager.RegisterGroup(this);
				groupsManager.ClaimFrequency(GetRadioFrequency(), GetFaction());
				groupsManager.OnGroupCreated(this);
			}
		}

		int temp;
		reader.ReadInt(temp);
		RPC_SetLeaderID(temp);

		reader.ReadBool(m_bPrivate);

		reader.ReadInt(m_iDescriptionAuthorID);
		reader.ReadInt(m_iNameAuthorID);

		string customDesc, customName;
		reader.ReadString(customDesc);
		reader.ReadString(customName);

		RPC_DoSetCustomDescription(customDesc, m_iDescriptionAuthorID);
		RPC_DoSetCustomName(customName, m_iNameAuthorID);

		if (!m_UiInfo)
		m_UiInfo = new SCR_AIGroupUIInfo();

		string flag;
		reader.ReadString(flag);
		m_UiInfo.SetGroupFlag(flag);

		bool isFromImageSet;
		reader.ReadBool(isFromImageSet);
		m_UiInfo.SetFlagIsFromImageSet(isFromImageSet);

		RplId groupID;
		reader.ReadRplId(groupID);
		m_MasterGroup = SCR_AIGroup.Cast(Replication.FindItem(groupID));
		reader.ReadRplId(groupID);
		m_SlaveGroup = SCR_AIGroup.Cast(Replication.FindItem(groupID));

		reader.ReadInt(m_iMaxMembers);

		reader.ReadInt(m_eRequiredRank);

		reader.ReadInt(m_eGroupRole);
		reader.ReadBool(m_bIsPrivacyChangeable);
		reader.ReadBool(m_bIsCreatedByCommander);

		reader.ReadInt(m_RallyPointId);
		reader.ReadBool(m_bForcedRallyPoint);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void LeaderLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		if (newLifeState == ECharacterLifeState.INCAPACITATED)
			SetNewConsciousLeader();
	}

	//------------------------------------------------------------------------------------------------
	void SetNewConsciousLeader()
	{
		array<AIAgent> groupAgents = {};
		GetAgents(groupAgents);

		for (int i = 1; i < groupAgents.Count(); i++)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(groupAgents.Get(i).GetControlledEntity());
			if (!character)
				continue;

			CharacterControllerComponent charController = character.GetCharacterController();
			if (!charController)
				continue;

			if (!charController.IsUnconscious())
			{
				SetNewLeader(groupAgents.Get(i));
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void IncreaseDeployedRadioCount()
	{
		m_iDeployedRadioCount++;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	void DecreaseDeployedRadioCount()
	{
		if (m_iDeployedRadioCount < 1)
			return;

		m_iDeployedRadioCount--;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	int GetDeployedRadioCount()
	{
		return m_iDeployedRadioCount;
	}

	//------------------------------------------------------------------------------------------------
	void SetDeleteWhenEmpty(bool deleteWhenEmpty)
	{
		m_bDeleteWhenEmpty = deleteWhenEmpty;
	}

	//------------------------------------------------------------------------------------------------
	void CompleteAllWaypoints()
	{
		array<AIWaypoint> aiWaypoints = {};
		GetWaypoints(aiWaypoints);

		foreach (AIWaypoint aiWaypoint : aiWaypoints)
		{
			CompleteWaypoint(aiWaypoint);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SCR_AIGroup(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_AIGroup()
	{
		// Group is playable so we have to unregister it locally as well
		if (m_bPlayable)
		{
			SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
			if (groupsManager)
			{
				groupsManager.UnregisterGroup(this);
				groupsManager.GetOnPlayableGroupRemoved().Invoke(this);
			}
		}

		SCR_SpawnPoint.GetOnSpawnPointFinalizeSpawn().Remove(OnSpawnPointFinalizeSpawn);

		DestroyEntities(m_aSceneGroupUnitInstances);
		DestroyEntities(m_aSceneWaypointInstances);
		RemoveStaticWaypointRefs(m_aStaticWaypoints);
	}
}

//------------------------------------------------------------------------------------------------
enum EGroupState
{
	IDLE = 0,
	ATTACKING = 1,
	MOVING = 2,
	INVESTIGATING = 3,
	RETREATING = 4,
	DEFENDING = 5,
	MANEUVERING = 6,
	REQ_SUPPORT = 7,
	REQ_ORDERS = 8,
	FOLLOW = 9,
}

//------------------------------------------------------------------------------------------------
//! Names of enum values must match formation names in AIWorld
enum SCR_EAIGroupFormation
{
	Wedge,
	Line,
	Column,
	StaggeredColumn
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_WaypointPrefabLocation
{
	[Attribute("{750A8D1695BD6998}AI/Entities/Waypoints/AIWaypoint.et", UIWidgets.ResourceAssignArray, "Prefab for the waypoint")]
	ResourceName m_WPPrefabName;

	[Attribute("", UIWidgets.EditBox, "Waypoint name")]
	string m_WPInstanceName;

	[Attribute("", UIWidgets.EditBox, "Waypoint location")]
	vector m_WPWorldLocation;

	[Attribute("0", UIWidgets.EditBox, "Waypoint completion radius (-1 dont override default)")]
	float m_WPRadiusOverride;

	[Attribute("0", UIWidgets.EditBox, "Waypoint completion time (-1 dont override default)")]
	float m_WPTimeOverride;
}

//---- REFACTOR NOTE END ----
