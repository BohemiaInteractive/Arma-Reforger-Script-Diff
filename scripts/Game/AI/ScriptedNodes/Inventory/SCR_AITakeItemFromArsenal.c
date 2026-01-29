class SCR_AITakeItemFromArsenal_InventoryCallback : ScriptedInventoryOperationCallback
{
	protected IEntity m_ItemEntity;
	
	IEntity GetEntity() 
	{ 
		return m_ItemEntity;
	}

	override protected void OnComplete()
	{
		RplId itemId = GetItem();
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(itemId));
		if (rplComp)
			m_ItemEntity = rplComp.GetEntity();
	}
}

class SCR_AITakeItemFromArsenal : AITaskScripted
{
	// Inputs
	protected static const string PORT_PREFAB_RESOURCE_NAME = "PrefabResourceName";
	protected static const string PORT_ARSENAL_ENTITY = "ArsenalEntity";
	
	// Outputs
	protected static const string PORT_ITEM_ENTITY = "ItemEntity";
	protected static const string PORT_COUNT_ITEMS_TAKEN = "CountItemsTaken";
	
	[Attribute("1", UIWidgets.EditBox, desc: "Max amount of items we will try to take")]
	protected int m_iMaxItemsToTake;
	
	//------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		// Read inputs
		ResourceName prefabResourceName;
		IEntity arsenalEntity;
		
		GetVariableIn(PORT_PREFAB_RESOURCE_NAME, prefabResourceName);
		GetVariableIn(PORT_ARSENAL_ENTITY, arsenalEntity);
		
		if (prefabResourceName.IsEmpty() || !arsenalEntity)
			return ENodeResult.FAIL;
		
		// Verify arsenal, verify that arsenal has item
		SCR_ArsenalInventoryStorageManagerComponent arsenalComp =
			SCR_ArsenalInventoryStorageManagerComponent.Cast(arsenalEntity.FindComponent(SCR_ArsenalInventoryStorageManagerComponent));
		
		if (!arsenalComp)
			return ENodeResult.FAIL;
		
		if (!arsenalComp.IsPrefabInArsenalStorage(prefabResourceName))
			return ENodeResult.FAIL;
		
		// Verify own inventory
		IEntity myEntity = owner.GetControlledEntity();
		if (!myEntity)
			return ENodeResult.FAIL;
		InventoryStorageManagerComponent myInvComp = InventoryStorageManagerComponent.Cast(myEntity.FindComponent(InventoryStorageManagerComponent));
		if (!myInvComp)
			return ENodeResult.FAIL;
		
		int nItemsTaken = 0;
		IEntity lastTakenItem = null;
		SCR_AITakeItemFromArsenal_InventoryCallback invCallback = new SCR_AITakeItemFromArsenal_InventoryCallback();
		
		//~ Get item cost
		float resourceCost = 0;
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(arsenalEntity.FindComponent(SCR_ResourceComponent));
		SCR_ResourceConsumer consumer = null;
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (entityCatalogManager)
		{
			//~ Get Supply cost only if arsenal has supplies enabled
			SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(arsenalEntity);
			if (!arsenalComponent || arsenalComponent.IsArsenalUsingSupplies())
			{
				SCR_Faction faction;
				if (arsenalComponent)
					faction = arsenalComponent.GetAssignedFaction();
				
				SCR_EntityCatalogEntry entry;
				
				if (faction)
					 entry = entityCatalogManager.GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType.ITEM, prefabResourceName, faction);
				else 
					entry = entityCatalogManager.GetEntryWithPrefabFromCatalog(EEntityCatalogType.ITEM, prefabResourceName);
				
				if (entry)
				{
					SCR_ArsenalItem data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
					if (data)
					{
						if (arsenalComponent)
							resourceCost = data.GetSupplyCost(arsenalComponent.GetSupplyCostType());
						else 
							resourceCost = data.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT);
					}
						
				}
			}
			
			if (resourceCost > 0)
			{
				if (!resourceComponent)
					return ENodeResult.FAIL;
				
				consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				if (!consumer)
					return ENodeResult.FAIL;
				
				resourceCost *= consumer.GetBuyMultiplier();
			}			
		}
		
		while (true)
		{
			if (consumer)
			{
				SCR_ResourceConsumtionResponse response = consumer.RequestAvailability(resourceCost);
				if (response.GetReason() != EResourceReason.SUFFICIENT)
					break; // Not enough supplies anymore to get another item
			}

			bool success = myInvComp.TrySpawnPrefabToStorage(prefabResourceName, purpose: EStoragePurpose.PURPOSE_DEPOSIT, cb: invCallback);
			IEntity itemEntity = invCallback.GetEntity();
			
			if (!success || !itemEntity)
				break;
			
			nItemsTaken++;
			lastTakenItem = itemEntity;
			
			// Successful taking to inventory, there was enough space, so now actually consume the supplies.
			if (consumer)
				consumer.RequestConsumtion(resourceCost);
			
			if (m_iMaxItemsToTake != -1 && nItemsTaken == m_iMaxItemsToTake)
				break;
		}

		if (nItemsTaken == 0)
			return ENodeResult.FAIL;
		
		SetVariableOut(PORT_ITEM_ENTITY, lastTakenItem);
		SetVariableOut(PORT_COUNT_ITEMS_TAKEN, nItemsTaken);
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------------
	protected ref TStringArray s_aVarsIn = { PORT_ARSENAL_ENTITY, PORT_PREFAB_RESOURCE_NAME };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected ref TStringArray s_aVarsOut = { PORT_ITEM_ENTITY, PORT_COUNT_ITEMS_TAKEN };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override static bool VisibleInPalette() { return true; }
	
	override static string GetOnHoverDescription() { return "Takes item from arsenal and transfers it to own inventory. Returns success if at least one item was taken"; }
}