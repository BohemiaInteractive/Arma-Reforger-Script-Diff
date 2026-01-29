class SCR_ArsenalPlayerLoadout
{
	string loadout;
	ref SCR_PlayerLoadoutData loadoutData;
	float suppliesCost = 0.0;
	float m_fMilitarySupplyAllocationCost;
	SCR_ECharacterRank m_eRequiredRank;
}

//! Used to allow certain arsenal types to be enabled or disabled
enum SCR_EArsenalTypes
{
	STATIC_ENTITIES				= 1 << 0, //!< Arsenals are enabled on static entities such as arsenal boxes
	VEHICLES					= 1 << 1, //!< Arsenals are enabled on vehicles
	GADGETS						= 1 << 2, //!< Arsenals are enabled on gadgets
}

//~ Scriptinvokers
void SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged(int playerId, bool hasValidLoadout);
typedef func SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged;

[ComponentEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_ArsenalManagerComponentClass : SCR_BaseGameModeComponentClass
{

}

class SCR_ArsenalManagerComponent : SCR_BaseGameModeComponent
{
	protected static SCR_ArsenalManagerComponent s_Instance;

	[Attribute("{27F28CF7C6698FF8}Configs/Arsenal/ArsenalSaveTypeInfoHolder.conf", desc: "Holds a list of save types than can be used for arsenals. Any new arsenal save type should be added to the config to allow it to be set by Editor", params: "conf class=SCR_ArsenalSaveTypeInfoHolder")]
	protected ResourceName m_sArsenalSaveTypeInfoHolder;
	
	[Attribute("{183361B6DA2C304F}Configs/Arsenal/ArsenalLoadoutSaveBlacklists.conf", desc: "This is server only, A blacklist of entities that are not allowed to be saved at arsenals if the blacklist the item is in is enabled. Can be null.",params: "conf class=SCR_LoadoutSaveBlackListHolder")]
	protected ResourceName m_sLoadoutSaveBlackListHolder;
	
	[Attribute()]
	protected bool m_bDisable
	
	//=== Authority
	protected ref map<UUID, ref SCR_ArsenalPlayerLoadout> m_aPlayerLoadouts = new map<UUID, ref SCR_ArsenalPlayerLoadout>();
	
	//=== Broadcast
	protected ref ScriptInvokerBase<SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged> m_OnPlayerLoadoutUpdated = new ScriptInvokerBase<SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged>();
	
	protected ref ScriptInvokerInt m_OnArsenalGameModeTypeChanged = new ScriptInvokerInt();
	protected ref ScriptInvokerInt m_OnArsenalTypeEnabledChanged = new ScriptInvokerInt();
	protected ref ScriptInvokerBool m_OnMilitarySupplyAllocationToggle = new ScriptInvokerBool();
	protected ref ScriptInvokerFloat m_OnLoadoutSpawnSupplyCostMultiplierChanged = new ScriptInvokerFloat();
	
	protected ref SCR_ArsenalSaveTypeInfoHolder m_ArsenalSaveTypeInfoHolder;
	protected ref SCR_LoadoutSaveBlackListHolder m_LoadoutSaveBlackListHolder;
	
	protected bool m_bLocalPlayerLoadoutAvailable;
	ref SCR_PlayerLoadoutData m_LocalPlayerLoadoutData;
	
	[Attribute("0", desc: "Cost multiplier for supplies on spawning. 0 means the cost is free. Only used in gamemodes that support supply cost on spawning", params: "0 inf"), RplProp(onRplName: "OnLoadoutSpawnSupplyCostMultiplierChanged")]
	protected float m_fLoadoutSpawnSupplyCostMultiplier;
	
	[RplProp(onRplName: "OnArsenalGameModeTypeChanged"), Attribute("-1", desc: "This value dictates which arsenal items are available in the gamemode. UNRESTRICTED means there are no restrictions and all items with an ArsenalItem data in the catalog manager", uiwidget: UIWidgets.SearchComboBox, enums: SCR_Enum.GetList(SCR_EArsenalGameModeType, new ParamEnum("UNRESTRICTED", "-1")))]
	protected SCR_EArsenalGameModeType m_eArsenalGameModeType;
	
	[Attribute("{39B145760CDDFF59}Configs/Arsenal/ArsenalGameModeUIData.conf", desc: "Holds a list of all Arsenal Game Mode data types. Any entry in here will be displayed in the Editor attributes and can be set.", params: "conf class=SCR_ArsenalGameModeUIDataHolder")]
	protected ResourceName m_sArsenalGameModeUIDataHolder;
	
	[RplProp(onRplName: "OnArsenalTypeEnabledChanged"), Attribute(SCR_Enum.GetFlagValues(SCR_EArsenalTypes).ToString(), desc: "This determinds if arsenals are enabled on All entities or non-vehicle only", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalTypes))]
	protected SCR_EArsenalTypes m_eArsenalTypesEnabled;
	
	[Attribute(desc: "Items can be ranked locked if true. Conflict functionality only.")]
	protected bool m_bRankLockedItems;
	
	[Attribute(desc: "Link to config that holds military supply allocation data", params: "conf class=SCR_MilitarySupplyAllocationConfig")]
	protected ResourceName m_sMilitarySupplyAllocationConfigHolder;
	
	[Attribute("0", desc: "Items consume Military Supply Allocation if true")]
	protected bool m_bUseMilitarySupplyAllocation;
	
	[Attribute("1", desc: "Multiplier of refund value for items that belong to the same faction as Arsenal.", params: "0 1 0.01")]
	protected float m_fRefundSameFactionItemMultiplier;
	
	[Attribute("1", desc: "Multiplier of refund value for items belonging to different faction from Arsenal.", params: "0 1 0.01")]
	protected float m_fRefundDifferentFactionItemMultiplier;
	
	protected ref SCR_ArsenalGameModeUIDataHolder m_ArsenalGameModeUIDataHolder;
	
	//------------------------------------------------------------------------------------------------
	static void GetArsenalLoadoutComponentsToCheck(out array<typename> componentsToCheck)
	{
		componentsToCheck = {
			SCR_CharacterInventoryStorageComponent,
			SCR_UniversalInventoryStorageComponent,
			EquipedWeaponStorageComponent,
			ClothNodeStorageComponent,
			SCR_EquipmentStorageComponent,
			SCR_WeaponAttachmentsStorageComponent
		};
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] arsenalManager The arsenal manager that is obtained
	//! \return If arsenal manager was succesfully obtained
	static bool GetArsenalManager(out SCR_ArsenalManagerComponent arsenalManager)
	{
		arsenalManager = s_Instance;
		return s_Instance != null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] item Item to get refund supply cost for. Weapons will also calculate the cost of attachments
	//! \param[in] arsenalComponent The arsenal component if the it is refunded to an arsenal
	//! \param[in] mustHaveSupplyStorage If true will return -1 if no arsenal component or no resource component
	//! \param[out] isSupplyStorageAvailable Will return false if there was no storage to put the supplies in or if there is no space left in any storage
	//! \return Will return the calculated supply cost (this can be 0) or -1 which is the same as 0 but will hide any UI related widgets rather than showing 0
	static float GetItemRefundAmount(notnull IEntity item, SCR_ArsenalComponent arsenalComponent, bool mustHaveSupplyStorage, out bool isSupplyStorageAvailable = true)
	{
		isSupplyStorageAvailable = !mustHaveSupplyStorage;
		
		//~ Get prefab data
		EntityPrefabData prefabData = item.GetPrefabData();
		if (!prefabData)
			return -1;
		
		//~ Get prefab
		ResourceName itemPrefab = prefabData.GetPrefabName();
		if (itemPrefab.IsEmpty())
			return -1;
		
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return -1;
		
		SCR_Faction faction;
		SCR_EArsenalSupplyCostType supplyCostType = SCR_EArsenalSupplyCostType.DEFAULT;
		float refundMultiplier = 1;
		
		SCR_ResourceGenerator generator;
		
		if (arsenalComponent)
		{
			//~ Does not use supplies
			if (!arsenalComponent.IsArsenalUsingSupplies())
				return -1;
			
			faction = arsenalComponent.GetAssignedFaction();
			supplyCostType = arsenalComponent.GetSupplyCostType();
			
			SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(arsenalComponent.GetOwner().FindComponent(SCR_ResourceComponent));
			if (resourceComponent)
			{
				generator = resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				if (generator)
					refundMultiplier = generator.GetResourceMultiplier();
			}
		}
		//~ No arsenal so so supply storage
		else if (mustHaveSupplyStorage)
		{
			return -1;
		}
		//~ No arsenal so check if supplies are enabled or not globally
		else if (!SCR_ResourceSystemHelper.IsGlobalResourceTypeEnabled())
		{
			return -1;
		}
		
		//~ No generator so no storage
		if (!generator && mustHaveSupplyStorage)
			return -1;
		
		//~ The total item refund amount
		set<IEntity> countedItems = new set<IEntity>();
		float refundAmount = GetItemRefundCost(countedItems, item, entityCatalogManager, supplyCostType, refundMultiplier, faction);
		
		//~ Refund amount is invalid
		if (refundAmount < 0)
			return -1;
		
		//~ Get stored Items refund amount
		refundAmount += GetStoredItemsRefundAmount(countedItems, item, entityCatalogManager, supplyCostType, refundMultiplier, faction);
	
		//~ Can the supplies be stored?
		if (mustHaveSupplyStorage)
		{
			isSupplyStorageAvailable = refundAmount == 0;
			
			if (refundAmount > 0 && generator)
				isSupplyStorageAvailable = refundAmount > 0 && generator.GetAggregatedResourceValue() + refundAmount <= generator.GetAggregatedMaxResourceValue();
		}
		
		return refundAmount;	
	}
	
	//------------------------------------------------------------------------------------------------
	static protected int GetItemRefundCost(notnull inout set<IEntity> countedItems, notnull IEntity item, notnull SCR_EntityCatalogManagerComponent entityCatalogManager, SCR_EArsenalSupplyCostType supplyCostType, float refundMultiplier, SCR_Faction faction, bool minusIfInvalid = true)
	{
		if (countedItems.Contains(item))
			return 0;
		
		//~ Get prefab data
		EntityPrefabData prefabData = item.GetPrefabData();
		if (!prefabData)
			return minusIfInvalid * -1;
		
		ResourceName itemPrefab = prefabData.GetPrefabName();
		if (itemPrefab.IsEmpty())
			return minusIfInvalid * -1;
		
		//~ Could not find the base entry
		SCR_EntityCatalogEntry entry = entityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.ITEM, itemPrefab, faction);
		if (!entry)
			return minusIfInvalid * -1;
		
		array<SCR_BaseEntityCatalogData> entityDataList = {};
		if (entry.GetEntityDataList(entityDataList) <= 0)
			return minusIfInvalid * -1;
		
		SCR_ArsenalManagerComponent arsenalManagerComponent;
		if (!GetArsenalManager(arsenalManagerComponent))
			return minusIfInvalid * -1;
		
		// Adjust refund multiplier based on arsenal and item factions
		if (entry.GetCatalogParent() == entityCatalogManager.GetFactionEntityCatalogOfType(EEntityCatalogType.ITEM, faction.GetFactionKey()))
			refundMultiplier *= arsenalManagerComponent.m_fRefundSameFactionItemMultiplier;
		else
			refundMultiplier *= arsenalManagerComponent.m_fRefundDifferentFactionItemMultiplier;
		
		SCR_ArsenalItem arsenalData;
		SCR_NonArsenalItemCostCatalogData nonArsenalData;
		
		float refundAmount;
		
		//~ Get refund amount of entry
		foreach (SCR_BaseEntityCatalogData data : entityDataList)
		{
			arsenalData = SCR_ArsenalItem.Cast(data);
			if (arsenalData)
			{
				refundAmount = SCR_ResourceSystemHelper.RoundRefundSupplyAmount(arsenalData.GetSupplyRefundAmount(supplyCostType, false) * refundMultiplier);
				countedItems.Insert(item);
				break;
			}
			
			nonArsenalData = SCR_NonArsenalItemCostCatalogData.Cast(data);
			if (nonArsenalData)
			{
				refundAmount = SCR_ResourceSystemHelper.RoundRefundSupplyAmount(nonArsenalData.GetSupplyCost(supplyCostType, false) * refundMultiplier);
				countedItems.Insert(item);
				break;
			}
		}
		
		//~ Get attachments refund amount if the item is a weapon
		if (arsenalData && (arsenalData.GetItemMode() == SCR_EArsenalItemMode.WEAPON || arsenalData.GetItemMode() == SCR_EArsenalItemMode.WEAPON_VARIANTS))
		{
			array<Managed> attachments = {};
			item.FindComponents(AttachmentSlotComponent, attachments);
			
			IEntity attachedEntity;
			ResourceName attachmentPrefab;
			
			AttachmentSlotComponent attachment;
			
			foreach (Managed managed : attachments)
			{
				attachment = AttachmentSlotComponent.Cast(managed);
				if (!attachment)
					continue;
				
				attachedEntity = attachment.GetAttachedEntity();
				if (!attachedEntity)
					continue;
				
				prefabData = attachedEntity.GetPrefabData();
				if (!prefabData)
					continue;
				
				attachmentPrefab = prefabData.GetPrefabName();
				if (attachmentPrefab.IsEmpty())
					continue;
				
				entry = entityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.ITEM, attachmentPrefab, faction);
				if (!entry)
					continue;
				
				if (entry.GetEntityDataList(entityDataList) <= 0)
					continue;
				
				//~ Get the supply cost of attachment
				foreach (SCR_BaseEntityCatalogData data : entityDataList)
				{
					arsenalData = SCR_ArsenalItem.Cast(data);
					if (arsenalData)
					{
						refundAmount += SCR_ResourceSystemHelper.RoundRefundSupplyAmount(arsenalData.GetSupplyRefundAmount(supplyCostType, false) * refundMultiplier);
						countedItems.Insert(attachedEntity);
						break;
					}
					
					nonArsenalData = SCR_NonArsenalItemCostCatalogData.Cast(data);
					if (nonArsenalData)
					{
						refundAmount += SCR_ResourceSystemHelper.RoundRefundSupplyAmount(nonArsenalData.GetSupplyCost(supplyCostType, false) * refundMultiplier);
						countedItems.Insert(attachedEntity);
						break;
					}
				}
			}
		}
		
		return refundAmount;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Gets all item in storage and calculate the refund amount.
	static protected int GetStoredItemsRefundAmount(notnull inout set<IEntity> countedItems, notnull IEntity item, notnull SCR_EntityCatalogManagerComponent entityCatalogManager, SCR_EArsenalSupplyCostType supplyCostType, float refundMultiplier, SCR_Faction faction)
	{
		BaseInventoryStorageComponent inventoryStorage = BaseInventoryStorageComponent.Cast(item.FindComponent(BaseInventoryStorageComponent));
		if (!inventoryStorage)
			return 0;
		
		array<IEntity> storedItems = {};
		inventoryStorage.GetAll(storedItems);
		
		int storedItemRefundAmount;
		
		foreach (IEntity storedItem : storedItems)
		{
			storedItemRefundAmount += GetItemRefundCost(countedItems, storedItem, entityCatalogManager, supplyCostType, refundMultiplier, faction, false);
			
			//~ Get stored item refund amount
			storedItemRefundAmount += GetStoredItemsRefundAmount(countedItems, storedItem, entityCatalogManager, supplyCostType, refundMultiplier, faction);
		}
		
		return storedItemRefundAmount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! When an item is refunded in the arsenal (Server Only)
	//! \param[in] item Item that is being refunded
	//! \param[in] playerController Player controller of player that refunds the item
	//! \param[in] arsenal Arsenal the item was refunded at (Can potentially be null)
	static void OnItemRefunded_S(notnull IEntity item, notnull PlayerController playerController, SCR_ArsenalComponent arsenal)
	{
		SCR_ArsenalManagerComponent arsenalManager;
		if (!GetArsenalManager(arsenalManager))
			return;
		
		//~ Only execute if has player controller and if server
		if ((arsenalManager && !arsenalManager.GetGameMode().IsMaster()) || (!arsenalManager && Replication.IsClient()))
			return;
		
		SCR_ArsenalRefundEffectComponent arsenalRefundEffectComp = SCR_ArsenalRefundEffectComponent.Cast(item.FindComponent(SCR_ArsenalRefundEffectComponent));
		if (!arsenalRefundEffectComp)
			return;
		
		//~ Get Prefab class data
		SCR_ArsenalRefundEffectComponentClass arsenalRefundEffectClass = SCR_ArsenalRefundEffectComponentClass.Cast(arsenalRefundEffectComp.GetComponentData(arsenalRefundEffectComp.GetOwner()));
		if (!arsenalRefundEffectClass)
			return;
		
		arsenalRefundEffectClass.ExecuteFirstValidRefundEffect(item, playerController, arsenal);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Arsenal type flags enabled in the game mode
	SCR_EArsenalTypes GetEnabledArsenalTypes()
	{
		return m_eArsenalTypesEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return True if given arsenal flag is enabled in gamemode
	static bool IsArsenalTypeEnabled_Static(SCR_EArsenalTypes arsenalType)
	{
		SCR_ArsenalManagerComponent arsenalManager;
		
		if (!GetArsenalManager(arsenalManager))
			return false;
		
		return arsenalManager.IsArsenalTypeEnabled(arsenalType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return True if given arsenal flag is enabled in gamemode
	bool IsArsenalTypeEnabled(SCR_EArsenalTypes arsenalType)
	{
		return SCR_Enum.HasFlag(m_eArsenalTypesEnabled, arsenalType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Changes the enabled arsenal types which will enable/disable the arsenals in the world if the player did not enable the arsenal already
	//! \param[in] newEnabledArsenalType All arsenal type flags that are enabled
	//! \param[in] playerID Optional to send notification on arsenal types changed
	void SetEnabledArsenalTypes(SCR_EArsenalTypes newEnabledArsenalTypes, int playerID = -1)
	{		
		if (newEnabledArsenalTypes == m_eArsenalTypesEnabled)
			return;
		
		//~ Server only
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if ((gameMode && !gameMode.IsMaster()) || (!gameMode && Replication.IsClient()))
			return;
		
		m_eArsenalTypesEnabled = newEnabledArsenalTypes;
		
		if (playerID > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_CHANGED_ARSENAL_TYPE_ENABLED, playerID);
		
		Replication.BumpMe();
		
		if ((gameMode && gameMode.IsMaster()) || (!gameMode && Replication.IsServer()))
			OnArsenalTypeEnabledChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	//! If items can be ranked locked
	bool AreItemsRankLocked()
	{
		return m_bRankLockedItems;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsMilitarySupplyAllocationEnabled()
	{
		SCR_ArsenalManagerComponent arsenalManager;

		if (!GetArsenalManager(arsenalManager))
			return false;

		return arsenalManager.m_bUseMilitarySupplyAllocation;
	}

	//------------------------------------------------------------------------------------------------
	//! Enables or disables Military Supply Allocation by Rank
	//! \param[in] enable
	void SetMilitarySupplyAllocationEnabled(bool enable)
	{
		if (enable == m_bUseMilitarySupplyAllocation)
			return;

		m_bUseMilitarySupplyAllocation = enable;
		OnMilitarySupplyAllocationToggle();
		Rpc(RpcAsk_SetMilitarySupplyAllocationEnabled, enable);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcAsk_SetMilitarySupplyAllocationEnabled(bool enable)
	{
		m_bUseMilitarySupplyAllocation = enable;
		OnMilitarySupplyAllocationToggle();
	}

	//------------------------------------------------------------------------------------------------
	//! \return Script invoker on military supply allocation enabled changed.
	ScriptInvokerBool GetOnMilitarySupplyAllocationToggle()
	{
		if (!m_OnMilitarySupplyAllocationToggle)
			m_OnMilitarySupplyAllocationToggle = new ScriptInvokerBool();

		return m_OnMilitarySupplyAllocationToggle;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMilitarySupplyAllocationToggle()
	{
		if (m_OnMilitarySupplyAllocationToggle)
			m_OnMilitarySupplyAllocationToggle.Invoke(m_bUseMilitarySupplyAllocation);
	}

	//------------------------------------------------------------------------------------------------
	//! \return Script invoker on enabled arsenal types changed. Sends over the current enabled arsenal flags
	ScriptInvokerInt GetOnArsenalTypeEnabledChanged()
	{
		return m_OnArsenalTypeEnabledChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnArsenalTypeEnabledChanged()
	{
		m_OnArsenalTypeEnabledChanged.Invoke(m_eArsenalTypesEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get Save type info holder
	SCR_ArsenalSaveTypeInfoHolder GetArsenalSaveTypeInfoHolder()
	{
		return m_ArsenalSaveTypeInfoHolder;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The current Arsenal Game mode type. -1 Means it is unrestricted
	static SCR_EArsenalGameModeType GetArsenalGameModeType_Static()
	{
		SCR_ArsenalManagerComponent arsenalManager;
		
		if (!GetArsenalManager(arsenalManager))
			return -1;
		
		return arsenalManager.GetArsenalGameModeType();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The current Arsenal Game mode type. -1 Means it is unrestricted
	SCR_EArsenalGameModeType GetArsenalGameModeType()
	{
		return m_eArsenalGameModeType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] arsenalGameModeType Mode to set, -1 means the mode is unrestricted
	//! \param[in] playerID Optional to send notification that the arsenal mode has been changed
	void SetArsenalGameModeType_S(SCR_EArsenalGameModeType arsenalGameModeType, int playerID = 0)
	{
		//~ Server only
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if ((gameMode && !gameMode.IsMaster()) || (!gameMode && Replication.IsClient()))
			return;
		
		//~ Set Unrestricted
		if (arsenalGameModeType <= 0)
			arsenalGameModeType = -1;
		
		//~ Send notification
		if (playerID > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_CHANGED_ARSENAL_GAMEMODE_TYPE, playerID, arsenalGameModeType);
		
		m_eArsenalGameModeType = arsenalGameModeType;
		Replication.BumpMe();
		
		if ((gameMode && gameMode.IsMaster()) || (!gameMode && Replication.IsServer()))
			OnArsenalGameModeTypeChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] arsenalGameModeType Type to get Ui info for. -1 gets the unrestricted game mode type
	//! \return UI info of given Arsenal Mode type.
	SCR_UIInfo GetArsenalGameModeUIInfo(SCR_EArsenalGameModeType arsenalGameModeType)
	{
		if (!m_ArsenalGameModeUIDataHolder)
		{
			Print("Arsenal Manager is missing the 'm_ArsenalGameModeUIDataHolder'!", LogLevel.WARNING);
		
			SCR_UIInfo unknown = new SCR_UIInfo();
			unknown.SetName("MISSING");
			return unknown;
		}
		
		return m_ArsenalGameModeUIDataHolder.GetArsenalGameModeUIInfo(arsenalGameModeType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[inout] arsenalGameModeTypeUIInfoList Return list off all the UI data for the arsenal Game mode type
	//! \return Count of all the arsenal game mode types
	int GetArsenalGameModeUIInfoList(inout notnull array<SCR_ArsenalGameModeUIData> arsenalGameModeTypeUIInfoList)
	{
		arsenalGameModeTypeUIInfoList.Clear();
		
		if (!m_ArsenalGameModeUIDataHolder)
		{
			Print("Arsenal Manager is missing the 'm_ArsenalGameModeUIDataHolder'!", LogLevel.WARNING);
			return 0;
		}
		
		return m_ArsenalGameModeUIDataHolder.GetArsenalGameModeUIInfoList(arsenalGameModeTypeUIInfoList);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnArsenalGameModeTypeChanged()
	{
		m_OnArsenalGameModeTypeChanged.Invoke(m_eArsenalGameModeType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Script invoker when arsenal Game mode type changes
	ScriptInvokerInt GetOnArsenalGameModeTypeChanged()
	{
		return m_OnArsenalGameModeTypeChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get Loadout save blacklist holder
	SCR_LoadoutSaveBlackListHolder GetLoadoutSaveBlackListHolder()
	{
		return m_LoadoutSaveBlackListHolder;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update BlackList active states (Server only)
	//! \param[in] orderedActiveStates An ordered list of all blacklists and their new active state
	//! \param[in] clearExistingLoadouts
	//! \param[in] editorPlayerIdClearedLoadout
	void SetLoadoutBlackListActiveStates(notnull array<bool> orderedActiveStates, bool clearExistingLoadouts, int editorPlayerIdClearedLoadout = -1)
	{
		if (!m_LoadoutSaveBlackListHolder || !GetGameMode().IsMaster())
			return;
		
		if (editorPlayerIdClearedLoadout > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_CHANGED_LOADOUT_SAVE_BLACKLIST, editorPlayerIdClearedLoadout);
		
		m_LoadoutSaveBlackListHolder.SetOrderedBlackListsActive(orderedActiveStates);
		
		//~ Clear existing loadouts
		if (clearExistingLoadouts)
		{
			//~ Clear local loadout
			Rpc(RPC_OnPlayerLoadoutCleared, editorPlayerIdClearedLoadout);
			RPC_OnPlayerLoadoutCleared(editorPlayerIdClearedLoadout);
			
			//~ Clear existing loadouts on server
			m_aPlayerLoadouts.Clear();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_OnPlayerLoadoutCleared(int playerIdClearedLoadout)
	{
		if (!m_bLocalPlayerLoadoutAvailable)
			return;
		
		//~ No local loadout available anymore
		m_bLocalPlayerLoadoutAvailable = false;
		
		//~ Player loadout was cleared. This can happen when respawn menu is open and needs to be refreshed
		m_OnPlayerLoadoutUpdated.Invoke(SCR_PlayerController.GetLocalPlayerId(), false);
		
		//~ Send notification to player to inform them their loadout was cleared
		if (playerIdClearedLoadout > 0)
			SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_CLEARED_BY_EDITOR);
		else 
			SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_CLEARED);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetLocalPlayerLoadoutAvailable()
	{
		if (!m_bLocalPlayerLoadoutAvailable || !m_LocalPlayerLoadoutData)
			return false;
		
		Faction faction = SCR_FactionManager.SGetLocalPlayerFaction();
		if (!faction)
			return false;

		return m_LocalPlayerLoadoutData.FactionIndex == GetGame().GetFactionManager().GetFactionIndex(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	 ScriptInvokerBase<SCR_ArsenalManagerComponent_OnPlayerLoadoutChanged> GetOnLoadoutUpdated()
	{
		return m_OnPlayerLoadoutUpdated;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Authority side
	//! \param[in] playerId
	//! \param[out] playerLoadout
	//! \return
	bool GetPlayerArsenalLoadout(UUID playerUID, out SCR_ArsenalPlayerLoadout playerLoadout)
	{
		return m_aPlayerLoadouts.Find(playerUID, playerLoadout) && playerLoadout.loadout != string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the loadout supply multiplier (Server only)
	//! \param[in] multiplier new multiplier to set
	//! \param[in] playerID To send Notifications (optional)
	void SetLoadoutSpawnCostMultiplier_S(float multiplier, int playerID = 0)
	{
		//~ Same or invalid values
		if (m_fLoadoutSpawnSupplyCostMultiplier == multiplier || multiplier < 0)
			return;
		
		m_fLoadoutSpawnSupplyCostMultiplier = multiplier;
		Replication.BumpMe();
		
		//~ Notification
		if (playerID > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_CHANGED_SPAWN_SUPPLYCOST_MULTIPLIER, playerID, m_fLoadoutSpawnSupplyCostMultiplier * 1000);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if ((gameMode && gameMode.IsMaster()) || (!gameMode && Replication.IsServer()))
			OnLoadoutSpawnSupplyCostMultiplierChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The spawn cost multiplier. Will always be 0 if supplies are disabled
	float GetCalculatedLoadoutSpawnSupplyCostMultiplier()
	{
		if (!SCR_ResourceSystemHelper.IsGlobalResourceTypeEnabled())
			return 0;
		
		return m_fLoadoutSpawnSupplyCostMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The spawn cost multiplier. Does not care if supplies are enabled. Only used by editor
	float GetLoadoutSpawnSupplyCostMultiplier()
	{
		return m_fLoadoutSpawnSupplyCostMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLoadoutSpawnSupplyCostMultiplierChanged()
	{
		m_OnLoadoutSpawnSupplyCostMultiplierChanged.Invoke(m_fLoadoutSpawnSupplyCostMultiplier);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return ScriptInvoker on Loadout spawn cost multiplier changed
	ScriptInvokerFloat GetOnLoadoutSpawnSupplyCostMultiplierChanged()
	{
		return m_OnLoadoutSpawnSupplyCostMultiplierChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the cost of spawning as a player at given base
	//! \param[in] playerLoadout Loadout to check supply cost for
	//! \param[in] getLocalPlayer If true and the loadout is a SCR_PlayerArsenalLoadout then it will get the locally stored data for the arsenal loadout
	//! \param[in] playerID Player ID of player that want's to spawn, Required only if !getLocalPlayer
	//! \param[in] playerFaction (Optional) Faction of player to spawn to speed up getting the spawn cost data from Catalog
	//! \param[in] spawnTarget Spawnpoint of spawn location. Not required but without it, it cannot know if the spawnpoint has supplies enabled or other values if the spawnpoint is a base
	//! \param[out] spawnPointParentBase Returns the base if the spawnpoint is a base composition
	//! \param[out] spawnpointResourceComponent Returns the Resource component from the spawnpoint or base
	//! \return Returns the total cost of spawning with the given loadout
	static float GetLoadoutCalculatedSupplyCost(notnull SCR_BasePlayerLoadout playerLoadout, bool getLocalPlayer, int playerID, SCR_Faction playerFaction, IEntity spawnTarget, out SCR_CampaignMilitaryBaseComponent spawnPointParentBase = null, out SCR_ResourceComponent spawnpointResourceComponent = null)
	{	
		//~ Check if spawn cost is enabled
		float multiplier = 1;
		SCR_ArsenalManagerComponent arsenalManager;
		if (!GetArsenalManager(arsenalManager))
			return 0;
		
		if (spawnTarget)
		{
			IEntity parent = spawnTarget;
			
			//~ Check if spawn target is a base
			while (parent)
			{
				spawnPointParentBase = SCR_CampaignMilitaryBaseComponent.Cast(parent.FindComponent(SCR_CampaignMilitaryBaseComponent));
	
				if (spawnPointParentBase)
					break;
	
				parent = parent.GetParent();
			}
			
			if (spawnPointParentBase)
				spawnpointResourceComponent = spawnPointParentBase.GetResourceComponent();
			else 
				spawnpointResourceComponent = SCR_ResourceComponent.FindResourceComponent(spawnTarget);
		}
		
		multiplier = arsenalManager.GetCalculatedLoadoutSpawnSupplyCostMultiplier();
		
		//~ Spawn multiplier is 0
		if (multiplier <= 0)
			return 0;
		
		//~ Supplies are disabled
		if (spawnpointResourceComponent && !spawnpointResourceComponent.IsResourceTypeEnabled())
			return 0;
		else if (!spawnpointResourceComponent && !SCR_ResourceSystemHelper.IsGlobalResourceTypeEnabled())
			return 0;

		float baseMultiplier = 0;
		
		if (spawnPointParentBase)
		{
			if (!spawnPointParentBase.CostSuppliesToSpawn())
				return 0;
			
			baseMultiplier = spawnPointParentBase.GetBaseSpawnCostFactor();
		}
			
		SCR_PlayerArsenalLoadout playerArsenalLoadout = SCR_PlayerArsenalLoadout.Cast(playerLoadout);
		
		if (playerArsenalLoadout)
		{
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			
			//~ Local
			if (getLocalPlayer && arsenalManager.m_LocalPlayerLoadoutData)
			{
				if (baseMultiplier > 0)
					return Math.Clamp(arsenalManager.m_LocalPlayerLoadoutData.LoadoutCost * baseMultiplier, 0, float.MAX);
				else
					return Math.Clamp(arsenalManager.m_LocalPlayerLoadoutData.LoadoutCost, 0, float.MAX);
			}
			//~ Server
			else if ((gameMode && gameMode.IsMaster()) || (!gameMode && Replication.IsServer()))
			{
				if (baseMultiplier > 0)
					return Math.Clamp(playerArsenalLoadout.GetLoadoutSuppliesCost(playerID) * baseMultiplier, 0, float.MAX);
				else
					return Math.Clamp(playerArsenalLoadout.GetLoadoutSuppliesCost(playerID), 0, float.MAX);
			}
			
			//~ No custom loadout found so spawn cost is 0
			return 0;
		}
		//~ Not an arsenal loadout so take cost from catalog
		else 
		{
			SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
			if (!entityCatalogManager)
				return 0;
			
			ResourceName loadoutResource = playerLoadout.GetLoadoutResource();
			if (!loadoutResource)
				return 0;
				
			Resource resource = Resource.Load(loadoutResource);
			if (!resource)
				return 0;
			
			SCR_EntityCatalogEntry entry = entityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.CHARACTER, resource.GetResource().GetResourceName(), playerFaction);
			if (!entry)
			{
				Print("Loadout: '" + resource.GetResource().GetResourceName() + "' is not in catalog so supply cost is 0!", LogLevel.WARNING);
				return 0;
			}
				
			SCR_EntityCatalogLoadoutData data = SCR_EntityCatalogLoadoutData.Cast(entry.GetEntityDataOfType(SCR_EntityCatalogLoadoutData));
			float supplyCost;
			
			if (data)
			{
				supplyCost = data.GetLoadoutSpawnCost();
			}
			//~ No data so try to get Campaign budget for supply cost
			else 
			{
				Print("Loadout: '" + resource.GetResource().GetResourceName() + "' has no 'SCR_EntityCatalogLoadoutData' assigned in catalog so supply cost is taken from Campaign budget on prefab!", LogLevel.WARNING);
				
				SCR_EditableEntityUIInfo editableUIInfo = SCR_EditableEntityUIInfo.Cast(entry.GetEntityUiInfo());
				if (!editableUIInfo)
					return 0;
				
				array<ref SCR_EntityBudgetValue> budgets = {};
				
				if (!editableUIInfo.GetEntityBudgetCost(budgets))
					return 0;
				
				//~ Get campaign budget
				foreach (SCR_EntityBudgetValue budget : budgets)
				{
					if (budget.GetBudgetType() == EEditableEntityBudget.CAMPAIGN)
					{
						supplyCost = budget.GetBudgetValue();
						break;
					}
				}
				
				//~ No campaign budget assigned
				if (supplyCost <= 0)
					return 0;
			}
			
			//~ Add multipliers to the supply cost
			if (baseMultiplier > 0)
				return Math.Clamp((supplyCost * multiplier) * baseMultiplier, 0, float.MAX);
			else 
				return Math.Clamp(supplyCost * multiplier, 0, float.MAX);
		}
		
		//~ If all fails set supply cost 0
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetCustomLoadoutMilitarySupplyAllocationCost(int playerId)
	{
		if (!m_bUseMilitarySupplyAllocation)
			return 0;

		if (SCR_PlayerController.GetLocalPlayerId() == playerId)
		{
			SCR_ArsenalManagerComponent arsenalManager;
			if (!GetArsenalManager(arsenalManager))
				return 0;

			return arsenalManager.m_LocalPlayerLoadoutData.LoadoutPlayerSupplyAllocationCost;
		}

		const UUID playerUID = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		SCR_ArsenalPlayerLoadout playerLoadout = m_aPlayerLoadouts.Get(playerUID);
		if (!playerLoadout)
			return 0;

		return playerLoadout.m_fMilitarySupplyAllocationCost * GetCalculatedLoadoutSpawnSupplyCostMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	protected float GetDefaultLoadoutMilitarySuppplyAllocationCost(notnull SCR_BasePlayerLoadout playerLoadout)
	{
		if (!m_bUseMilitarySupplyAllocation)
			return 0;

		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return 0;

		ResourceName loadoutResource = playerLoadout.GetLoadoutResource();
		if (!loadoutResource)
			return 0;

		Resource resource = Resource.Load(loadoutResource);
		if (!resource)
			return 0;

		SCR_EntityCatalogEntry entry = entityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.CHARACTER, resource.GetResource().GetResourceName());
		if (!entry)
		{
			Print("Loadout: '" + resource.GetResource().GetResourceName() + "' is not in catalog so supply cost is 0!", LogLevel.WARNING);
			return 0;
		}

		SCR_EntityCatalogLoadoutData data = SCR_EntityCatalogLoadoutData.Cast(entry.GetEntityDataOfType(SCR_EntityCatalogLoadoutData));
		float supplyCost;

		if (data)
		{
			supplyCost = data.GetLoadoutSpawnCost();
		}
		//~ No data so try to get Campaign budget for supply cost
		else 
		{
			Print("Loadout: '" + resource.GetResource().GetResourceName() + "' has no 'SCR_EntityCatalogLoadoutData' assigned in catalog so supply cost is taken from Campaign budget on prefab!", LogLevel.WARNING);

			SCR_EditableEntityUIInfo editableUIInfo = SCR_EditableEntityUIInfo.Cast(entry.GetEntityUiInfo());
			if (!editableUIInfo)
				return 0;

			array<ref SCR_EntityBudgetValue> budgets = {};

			if (!editableUIInfo.GetEntityBudgetCost(budgets))
				return 0;

			//~ Get campaign budget
			foreach (SCR_EntityBudgetValue budget : budgets)
			{
				if (budget.GetBudgetType() == EEditableEntityBudget.CAMPAIGN)
				{
					supplyCost = budget.GetBudgetValue();
					break;
				}
			}

			//~ No campaign budget assigned
			if (supplyCost <= 0)
				return 0;
		}

		return supplyCost * GetCalculatedLoadoutSpawnSupplyCostMultiplier();
	}

	//------------------------------------------------------------------------------------------------
	float GetLoadoutMilitarySupplyAllocationCost(notnull SCR_BasePlayerLoadout playerLoadout, int playerId)
	{
		if (!m_bUseMilitarySupplyAllocation)
			return 0;

		SCR_PlayerArsenalLoadout playerArsenalLoadout = SCR_PlayerArsenalLoadout.Cast(playerLoadout);
		if (playerArsenalLoadout)
			return GetCustomLoadoutMilitarySupplyAllocationCost(playerId);

		//~ Default loadouts cost the same as their supply cost
		return GetDefaultLoadoutMilitarySuppplyAllocationCost(playerLoadout);
	}

	//------------------------------------------------------------------------------------------------
	int GetItemMilitarySupplyAllocationCost(ResourceName resource, SCR_ArsenalComponent arsenalComponent, bool addAdditionalCosts = false)
	{
		if (!m_bUseMilitarySupplyAllocation)
			return 0;

		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return 0;

		SCR_EntityCatalogEntry catalogEntry = entityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.ITEM, resource, arsenalComponent.GetAssignedFaction());
		if (!catalogEntry)
			return 0;

		SCR_ArsenalItem arsenalItemData = SCR_ArsenalItem.Cast(catalogEntry.GetEntityDataOfType(SCR_ArsenalItem));
		if (!arsenalItemData)
		{
			SCR_NonArsenalItemCostCatalogData nonArsenalItemData = SCR_NonArsenalItemCostCatalogData.Cast(catalogEntry.GetEntityDataOfType(SCR_NonArsenalItemCostCatalogData));
			if (nonArsenalItemData)
				return nonArsenalItemData.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT, addAdditionalCosts);
			else
				return 0;
		}

		if (!arsenalItemData.GetUseMilitarySupplyAllocation())
			return 0;

		return arsenalItemData.GetMilitarySupplyAllocationCost(SCR_EArsenalSupplyCostType.DEFAULT, addAdditionalCosts);
	}

	//------------------------------------------------------------------------------------------------
	int GetItemMilitarySupplyAllocationRefundAmount(notnull IEntity item, SCR_ArsenalComponent arsenalComponent)
	{
		if (!m_bUseMilitarySupplyAllocation)
			return 0;

		if (!arsenalComponent || !arsenalComponent.IsArsenalUsingSupplies())
			return 0;

		SCR_Faction faction = arsenalComponent.GetAssignedFaction();
		if (!faction)
			return 0;

		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return 0;

		set<IEntity> countedItems = new set<IEntity>();
		return GetItemMilitarySupplyAllocationRefundCost(item, countedItems, entityCatalogManager, arsenalComponent, faction);
	}

	//------------------------------------------------------------------------------------------------
	protected int GetItemMilitarySupplyAllocationRefundCost(notnull IEntity item, inout set<IEntity> countedItems, SCR_EntityCatalogManagerComponent entityCatalogManager, SCR_ArsenalComponent arsenalComponent, SCR_Faction faction)
	{
		if (countedItems.Contains(item))
			return 0;

		int storedItemRefundAmount;
		BaseInventoryStorageComponent inventoryStorage = BaseInventoryStorageComponent.Cast(item.FindComponent(BaseInventoryStorageComponent));
		if (inventoryStorage)
		{
			array<IEntity> storedItems = {};
			inventoryStorage.GetAll(storedItems);

			foreach (IEntity storedItem : storedItems)
			{
				storedItemRefundAmount += GetItemMilitarySupplyAllocationRefundCost(storedItem, countedItems, entityCatalogManager, arsenalComponent, faction);
			}
		}

		EntityPrefabData prefabData = item.GetPrefabData();
		if (!prefabData)
			return storedItemRefundAmount;

		ResourceName itemPrefab = prefabData.GetPrefabName();
		if (itemPrefab.IsEmpty())
			return storedItemRefundAmount;

		SCR_EntityCatalogEntry entry = entityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.ITEM, itemPrefab, faction);
		if (!entry)
			return storedItemRefundAmount;

		float refundMultiplier = 1;
		// Adjust refund multiplier based on arsenal and item factions
		if (entry.GetCatalogParent() == entityCatalogManager.GetFactionEntityCatalogOfType(EEntityCatalogType.ITEM, faction.GetFactionKey()))
			refundMultiplier *= m_fRefundSameFactionItemMultiplier;
		else
			refundMultiplier *= m_fRefundDifferentFactionItemMultiplier;

		// Calculate refund cost
		int refundCost = SCR_ResourceSystemHelper.RoundRefundSupplyAmount(GetItemMilitarySupplyAllocationCost(itemPrefab, arsenalComponent) * refundMultiplier);
		countedItems.Insert(item);

		return refundCost + storedItemRefundAmount;
	}

	//------------------------------------------------------------------------------------------------
	SCR_MilitarySupplyAllocationConfig GetMilitarySupplyApplicationConfigData()
	{
		Resource holder = BaseContainerTools.LoadContainer(m_sMilitarySupplyAllocationConfigHolder);
		if (!holder || !holder.IsValid())
			return null;

		return SCR_MilitarySupplyAllocationConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanSaveLoadout(int playerId, notnull GameEntity characterEntity, FactionAffiliationComponent playerFactionAffiliation, SCR_ArsenalComponent arsenalComponent, bool sendNotificationOnFailed)
	{
		//~ Always allow saving if no arsenal component or no restrictions
		if (!arsenalComponent || (arsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.NO_RESTRICTIONS && (!m_LoadoutSaveBlackListHolder || m_LoadoutSaveBlackListHolder.GetBlackListsCount() == 0)))
			return true;
		
		SCR_InventoryStorageManagerComponent characterInventory = SCR_InventoryStorageManagerComponent.Cast(characterEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!characterInventory)
		{
			Print("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' but character has no inventory manager!", LogLevel.ERROR);
			
			if (sendNotificationOnFailed)
				SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_NOT_SAVED);
			
			return false;
		}
			
		array<SCR_EntityCatalog> itemCatalogs = {};
		SCR_ArsenalInventoryStorageManagerComponent arsenalInventory;
		
		//~ Check either faction or arsenal inventory if item is in it
		if (arsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.FACTION_ITEMS_ONLY || arsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.FRIENDLY_AND_FACTION_ITEMS_ONLY || arsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.IN_ARSENAL_ITEMS_ONLY)
		{			
			if (arsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.FACTION_ITEMS_ONLY)
			{				
				//~ Player has no faction affiliation comp
				if (!playerFactionAffiliation)
				{
					Print("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' but player has no faction affiliation component, arsenal saving will simply be allowed", LogLevel.WARNING);
					return true;
				}
				
				SCR_Faction playerFaction = SCR_Faction.Cast(playerFactionAffiliation.GetAffiliatedFaction());
				//~ Player has no faction
				if (!playerFaction)
				{
					Print("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' but player has no SCR_Faction, arsenal saving will simply be allowed", LogLevel.WARNING);
					return true;
				}
				
				SCR_EntityCatalog itemCatalog = playerFaction.GetFactionEntityCatalogOfType(EEntityCatalogType.ITEM);
				if (!itemCatalog)
				{
					Print(string.Format("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' but player faction '%1' has no ITEM catalog!", playerFaction.GetFactionKey()), LogLevel.ERROR);
					
					if (sendNotificationOnFailed)
						SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_NOT_SAVED);
					
					return false;
				}
				else 
				{
					itemCatalogs.Insert(itemCatalog);
				}
			}
			else if (arsenalComponent.GetArsenalSaveType() == SCR_EArsenalSaveType.FRIENDLY_AND_FACTION_ITEMS_ONLY)
			{				
				//~ Player has no faction affiliation comp
				if (!playerFactionAffiliation)
				{
					Print("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' but player has no faction affiliation component, arsenal saving will simply be allowed", LogLevel.WARNING);
					return true;
				}
				
				SCR_Faction playerFaction = SCR_Faction.Cast(playerFactionAffiliation.GetAffiliatedFaction());
				//~ Player has no faction
				if (!playerFaction)
				{
					Print("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' but player has no SCR_Faction, arsenal saving will simply be allowed", LogLevel.WARNING);
					return true;
				}
				
				array<Faction> friendlyFactions = {};
				if (playerFaction.GetFriendlyFactions(friendlyFactions) == 0)
				{
					if (sendNotificationOnFailed)
						SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_NOT_SAVED);
					
					return false;
				}
					
				SCR_Faction scrFaction;
				SCR_EntityCatalog itemCatalog;
				
				foreach (Faction faction : friendlyFactions)
				{
					scrFaction = SCR_Faction.Cast(faction);
					if (!scrFaction)
						continue;
					
					itemCatalog = scrFaction.GetFactionEntityCatalogOfType(EEntityCatalogType.ITEM);
					if (!itemCatalog)
						continue;
					
					itemCatalogs.Insert(itemCatalog);
				}
				
				if (itemCatalogs.IsEmpty())
				{
					if (sendNotificationOnFailed)
						SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_NOT_SAVED);
				
					//~ Failed to find any item catalogs
					return false;
				}
			}
			else 
			{
				arsenalInventory = arsenalComponent.GetArsenalInventoryComponent();
				if (!arsenalInventory)
				{
					Print("'SCR_ArsenalManagerComponent' is checking 'CanSaveArsenal()' and arsenal check type is 'IN_ARSENAL_ITEMS_ONLY' but arsenal has no SCR_ArsenalInventoryStorageManagerComponent, so saving is simply allowed", LogLevel.WARNING);
					return true;
				}
			}
		}

		set<ResourceName> validatedPrefabs = new set<ResourceName>();
		array<IEntity> allPlayerItems = {};
		characterInventory.GetItems(allPlayerItems, EStoragePurpose.PURPOSE_DEPOSIT | EStoragePurpose.PURPOSE_WEAPON_PROXY);

		EntityPrefabData prefabData;
		RplComponent itemRplComponent;
		ResourceName resourceName;

		int invalidItemCount = 0;
		int loadoutMilitarySupplyAllocationCost;
		SCR_ECharacterRank requiredRank = SCR_ECharacterRank.RENEGADE;
		
		foreach (IEntity item : allPlayerItems)
		{			
			prefabData = item.GetPrefabData();
			
			//~ Ignore if no prefab data
			if (!prefabData)
				continue;
			
			//~ Ignore if no prefab resource name
			resourceName = prefabData.GetPrefabName();
			if (resourceName.IsEmpty())
				continue;
			
			//~ Add item supply cost to Military Supply Allocation cost
			loadoutMilitarySupplyAllocationCost += GetItemMilitarySupplyAllocationRefundAmount(item, arsenalComponent);
			
			//~ Do not check the same prefab twice
			if (!validatedPrefabs.Insert(resourceName))
				continue;

			//~ Check if item is blacklisted if there is a blacklist
			if (m_LoadoutSaveBlackListHolder)
			{
				//~ Item is blackListed so do not allow saving
				if (m_LoadoutSaveBlackListHolder.IsPrefabBlacklisted(resourceName))
				{
					if (sendNotificationOnFailed)
					{
						itemRplComponent = RplComponent.Cast(item.FindComponent(RplComponent));
						if (itemRplComponent)
							SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_ITEM_FAILED_ITEM_BLACKLISTED, itemRplComponent.Id());
					}
				
					invalidItemCount++;
					continue;
				}
			}
			
			//~ Check arsenal inventory
			if (arsenalInventory)
			{
				//~ Not in inventory of arsenal so cannot save
				if (!arsenalInventory.IsPrefabInArsenalStorage(resourceName))
				{
					if (sendNotificationOnFailed)
					{
						itemRplComponent = RplComponent.Cast(item.FindComponent(RplComponent));
						if (itemRplComponent)
							SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_ITEM_FAILED_NOT_IN_ARSENAL, itemRplComponent.Id());
					}
						
					invalidItemCount++;
					continue;
				}
			}
			//~ Check item catalog(s)
			else if (!itemCatalogs.IsEmpty())
			{
				bool itemFound;
				SCR_EntityCatalogEntry itemEntry;
				foreach(SCR_EntityCatalog itemCatalog : itemCatalogs)
				{
					itemEntry = itemCatalog.GetEntryWithPrefab(resourceName);
					if (itemEntry)
					{
						itemFound = true;
						break;
					}
				}
				
				if (!itemFound)
				{
					if (sendNotificationOnFailed)
					{
						itemRplComponent = RplComponent.Cast(item.FindComponent(RplComponent));
						if (itemRplComponent)
							SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_ITEM_FAILED_NOT_FACTION, itemRplComponent.Id());
					}
					
					invalidItemCount++;
					continue;
				}

				if (AreItemsRankLocked())
				{
					SCR_ArsenalItem arsenalItemData = SCR_ArsenalItem.Cast(itemEntry.GetEntityDataOfType(SCR_ArsenalItem));
					if (arsenalItemData)
						requiredRank = Math.Max(requiredRank, arsenalItemData.GetRequiredRank());
				}
			}
		}
		
		//~ Saving failed because of invalid items
		if (invalidItemCount > 0 && sendNotificationOnFailed)
			SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_NOT_SAVED_INVALID_ITEMS, invalidItemCount);

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (playerController && AreItemsRankLocked())
		{
			SCR_PlayerXPHandlerComponent xpHandler = SCR_PlayerXPHandlerComponent.Cast(playerController.FindComponent(SCR_PlayerXPHandlerComponent));
			if (xpHandler && xpHandler.GetPlayerRankByXP() < requiredRank)
			{
				if (sendNotificationOnFailed)
					SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_NOT_SAVED_INSUFFICIENT_RANK);

				return false;
			}
		}

		//~ Get the Military Supply Allocation cost of the player current loadout
		int playerMilitarySupplyAllocationValue;
		if (playerController)
		{
			SCR_PlayerSupplyAllocationComponent playerSupplyAllocationComponent = SCR_PlayerSupplyAllocationComponent.Cast(playerController.FindComponent(SCR_PlayerSupplyAllocationComponent));
			if (playerSupplyAllocationComponent)
				playerMilitarySupplyAllocationValue = playerSupplyAllocationComponent.GetPlayerMilitarySupplyAllocation();
		}

		//~ The player does not have enough personal supplies allocated for the loadout
		if (m_bUseMilitarySupplyAllocation && loadoutMilitarySupplyAllocationCost > playerMilitarySupplyAllocationValue)
		{
			if (sendNotificationOnFailed)
				SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.PLAYER_LOADOUT_NOT_SAVED_NOT_ENOUGH_MSA);

			return false;
		}

		return invalidItemCount == 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Authority side
	//! \param[in] playerId
	//! \param[in] characterEntity
	//! \param[in] arsenalComponent
	//! \param[in] arsenalSupplyType
	void SetPlayerArsenalLoadout(int playerId, GameEntity characterEntity, SCR_ArsenalComponent arsenalComponent, SCR_EArsenalSupplyCostType arsenalSupplyType)
	{
		//~ If Not Authority return
		if (!GetGameMode().IsMaster())
			return;
		
		const UUID playerUID = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		if (!characterEntity)
		{
			DoSetPlayerLoadout(playerId, string.Empty, characterEntity, arsenalSupplyType);
			return;
		}
		
		SCR_PlayerController clientPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!clientPlayerController || clientPlayerController.IsPossessing())
			return;
		
		string factionKey = SCR_PlayerArsenalLoadout.ARSENALLOADOUT_FACTIONKEY_NONE;
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(characterEntity.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
			factionKey = factionAffiliation.GetAffiliatedFaction().GetFactionKey();
		
		if (!CanSaveLoadout(playerId, characterEntity, factionAffiliation, arsenalComponent, true))
			return;
		
		SCR_JsonSaveContext context = new SCR_JsonSaveContext();
		if (!context.WriteValue(SCR_PlayerArsenalLoadout.ARSENALLOADOUT_FACTION_KEY, factionKey) || !SCR_PlayerArsenalLoadout.ReadLoadoutString(characterEntity, context))
			return;
		
		DoSetPlayerLoadout(playerId, context.ExportToString(), characterEntity, arsenalSupplyType);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_PlayerLoadoutData GetPlayerLoadoutData(GameEntity characterEntity)
	{
		SCR_PlayerLoadoutData loadoutData = new SCR_PlayerLoadoutData();
		
		EquipedLoadoutStorageComponent loadoutStorage = EquipedLoadoutStorageComponent.Cast(characterEntity.FindComponent(EquipedLoadoutStorageComponent));
		if (loadoutStorage)
		{
			int slotsCount = loadoutStorage.GetSlotsCount();
			for (int i = 0; i < slotsCount; ++i)
			{
				InventoryStorageSlot slot = loadoutStorage.GetSlot(i);
				if (!slot)
					continue;
				
				IEntity attachedEntity = slot.GetAttachedEntity();
				if (!attachedEntity)
					continue;
				
				ResourceName prefabName;
				BaseContainer prefab = attachedEntity.GetPrefabData().GetPrefab();
				while (prefabName.IsEmpty() && prefab)
				{
					prefabName = prefab.GetResourceName();
					prefab = prefab.GetAncestor();
				}
				
				if (prefabName.IsEmpty())
					continue;
				
				SCR_ClothingLoadoutData clothingData = new SCR_ClothingLoadoutData();
				clothingData.SlotIdx = i;
				clothingData.ClothingPrefab = prefabName;
				
				loadoutData.Clothings.Insert(clothingData);
			}
		}
		
		const BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(characterEntity.FindComponent(BaseWeaponManagerComponent));
		EquipedWeaponStorageComponent weaponStorage = EquipedWeaponStorageComponent.Cast(characterEntity.FindComponent(EquipedWeaponStorageComponent));
		if (weaponStorage)
		{
			int slotsCount = weaponStorage.GetSlotsCount();
			for (int i = 0; i < slotsCount; ++i)
			{
				InventoryStorageSlot slot = weaponStorage.GetSlot(i);
				if (!slot)
					continue;
				
				IEntity attachedEntity = slot.GetAttachedEntity();
				if (!attachedEntity)
					continue;
				
				const ResourceName prefabName = SCR_ResourceNameUtils.GetPrefabName(attachedEntity);
				if (prefabName.IsEmpty())
					continue;
				
				bool active = false;
				if (weaponManager && weaponManager.GetCurrentWeapon())
					active = weaponManager.GetCurrentWeapon().GetOwner() == attachedEntity;
				
				SCR_WeaponLoadoutData weaponData = new SCR_WeaponLoadoutData();
				weaponData.Active = active;
				weaponData.SlotIdx = i;
				weaponData.WeaponPrefab = prefabName;
				
				WeaponAttachmentsStorageComponent attachmentsStorage = WeaponAttachmentsStorageComponent.Cast(attachedEntity.FindComponent(WeaponAttachmentsStorageComponent));
				if (attachmentsStorage)
				{
					const int slotCount = attachmentsStorage.GetSlotsCount();
					weaponData.Attachments.Reserve(slotCount);
					for (int nSlot = 0; nSlot < slotCount; ++nSlot)
					{
						weaponData.Attachments.Insert(SCR_ResourceNameUtils.GetPrefabName(attachmentsStorage.Get(nSlot)));
					}
				}

				loadoutData.Weapons.Insert(weaponData);
			}
		}
		
		return loadoutData;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ComputeSuppliesCost(notnull SCR_Faction faction, notnull SCR_ArsenalPlayerLoadout playerLoadout, SCR_EArsenalSupplyCostType arsenalSupplyType)
	{
		playerLoadout.suppliesCost = 0.0;
		playerLoadout.m_fMilitarySupplyAllocationCost = 0;
		
		SCR_JsonLoadContext context = new SCR_JsonLoadContext(false);
		if (!context.ImportFromString(playerLoadout.loadout))
			return;
		
		context.StartObject(SCR_PlayerArsenalLoadout.ARSENALLOADOUT_KEY);
		SCR_PlayerArsenalLoadout.ComputeSuppliesCost(context, faction, playerLoadout, arsenalSupplyType);
	}

	//------------------------------------------------------------------------------------------------
	protected void DoSetPlayerLoadout(int playerId, string loadoutString, GameEntity characterEntity, SCR_EArsenalSupplyCostType arsenalSupplyType)
	{
		const UUID playerUID = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		SCR_ArsenalPlayerLoadout playerLoadout;
		bool loadoutChanged;
		if (!loadoutString.IsEmpty())
		{
			playerLoadout = m_aPlayerLoadouts.Get(playerUID);
			if (!playerLoadout)
				playerLoadout = new SCR_ArsenalPlayerLoadout();

			loadoutChanged = loadoutString != playerLoadout.loadout;
			playerLoadout.loadout = loadoutString;
			DoSetPlayerLoadout(playerUID, playerLoadout); // Need to set early for cost calculations below

			if (loadoutChanged)
			{
				FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(characterEntity.FindComponent(FactionAffiliationComponent));
				if (factionAffiliation && factionAffiliation.GetAffiliatedFaction())
					ComputeSuppliesCost(SCR_Faction.Cast(factionAffiliation.GetAffiliatedFaction()), playerLoadout, arsenalSupplyType);

				playerLoadout.loadoutData = GetPlayerLoadoutData(characterEntity);
				playerLoadout.loadoutData.LoadoutCost = SCR_PlayerArsenalLoadout.GetLoadoutSuppliesCost(playerUID);
				playerLoadout.loadoutData.LoadoutPlayerSupplyAllocationCost = SCR_PlayerArsenalLoadout.GetLoadoutMilitarySupplyAllocationCost(playerUID);
				playerLoadout.loadoutData.FactionIndex = GetGame().GetFactionManager().GetFactionIndex(SCR_FactionManager.SGetPlayerFaction(playerId));
			}
		}
		else
		{
			m_aPlayerLoadouts.Remove(playerUID); // Invalid loadout = no loadout
		}

		SyncPlayerloadout(playerId, playerLoadout, loadoutChanged, true);
	}

	//------------------------------------------------------------------------------------------------
	[Friend(SCR_PlayerLoadoutComponentSerializer)]
	protected void DoSetPlayerLoadout(const UUID playerIdentity, SCR_ArsenalPlayerLoadout loadout)
	{
		m_aPlayerLoadouts.Set(playerIdentity, loadout);
	}

	//------------------------------------------------------------------------------------------------
	[Friend(SCR_PlayerLoadoutComponentSerializer)]
	protected void SyncPlayerloadout(int playerId, SCR_ArsenalPlayerLoadout playerLoadout, bool loadoutChanged = true, bool notification = false)
	{
		if (playerLoadout && loadoutChanged)
		{
			DoSendPlayerLoadout(playerId, playerLoadout.loadoutData);
			Rpc(DoSendPlayerLoadout, playerId, playerLoadout.loadoutData);
		}

		const bool loadoutValid = playerLoadout && !playerLoadout.loadout.IsEmpty();
		DoSetPlayerHasLoadout(playerId, loadoutValid, loadoutChanged, notification);
		Rpc(DoSetPlayerHasLoadout, playerId, loadoutValid, loadoutChanged, notification);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPlayerAuditSuccess(int playerId)
	{
		super.OnPlayerAuditSuccess(playerId);

		const UUID playerUID = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		SCR_ArsenalPlayerLoadout playerLoadout = m_aPlayerLoadouts.Get(playerUID);
		if (!playerLoadout || playerLoadout.loadout.IsEmpty())
			return;

		SyncPlayerloadout(playerId, playerLoadout);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void DoSetPlayerHasLoadout(int playerId, bool loadoutValid, bool loadoutChanged, bool notification)
	{
		if (playerId == SCR_PlayerController.GetLocalPlayerId())
		{
			if (notification)
			{
				if (m_bLocalPlayerLoadoutAvailable != loadoutValid || loadoutChanged)
				{
					//~ Send notification with loadout cost
					if (m_LocalPlayerLoadoutData && GetCalculatedLoadoutSpawnSupplyCostMultiplier() > 0)
						SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_SAVED_SUPPLY_COST, m_LocalPlayerLoadoutData.LoadoutCost);
					//~ Set notification without loadout cost
					else
						SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_SAVED);
				}
				else
				{
					SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_LOADOUT_NOT_SAVED_UNCHANGED);
				} 
			}
			
			m_bLocalPlayerLoadoutAvailable = loadoutValid;
		}
		m_OnPlayerLoadoutUpdated.Invoke(playerId, loadoutValid);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void DoSendPlayerLoadout(int playerId, SCR_PlayerLoadoutData loadoutData)
	{
		if (playerId == SCR_PlayerController.GetLocalPlayerId())
			m_LocalPlayerLoadoutData = loadoutData;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void DoPlayerClearHasLoadout(int playerId)
	{
		if (playerId == SCR_PlayerController.GetLocalPlayerId())
			m_bLocalPlayerLoadoutAvailable = false;

		m_OnPlayerLoadoutUpdated.Invoke(playerId, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		//~ Init item black list (Server only). Call one frame later to make sure catalogs are initalized
		if (m_LoadoutSaveBlackListHolder)
			GetGame().GetCallqueue().CallLater(m_LoadoutSaveBlackListHolder.Init);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		if (s_Instance || SCR_Global.IsEditMode())
			return;

		GetArsenalLoadoutComponentsToCheck(SCR_PlayerArsenalLoadout.ARSENALLOADOUT_COMPONENTS_TO_CHECK);

		s_Instance = SCR_ArsenalManagerComponent.Cast(GetGameMode().FindComponent(SCR_ArsenalManagerComponent));
		SetEventMask(owner, EntityEvent.INIT);
		
		m_ArsenalGameModeUIDataHolder = SCR_ArsenalGameModeUIDataHolder.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sArsenalGameModeUIDataHolder, true));
		if (!m_ArsenalGameModeUIDataHolder)
			Print("'SCR_ArsenalManagerComponent' failed to loadm_Arsenal GameMode UI Data Holder config!", LogLevel.WARNING);
		
		//~ Get config for saveType holder. This is used by arsenals in the world
		m_ArsenalSaveTypeInfoHolder = SCR_ArsenalSaveTypeInfoHolder.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sArsenalSaveTypeInfoHolder, true));
		if (!m_ArsenalSaveTypeInfoHolder)
			Print("'SCR_ArsenalManagerComponent' failed to load Arsenal Save Type Holder config!", LogLevel.ERROR);
		
		//~ Get loadout save blacklist config, Can be null (Server Only)
		if (GetGameMode().IsMaster())
			m_LoadoutSaveBlackListHolder = SCR_LoadoutSaveBlackListHolder.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sLoadoutSaveBlackListHolder, false));		
	}
}

[BaseContainerProps(configRoot: true)]
class SCR_ArsenalGameModeUIDataHolder
{
	[Attribute(desc: "List of UIInfo for Arsenal GameMode types, Used for editor attributes and notifications. Only types in this list will be displayed in editor")]
	protected ref array<ref SCR_ArsenalGameModeUIData> m_aArsenalGameModeTypeUIInfoList;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] arsenalGameModeType Type to get Ui info for. -1 gets the unrestricted game mode type
	//! \return UI info of given Arsenal Mode type.
	SCR_UIInfo GetArsenalGameModeUIInfo(SCR_EArsenalGameModeType arsenalGameModeType)
	{
		if (arsenalGameModeType <= 0)
			arsenalGameModeType = -1;
		
		//~ Get the game mode type
		foreach (SCR_ArsenalGameModeUIData data : m_aArsenalGameModeTypeUIInfoList)
		{
			if (data.m_eArsenalGameModeType == arsenalGameModeType)
				return data.m_UIInfo;
		}
		
		Print("Cannot find  arsenal Game Mode type for '" + typename.EnumToString(SCR_EArsenalGameModeType, arsenalGameModeType) + "'!", LogLevel.WARNING);
		
		SCR_UIInfo unknown = new SCR_UIInfo();
		unknown.SetName("MISSING");

		return unknown; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[inout] arsenalGameModeTypeUIInfoList Returns a list of all the UI data for the arsenal Game mode type
	//! \return Count of all the arsenal game mode types
	int GetArsenalGameModeUIInfoList(inout notnull array<SCR_ArsenalGameModeUIData> arsenalGameModeTypeUIInfoList)
	{
		arsenalGameModeTypeUIInfoList.Clear();
		
		foreach (SCR_ArsenalGameModeUIData data : m_aArsenalGameModeTypeUIInfoList)
		{
			if (!data)
				continue;
			
			arsenalGameModeTypeUIInfoList.Insert(data);
		}
		
		return arsenalGameModeTypeUIInfoList.Count();
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_UIInfo")]
class SCR_ArsenalGameModeUIData
{
	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: SCR_Enum.GetList(SCR_EArsenalGameModeType, new ParamEnum("UNRESTRICTED", "-1")))]
	SCR_EArsenalGameModeType m_eArsenalGameModeType;
	
	[Attribute()]
	ref SCR_UIInfo m_UIInfo;
}
