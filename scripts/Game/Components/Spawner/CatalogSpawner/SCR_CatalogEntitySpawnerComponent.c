[EntityEditorProps(category: "GameScripted/Components", description: "Allows player to spawn entities configured in EntityCatalogs, usually found on factions. Requires SCR_EntitySpawnerSlotComponent in vicinity.", color: "0 0 255 255")]
class SCR_CatalogEntitySpawnerComponentClass : SCR_SlotServiceComponentClass
{
	[Attribute(defvalue: "{56EBF5038622AC95}Assets/Conflict/CanBuild.emat", params: "emat", desc: "Material used on entity previews, visible to local players only", category: "Entity Spawner")]
	ResourceName m_sPreviewEntityMaterial;
	
	[Attribute(defvalue: "{14A9DCEA57D1C381}Assets/Conflict/CannotBuild.emat", params: "emat", desc: "Material used on unavailable entity previews, visible to local players only", category: "Entity Spawner")]
	ResourceName m_sPreviewEntityMaterialUnavailable;

	[Attribute(defvalue :"{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et", UIWidgets.ResourceNamePicker, "Default group to be initially assigned to created units", "et", category: "Entity Spawner")]
	protected ResourceName m_sDefaultGroupPrefab;

	[Attribute(defvalue: "{FFF9518F73279473}PrefabsEditable/Auto/AI/Waypoints/E_AIWaypoint_Move.et", UIWidgets.ResourceNamePicker, "Defend waypoint prefab", "et", category: "Entity Spawner")]
	protected ResourceName m_sDefaultWaypointPrefab;

	[Attribute(defvalue: "150", params: "0 inf", desc: "Supply component search radius.", category: "Supplies")]
	protected float m_fSupplyComponentSearchRadius;
	
	[Attribute(defvalue: "2.5", params: "0 inf", desc: "Completion radius of initial move waypoint", "et", category: "Entity Spawner")]
	protected float m_fMoveWaypointCompletionRadius;
	
