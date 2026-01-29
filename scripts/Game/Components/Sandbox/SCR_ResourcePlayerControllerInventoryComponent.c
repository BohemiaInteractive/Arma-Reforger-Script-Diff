enum EResourcePlayerInteractionType
{
	VEHICLE_LOAD,
	VEHICLE_UNLOAD,
	STORAGE,
	INVENTORY_SPLIT
}

void ScriptInvoker_ResourceOnPlayerInteraction(EResourcePlayerInteractionType interactionType, PlayerController playerController, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue);
typedef func ScriptInvokerActiveWidgetInteractionFunc;
typedef ScriptInvokerBase<ScriptInvokerActiveWidgetInteractionFunc> ScriptInvokerResourceOnPlayerInteraction;

void OnArsenalItemRequestedMethod(SCR_ResourceComponent resourceComponent, ResourceName resourceName, IEntity requesterEntity, BaseInventoryStorageComponent inventoryStorageComponent, EResourceType resourceType, int resourceValue);
typedef func OnArsenalItemRequestedMethod;
typedef ScriptInvokerBase<OnArsenalItemRequestedMethod> OnArsenalItemRequestedInvoker;

[ComponentEditorProps(category: "GameScripted/Resources", description: "")]
class SCR_ResourcePlayerControllerInventoryComponentClass : ScriptComponentClass
{	
}

typedef SCR_ResourceSystemSubscriptionHandle<SCR_ResourcePlayerControllerInventoryComponent> SCR_SCR_ResourcePlayerControllerInventoryComponentSubscriptionHandle;

class SCR_ResourcePlayerControllerInventoryComponent : ScriptComponent
{
	protected ref ScriptInvokerResourceOnPlayerInteraction m_OnPlayerInteractionInvoker;
	protected ref ScriptInvokerResourceOnPlayerInteraction m_OnBeforePlayerInteractionInvoker;
	protected static ref OnArsenalItemRequestedInvoker s_OnArsenalItemRequested;
	
