class SCR_ArsenalInventoryStorageManagerComponentClass : ScriptedInventoryStorageManagerComponentClass
{
}

class SCR_ArsenalInventoryStorageManagerComponent : ScriptedInventoryStorageManagerComponent
{
	[Attribute("1", desc: "If true it will negate the weight and volume of the items within the arsenal, if false the items within the arsenal will have weight and volume")]
	protected bool m_bNegateItemWeightAndVolume;
	
	//~ Total weight of items that were added 1 frame OnBeforePossessed
	protected float m_fTotalItemWeight;
	
	//~ If next frame the weight will be set when items are OnItemAdded
	protected bool m_bItemAddCallQuequeCalled;
	
	protected ref set<ResourceName> m_ItemsInArsenal = new set<ResourceName>();
	
	//------------------------------------------------------------------------------------------------
	//! Check if given prefab is in the arsenal storage.
	//! \param[in] item Prefab to check if it is in the arsenal storage
	//! \return True if prefab found, else returns false
	bool IsPrefabInArsenalStorage(ResourceName item)
	{
		return m_ItemsInArsenal.Contains(item);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if given item is in the arsenal storage. (Prefabs only)
	//! \param[in] item Item to check if it is in the arsenal storage
	//! \return True if item found, else returns false
	bool IsItemInArsenalStorage(notnull IEntity item)
	{
		EntityPrefabData prefabData = item.GetPrefabData();
		
		if (!prefabData)
			return false;
		
		return IsPrefabInArsenalStorage(prefabData.GetPrefabName());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnArsenalUpdated(array<ResourceName> newArsenalContent)
	{
		m_ItemsInArsenal.Clear();
		if (!newArsenalContent)
			return;

		foreach (ResourceName prefab : newArsenalContent)
		{
			m_ItemsInArsenal.Insert(prefab);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Callback when item is added (will be performed locally after server completed the Insert/Move operation)
	override protected void OnItemAdded(BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		super.OnItemAdded(storageOwner, item);

		//~ Get the total weight and volume of all items added to negate that weight and volume the next frame
		if (m_bNegateItemWeightAndVolume && Replication.IsServer())
		{	
			if (storageOwner.GetParentSlot())
				return;

			if (!m_bItemAddCallQuequeCalled)
			{
				m_fTotalItemWeight = 0;
				m_bItemAddCallQuequeCalled = true;

				//~ All items are added the same frame to negate the weight and volume the next frame
				GetGame().GetCallqueue().CallLater(OnItemsAddedDelay, param1: storageOwner);
			}

			InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
			m_fTotalItemWeight += inventoryItem.GetTotalWeight();
			inventoryItem.SetAdditionalVolume(-inventoryItem.GetTotalVolume());
		}
	}

	//------------------------------------------------------------------------------------------------
	//~ Called one frame later after items were added to the arsenal. All items are added the same frame so the overal weight and volume of all those items added prev frame is removed to negate them
	protected void OnItemsAddedDelay(BaseInventoryStorageComponent storageOwner)
	{
		m_bItemAddCallQuequeCalled = false;
		
		if (!storageOwner)
			return;
		
		storageOwner.SetAdditionalWeight(-m_fTotalItemWeight);
	}
	
	//------------------------------------------------------------------------------------------------
	// Callback when item is removed (will be performed locally after server completed the Remove/Move operation)
	override protected void OnItemRemoved(BaseInventoryStorageComponent storageOwner, IEntity item)
	{
		super.OnItemRemoved(storageOwner, item);
		
		if (m_OnItemRemovedInvoker && !item.IsDeleted())
			m_OnItemRemovedInvoker.Invoke(item, storageOwner);
		
		//~ Clear any weight as the arsenal is being cleared
		if (m_bNegateItemWeightAndVolume && !m_bItemAddCallQuequeCalled && Replication.IsServer())
		{
			if (storageOwner.GetAdditionalWeight() != 0)
				storageOwner.SetAdditionalWeight(0);
			
			InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
			if (!inventoryItem)
				return;
			if (inventoryItem.GetAdditionalVolume() != 0)
				inventoryItem.SetAdditionalVolume(0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void FillInitialPrefabsToStore(out array<ResourceName> prefabsToSpawn)
	{
		super.FillInitialPrefabsToStore(prefabsToSpawn);
		
		if (SCR_Global.IsEditMode())
			return;
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(GetOwner().FindComponent(SCR_ArsenalComponent));
		if (!arsenalComponent)
			return;
		
		array<ResourceName> prefabs = {};
		if (arsenalComponent.GetAvailablePrefabs(prefabs))
			OnArsenalUpdated(prefabs);
		else if (!SCR_EntityCatalogManagerComponent.GetInstance())
			SCR_EntityCatalogManagerComponent.GetOnEntityCatalogInitialized().Insert(OnEntityCatalogInitialized);
		
		//~ Subscribe to arsenal changes
		arsenalComponent.GetOnArsenalUpdated().Insert(OnArsenalUpdated);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntityCatalogInitialized()
	{
		SCR_EntityCatalogManagerComponent.GetOnEntityCatalogInitialized().Remove(OnEntityCatalogInitialized);
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(GetOwner().FindComponent(SCR_ArsenalComponent));
		if (!arsenalComponent)
			return;

		array<ResourceName> prefabs = {};
		if (!arsenalComponent.GetAvailablePrefabs(prefabs))
			return;

		OnArsenalUpdated(prefabs);
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_ArsenalInventoryStorageManagerComponent()
	{
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(GetOwner().FindComponent(SCR_ArsenalComponent));
		if (!arsenalComponent)
			return;
		
		arsenalComponent.GetOnArsenalUpdated().Remove(OnArsenalUpdated);
	}
}
