class SCR_ArsenalComponentClass : ScriptComponentClass
{
}

void ScriptInvokerArsenalUpdatedMethod(array<ResourceName> arsenalItems);
typedef func ScriptInvokerArsenalUpdatedMethod;
typedef ScriptInvokerBase<ScriptInvokerArsenalUpdatedMethod> ScriptInvokerArsenalUpdated;

class SCR_ArsenalComponent : ScriptComponent
{
	[Attribute(SCR_EArsenalSupplyCostType.DEFAULT.ToString(), desc: "Cost type of items. If it is not DEFAULT than it will try to get the diffrent supply cost if the item has it assigned" , uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalSupplyCostType), category: "Settings")]
	protected SCR_EArsenalSupplyCostType m_eSupplyCostType;
	
	[Attribute("", desc: "Toggle supported SCR_EArsenalItemType by this arsenal, items are gathered from SCR_Faction or from the overwrite config", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType), category: "Settings")]
	protected SCR_EArsenalItemType m_eSupportedArsenalItemTypes;

	[Attribute("", desc: "Toggle supported SCR_EArsenalItemMode by this arsenal, items are gathered from SCR_Faction or from the overwrite config", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode), category: "Settings")]
	protected SCR_EArsenalItemMode m_eSupportedArsenalItemModes;

	[Attribute("", desc: "Toggle which SCR_EArsenalItemType groups are exposed for gamemaster attribute editing", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalAttributeGroup), category: "Settings")]
	protected SCR_EArsenalAttributeGroup m_eEditableAttributeGroups;

	[Attribute(category: "Overwrite", desc: "If empty this will be ignored. For ease of use do not edit this directly in the prefab, use a config instead. The items added the config will allow the arsenal to spawn these items within it. Note that it will still filter on type, meaning that if weapons are added to the list but disabled in the Supported types then they will not show in the arsenal. Changing faction will not have any effect if this is not null")]
	protected ref SCR_ArsenalItemListConfig m_OverwriteArsenalConfig;
	
	[Attribute("0", desc: "Whether or not the overwrite arsenal config still checks if the items are in the correct faction. Hides any items not belonging to the arsenal's faction.", category: "Overwrite")]
	protected bool m_bCheckFactionForOverwriteArsenalConfig;

	[Attribute("1", desc: "Get default faction if current faction could not be found", category: "Settings")]
	protected bool m_bGetDefaultIfNoFaction;
	
	[Attribute("0", desc: "If true will try and find the SCR_FactionAffiliationComponent from parent if it was not found on self", category: "Settings")]
	protected bool m_bAllowGetFactionFromParent;
	
	[Attribute("0", desc: "Save type of Arsenal. Only applicable if there is a save arsenal action attached to the arsenal.\nSAVING_DISABLED: Saving action is disabled.\nIN_ARSENAL_ITEMS_ONLY: Only allow saving if all the items the player has are in the arsenal as well.\nFACTION_ITEMS_ONLY: Only allow saving if all the items the player has in their inventory are of the faction of the arsenal.\nNO_RESTRICTIONS: No restriction as what the arsenal is allowed to save.", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalSaveType), category: "Settings")]
	protected SCR_EArsenalSaveType m_eArsenalSaveType;
	
	[Attribute(SCR_EArsenalTypes.STATIC_ENTITIES.ToString(), desc: "What arsenal type is this arsenal. As the arsenal manager can disable an arsenal", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalTypes), category: "Settings")]
	protected SCR_EArsenalTypes m_eArsenalTypes;
	
	[Attribute("0", desc: "If false it will get the arsenals current faction and change when the faction is updated. If true it will never check the on faction and use the default faction assigned. Use this if you want the content to never change or if there is never a current faction", category: "Settings")]
	protected bool m_bAlwaysUseDefaultFaction;
	
	[Attribute("1", desc: "If false the arsenal is disabled but GM can set it enabled. Setting this to false counts as being overwritten meaning the arsenal is not enabled when the arsenal type flag is set in the manager", category: "Settings")]
	protected bool m_bArsenalEnabled;
	
	//~ Has arsenal enabled been overwritten before?
	protected bool m_bArsenalEnabledOverwritten;
	
	protected SCR_EArsenalItemMode m_eOnDisableArsenalModes; //~ Saves the arsenal item mode when the arsenal is disabled and the mode is set to 0
	
	protected bool m_bArsenalSavingDisplayedIfDisabled = true;	//!< This is auto set on init and when arsenal save type is changed. If the prefab has saving disabled than the arsenal save action will never show unless saving has been changed in runtime
	protected bool m_bHasSaveArsenalAction = false;				//!< Set by action in action manager for the editor to know if the Arsenal save action exists on this arsenal
	
	protected SCR_ArsenalInventoryStorageManagerComponent m_InventoryComponent;
	protected UniversalInventoryStorageComponent m_StorageComponent;

	protected FactionManager m_FactionManager;
	protected SCR_FactionAffiliationComponent m_FactionComponent;
	
	protected ref ScriptInvokerArsenalUpdated m_OnArsenalUpdated;

	protected bool m_bIsClearingInventory;

	//------------------------------------------------------------------------------------------------
	//! \return The arsenal type
	SCR_EArsenalTypes GetArsenalType()
	{
		return m_eArsenalTypes;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return If the arsenal type is enabled by the Arsenal Manager
	bool IsArsenalEnabledByType()
	{
		return SCR_ArsenalManagerComponent.IsArsenalTypeEnabled_Static(GetArsenalType());
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Server only
	protected void OnArsenalTypeEnabledChanged(SCR_EArsenalTypes typesEnabled)
	{
		//~ Arsenal enabled state has been overwritten so do not change it when arsenal enabled change was overwritten
		if (m_bArsenalEnabledOverwritten)
			return;
		
		if (SCR_Enum.HasFlag(typesEnabled, GetArsenalType()) && !IsArsenalEnabled())
			SetArsenalEnabled(true, false);
		else if (!SCR_Enum.HasFlag(typesEnabled, GetArsenalType()) && IsArsenalEnabled())
			SetArsenalEnabled(false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return If arsenal is enabled and the arsenal type is enabled by the Arsenal Manager
	bool IsArsenalEnabled()
	{
		return m_bArsenalEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] enable Sets the arsenal enabled true or false. Both the arsenal Type needs to be enabled in the manager and the arsenal entity for arsenal items to show up
	void SetArsenalEnabled(bool enable, bool isOverwrite = true)
	{
		if (m_bArsenalEnabled == enable)
			return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if ((gameMode && !gameMode.IsMaster()) || (!gameMode && Replication.IsClient()))
			return;
		
		SetArsenalEnabledBroadcast(enable);
		Rpc(SetArsenalEnabledBroadcast, enable);	
		
		if (isOverwrite)
			m_bArsenalEnabledOverwritten = true;
		
		UpdateArsenalEnabledState();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateArsenalEnabledState()
	{
		if (!m_bArsenalEnabled && m_eOnDisableArsenalModes == 0)
		{
			m_eOnDisableArsenalModes = m_eSupportedArsenalItemModes;
			SetSupportedArsenalItemModes(0);
		}
		else if (m_bArsenalEnabled && m_eOnDisableArsenalModes != 0)
		{
			SetSupportedArsenalItemModes(m_eOnDisableArsenalModes);
			m_eOnDisableArsenalModes = 0;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetArsenalEnabledBroadcast(bool enable)
	{
		m_bArsenalEnabled = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Supply cost type which will be used to get the cost of items in the arsenal. Default is used if the item does not have the specific supply cost type
	SCR_EArsenalSupplyCostType GetSupplyCostType()
	{
		return m_eSupplyCostType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsArsenalUsingSupplies()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return null;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(owner);
		
		return resourceComponent && resourceComponent.IsResourceTypeEnabled();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get Current Arsenal save type
	SCR_EArsenalSaveType GetArsenalSaveType()
	{
		return m_eArsenalSaveType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return If arsenal action can be displayed if disabled. This is auto set on init and when arsenal save type is changed. If the prefab has saving disabled than the arsenal save action will never show unless saving has been changed in runtime
	bool IsArsenalSavingDisplayedIfDisabled()
	{
		return m_bArsenalSavingDisplayedIfDisabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set if arsenal save action is on the action manager.
	//! Set by the save action on init
	//! \param[in] Set true if action is available
	void SetHasSaveArsenalAction(bool hasSaveArsenalAction)
	{
		m_bHasSaveArsenalAction = hasSaveArsenalAction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if Arsenal has an arsenal action attached to it. If true it will allow the editor to set if saving is allowed
	//! \return True if Arsenal action is available
	bool HasSaveArsenalAction()
	{
		return m_bHasSaveArsenalAction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set Arsenal Save type (Server Only)
	//! \param[in] saveType New Save type to set
	void SetArsenalSaveType(SCR_EArsenalSaveType saveType)
	{		
		if (m_eArsenalSaveType == saveType)
			return;
		
		RPL_SetArsenalSaveType(saveType);
		Rpc(RPL_SetArsenalSaveType, saveType);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] saveType
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPL_SetArsenalSaveType(SCR_EArsenalSaveType saveType)
	{
		//~ Nothing changed so make sure the action displayed if disabled is not set
		if (m_eArsenalSaveType == saveType)
			return;
		
		m_eArsenalSaveType = saveType;

		if (saveType != SCR_EArsenalSaveType.SAVING_DISABLED)
			m_bArsenalSavingDisplayedIfDisabled = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EArsenalItemType GetSupportedArsenalItemTypes()
	{
		return m_eSupportedArsenalItemTypes;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the first arsenal component found on the children of the given parent
	//! \param[in] parent To check children for arsenal component
	//! \return Arsenal Component if any is found, null otherwise
	static SCR_ArsenalComponent FindArsenalComponent(notnull IEntity entity, bool getFromSlotted = true)
	{
		SCR_ArsenalComponent arsenalComponent;
		
		if (getFromSlotted)
			arsenalComponent = SCR_ArsenalComponent.Cast(SCR_EntityHelper.FindComponent(entity, SCR_ArsenalComponent));
		else
			arsenalComponent = SCR_ArsenalComponent.Cast(entity.FindComponent(SCR_ArsenalComponent));
		
		return arsenalComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] types
	void SetSupportedArsenalItemTypes(SCR_EArsenalItemType types)
	{
		m_eSupportedArsenalItemTypes = types;
		RefreshArsenal();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EArsenalItemMode GetSupportedArsenalItemModes()
	{
		return m_eSupportedArsenalItemModes;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] modes
	void SetSupportedArsenalItemModes(SCR_EArsenalItemMode modes)
	{
		m_eSupportedArsenalItemModes = modes;
		RefreshArsenal();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EArsenalAttributeGroup GetEditableAttributeGroups()
	{
		return m_eEditableAttributeGroups;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ArsenalInventoryStorageManagerComponent GetArsenalInventoryComponent()
	{
		return m_InventoryComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! Get overwrite Arsenal config
	//! \return Config of overwrite arsenal. Can be null if not overwritten
	SCR_ArsenalItemListConfig GetOverwriteArsenalConfig()
	{
		return m_OverwriteArsenalConfig;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] availablePrefabs
	//! \return
	bool GetAvailablePrefabs(out notnull array<ResourceName> availablePrefabs)
	{
		array<SCR_ArsenalItem> arsenalItems = {};
		if (!GetFilteredArsenalItems(arsenalItems))
		{
			return false;
		}

		for (int i = 0; i < arsenalItems.Count(); i++)
		{
			SCR_ArsenalItem itemToSpawn = arsenalItems[i % arsenalItems.Count()];
			if (!itemToSpawn)
				continue;

			availablePrefabs.Insert(itemToSpawn.GetItemResourceName());
		}
		return !availablePrefabs.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_Faction GetAssignedFaction()
	{
		//~ Safty if get assigned faction is called before the init
		if (!m_FactionComponent)
			FactionInit(GetOwner());
		
		if (m_FactionComponent)
		{
			//~ Always use default faction
			if (m_bAlwaysUseDefaultFaction)
				return SCR_Faction.Cast(m_FactionComponent.GetDefaultAffiliatedFaction());
			
			SCR_Faction faction = SCR_Faction.Cast(m_FactionComponent.GetAffiliatedFaction());
			if (!faction && m_bGetDefaultIfNoFaction)
			{
				faction = SCR_Faction.Cast(m_FactionComponent.GetDefaultAffiliatedFaction());
			}
			
			return faction;
		}
		
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] filteredArsenalItems
	//! \param[in] requiresDisplayType
	//! \return
	bool GetFilteredArsenalItems(out notnull array<SCR_ArsenalItem> filteredArsenalItems, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		//~ Is overwritting Arsenal
		if (m_OverwriteArsenalConfig)
		{
			GetFilteredOverwriteArsenalItems(filteredArsenalItems, requiresDisplayType);
			return !filteredArsenalItems.IsEmpty();
		}

		//~ Cannot get items if no entity catalog manager
		SCR_EntityCatalogManagerComponent catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!catalogManager)
			return false;

		//~ Get filtered list from faction (if any) or default
		filteredArsenalItems = catalogManager.GetFilteredArsenalItems(m_eSupportedArsenalItemTypes, m_eSupportedArsenalItemModes, SCR_ArsenalManagerComponent.GetArsenalGameModeType_Static(), GetAssignedFaction(), requiresDisplayType);
		return !filteredArsenalItems.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Get filtered overwrite Arsenal EWorkshopItemState
	//! \param[out] List of all filtered arsenal items
	//! \return true if any filtered items were found
	bool GetFilteredOverwriteArsenalItems(out notnull array<SCR_ArsenalItem> filteredArsenalItems, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		//~ If overwrite should check if item is valid for arsenal faction
		if (m_bCheckFactionForOverwriteArsenalConfig)
			filteredArsenalItems = m_OverwriteArsenalConfig.GetFilteredArsenalItems(m_eSupportedArsenalItemTypes, m_eSupportedArsenalItemModes, requiresDisplayType, GetAssignedFaction());
		//~ Simply get the filtered items as no need to check the faction
		else 
			filteredArsenalItems = m_OverwriteArsenalConfig.GetFilteredArsenalItems(m_eSupportedArsenalItemTypes, m_eSupportedArsenalItemModes, requiresDisplayType);
		
		return !filteredArsenalItems.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//~ Used by SCR_ArsenalDisplayComponent
	void ClearArsenal()
	{
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] faction unused parameter
	void RefreshArsenal(bool init = false, SCR_Faction faction = null)
	{
		ClearArsenal();

		array<SCR_ArsenalItem> arsenalItems = {};
		if (!GetFilteredArsenalItems(arsenalItems))
			return;

		//SCR_Sorting<SCR_ArsenalItem, SCR_CompareArsenalItemPriority>.HeapSort(arsenalItems);
		
		array<ResourceName> arsenalPrefabs = {};

		int arsenalItemCount = arsenalItems.Count();
		for (int i = 0; i < arsenalItemCount; i++)
		{
			SCR_ArsenalItem itemToSpawn = arsenalItems[i % arsenalItemCount];
			if (!itemToSpawn)
				continue;

			arsenalPrefabs.Insert(itemToSpawn.GetItemResourceName());
		}
		
		if (m_OnArsenalUpdated)
			m_OnArsenalUpdated.Invoke(arsenalPrefabs);
		
		// If we know this is called form EOnInit we don't send any RPC.
		// At that point RplComponent is not initialized yet and calling the RPC would only result in error spam.
		if (!init && Replication.IsServer())
			Rpc(RPC_OnArsenalUpdated, m_eSupportedArsenalItemTypes, m_eSupportedArsenalItemModes);	
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_OnArsenalUpdated(SCR_EArsenalItemType itemTypes, SCR_EArsenalItemMode itemModes)
	{
		//~ Call later so faction is correctly updated
		GetGame().GetCallqueue().CallLater(DelayedOnArsenalUpdatedClient, param1: itemTypes, param2: itemModes);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedOnArsenalUpdatedClient(SCR_EArsenalItemType itemTypes, SCR_EArsenalItemMode itemModes)
	{
		m_eSupportedArsenalItemTypes = itemTypes;
		m_eSupportedArsenalItemModes = itemModes;
		
		if (!m_FactionComponent)
			FactionInit(GetOwner());
		
		if (m_OnArsenalUpdated)
		{
			array<ResourceName> arsenalPrefabs = {};
			GetAvailablePrefabs(arsenalPrefabs);
			
			m_OnArsenalUpdated.Invoke(arsenalPrefabs);
		}			
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetItemValid(SCR_EArsenalItemType arsenalItemType, SCR_EArsenalItemMode arsenalItemMode)
	{
		return arsenalItemType & m_eSupportedArsenalItemTypes
			&& arsenalItemMode & m_eSupportedArsenalItemModes;
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetItemValid(SCR_Faction faction, int index, out bool isEmpty = true)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get scriptinvoker called when arsenal is updated
	ScriptInvokerArsenalUpdated GetOnArsenalUpdated()
	{
		if (!m_OnArsenalUpdated)
			m_OnArsenalUpdated = new ScriptInvokerArsenalUpdated();
		
		return m_OnArsenalUpdated;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction newFaction)
	{
		RefreshArsenal();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnArsenalGameModeTypeChanged(SCR_EArsenalGameModeType newArsenalGameModeType)
	{
		RefreshArsenal();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		//~ Make sure it knows that it is disabled at the start
		if (GetArsenalSaveType() == SCR_EArsenalSaveType.SAVING_DISABLED)
			m_bArsenalSavingDisplayedIfDisabled = false;

		SetEventMask(owner, EntityEvent.INIT);

		// Initialize inventory of arsenal, if applicable (Display racks without additional inventory will return here)
		m_InventoryComponent = SCR_ArsenalInventoryStorageManagerComponent.Cast(owner.FindComponent(SCR_ArsenalInventoryStorageManagerComponent));
		m_StorageComponent = UniversalInventoryStorageComponent.Cast(owner.FindComponent(UniversalInventoryStorageComponent));
		// Arsenal inventory is filled after OnPostInit by SCR_ArsenalInventoryStorageManagerComponent.FillInitialPrefabsToStore
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{				
		m_FactionManager = GetGame().GetFactionManager();
		SCR_FactionAffiliationComponent factionComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		if (!factionComponent && m_bAllowGetFactionFromParent)
			GetGame().GetCallqueue().CallLater(FactionInit, param1: owner);
		else 
			FactionInit(owner);

	}
	
	//------------------------------------------------------------------------------------------------
	protected void FactionInit(IEntity owner)
	{
		//~ Faction is already init
		if (m_FactionComponent)
			return;
		
		m_FactionComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		if (!m_FactionComponent && m_bAllowGetFactionFromParent)
		{
			if (owner.GetParent())
				m_FactionComponent = SCR_FactionAffiliationComponent.Cast(owner.GetParent().FindComponent(SCR_FactionAffiliationComponent));
			
			if (!m_FactionComponent)
				return;
		}
		else if (!m_FactionComponent)
			return;
		
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (rplComponent && rplComponent.Role() == RplRole.Authority)
		{
			if (!m_bAlwaysUseDefaultFaction)
				m_FactionComponent.GetOnFactionChanged().Insert(OnFactionChanged);
			
			SCR_ArsenalManagerComponent arsenalManager;
			
			if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			{
				arsenalManager.GetOnArsenalGameModeTypeChanged().Insert(OnArsenalGameModeTypeChanged);
				arsenalManager.GetOnArsenalTypeEnabledChanged().Insert(OnArsenalTypeEnabledChanged);
			}
			
			if (IsArsenalEnabled())
			{
				//~ If arsenal is enabled but the type is not on the manager, than disable it instead
				if (!IsArsenalEnabledByType())
					SetArsenalEnabled(false, false);
			}
			else 
			{
				//~ If arsenal is disabled on init than it is considered an overwrite
				m_bArsenalEnabledOverwritten = true;
			}

			//~ Check if arsenal is enabled on init
			UpdateArsenalEnabledState();
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rplComponent && rplComponent.Role() == RplRole.Authority)
		{
			if (!m_bAlwaysUseDefaultFaction && m_FactionComponent)
				m_FactionComponent.GetOnFactionChanged().Remove(OnFactionChanged);
			
			SCR_ArsenalManagerComponent arsenalManager;
			
			if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			{
				arsenalManager.GetOnArsenalGameModeTypeChanged().Remove(OnArsenalGameModeTypeChanged);
				arsenalManager.GetOnArsenalTypeEnabledChanged().Remove(OnArsenalTypeEnabledChanged);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
    {	
		writer.WriteInt(m_eSupportedArsenalItemTypes);
		writer.WriteInt(m_eSupportedArsenalItemModes);
        writer.WriteInt(GetArsenalSaveType()); 
        writer.WriteBool(m_bArsenalEnabled); 
        return true;
    }
	
	//------------------------------------------------------------------------------------------------
    override bool RplLoad(ScriptBitReader reader)
    {
		int itemTypes, itemModes, saveType;
		bool isArsenalEnabled;
		
		reader.ReadInt(itemTypes);
		reader.ReadInt(itemModes);
        reader.ReadInt(saveType);
        reader.ReadBool(isArsenalEnabled);
		
		RPC_OnArsenalUpdated(itemTypes, itemModes);
		RPL_SetArsenalSaveType(saveType);
		SetArsenalEnabledBroadcast(isArsenalEnabled);
		
        return true;
	}
}

//------------------------------------------------------------------------------------------------
enum SCR_EArsenalSaveType
{
	SAVING_DISABLED = 0,					//!< Saving is disabled
	IN_ARSENAL_ITEMS_ONLY = 10,				//!< Only allows arsenal saving if all the items the player has are in the arsenal
	FACTION_ITEMS_ONLY = 20,				//!< Only allow saving if items the player has are items of the player faction
	FRIENDLY_AND_FACTION_ITEMS_ONLY = 21,	//!< Only allow saving if items the player has are items of the player faction or from a faction friendly to the faction
	NO_RESTRICTIONS = 30,					//!< Always allow saving of arsenal
}

//------------------------------------------------------------------------------------------------
enum SCR_EArsenalSupplyCostType
{
	DEFAULT,		//!< Default cost will be used
	GADGET_ARSENAL,	//!< If the arsenal is on a gadget it will try to take this cost if any is assigned in the catalog
	RESPAWN_COST,	//!< Cost of item when the player spawns with the item
}