	protected const float VERTICAL_SPAWN_OFFSET = 10;
	protected const float HORIZONTAL_SPAWN_OFFSET = 2.5;

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerResourceOnPlayerInteraction GetOnPlayerInteraction()
	{
		if (!m_OnPlayerInteractionInvoker)
			m_OnPlayerInteractionInvoker = new ScriptInvokerResourceOnPlayerInteraction();
		
		return m_OnPlayerInteractionInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerResourceOnPlayerInteraction GetOnBeforePlayerInteraction()
	{
		if (!m_OnBeforePlayerInteractionInvoker)
			m_OnBeforePlayerInteractionInvoker = new ScriptInvokerResourceOnPlayerInteraction();
		
		return m_OnBeforePlayerInteractionInvoker;
	}

	//------------------------------------------------------------------------------------------------
	static OnArsenalItemRequestedInvoker GetOnArsenalItemRequested()
	{
		if (!s_OnArsenalItemRequested)
			s_OnArsenalItemRequested = new OnArsenalItemRequestedInvoker();

		return s_OnArsenalItemRequested;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceActor TryGetGenerationActor(notnull SCR_ResourceComponent resourceComponent, EResourceType resourceType, out float currentResourceValue, out float maxResourceValue)
	{
		SCR_ResourceContainer containerFrom = resourceComponent.GetContainer(resourceType);
		
		if (containerFrom)
		{
			currentResourceValue	= containerFrom.GetResourceValue();
			maxResourceValue		= containerFrom.GetMaxResourceValue();
			
			return containerFrom;
		}
		
		SCR_ResourceGenerator generator = resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT_STORAGE, resourceType);
		
		if (generator)
		{
			currentResourceValue	= generator.GetAggregatedResourceValue();
			maxResourceValue		= generator.GetAggregatedMaxResourceValue();
			
			return generator;
		}
		
		generator = resourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_UNLOAD, resourceType);
		
		if (generator)
		{
			currentResourceValue	= generator.GetAggregatedResourceValue();
			maxResourceValue		= generator.GetAggregatedMaxResourceValue();
			
			return generator;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceActor TryGetConsumptionActor(notnull SCR_ResourceComponent resourceComponent, EResourceType resourceType, out float currentResourceValue, out float maxResourceValue)
	{
		SCR_ResourceContainer containerFrom = resourceComponent.GetContainer(resourceType);
		
		if (containerFrom)
		{
			currentResourceValue	= containerFrom.GetResourceValue();
			maxResourceValue		= containerFrom.GetMaxResourceValue();
			
			return containerFrom;
		}
		
		SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, resourceType);
		
		if (consumer)
		{
			currentResourceValue	= consumer.GetAggregatedResourceValue();
			maxResourceValue		= consumer.GetAggregatedMaxResourceValue();
			
			return consumer;
		}
		
		consumer = resourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, resourceType);
		
		if (consumer)
		{
			currentResourceValue	= consumer.GetAggregatedResourceValue();
			maxResourceValue		= consumer.GetAggregatedMaxResourceValue();
			
			return consumer;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if it's possible to comnsume resourceValue resources from the specified actor.
	//! \param[in] actor Actor to consume from.
	//! \param[in] resourceValue The resource value to consume.
	protected bool CheckResourceConsumptionAvailability(notnull SCR_ResourceActor actor, float resourceValue)
	{
		SCR_ResourceContainer container = SCR_ResourceContainer.Cast(actor);
		
		if (container)
			return container.GetResourceValue() >= resourceValue;
		
		SCR_ResourceConsumer consumer = SCR_ResourceConsumer.Cast(actor);
		
		if (consumer)
			return consumer.RequestAvailability(resourceValue).GetReason() == EResourceReason.SUFFICIENT;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if it's possible to generate resourceValue resources from the specified actor.
	//! \param[in] actor Actor to generate to.
	//! \param[in] resourceValue The resource value to generate.
	protected bool CheckResourceGenerationAvailability(notnull SCR_ResourceActor actor, float resourceValue)
	{
		SCR_ResourceContainer container = SCR_ResourceContainer.Cast(actor);
		
		if (container)
			return container.ComputeResourceDifference() >= resourceValue;
		
		SCR_ResourceGenerator generator = SCR_ResourceGenerator.Cast(actor);
		
		if (generator)
			return generator.RequestAvailability(resourceValue).GetReason() == EResourceReason.SUFFICIENT;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] actor from which we want to try to consume resources from
	//! \param[in] resourceValue amount of resources that should be consumed
	//! \param[in] ignoreOnEmptyBehavior
	protected bool TryPerformResourceConsumption(notnull SCR_ResourceActor actor, float resourceValue, bool ignoreOnEmptyBehavior = false)
	{
		SCR_ResourceContainer container = SCR_ResourceContainer.Cast(actor);
		
		if (container)
		{
			EResourceContainerOnEmptyBehavior emptyBehavior = container.GetOnEmptyBehavior();
			
			if (ignoreOnEmptyBehavior)
				container.SetOnEmptyBehavior(EResourceContainerOnEmptyBehavior.NONE);
			
			if (!container.DecreaseResourceValue(resourceValue))
				return false;
			
			container.SetOnEmptyBehavior(emptyBehavior);
			
			return true;
		}
		
		SCR_ResourceConsumer consumer = SCR_ResourceConsumer.Cast(actor);
		
		if (consumer)
			return consumer.RequestConsumtion(resourceValue).GetReason() == EResourceReason.SUFFICIENT;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] actor to which we want to try to add resources
	//! \param[in] resourceValue amount of resources that should be added
	protected bool TryPerformResourceGeneration(notnull SCR_ResourceActor actor, float resourceValue)
	{
		SCR_ResourceContainer container = SCR_ResourceContainer.Cast(actor);
		
		if (container)
			return container.IncreaseResourceValue(resourceValue);
		
		SCR_ResourceGenerator generator = SCR_ResourceGenerator.Cast(actor);
		
		if (generator)
			return generator.RequestGeneration(resourceValue).GetReason() == EResourceReason.SUFFICIENT;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] resourceComponentRplId
	//! \param[in] interactorType
	//! \param[in] resourceType
	//! \param[in] resourceIdentifier
	void RequestSubscription(RplId resourceComponentRplId, typename interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		Rpc(RpcAsk_RequestSubscription, resourceComponentRplId, interactorType.ToString(), resourceType, resourceIdentifier);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] resourceComponentRplId
	//! \param[in] interactorType
	//! \param[in] resourceType
	//! \param[in] resourceIdentifier
	void RequestUnsubscription(RplId resourceComponentRplId, typename interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		Rpc(RpcAsk_RequestUnsubscription, resourceComponentRplId, interactorType.ToString(), resourceType, resourceIdentifier);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] resourceComponentRplId
	//! \param[in] interactorType
	//! \param[in] resourceType
	//! \param[in] resourceIdentifier
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_RequestSubscription(RplId resourceComponentRplId, string interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		if (!resourceComponentRplId.IsValid())
			return;
		
		const typename interactorTypename = interactorType.ToType();
		
		if (!interactorTypename)
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(Replication.FindItem(resourceComponentRplId));
		
		if (!resourceComponent)
			return;

		SCR_ResourceInteractor interactor;
				
		if (interactorTypename.IsInherited(SCR_ResourceGenerator))
			interactor = resourceComponent.GetGenerator(resourceIdentifier, resourceType);
		else if (interactorTypename.IsInherited(SCR_ResourceConsumer))
			interactor = resourceComponent.GetConsumer(resourceIdentifier, resourceType);
		else
			return;
		
		GetGame().GetResourceSystemSubscriptionManager().SubscribeListener(Replication.FindId(this), interactor);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] resourceComponentRplId
	//! \param[in] interactorType
	//! \param[in] resourceType
	//! \param[in] resourceIdentifier
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_RequestUnsubscription(RplId resourceComponentRplId, string interactorType, EResourceType resourceType, EResourceGeneratorID resourceIdentifier)
	{
		if (!resourceComponentRplId.IsValid())
			return;
		
		const typename interactorTypename = interactorType.ToType();
		
		if (!interactorTypename)
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(Replication.FindItem(resourceComponentRplId));
		
		if (!resourceComponent)
			return;

		SCR_ResourceInteractor interactor;
		
		if (interactorTypename.IsInherited(SCR_ResourceGenerator))
			interactor = resourceComponent.GetGenerator(resourceIdentifier, resourceType);
		else if (interactorTypename.IsInherited(SCR_ResourceConsumer))
			interactor = resourceComponent.GetConsumer(resourceIdentifier, resourceType);
		else
			return;
		
		GetGame().GetResourceSystemSubscriptionManager().UnsubscribeListener(Replication.FindId(this), interactor);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] interactionType
	//! \param[in] rplIdResourceComponentFrom
	//! \param[in] rplIdResourceComponentTo
	//! \param[in] resourceType
	//! \param[in] resourceValue
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcAsk_OnPlayerInteraction(EResourcePlayerInteractionType interactionType, RplId rplIdResourceComponentFrom, RplId rplIdResourceComponentTo, EResourceType resourceType, float resourceValue)
	{
		if (!rplIdResourceComponentFrom.IsValid())
			return;
		
		PlayerController playerController = PlayerController.Cast(GetOwner());
		
		if (!playerController)
			return;
		
		SCR_ResourceComponent resourceComponentFrom = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdResourceComponentFrom));
		
		if (!resourceComponentFrom)
			return;
		
		SCR_ResourceComponent resourceComponentTo = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdResourceComponentTo));
		