	[Attribute(defvalue: "10", params: "0 inf", desc: "How long should be vehicle locked after spawn", category: "Vehicle lock protection")]
	protected int m_iVehicleLockedDuration;

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetDefaultWaypointPrefab()
	{
		return m_sDefaultWaypointPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetDefaultGroupPrefab()
	{
		return m_sDefaultGroupPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetSupplySearchRadius()
	{
		return m_fSupplyComponentSearchRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetMoveWaypointCompletionRadius()
	{
		return m_fMoveWaypointCompletionRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetVehicleLockedDuration()
	{
		return m_iVehicleLockedDuration;
	}
}

//! Component allowing user to request spawning entities using asset catalogs
//! Requires ActionManager with enough SCR_CatalogSpawnerUserAction attached to it (they cannot be generated through script) and SCR_EntitySlotComponents in hiearchy or vicinity of owner
class SCR_CatalogEntitySpawnerComponent : SCR_SlotServiceComponent
{
	//! static variable used to store all the instances created of this component.
	static const ref array<SCR_CatalogEntitySpawnerComponent> INSTANCES = {};
	
	protected const float UPDATE_PERIOD = 10.0 / 60.0;
	
	[Attribute(category: "Catalog Parameters", desc: "Type of entity catalogs that will be allowed on this spawner.", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EEntityCatalogType))]
	protected ref array<EEntityCatalogType> m_aCatalogTypes;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, category: "Catalog Parameters", desc: "Allowed labels.", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aAllowedLabels;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, category: "Catalog Parameters", desc: "Ignored labels.", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aIgnoredLabels;

	[RplProp()]
	protected ref array<RplId> m_aGracePeriodEntries = {};

	[RplProp()]
	protected ref array<RplId> m_aGracePeriodRequesters = {};

	protected ref array<float> m_aGracePeriodStartingTimes = {};
	protected float m_fLastUpdateElapsedTime;
	
	[Attribute(defvalue: "1", desc: "If true, Spawner will require from entities that they have all allowed labels.", category: "Catalog Parameters")]
	protected bool m_bNeedAllLabels;

	[Attribute(desc: "Enables supplies usage.", category: "Supplies")]
	protected bool m_bSuppliesConsumptionEnabled;

	[Attribute(params: "0 inf", desc: "Custom supplies value.", category: "Supplies")]
	protected int m_iCustomSupplies;
	
	[Attribute(desc: "Enables the refund grace period.", category: "Supplies")]
	protected bool m_bEnableGracePeriod;

	[Attribute(params: "0 inf", desc: "Time in seconds for the duration of the grace period for refunding.", category: "Supplies")]
	protected float m_fGracePeriodTime;

	[Attribute(params: "0 inf", desc: "Range in meters for the area of the grace period for refunding.", category: "Supplies")]
	protected float m_fGracePeriodAreaRange;

	[Attribute(params: "0 inf", desc: "Multiplier used to affect the resource usage of the refund actions after the grace period has expired.", category: "Supplies")]
	protected float m_fPostGracePeriodRefundMultiplier;
	
	protected ActionsManagerComponent m_ActionManager;
	protected IEntity m_SpawnedEntity;
	protected SCR_PrefabPreviewEntity m_PreviewEntity;
	protected SCR_ResourceComponent m_ResourceComponent;
	protected SCR_CampaignSuppliesComponent m_SupplyComponent
	protected RplComponent m_RplComponent;

	protected ref ScriptInvoker m_OnEntitySpawned; //~ Sends Spawned IEntity
	protected ref ScriptInvoker m_OnSpawnerSuppliesChanged; //~ Sends Spawned prev and new spawn supplies

	static const int SLOT_CHECK_INTERVAL = 60;

	protected ref array<SCR_EntityCatalogEntry> m_aAssetList = {};
	protected ref array<ref Tuple2<SCR_AIGroup, SCR_AIGroup>> m_aGroupsToAssign = {};
	protected ref map<AIWaypoint, SCR_AIGroup> m_mGroupWaypoints;

	//arrays used for UI Slot checks
	protected ref map<SCR_EntitySpawnerSlotComponent, WorldTimestamp> m_mKnownFreeSlots;
	protected ref map<SCR_EntitySpawnerSlotComponent, WorldTimestamp> m_mKnownOccupiedSlots;

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsSuppliesConsumptionEnabled()
	{
		return m_bSuppliesConsumptionEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetPostGracePeriodRefundMultiplier()
	{
		return m_fPostGracePeriodRefundMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \return
	bool IsInGracePeriod(IEntity entity)
	{
		RplId entityId = Replication.FindId(entity);
		
		if (entityId.IsValid())
			return m_aGracePeriodEntries.Contains(entityId);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entityId
	//! \return
	bool IsInGracePeriod(RplId entityId)
	{
		if (entityId.IsValid())
			return m_aGracePeriodEntries.Contains(entityId);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entityId
	//! \param[in] userId
	//! \return
	bool CanRefund(RplId entityId, RplId userId)
	{
		if (!entityId.IsValid() || !userId.IsValid())
			return false;
			
		int idx = m_aGracePeriodEntries.Find(entityId);
		
		return	idx == -1 
			||	(	m_aGracePeriodEntries[idx] == entityId 
				&&	m_aGracePeriodRequesters[idx] == userId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \param[in] user
	//! \return
	bool CanRefund(notnull IEntity entity, notnull IEntity user)
	{
		RplId entityId = Replication.FindId(entity);
		RplId userId = Replication.FindId(user);
		
		if (!entityId.IsValid() || !userId.IsValid())
			return false;
			
		int idx = m_aGracePeriodEntries.Find(entityId);
		
		return	idx == -1 
			||	(	m_aGracePeriodEntries[idx] == entityId 
				&&	m_aGracePeriodRequesters[idx] == userId);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	void UnregisterGracePeriod(notnull IEntity entity)
	{
		RplId entityId = Replication.FindId(entity);
		
		if (!entityId.IsValid())
			return;
		
		int idx = m_aGracePeriodEntries.Find(entityId);
		
		m_aGracePeriodEntries.Remove(idx);
		m_aGracePeriodStartingTimes.Remove(idx);
		m_aGracePeriodRequesters.Remove(idx);
		
		if (m_aGracePeriodEntries.Count() == 0)
			ClearEventMask(GetOwner(), EntityEvent.FRAME);
		
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entityId
	void UnregisterGracePeriod(RplId entityId)
	{
		if (!entityId.IsValid())
			return;
		
		int idx = m_aGracePeriodEntries.Find(entityId);
		
		m_aGracePeriodEntries.Remove(idx);
		m_aGracePeriodStartingTimes.Remove(idx);
		m_aGracePeriodRequesters.Remove(idx);
		
		if (m_aGracePeriodEntries.Count() == 0)
			ClearEventMask(GetOwner(), EntityEvent.FRAME);
		
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \param[in] user
	//! \param[in] startingTime
	void RegisterGracePeriod(notnull IEntity entity, notnull IEntity user, float startingTime = FLT_INF)
	{
		RplId entityId = Replication.FindId(entity);
		RplId userId = Replication.FindId(user);
		
		if (!entityId.IsValid())
			return;
		
		if (!userId.IsValid())
			return;
		
		if (startingTime == FLT_INF)
			startingTime = GetGame().GetWorld().GetWorldTime() / 1000.0;
		
		if (m_aGracePeriodEntries.Count() == 0)
			SetEventMask(GetOwner(), EntityEvent.FRAME);
		
		m_aGracePeriodEntries.Insert(entityId);
		m_aGracePeriodRequesters.Insert(userId);
		m_aGracePeriodStartingTimes.Insert(startingTime + m_fGracePeriodTime);
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entityId
	//! \param[in] userId
	//! \param[in] startingTime
	void RegisterGracePeriod(RplId entityId, RplId userId, float startingTime = FLT_INF)
	{
		if (startingTime == FLT_INF)
			startingTime = GetGame().GetWorld().GetWorldTime() / 1000.0;
		
		if (m_aGracePeriodEntries.Count() == 0)
			SetEventMask(GetOwner(), EntityEvent.FRAME);
		
		m_aGracePeriodEntries.Insert(entityId);
		m_aGracePeriodRequesters.Insert(userId);
		m_aGracePeriodStartingTimes.Insert(startingTime + m_fGracePeriodTime);
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if user is elibigle for requesting specified entity
	//! \param[in] entityEntry entity entry of item to be checked
	//! \param[in] user controlled entity of requester
	bool RankCheck(notnull SCR_EntityCatalogEntry entityEntry, notnull IEntity user)
	{
		if (!entityEntry)
			return false;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return false;

		if (campaign.CanRequestVehicleWithoutRank())
			return true;
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (playerId == 0)
			return false;

		IEntity playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return false;

		SCR_ECharacterRank rank = SCR_CharacterRankComponent.GetCharacterRank(user);

		//Check if the player has high enough rank
		SCR_EntityCatalogSpawnerData spawnerData = SCR_EntityCatalogSpawnerData.Cast(entityEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));

		return spawnerData && spawnerData.HasRequiredRank(rank);
	}

	//------------------------------------------------------------------------------------------------
	//! Set Asset Catalog from currently owning faction. If null, asset list will be cleared.
	protected void SetCurrentFactionCatalog()
	{	
		m_aAssetList.Clear();
		
		SCR_Faction faction = SCR_Faction.Cast(GetFaction());
		if (!faction || m_aCatalogTypes.IsEmpty())
		{
			AssignUserActions();
			return;
		}

		SCR_EntityCatalog catalog;
		foreach (EEntityCatalogType catalogType : m_aCatalogTypes)
		{
			catalog = faction.GetFactionEntityCatalogOfType(catalogType);
			if (!catalog)
				continue;

			AddAssetsFromCatalog(catalog);
		}

		AssignUserActions();
	}

	//------------------------------------------------------------------------------------------------
	//! Add Assets from entityCatalog.
	//! \param[in] entityCatalog Entity catalog which should be added
	//! \param[in] overwriteOld IF true, old entities are overwriten by new ones, thus removing their availability
	protected void AddAssetsFromCatalog(notnull SCR_EntityCatalog entityCatalog, bool overwriteOld = false)
	{
		array<SCR_EntityCatalogEntry> newAssets = {};
		array<typename> includedDataClasses = {};
		includedDataClasses.Insert(SCR_EntityCatalogSpawnerData); //~ The Data the entity must have

		entityCatalog.GetFullFilteredEntityList(newAssets, m_aAllowedLabels, m_aIgnoredLabels, includedDataClasses, null, m_bNeedAllLabels);

		if (overwriteOld)
			m_aAssetList = newAssets;
		else
			m_aAssetList.InsertAll(newAssets);
	}

	//------------------------------------------------------------------------------------------------
	//! Obtain Faction entity from AssetList on specific index
	SCR_EntityCatalogEntry GetEntryAtIndex(int index)
	{
		if (!m_aAssetList || !m_aAssetList.IsIndexValid(index))
			return null;

		return m_aAssetList[index];
	}

	//------------------------------------------------------------------------------------------------
	//! Fills SCR_CatalogSpawnerUserAction actions on ActionManager.
	//! The method should always be called when the Asset list is changed.
	protected void AssignUserActions()
	{
		if (!m_ActionManager)
			return;

		if (m_aAssetList.IsEmpty())
			return;

		array<BaseUserAction> userActions = {};
		m_ActionManager.GetActionsList(userActions);

		if (userActions.Count() < m_aAssetList.Count())
			Print("There is not enough of SCR_CatalogSpawnerUserAction attached to actionManager. Some assets won't appear.", LogLevel.WARNING);

		SCR_CatalogSpawnerUserAction spawnerUserAction;

		foreach (int i, BaseUserAction userAction : userActions)
		{
			spawnerUserAction = SCR_CatalogSpawnerUserAction.Cast(userAction);
			if (!spawnerUserAction)
				continue;

			if (m_aAssetList.IsIndexValid(i))
				spawnerUserAction.SetSpawnerData(m_aAssetList[i]);
			else
				spawnerUserAction.SetSpawnerData(null);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Assign resource component to handle resources of spawner.
	void AssignResourceComponent(notnull SCR_ResourceComponent component)
	{
		m_ResourceComponent = component;
	}

	//------------------------------------------------------------------------------------------------
	//! Assign supply component to handle supplies of spawner. Currently uses Campaign specific supplies (temporarily)
	//! \param supplyComp
	[Obsolete("SCR_CatalogEntitySpawnerComponent.AssignSupplyComponent() should be used instead.")]
	void AssignSupplyComponent(notnull SCR_CampaignSuppliesComponent supplyComp)
	{
		m_SupplyComponent = supplyComp;
	}

	//------------------------------------------------------------------------------------------------
	//! Manually clears known slots, thus forcing Spawner to do otherwise ignored checks
	void ClearKnownSlots()
	{
		if (m_mKnownFreeSlots)
			m_mKnownFreeSlots.Clear();

		if (m_mKnownOccupiedSlots)
			m_mKnownOccupiedSlots.Clear();
	}
	//------------------------------------------------------------------------------------------------
	//! Set slot as occupied
	//! \param[in] slot
	void AddKnownOccupiedSlot(notnull SCR_EntitySpawnerSlotComponent slot)
	{
		if (!m_mKnownOccupiedSlots)
			m_mKnownOccupiedSlots = new map<SCR_EntitySpawnerSlotComponent, WorldTimestamp>();

		ChimeraWorld world = GetOwner().GetWorld();
		m_mKnownOccupiedSlots.Set(slot, world.GetServerTimestamp());
	}

	//------------------------------------------------------------------------------------------------
	//! Update currently known occupied slots.
	//! If certain amount of time passed since last slot check, it will be removed from known slots and checked again next time it is needed.
	protected void UpdateOccupiedSlots(out array<SCR_EntitySpawnerSlotComponent> occupiedSlots)
	{
		if (!m_mKnownOccupiedSlots)
			return;

		occupiedSlots = {};

		ChimeraWorld world = GetOwner().GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		foreach (SCR_EntitySpawnerSlotComponent slot, WorldTimestamp timestamp : m_mKnownOccupiedSlots)
		{
			if (currentTime.DiffMilliseconds(timestamp) > (SLOT_CHECK_INTERVAL * 100))
				m_mKnownOccupiedSlots.Remove(slot);
			else
				occupiedSlots.Insert(slot);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns last known suitable free slot. Used to reduce amount of redundant slot checks and save performance.
	protected SCR_EntitySpawnerSlotComponent GetLastKnownSuitableSlot(notnull SCR_EntityCatalogSpawnerData spawnerData)
	{
		if (!m_mKnownFreeSlots)
			return null;

		ChimeraWorld world = GetOwner().GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		foreach (SCR_EntitySpawnerSlotComponent slot, WorldTimestamp timestamp : m_mKnownFreeSlots)
		{
			if (currentTime.DiffMilliseconds(timestamp) > (SLOT_CHECK_INTERVAL * 100))
				m_mKnownFreeSlots.Remove(slot);
			else if (spawnerData.CanSpawnInSlot(slot.GetSlotType()))
				return slot;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Used in GetFreeSlot to obtain free slot from inserted array.
	//! \param[in] slots Array containing slots that will be checked
	//! \param[in] spawnerData Data containing parameters needed for free slot (slotType, for example)
	//! \param[in] occupiedSlots
	protected SCR_EntitySpawnerSlotComponent GetFreeSlotFromArray(notnull array<SCR_EntitySpawnerSlotComponent> slots, notnull SCR_EntityCatalogSpawnerData spawnerData, notnull array<SCR_EntitySpawnerSlotComponent> occupiedSlots)
	{
		foreach (SCR_EntitySpawnerSlotComponent slot : slots)
		{
			if (!spawnerData.CanSpawnInSlot(slot.GetSlotType()) || occupiedSlots.Contains(slot))
				continue;

			if (!slot.IsOccupied())
			{
				if (!m_mKnownFreeSlots)
					m_mKnownFreeSlots = new map<SCR_EntitySpawnerSlotComponent, WorldTimestamp>();

				ChimeraWorld world = GetOwner().GetWorld();
				m_mKnownFreeSlots.Insert(slot, world.GetServerTimestamp());

				return slot;
			}

			//Add to array of recenly found occupied slots
			AddKnownOccupiedSlot(slot);
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//!Returns free slot suitable for requested Entity
	//! \param[in] spawnerData SCR_EntityCatalogSpawnerData found on Catalog Entry
	SCR_EntitySpawnerSlotComponent GetFreeSlot(notnull SCR_EntityCatalogSpawnerData spawnerData)
	{
		// First go through recently checked slots to prevent redundant checks. Should be used only for Interaction visualisation.
		SCR_EntitySpawnerSlotComponent freeSlot = GetLastKnownSuitableSlot(spawnerData);
		if (freeSlot)
			return freeSlot;

		array<SCR_EntitySpawnerSlotComponent> occupiedSlots = {};
		UpdateOccupiedSlots(occupiedSlots);

		// Second check for slots in hiearchy, If no child slot available, look for nearby slots
		freeSlot = GetFreeSlotFromArray(m_aChildSlots, spawnerData, occupiedSlots);
		if (!freeSlot)
			freeSlot = GetFreeSlotFromArray(m_aNearSlots, spawnerData, occupiedSlots);

		return freeSlot;
	}

	//------------------------------------------------------------------------------------------------
	//! Initiate spawn. Called through RPC from SCR_SpawnerRequestComponent on CharacterComponent
	//! \param[in] entityEntry entity entry to be spawned
	//! \param[in] userId Id of user requesting spawn
	//! \param[in] slot Slot on which user requests spawn. As last check for empty position is done by PerformSpawn itselt, this might be ignored, should slot be already occupied.
	void InitiateSpawn(notnull SCR_EntityCatalogEntry entityEntry, int userId, SCR_EntitySpawnerSlotComponent slot)
	{
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(userId);
		if (!user)
			return;

		int supplies = GetSpawnerResourceValue();
		SCR_EntityCatalogSpawnerData spawnerData = SCR_EntityCatalogSpawnerData.Cast(entityEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (!spawnerData)
			return;

		if (m_bSuppliesConsumptionEnabled && supplies < spawnerData.GetSupplyCost())
			return;

		if (GetRequestState(entityEntry, user) == SCR_EEntityRequestStatus.CAN_SPAWN)
			PerformSpawn(entityEntry, user, slot);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns SCR_EEntityRequestStatus for request of selected entity with optional parameter of user requesting it.
	//! \param[in] entityEntry entityEntry in catalog to be spawned
	//! \param[in] user User requesting spawn
	SCR_EEntityRequestStatus GetRequestState(notnull SCR_EntityCatalogEntry entityEntry, IEntity user = null)
	{
		if (!m_aAssetList || !m_aAssetList.Contains(entityEntry))
			return SCR_EEntityRequestStatus.NOT_AVAILABLE;

		SCR_EntityCatalogSpawnerData entitySpawnerData = SCR_EntityCatalogSpawnerData.Cast(entityEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));

		//Supply consumption specific states
		if (m_bSuppliesConsumptionEnabled)
		{
			if (!m_ResourceComponent)
				return SCR_EEntityRequestStatus.NOT_AVAILABLE;
			
			SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
			
			if (consumer && entitySpawnerData && (entitySpawnerData.GetSupplyCost() > consumer.GetAggregatedResourceValue()))
				return SCR_EEntityRequestStatus.NOT_ENOUGH_SUPPLIES;
			else if (!consumer && entitySpawnerData && (entitySpawnerData.GetSupplyCost() > m_iCustomSupplies))
				return SCR_EEntityRequestStatus.NOT_ENOUGH_SUPPLIES;
		}

		
		//Do not show anything, if user is of another faction than spawner
		if (GetRequesterFaction(user) != GetFaction())
			return SCR_EEntityRequestStatus.NOT_AVAILABLE;

		// Campaign dependent ranking and cooldowns. To be replaced once stand-alone rank system is present
		if (SCR_GameModeCampaign.GetInstance())
		{
			if (!RankCheck(entityEntry, user))
				return SCR_EEntityRequestStatus.RANK_LOW;

			if (!CooldownCheck(user))
				return SCR_EEntityRequestStatus.COOLDOWN;
		}

		EEntityCatalogType catalogType = entityEntry.GetCatalogParent().GetCatalogType();
		// Additional states to be returned if entity Entry is of Character or Group type
		if (catalogType == EEntityCatalogType.CHARACTER || catalogType == EEntityCatalogType.GROUP)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (!gameMode)
				return SCR_EEntityRequestStatus.NOT_AVAILABLE;
			
			SCR_SpawnerAIGroupManagerComponent groupSpawningManager = SCR_SpawnerAIGroupManagerComponent.Cast(gameMode.FindComponent(SCR_SpawnerAIGroupManagerComponent));
			if (!groupSpawningManager || groupSpawningManager.IsAtAILimit())
				return SCR_EEntityRequestStatus.AI_LIMIT_REACHED;
			
			SCR_PlayerController controller = GetPlayerControllerFromEntity(user);
			if (!controller)
				return SCR_EEntityRequestStatus.NOT_AVAILABLE;

			//Do not allow requesting, if player is not in group or is not its leader
			SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.Cast(controller.FindComponent(SCR_PlayerControllerGroupComponent));
			if (!groupController || !groupController.IsPlayerLeaderOwnGroup())
				return SCR_EEntityRequestStatus.REQUESTER_NOT_GROUPLEADER;

			if (!CanRequestAI(user, entitySpawnerData.GetEntityCount()))
				return SCR_EEntityRequestStatus.GROUP_FULL;
		}
		
		// Campaign dependent ranking and cooldowns. To be replaced once stand-alone rank system is present
		if (SCR_GameModeCampaign.GetInstance())
		{
			if (!RankCheck(entityEntry, user))
				return SCR_EEntityRequestStatus.RANK_LOW;

			if (!CooldownCheck(user))
				return SCR_EEntityRequestStatus.COOLDOWN;
		}

		return SCR_EEntityRequestStatus.CAN_SPAWN;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] user Entity of user requesting AI
	//! \param[in] aiCount Amount of AI's to be added
	//! \return true if player can request another AI into group, false otherwise
	bool CanRequestAI(notnull IEntity user, int aiCount = 1)
	{
		SCR_CommandingManagerComponent commandingManager = SCR_CommandingManagerComponent.GetInstance();
		if (!commandingManager)
			return false;

		SCR_PlayerController playerController = GetPlayerControllerFromEntity(user);
		if (!playerController)
			return false;

		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!groupController)
			return false;

		SCR_AIGroup group = groupController.GetPlayersGroup();
		if (!group)
			return false;

		SCR_AIGroup slaveGroup = group.GetSlave();
		if (!slaveGroup)
			return false;

		SCR_SpawnerRequestComponent reqComponent = SCR_SpawnerRequestComponent.Cast(playerController.FindComponent(SCR_SpawnerRequestComponent));
		if (!reqComponent)
			return false;

		int queuedAICount = reqComponent.GetQueuedAIs();

		return commandingManager.GetMaxAIPerGroup() >= (slaveGroup.GetAIMembers().Count() + queuedAICount + aiCount);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when AI finishes initial WP (making it move away from spawning position, before joining player squad)
	//! \param[in] wp
	protected void OnGroupWaypointFinished(notnull AIWaypoint wp)
	{
		if (!m_mGroupWaypoints || !m_aGroupsToAssign)
			return;

		SCR_AIGroup group = m_mGroupWaypoints.Get(wp);
		if (!group)
			return;

		SCR_AIGroup playerGroup;
		foreach (int index, Tuple2<SCR_AIGroup, SCR_AIGroup> groups : m_aGroupsToAssign)
		{
			if (groups.param1 == group)
			{	
				playerGroup = groups.param2;
				m_aGroupsToAssign.Remove(index);
				break;
			}
		}

		if (!playerGroup)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerGroup.GetLeaderID()));
		if (!playerController)
			return;

		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!groupController)
			return;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		group.GetOnAgentRemoved().Remove(OnAIAgentRemoved);

		SCR_ChimeraCharacter aiCharacter;
		foreach (AIAgent agent : agents)
		{
			aiCharacter = SCR_ChimeraCharacter.Cast(agent.GetControlledEntity());
			if (!aiCharacter)
				continue;

			groupController.AddAIToSlaveGroup(agent.GetControlledEntity(), playerGroup); //AddAISoldierToPlayerGroup(aiCharacter, groupLeaderEntity);
		}

		SCR_SpawnerRequestComponent spawnerReqComponent = SCR_SpawnerRequestComponent.Cast(playerController.FindComponent(SCR_SpawnerRequestComponent));
		if (spawnerReqComponent)
			spawnerReqComponent.AddQueuedAI(-1);

		//Delete old entries or empty maps
		m_mGroupWaypoints.Remove(wp);
		if (m_mGroupWaypoints.IsEmpty())
			m_mGroupWaypoints = null;

		if (m_aGroupsToAssign.IsEmpty())
			m_aGroupsToAssign = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddAISoldierToPlayerGroup(notnull SCR_ChimeraCharacter ai, notnull IEntity user)
	{
		SCR_PlayerController controller = GetPlayerControllerFromEntity(user);
		if (!controller)
			return;

		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.Cast(controller.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!groupController)
			return;

		groupController.RequestAddAIAgent(ai, controller.GetPlayerId());
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] user
	//! \return
	Faction GetRequesterFaction(notnull IEntity user)
	{
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(user);
		if (!player)
			return null;

		return player.GetFaction();
	}

	//------------------------------------------------------------------------------------------------
	//! Gets script invoker on entity spawns.
	//! Invoker will send over spawned IEntity
	//! \return ScriptInvoker Event when entity is spawned
	ScriptInvoker GetOnEntitySpawned()
	{
		if (!m_OnEntitySpawned)
			m_OnEntitySpawned = new ScriptInvoker();

		return m_OnEntitySpawned;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets script invoker on supplies changes.
	//! Invoker will send over previous supply amount and changed one
	//! \return ScriptInvoker Event when entity is spawned
	ScriptInvoker GetOnSpawnerSuppliesChanged()
	{
		if (!m_OnSpawnerSuppliesChanged)
			m_OnSpawnerSuppliesChanged = new ScriptInvoker();

		return m_OnSpawnerSuppliesChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] userEntity
	//! \return
	SCR_PlayerController GetPlayerControllerFromEntity(notnull IEntity userEntity)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		int playerId = playerManager.GetPlayerIdFromControlledEntity(userEntity);

		return SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));

	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerEntity
	//! \return
	SCR_SpawnerRequestComponent GetRequestComponentFromPlayerEntity(notnull IEntity playerEntity)
	{
		SCR_PlayerController playerController = GetPlayerControllerFromEntity(playerEntity);
		if (!playerController)
			return null;

		return SCR_SpawnerRequestComponent.Cast(playerController.FindComponent(SCR_SpawnerRequestComponent));
	}

	//------------------------------------------------------------------------------------------------
	// Send notification to player requesting spawn
	protected void SendNotification(int msgId, notnull IEntity user, int assetId = -1, int catalogType = -1)
	{
		SCR_SpawnerRequestComponent reqComponent = GetRequestComponentFromPlayerEntity(user);
		if (!reqComponent)
			return;

		reqComponent.SendPlayerFeedback(msgId, assetId, catalogType);
	}

	//------------------------------------------------------------------------------------------------
	//! Used to create local "preview" model on specified slot. Used to visualise what entity is player requesting and position where it will appear
	//! \param[in] spawnData SCR_EntityCatalogEntry containing information about entity (prefab data is required)
	//! \param[in] slot Slot on which preview should appear
	//! \param[in] reqStatus
	void CreatePreviewEntity(notnull SCR_EntityCatalogEntry spawnData, notnull SCR_EntitySpawnerSlotComponent slot, SCR_EEntityRequestStatus reqStatus = SCR_EEntityRequestStatus.CAN_SPAWN)
	{
		SCR_CatalogEntitySpawnerComponentClass prefabData = SCR_CatalogEntitySpawnerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;

		//This will delete any existing preview entity
		DeletePreviewEntity();

		// Don't show preview for Groups.
		if (spawnData.GetCatalogParent().GetCatalogType() == EEntityCatalogType.GROUP)
			return;

		if (!m_aAssetList || m_aAssetList.IsEmpty() || !m_aAssetList.Contains(spawnData))
			return;

		Resource resource = Resource.Load(spawnData.GetPrefab());

		if (!resource || !resource.IsValid())
			return;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		slot.GetOwner().GetTransform(params.Transform);
		
		ResourceName material;
		if (reqStatus == SCR_EEntityRequestStatus.CAN_SPAWN)
			material = prefabData.m_sPreviewEntityMaterial;
		else
			material = prefabData.m_sPreviewEntityMaterialUnavailable;
		
		m_PreviewEntity = SCR_PrefabPreviewEntity.Cast(SCR_PrefabPreviewEntity.SpawnPreviewFromPrefab(resource, "SCR_PrefabPreviewEntity", GetOwner().GetWorld(), params, material));
		
		if (m_PreviewEntity)
		{
			// Align preview with terrain and update it to make it appear correctly
			m_PreviewEntity.SetPreviewTransform(params.Transform, EEditorTransformVertical.TERRAIN);
			m_PreviewEntity.Update();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Delete existing Preview entity at spawner
	void DeletePreviewEntity()
	{
		if (m_PreviewEntity)
			SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! \return existing preview entity
	SCR_PrefabPreviewEntity GetPreviewEntity()
	{
		return m_PreviewEntity;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn the actual entity
	//! \param[in] entityEntry
	//! \param[in] user
	//! \param[in] preferredSlot optional slot to be used for spawning
	protected void PerformSpawn(notnull SCR_EntityCatalogEntry entityEntry, IEntity user = null, SCR_EntitySpawnerSlotComponent preferredSlot = null)
	{
		if (IsProxy())
			return;

		m_SpawnedEntity = null;

		SCR_EntityCatalogSpawnerData spawnerData = SCR_EntityCatalogSpawnerData.Cast(entityEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));

		//Slot from preferredSlot parameter is checked first, should it be occupied, new slot will be looked for (if there is any)
		SCR_EntitySpawnerSlotComponent slot;
		if (preferredSlot && spawnerData.CanSpawnInSlot(preferredSlot.GetSlotType()) && !preferredSlot.IsOccupied())
		{
			slot = preferredSlot;
		}
		else
		{
			ClearKnownSlots();
			slot = GetFreeSlot(spawnerData)
		}

		if (!slot)
		{
			SendNotification(SCR_EEntityRequestStatus.NOT_ENOUGH_SPACE, user);
			return;
		}
		
		//Prevents next spawned SCR_AIGroup from being fully spawned
		if (entityEntry.GetCatalogParent().GetCatalogType() == EEntityCatalogType.GROUP)
			SCR_AIGroup.IgnoreSpawning(true);
		
		//Spawns entity using Resource Name from prefab data and slot owning entity
		m_SpawnedEntity = SpawnEntity(spawnerData.GetRandomDefaultOrVariantPrefab(), slot.GetOwner());

		if (!m_SpawnedEntity)
		{
			if (entityEntry.GetCatalogParent().GetCatalogType() == EEntityCatalogType.GROUP)
				SCR_AIGroup.IgnoreSpawning(false);

			return;
		}
		
		if (m_bSuppliesConsumptionEnabled)
			AddSpawnerSupplies(-spawnerData.GetSupplyCost());

		if (m_OnEntitySpawned)
			m_OnEntitySpawned.Invoke(m_SpawnedEntity, user, SCR_Faction.Cast(GetFaction()), this);

		//Send notification to player, whom requested entity
		SCR_EntityCatalog parentCatalog = entityEntry.GetCatalogParent();
		if (parentCatalog)
		{
			array<SCR_EntityCatalogEntry> entityList = {};
			parentCatalog.GetEntityList(entityList);
			int assetIndex = entityList.Find(entityEntry);
			
			if (assetIndex > -1)
				SendNotification(0, user, entityList.Find(entityEntry), parentCatalog.GetCatalogType());
		}
		
		//Called, if spawned entity is vehicle
		if (m_SpawnedEntity.IsInherited(Vehicle))
		{
			CarControllerComponent carController = CarControllerComponent.Cast(m_SpawnedEntity.FindComponent(CarControllerComponent));
			if (carController)
				carController.SetPersistentHandBrake(true);
			
			Physics physicsComponent = m_SpawnedEntity.GetPhysics();
			if (physicsComponent)
				physicsComponent.SetVelocity("0 -0.1 0"); // Make the entity copy the terrain properly
			
			LockSpawnedVehicle(user);

			ActionsManagerComponent actionsManagerComponent = ActionsManagerComponent.Cast(m_SpawnedEntity.FindComponent(ActionsManagerComponent));
			
			if (!actionsManagerComponent)
				return;
			
			SCR_ResourceEntityRefundAction refundAction;
			array<BaseUserAction> outActions = {};
			
			actionsManagerComponent.GetActionsList(outActions);
			
			foreach (BaseUserAction action : outActions)
			{
				refundAction = SCR_ResourceEntityRefundAction.Cast(action);
				
				if (!refundAction)
					continue;
			}
		}

		//Called, if spawned entity is SCR_ChimeraCharacter or inherits from it.
		if (m_SpawnedEntity.IsInherited(SCR_ChimeraCharacter))
		{
			SCR_SpawnerRequestComponent requestComponent = GetRequestComponentFromPlayerEntity(user);
			if (!requestComponent)
				return;

			OnChimeraCharacterSpawned(SCR_ChimeraCharacter.Cast(m_SpawnedEntity), user, slot.GetRallyPoint());
			requestComponent.AddQueuedAI(spawnerData.GetEntityCount());
		}
		else
		{
			slot.MoveCharactersFromSlot();
		}

		//Called, if spawned entity is SCR_AIGroup or inherits from it.
		if (m_SpawnedEntity.IsInherited(SCR_AIGroup))
			OnAIGroupSpawned(SCR_AIGroup.Cast(m_SpawnedEntity), user, slot.GetOwner(), slot.GetRallyPoint());
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity SpawnEntity(ResourceName entityResourceName, notnull IEntity slotOwner)
	{
		Resource entityResource = Resource.Load(entityResourceName);
		if (!entityResource || !entityResource.IsValid())
		{
			Print("CatalogEntitySpawnerComponent - SpawnEntity cannot spawn new entity without valid resource. Used SCR_EntityCatalogEntry is probably set incorrectly", LogLevel.ERROR);
			return null;
		}

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		slotOwner.GetTransform(params.Transform);
		
		return GetGame().SpawnEntityPrefab(entityResource, GetOwner().GetWorld(), params);
	}

	//------------------------------------------------------------------------------------------------
	protected void LockSpawnedVehicle(notnull IEntity owningUser)
	{
		if (!m_SpawnedEntity)
			return;
		
		SCR_VehicleSpawnProtectionComponent protectionComp = SCR_VehicleSpawnProtectionComponent.Cast(m_SpawnedEntity.FindComponent(SCR_VehicleSpawnProtectionComponent));
		if (!protectionComp)
			return;
		
		SCR_CatalogEntitySpawnerComponentClass prefabData = SCR_CatalogEntitySpawnerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;
		
		protectionComp.SetVehicleOwner(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(owningUser));
		protectionComp.SetProtectionTime(prefabData.GetVehicleLockedDuration());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the resource component that is assigned to the spawner.
	//! \return SCR_ResourceComponent resource component.
	SCR_ResourceComponent GetSpawnerResourceComponent()
	{
		return m_ResourceComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! Get supply component assigned to spawner
	//! \return SCR_CampaignSuppliesComponent spawn supplies
	[Obsolete("SCR_CatalogEntitySpawnerComponent.GetSpawnerResourceComponent() should be used instead.")]
	SCR_CampaignSuppliesComponent GetSpawnerSupplyComponent()
	{
		return m_SupplyComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! Get resources left for spawner
	//! \return float spawn supplies
	float GetSpawnerResourceValue()
	{
		if (!m_ResourceComponent)
			return 0.0;
		
		SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
			
		if (!consumer)
			return 0.0;
		
		return consumer.GetAggregatedResourceValue();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get supplies left for spawner
	//! \return spawn supplies
	[Obsolete("SCR_CatalogEntitySpawnerComponent.GetSpawnerResourceValue() should be used instead.")]
	float GetSpawnerSupplies()
	{
		if (!m_SupplyComponent)
			return m_iCustomSupplies;

		return m_SupplyComponent.GetSupplies();
	}

	//------------------------------------------------------------------------------------------------
	//! Add or reduce (by entering negative value) supplies available to spawner
	//! \param[in] supplies amount of supplies to be added/reduced
	void AddSpawnerSupplies(float supplies)
	{
		if (m_ResourceComponent)
		{
			SCR_ResourceInteractor interactor;
			
			if (supplies >= 0)
			{
				SCR_ResourceGenerator generator = m_ResourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				
				if (!generator)
					return;
				
				interactor = generator;
				
				generator.RequestGeneration(supplies);
			}
			else
			{
				SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				
				if (!consumer)
					return;
				
				interactor = consumer;
				
				consumer.RequestConsumtion(-supplies);
			}
			
			if (m_OnSpawnerSuppliesChanged)
				m_OnSpawnerSuppliesChanged.Invoke(interactor.GetAggregatedResourceValue());
		}

		m_iCustomSupplies += supplies;
		Replication.BumpMe();

		if (m_OnSpawnerSuppliesChanged)
			m_OnSpawnerSuppliesChanged.Invoke(m_iCustomSupplies);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] user
	//! \return false if player spawned too recently
	protected bool CooldownCheck(notnull IEntity user)
	{
		int userId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (userId == 0)
			return false;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(userId));
		if (!playerController)
			return false;

		SCR_CampaignNetworkComponent networkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));
		if (!networkComponent)
			return false;

		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return false;

		SCR_ECharacterRank rank = SCR_CharacterRankComponent.GetCharacterRank(user);
		WorldTimestamp lastTimestamp = networkComponent.GetLastRequestTimestamp();
		WorldTimestamp timeout = lastTimestamp.PlusMilliseconds(factionManager.GetRankRequestCooldown(rank));
		ChimeraWorld world = GetOwner().GetWorld();
		return (lastTimestamp == 0 || timeout.Less(world.GetServerTimestamp()));
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction faction)
	{
		SetCurrentFactionCatalog();
	}

	//------------------------------------------------------------------------------------------------
	//! Called from PerformSpawn, if spawned entity is AIGroup
	protected void OnAIGroupSpawned(notnull SCR_AIGroup group, notnull IEntity user, notnull IEntity slotEntity, SCR_EntityLabelPointComponent rallyPoint = null)
	{
		SCR_SpawnerRequestComponent requestComponent = GetRequestComponentFromPlayerEntity(user);
		if (!requestComponent)
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_SpawnerAIGroupManagerComponent groupSpawningManager = SCR_SpawnerAIGroupManagerComponent.Cast(gameMode.FindComponent(SCR_SpawnerAIGroupManagerComponent));
		if (!groupSpawningManager)
			return;

		foreach (int i, ResourceName resName : group.m_aUnitPrefabSlots)
		{
			groupSpawningManager.QueueSpawn(this, resName, user, slotEntity, rallyPoint);
		}

		requestComponent.AddQueuedAI(group.m_aUnitPrefabSlots.Count());
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] resName
	//! \param[in] user
	//! \param[in] slotEntity
	//! \param[in] rallyPoint
	//protected void SpawnAIGroupMember(ResourceName resName, notnull IEntity user, IEntity slotEntity, SCR_EntityLabelPointComponent rallyPoint = null)
	void SpawnAIGroupMember(ResourceName resName, notnull IEntity user, IEntity slotEntity, SCR_EntityLabelPointComponent rallyPoint = null)
	{
		SCR_ChimeraCharacter ai = SCR_ChimeraCharacter.Cast(SpawnEntity(resName, slotEntity));
		if (!ai)
			return;

		OnChimeraCharacterSpawned(ai, user, rallyPoint);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAIAgentRemoved(SCR_AIGroup group, AIAgent ai)
	{
		SCR_AIGroup playerGroup;

		foreach (Tuple2<SCR_AIGroup, SCR_AIGroup> groups : m_aGroupsToAssign)
		{
			if (groups.param1 != group)
				continue;
			
			playerGroup = groups.param2;
		}

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerGroup.GetLeaderID()));
		if (!playerController)
			return;
		
		SCR_SpawnerRequestComponent spawnerReqComponent = SCR_SpawnerRequestComponent.Cast(playerController.FindComponent(SCR_SpawnerRequestComponent));
		if (spawnerReqComponent)
			spawnerReqComponent.AddQueuedAI(-1);
	}

	//------------------------------------------------------------------------------------------------
	//! Called from PerformSpawn, if spawned entity is ChimeraCharacter
	protected void OnChimeraCharacterSpawned(notnull SCR_ChimeraCharacter ai, notnull IEntity user, SCR_EntityLabelPointComponent rallyPoint = null)
	{
		AIControlComponent control = AIControlComponent.Cast(ai.FindComponent(AIControlComponent));
		if (!control)
			return;

		control.ActivateAI();

		// If other type of slot is used, thus without default waypoint, AI unit will be added directly into player group, not going anywhere before that.
		if (!rallyPoint)
		{
			AddAISoldierToPlayerGroup(ai, user);
			return;
		}

		SCR_CatalogEntitySpawnerComponentClass prefabData = SCR_CatalogEntitySpawnerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;

		SCR_AIWaypoint wp = CreateRallyPointWaypoint(rallyPoint);
		wp.SetCompletionRadius(prefabData.GetMoveWaypointCompletionRadius());

		AIAgent agent = control.GetAIAgent();
		if (!agent)
			return;

		SCR_AIGroup group = SCR_AIGroup.Cast(agent.GetParentGroup());
		if (!group)
		{
			Resource res = Resource.Load(prefabData.GetDefaultGroupPrefab());
			group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(res, GetGame().GetWorld()));
			if (!group)
				return;
		}

		group.AddAgent(agent);
		group.AddWaypoint(wp);

		if (!m_mGroupWaypoints)
			m_mGroupWaypoints = new map<AIWaypoint, SCR_AIGroup>();

		m_mGroupWaypoints.Insert(wp, group);

		group.GetOnWaypointCompleted().Insert(OnGroupWaypointFinished);
		group.GetOnAgentRemoved().Insert(OnAIAgentRemoved);

		if (!m_aGroupsToAssign)
			m_aGroupsToAssign = {};

		SCR_PlayerController playerController = GetPlayerControllerFromEntity(user);
		if (!playerController)
			return;

		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		m_aGroupsToAssign.Insert(new Tuple2<SCR_AIGroup, SCR_AIGroup>(group, groupManager.GetPlayerGroup(playerController.GetPlayerId())));
	}

	//------------------------------------------------------------------------------------------------
	//! Returns waypoint created as rally point for spawned unit
	protected SCR_AIWaypoint CreateRallyPointWaypoint(notnull SCR_EntityLabelPointComponent rallyPoint)
	{
		SCR_CatalogEntitySpawnerComponentClass prefabData = SCR_CatalogEntitySpawnerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return null;

		EntitySpawnParams wpParams = new EntitySpawnParams();
		wpParams.TransformMode = ETransformMode.WORLD;
		rallyPoint.GetOwner().GetTransform(wpParams.Transform);

		Resource wpRes = Resource.Load(prefabData.GetDefaultWaypointPrefab());
		if (!wpRes.IsValid())
			return null;

		return SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(wpRes, null, wpParams));
	}

	//------------------------------------------------------------------------------------------------
	//! Callback for Search query in EOnInit. Search will be stopped, if Base with Supply component is found.
	protected bool SupplyComponentSearchCallback(IEntity ent)
	{
		if (ent.FindComponent(SCR_CampaignMilitaryBaseComponent))
		{
			SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(ent);
			
			if (resourceComponent)
			{
				AssignResourceComponent(resourceComponent);
				
				return false;
			}
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_bEnableGracePeriod)
			return;
		
		m_fLastUpdateElapsedTime += timeSlice;
		
		if (m_fLastUpdateElapsedTime > UPDATE_PERIOD)
		{
			float worldTime = GetGame().GetWorld().GetWorldTime() / 1000.0;
			IEntity entity;
			vector entityMins;
			vector entityMaxs;
			vector entityMat[4];
			
			for (int idx, count = m_aGracePeriodEntries.Count(); idx < count; idx++)
			{
				RplId id = m_aGracePeriodEntries[idx];
				
				entity = IEntity.Cast(Replication.FindItem(id));
				
				if (!entity)
				{
					m_aGracePeriodEntries.Remove(idx);
					m_aGracePeriodStartingTimes.Remove(idx);
					idx = Math.Max(0, idx - 1);
					count = Math.Max(0, count - 1);
					
					continue;
				}
				
				entity.GetBounds(entityMins, entityMaxs);
				entity.GetWorldTransform(entityMat);
				
				if (worldTime < m_aGracePeriodStartingTimes[idx] && Math3D.IntersectionSphereAABB(GetOwner().GetOrigin().InvMultiply4(entityMat), m_fGracePeriodAreaRange, entityMins, entityMaxs))
					continue;
				
				UnregisterGracePeriod(id);
				idx = Math.Max(0, idx - 1);
				count = Math.Max(0, count - 1);
			}
			
			m_fLastUpdateElapsedTime = 0.0;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!m_RplComponent)
		{
			Print("SCR_CatalogEntitySpawnerComponent requires RplComponent for its functionality!", LogLevel.WARNING);
			return;
		}

		m_ActionManager = ActionsManagerComponent.Cast(owner.FindComponent(ActionsManagerComponent));
		if (!m_ActionManager)
			Print("No Action Manager detected on owner of Spawner Component!", LogLevel.WARNING);

		if (SCR_GameModeCampaign.GetInstance())
		{
			SCR_CatalogEntitySpawnerComponentClass prefabData = SCR_CatalogEntitySpawnerComponentClass.Cast(GetComponentData(GetOwner()));
			if (prefabData)
				GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), prefabData.GetSupplySearchRadius(), SupplyComponentSearchCallback);
		}
		
		m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(owner);
		
		SCR_CatalogEntitySpawnerComponent.INSTANCES.Insert(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		SCR_CatalogEntitySpawnerComponent.INSTANCES.RemoveItem(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		super.OnPostInit(owner);

		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CatalogEntitySpawnerComponent()
	{
		SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
	}
}

enum SCR_EEntityRequestStatus
{
	NOT_ENOUGH_SUPPLIES = 1,
	NOT_ENOUGH_SPACE = 2,
	RANK_LOW = 3,
	COOLDOWN = 4,
	REQUESTER_NOT_GROUPLEADER = 5,
	GROUP_FULL = 6,
	NOT_AVAILABLE = 7,
	CAN_SPAWN = 8,
	CAN_SPAWN_TRIGGER = 9,
	AI_LIMIT_REACHED = 10
}