		OnPlayerInteraction(interactionType, resourceComponentFrom, resourceComponentTo, resourceType, resourceValue, false);	
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rplIdResourceComponent
	//! \param[in] rplIdInventoryManager
	//! \param[in] rplIdStorageComponent
	//! \param[in] resourceNameItem
	//! \param[in] resourceType
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ArsenalRequestItem_(RplId rplIdResourceComponent, RplId rplIdInventoryManager, RplId rplIdStorageComponent, ResourceName resourceNameItem, EResourceType resourceType)
	{
		if (!rplIdInventoryManager.IsValid())
			return;
		
		SCR_InventoryStorageManagerComponent inventoryManagerComponent = SCR_InventoryStorageManagerComponent.Cast(Replication.FindItem(rplIdInventoryManager));
		
		if (!inventoryManagerComponent)
			return;
		
		
		if (!rplIdStorageComponent.IsValid())
			return;
		
		BaseInventoryStorageComponent storageComponent = BaseInventoryStorageComponent.Cast(Replication.FindItem(rplIdStorageComponent));
		
		if (!storageComponent)
			return;
		
		if (!rplIdResourceComponent.IsValid())
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdResourceComponent));
		if (!resourceComponent)
			return;
		
		SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, resourceType);
		if (!consumer)
			return;
		
		float resourceCost = 0;
		int personalResourceCost;
		
		SCR_ArsenalItem data;
		SCR_Faction faction;
		//~ Get item cost
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (entityCatalogManager)
		{
			IEntity resourcesOwner = resourceComponent.GetOwner();
			if (!resourcesOwner)
				return;

			// Spatial validation check 
			if (!inventoryManagerComponent.ValidateStorageRequest(resourcesOwner))
				return;
			
			//~ Get Supply cost only if arsenal has supplies enabled
			SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(resourcesOwner);
			if ((arsenalComponent && arsenalComponent.IsArsenalUsingSupplies()) || !arsenalComponent)
			{
				if (arsenalComponent)
					faction = arsenalComponent.GetAssignedFaction();
				
				SCR_EntityCatalogEntry entry;
				
				if (faction)
					 entry = entityCatalogManager.GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType.ITEM, resourceNameItem, faction);
				else 
					entry = entityCatalogManager.GetEntryWithPrefabFromCatalog(EEntityCatalogType.ITEM, resourceNameItem);
				
				if (entry)
				{
					data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
					if (data)
					{
						if (arsenalComponent)
							resourceCost = data.GetSupplyCost(arsenalComponent.GetSupplyCostType());
						else 
							resourceCost = data.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT);
					}
						
				}
			}

			if (arsenalComponent)
			{
				SCR_ArsenalManagerComponent arsenalManager;
				if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
					personalResourceCost = arsenalManager.GetItemMilitarySupplyAllocationCost(resourceNameItem, arsenalComponent, true);
			}
		}

		//~ If Military Supply Allocation is enabled, check if player has enough Available Allocated Supplies
		SCR_PlayerSupplyAllocationComponent playerSupplyAllocationComponent = SCR_PlayerSupplyAllocationComponent.Cast(GetOwner().FindComponent(SCR_PlayerSupplyAllocationComponent));
		if (playerSupplyAllocationComponent && faction && personalResourceCost > 0)
		{
			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			if (factionManager)
			{
				SCR_Faction playerFaction = SCR_Faction.Cast(factionManager.GetPlayerFaction(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(SCR_PlayerController.Cast(GetOwner()).GetControlledEntity())));
				if (playerFaction)
				{
					if (playerFaction == faction && !playerSupplyAllocationComponent.HasPlayerEnoughAvailableAllocatedSupplies(personalResourceCost))
						return;
				}
			}
		}

		resourceCost *= consumer.GetBuyMultiplier();
		if (!TryPerformResourceConsumption(consumer, resourceCost))
			return;

		if (inventoryManagerComponent.TrySpawnPrefabToStorage(resourceNameItem, storageComponent, cb: new SCR_PrefabSpawnCallback(storageComponent)) && s_OnArsenalItemRequested)
			s_OnArsenalItemRequested.Invoke(resourceComponent, resourceNameItem, GetOwner(), storageComponent, resourceType, resourceCost);
	}

	//------------------------------------------------------------------------------------------------
	void RpcAsk_ArsenalRequestItem(RplId rplIdResourceComponent, RplId rplIdInventoryManager, RplId rplIdStorageComponent, ResourceName resourceNameItem, EResourceType resourceType)
	{
		Rpc(RpcAsk_ArsenalRequestItem_, rplIdResourceComponent, rplIdInventoryManager, rplIdStorageComponent, resourceNameItem, resourceType);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rplIdResourceComponent
	//! \param[in] rplIdInventoryItem
	//! \param[in] resourceType
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ArsenalRefundItem_(RplId rplIdResourceComponent, RplId rplIdInventoryItem, EResourceType resourceType)
	{
		if (!rplIdInventoryItem.IsValid())
			return;
		
		InventoryItemComponent inventoryItemComponent = InventoryItemComponent.Cast(Replication.FindItem(rplIdInventoryItem));
		
		if (!inventoryItemComponent)
			return;
		
		if (!rplIdResourceComponent.IsValid())
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdResourceComponent));
		if (!resourceComponent)
			return;
		
		SCR_ResourceGenerator generator	= resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, resourceType);
		if (!generator)
			return;
		
		IEntity inventoryItemEntity	= inventoryItemComponent.GetOwner();
		if (!inventoryItemEntity)
			return;
		
		IEntity resourcesOwner = resourceComponent.GetOwner();
		if (!resourcesOwner)
			return;
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(resourcesOwner);
		
		//~ Get resource cost (cap at 0 minimum as function can return -1)
		float resourceCost = Math.Clamp(SCR_ArsenalManagerComponent.GetItemRefundAmount(inventoryItemEntity, arsenalComponent, false), 0, float.MAX);
		
		//~ Check if it can refund if resource cost is greater than 0
		if (resourceCost > 0 && !TryPerformResourceGeneration(generator, resourceCost))
			return;
		
		int personalResourceCost;

		SCR_ArsenalManagerComponent arsenalManager;
		if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager) && arsenalManager.IsMilitarySupplyAllocationEnabled())
			personalResourceCost = arsenalManager.GetItemMilitarySupplyAllocationRefundAmount(inventoryItemEntity, arsenalComponent);

		IEntity parentEntity = inventoryItemEntity.GetParent();
		SCR_InventoryStorageManagerComponent inventoryManagerComponent;
		ResourceName resourceNameItem = inventoryItemEntity.GetPrefabData().GetPrefabName();
		
		//~ On item refunded just before the item is deleted
		SCR_ArsenalManagerComponent.OnItemRefunded_S(inventoryItemEntity, PlayerController.Cast(GetOwner()), arsenalComponent);
		
		if (parentEntity)
			inventoryManagerComponent = SCR_InventoryStorageManagerComponent.Cast(parentEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (inventoryManagerComponent && !inventoryManagerComponent.TryDeleteItem(inventoryItemEntity))
			return;
		else if (!inventoryManagerComponent)
			RplComponent.DeleteRplEntity(inventoryItemEntity, false);
		
		if (personalResourceCost == 0)
			return;

		//~ Add refund cost to player's Available Allocated Supplies
		SCR_PlayerSupplyAllocationComponent playerSupplyAllocationComponent = SCR_PlayerSupplyAllocationComponent.Cast(GetOwner().FindComponent(SCR_PlayerSupplyAllocationComponent));
		if (playerSupplyAllocationComponent)
			playerSupplyAllocationComponent.AddPlayerAvailableAllocatedSupplies(personalResourceCost);
	}
	void RpcAsk_ArsenalRefundItem(RplId rplIdResourceComponent, RplId rplIdInventoryItem, EResourceType resourceType)
	{
		Rpc(RpcAsk_ArsenalRefundItem_, rplIdResourceComponent, rplIdInventoryItem, resourceType);
	}
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rplIdFrom
	//! \param[in] rplIdTo
	//! \param[in] resourceType
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_MergeContainerWithContainer(RplId rplIdFrom, RplId rplIdTo, EResourceType resourceType)
	{
		if (!rplIdFrom.IsValid() || !rplIdTo.IsValid())
			return;
		
		SCR_ResourceComponent componentFrom = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdFrom));
		
		if (!componentFrom)
			return;
		
		SCR_ResourceComponent componentTo = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdTo));
		
		if (!componentTo)
			return;
		
		float resourceValueCurrentFrom, resourceValueMaxFrom;
		SCR_ResourceActor actorFrom = TryGetConsumptionActor(componentFrom, resourceType, resourceValueCurrentFrom, resourceValueMaxFrom);
		
		float resourceValueCurrentTo, resourceValueMaxTo;
		SCR_ResourceActor actorTo = TryGetGenerationActor(componentTo, resourceType, resourceValueCurrentTo, resourceValueMaxTo);
		
		float resourceUsed = Math.Min(resourceValueCurrentFrom, resourceValueMaxTo - resourceValueCurrentTo);
		
		if (CheckResourceConsumptionAvailability(actorFrom, resourceUsed) && CheckResourceGenerationAvailability(actorTo, resourceUsed))
		{
			OnBeforePlayerInteraction(EResourcePlayerInteractionType.INVENTORY_SPLIT, componentFrom, componentTo, resourceType, resourceUsed);
			TryPerformResourceConsumption(actorFrom, resourceUsed);
			TryPerformResourceGeneration(actorTo, resourceUsed);
			OnPlayerInteraction(EResourcePlayerInteractionType.INVENTORY_SPLIT, componentFrom, componentTo, resourceType, resourceUsed);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rplIdFrom
	//! \param[in] rplIdTo
	//! \param[in] resourceType
	//! \param[in] requestedResources
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_MergeContainerWithContainerPartial_(RplId rplIdFrom, RplId rplIdTo, EResourceType resourceType, float requestedResources)
	{
		if (!rplIdFrom.IsValid() || !rplIdTo.IsValid())
			return;
		
		SCR_ResourceComponent componentFrom = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdFrom));
		
		if (!componentFrom)
			return;
		
		SCR_ResourceComponent componentTo = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdTo));
		
		if (!componentTo)
			return;
		
		float resourceValueCurrentFrom, resourceValueMaxFrom;
		SCR_ResourceActor actorFrom = TryGetConsumptionActor(componentFrom, resourceType, resourceValueCurrentFrom, resourceValueMaxFrom);
		
		float resourceValueCurrentTo, resourceValueMaxTo;
		SCR_ResourceActor actorTo = TryGetGenerationActor(componentTo, resourceType, resourceValueCurrentTo, resourceValueMaxTo);
		
		float resourceUsed = Math.Min(Math.Min(resourceValueCurrentFrom, resourceValueMaxTo - resourceValueCurrentTo), requestedResources);
		
		if (CheckResourceConsumptionAvailability(actorFrom, resourceUsed) && CheckResourceGenerationAvailability(actorTo, resourceUsed))
		{
			OnBeforePlayerInteraction(EResourcePlayerInteractionType.INVENTORY_SPLIT, componentFrom, componentTo, resourceType, resourceUsed);
			TryPerformResourceConsumption(actorFrom, resourceUsed);
			TryPerformResourceGeneration(actorTo, resourceUsed);
			OnPlayerInteraction(EResourcePlayerInteractionType.INVENTORY_SPLIT, componentFrom, componentTo, resourceType, resourceUsed);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rplIdFrom
	//! \param[in] rplIdTo
	//! \param[in] resourceType
	//! \param[in] requestedResources
	void RpcAsk_MergeContainerWithContainerPartial(RplId rplIdFrom, RplId rplIdTo, EResourceType resourceType, float requestedResources)
	{
		Rpc(RpcAsk_MergeContainerWithContainerPartial_, rplIdFrom, rplIdTo, resourceType, requestedResources);
	}

	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rplIdResourceComponent
	//! \param[in] rplIdInventoryManager
	//! \param[in] rplIdStorageComponent
	//! \param[in] resourceType
	//! \param[in] requestedResources
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CreatePhysicalContainerWithContainer_(RplId rplIdResourceComponent, RplId rplIdInventoryManager, RplId rplIdStorageComponent, EResourceType resourceType, float requestedResources)
	{
		if (!rplIdResourceComponent.IsValid())
			return;
		
		SCR_ResourceComponent resourceComponentFrom = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdResourceComponent));
		
		if (!resourceComponentFrom)
			return;
		
		float resourceValueCurrentFrom, resourceValueMaxFrom;
		SCR_ResourceActor actorFrom = TryGetConsumptionActor(resourceComponentFrom, resourceType, resourceValueCurrentFrom, resourceValueMaxFrom);
		
		if (!CheckResourceConsumptionAvailability(actorFrom, requestedResources))
			return;
		
		float resourceValueCurrentTo, resourceValueMaxTo;
		SCR_ResourceActor actorConsumptionTo, actorGenerationTo;
		
		if (!rplIdInventoryManager.IsValid())
		{
			SCR_EntityCatalog resourceContainerCatalog	= SCR_EntityCatalogManagerComponent.GetInstance().GetEntityCatalogOfType(EEntityCatalogType.SUPPLY_CONTAINER_ITEM); 
			array<SCR_EntityCatalogEntry> entries		= {};
			array<SCR_BaseEntityCatalogData> data		= {};
			SCR_ResourceContainerItemData datum;
			float resourceUsed;
			int selectedEntryIdx;
			
			resourceContainerCatalog.GetEntityListWithData(SCR_ResourceContainerItemData, entries, data);
			
			for (selectedEntryIdx = data.Count() - 1; selectedEntryIdx >= 0; --selectedEntryIdx)
			{
				datum			= SCR_ResourceContainerItemData.Cast(data[selectedEntryIdx]);
				resourceUsed	= datum.GetMaxResourceValue();
				
				if (resourceUsed >= requestedResources)
					break;
			}
			
			if (selectedEntryIdx < 0)
				selectedEntryIdx = 0;
			
			resourceUsed = Math.Min(resourceUsed, requestedResources);
			
			if (!TryPerformResourceConsumption(actorFrom, resourceUsed))
				return;

			// WARNING: From now on resourceComponentFrom.GetOwner().IsDeleted() can be true!
			// That happens, for example, when moving one last supply from player to supply truck. 
			// Due to a bug in Enfusion GenericEntity owner = resourceComponentFrom.GetOwner() 
			// will be NULL, so we always need to access the entity via resource component
			// and shall from now on never assign it directly to GenericEntity.
			
			vector position = resourceComponentFrom.GetOwner().GetOrigin();
			FindSuitablePosition(position, HORIZONTAL_SPAWN_OFFSET, VERTICAL_SPAWN_OFFSET);
			
			EntitySpawnParams spawnParams				= new EntitySpawnParams();
			spawnParams.TransformMode					= ETransformMode.WORLD;
			spawnParams.Transform[3]					= position;
			IEntity newStorageEntity					= GetGame().SpawnEntityPrefab(Resource.Load(entries[selectedEntryIdx].GetPrefab()), GetGame().GetWorld(), spawnParams);
			SCR_ResourceComponent resourceComponentTo	= SCR_ResourceComponent.FindResourceComponent(newStorageEntity);
			actorConsumptionTo							= TryGetConsumptionActor(resourceComponentTo, resourceType, resourceValueCurrentTo, resourceValueMaxTo);
			actorGenerationTo							= TryGetGenerationActor(resourceComponentTo, resourceType, resourceValueCurrentTo, resourceValueMaxTo);
			
			TryPerformResourceConsumption(actorConsumptionTo, resourceValueCurrentTo, true);
			
			if (TryPerformResourceGeneration(actorGenerationTo, resourceUsed))
			{
				OnPlayerInteraction(EResourcePlayerInteractionType.INVENTORY_SPLIT, resourceComponentFrom, resourceComponentTo, resourceType, resourceUsed);
				
				return;
			}
			
			delete newStorageEntity;
			
			return;
		}
		
		SCR_InventoryStorageManagerComponent inventoryManagerComponent = SCR_InventoryStorageManagerComponent.Cast(Replication.FindItem(rplIdInventoryManager));
		
		if (!inventoryManagerComponent)
			return;
		
		if (!rplIdStorageComponent.IsValid())
			return;
		
		BaseInventoryStorageComponent storageComponent = BaseInventoryStorageComponent.Cast(Replication.FindItem(rplIdStorageComponent));
		
		if (!storageComponent)
			return;
		
		IEntity resourcesOwner						= resourceComponentFrom.GetOwner();
		SCR_EntityCatalogEntry selectedEntry		= null;
		int selectedEntryIdx						= -1;
		SCR_EntityCatalog resourceContainerCatalog	= SCR_EntityCatalogManagerComponent.GetInstance().GetEntityCatalogOfType(EEntityCatalogType.SUPPLY_CONTAINER_ITEM); 
		array<SCR_EntityCatalogEntry> entries		= {};
		array<SCR_BaseEntityCatalogData> data		= {};
		
		resourceContainerCatalog.GetEntityListWithData(SCR_ResourceContainerItemData, entries, data);
		
		foreach (int idx, SCR_EntityCatalogEntry entry: entries)
		{
			if (inventoryManagerComponent.CanInsertResourceInStorage(entry.GetPrefab(), storageComponent))
			{
				selectedEntry		= entry;
				selectedEntryIdx	= idx;
	
				break;
			}
		}
		
		if (!selectedEntry)
			return;
		
		SCR_ResourceContainerItemData datum		= SCR_ResourceContainerItemData.Cast(data[selectedEntryIdx]);
		float maxStoredResources				= Math.Min(resourceValueCurrentFrom, datum.GetMaxResourceValue());
		float resourceUsed						= Math.Min(requestedResources, maxStoredResources);
		
		if (!TryPerformResourceConsumption(actorFrom, resourceUsed))
			return;
		
		EntitySpawnParams spawnParams	= new EntitySpawnParams();
		spawnParams.TransformMode		= ETransformMode.WORLD;
		
		inventoryManagerComponent.GetOwner().GetTransform(spawnParams.Transform);
		
		IEntity newStorageEntity = GetGame().SpawnEntityPrefab(Resource.Load(selectedEntry.GetPrefab()), GetGame().GetWorld(), spawnParams);
		
		if (!newStorageEntity)
			return;
		
		SCR_ResourceComponent resourceComponentTo	= SCR_ResourceComponent.FindResourceComponent(newStorageEntity);
		actorConsumptionTo							= TryGetConsumptionActor(resourceComponentTo, resourceType, resourceValueCurrentTo, resourceValueMaxTo);
		actorGenerationTo							= TryGetGenerationActor(resourceComponentTo, resourceType, resourceValueCurrentTo, resourceValueMaxTo);
		
		TryPerformResourceConsumption(actorConsumptionTo, resourceValueCurrentTo, true);
		
		if (!TryPerformResourceGeneration(actorGenerationTo, resourceUsed))
		{
			delete newStorageEntity;
			
			return;
		}
		
		inventoryManagerComponent.TryInsertItemInStorage(newStorageEntity, storageComponent);
		OnPlayerInteraction(EResourcePlayerInteractionType.INVENTORY_SPLIT, resourceComponentFrom, resourceComponentTo, resourceType, resourceUsed);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used for finding a position at which supplies can be spawned
	//! \param[in,out] position around which we want to search
	//! \param[in] maxHorizontalOffset determines how far the position can be on the X and Z axis
	//! \param[in] maxVerticalOffset determines how much higher search will be starting
	static void FindSuitablePosition(inout vector position, float maxHorizontalOffset, float maxVerticalOffset)
	{
		position = vector.Up * position[1] + SCR_Math2D.GenerateRandomPointInRadius(0, maxHorizontalOffset, position);

		TraceParam param	= new TraceParam();
		param.Start			= position + vector.Up * maxVerticalOffset;
		param.End			= position;
		param.Flags			= TraceFlags.WORLD | TraceFlags.ENTS;
		param.LayerMask		= EPhysicsLayerDefs.Projectile;

		float distanceTraveled = GetGame().GetWorld().TraceMove(param, TraceFilter);
		if (param.TraceEnt == null && distanceTraveled == 1)
		{ // If f.e. someone dropped supplies from a helo, then find the position on the ground, so they dont stay mid air
			position[1] = GetGame().GetWorld().GetSurfaceY(param.End[0], param.End[2]);
			param.End = position;
			param.Start = param.End + vector.Up * maxVerticalOffset;
			distanceTraveled = GetGame().GetWorld().TraceMove(param, TraceFilter);
		}

		if (distanceTraveled < 1)
			position = (param.End - param.Start) * distanceTraveled + param.Start;
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to filter entities found by the trace
	//! \param[in] e
	//! \param[in] start
	//! \param[in] dir
	protected static bool TraceFilter(notnull IEntity e, vector start = vector.Zero, vector dir = vector.Zero)
	{
		if (BaseTree.Cast(e))
			return false;//ignore trees and bushes

		if (Vehicle.Cast(e) || Vehicle.Cast(e.GetRootParent()))
			return false;//ignore vehicles and its parts

		if (ChimeraCharacter.Cast(e))
			return false;//ignore characters

		if (ChimeraCharacter.Cast(e.GetRootParent()))
			return false;//ignore character equipment

		if (SCR_BaseDebrisSmallEntity.Cast(e))
			return false;//ignore debris

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void RpcAsk_CreatePhysicalContainerWithContainer(RplId rplIdResourceComponent, RplId rplIdInventoryManager, RplId rplIdStorageComponent, EResourceType resourceType, float requestedResources)
	{
		Rpc(RpcAsk_CreatePhysicalContainerWithContainer_, rplIdResourceComponent, rplIdInventoryManager, rplIdStorageComponent, resourceType, requestedResources);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] interactionType
	//! \param[in] resourceComponentFrom
	//! \param[in] resourceComponentTo
	//! \param[in] resourceType
	//! \param[in] resourceValue
	//! \param[in] shouldBroadcast
	void OnPlayerInteraction(EResourcePlayerInteractionType interactionType, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue, bool shouldBroadcast = true)
	{
		IEntity owner = GetOwner();
		PlayerController playerController = PlayerController.Cast(owner);
		
		if (!playerController)
			return;
		
		if (shouldBroadcast)
		{
			RplComponent replicationComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
			
			if (replicationComponent && !replicationComponent.IsProxy() && !replicationComponent.IsOwner())
				Rpc(RpcAsk_OnPlayerInteraction, interactionType, Replication.FindId(resourceComponentFrom), Replication.FindId(resourceComponentTo), resourceType, resourceValue);
			
			ChimeraWorld world = GetGame().GetWorld();
			SCR_SupplySystem system = SCR_SupplySystem.Cast(world.FindSystem(SCR_SupplySystem));
			if (system)
				system.ActivateEffects(interactionType, resourceComponentFrom, resourceComponentTo, resourceType, resourceValue);		
		}

			GetOnPlayerInteraction().Invoke(interactionType, owner, resourceComponentFrom, resourceComponentTo, resourceType, resourceValue);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] interactionType
	//! \param[in] resourceComponentFrom
	//! \param[in] resourceComponentTo
	//! \param[in] resourceType
	//! \param[in] resourceValue
	void OnBeforePlayerInteraction(EResourcePlayerInteractionType interactionType, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue)
	{
		GetOnBeforePlayerInteraction().Invoke(interactionType, GetOwner(), resourceComponentFrom, resourceComponentTo, resourceType, resourceValue);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when Entity is being to be destroyed (deleted) or component to be deleted (see Game::DeleteScriptComponent).
	//! \param[in] owner Entity which owns the component
	override event protected void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
		
		GetGame().GetResourceSystemSubscriptionManager().UnsubscribeListenerCompletely(Replication.FindId(this));
	}
}
