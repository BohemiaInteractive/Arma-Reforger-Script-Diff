[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionItemSafeguard : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Actions that will be executed when target item is dropped", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnItemDropped;
	
	[Attribute(desc: "Actions that will be executed when target item is possesed by someone/something", UIWidgets.Auto)]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnItemPossessed;
	
	protected IEntity m_ItemEntity;
	SCR_EScenarioFrameworkItemGCState m_eGarbageCollectionStatus = SCR_EScenarioFrameworkItemGCState.UNDEFINED;
	
	//------------------------------------------------------------------------------------------------
	void OnItemPossessed(IEntity item, BaseInventoryStorageComponent pStorageOwner)
	{
		if(!item || item != m_ItemEntity)
			return;
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnItemPossessed)
		{
			action.OnActivate(pStorageOwner.GetOwner());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnItemDropped(IEntity item, BaseInventoryStorageComponent pStorageOwner)
	{
		if(!item || item != m_ItemEntity)
			return;

		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnItemDropped)
		{
			action.OnActivate(pStorageOwner.GetOwner());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemCarrierChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		EventHandlerManagerComponent eventHandlerMgr;
		InventoryItemComponent inventoryComponent;
		IEntity owner;
		IEntity rootOwner;
		if (oldSlot)
		{
			owner = oldSlot.GetOwner();
			rootOwner = owner.GetRootParent();
			
			//Handles the case when for example item was inserted into the character inventory somewhere
			eventHandlerMgr = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
			if (eventHandlerMgr)
				eventHandlerMgr.RemoveScriptHandler("OnDestroyed", this, OnDestroyed);
			
			//Handles the case when for example item was inserted into a backpack that was already in a vehicle
			if (owner != rootOwner)
			{
				eventHandlerMgr = EventHandlerManagerComponent.Cast(rootOwner.FindComponent(EventHandlerManagerComponent));
				if (eventHandlerMgr)
					eventHandlerMgr.RemoveScriptHandler("OnDestroyed", this, OnDestroyed);
				
				//Handles possible revert of case prevention where GarbageSystem might delete root entity
				if (m_eGarbageCollectionStatus != SCR_EScenarioFrameworkItemGCState.UNDEFINED)
				{
					HandleOldSlotGC(rootOwner);
				}
			}
			else
			{
				HandleOldSlotGC(owner);
			}
			
			//Handles the case when for example item was inserted into a backpack and then this backpack was inserted into a vehicle
			inventoryComponent = InventoryItemComponent.Cast(owner.FindComponent(InventoryItemComponent));
			if (inventoryComponent)
				inventoryComponent.m_OnParentSlotChangedInvoker.Remove(OnItemCarrierChanged);
		}
		
		if (newSlot)
		{
			owner = newSlot.GetOwner();
			rootOwner = owner.GetRootParent();
			
			//Handles the case when for example item is inserted into the character inventory somewhere
			eventHandlerMgr = EventHandlerManagerComponent.Cast(owner.FindComponent(EventHandlerManagerComponent));
			if (eventHandlerMgr)
				eventHandlerMgr.RegisterScriptHandler("OnDestroyed", this, OnDestroyed);
			
			//Handles the case when for example item is inserted into a backpack that is already in a vehicle
			if (owner != rootOwner)
			{
				eventHandlerMgr = EventHandlerManagerComponent.Cast(rootOwner.FindComponent(EventHandlerManagerComponent));
				if (eventHandlerMgr)
					eventHandlerMgr.RegisterScriptHandler("OnDestroyed", this, OnDestroyed);
				
				HandleNewSlotGC(rootOwner);
			}
			else
			{
				HandleNewSlotGC(owner);
			}
			
			//Handles the case when for example item is inserted into a backpack and then this backpack is inserted into a vehicle
			inventoryComponent = InventoryItemComponent.Cast(owner.FindComponent(InventoryItemComponent));
			if (inventoryComponent)
			{
				inventoryComponent.m_OnParentSlotChangedInvoker.Insert(OnItemCarrierChanged);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles the cases where GarbageSystem might delete new slot entity
	protected void HandleNewSlotGC(IEntity entity)
	{
		SCR_GarbageSystem garbageSystem = SCR_GarbageSystem.GetByEntityWorld(entity);
		if (!garbageSystem)
			return;
		
		if (garbageSystem.IsInserted(entity))
			m_eGarbageCollectionStatus = SCR_EScenarioFrameworkItemGCState.NOT_EXCLUDED;
		else
			m_eGarbageCollectionStatus = SCR_EScenarioFrameworkItemGCState.EXCLUDED;
					
		garbageSystem.UpdateBlacklist(entity, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles possible revert of case prevention where GarbageSystem might delete root entity
	protected void HandleOldSlotGC(IEntity entity)
	{
		SCR_GarbageSystem garbageSystem = SCR_GarbageSystem.GetByEntityWorld(entity);
		if (garbageSystem && m_eGarbageCollectionStatus == SCR_EScenarioFrameworkItemGCState.NOT_EXCLUDED)
		{
			garbageSystem.UpdateBlacklist(entity, false);
			garbageSystem.Insert(entity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used to remove task item from destroyed entity inventory and drop it to the ground
	protected void OnDestroyed(IEntity destroyedEntity)
	{
		if (!destroyedEntity)
			return;
		
		if (!m_ItemEntity)
			return;
		
		InventoryItemComponent invComp = InventoryItemComponent.Cast(m_ItemEntity.FindComponent(InventoryItemComponent));
		if (!invComp)
			return;
		
		InventoryStorageSlot parentSlot = invComp.GetParentSlot();
		if (!parentSlot)
			return;
		
		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(destroyedEntity.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryComponent)
			return;
		
		if (!inventoryComponent.Contains(m_ItemEntity))
			return;
		
		inventoryComponent.TryRemoveItemFromStorage(m_ItemEntity, parentSlot.GetStorage());
		
		vector position;
		array<IEntity> excludedEntities = {};
		excludedEntities.Insert(destroyedEntity);
		excludedEntities.Insert(m_ItemEntity);
		
		ChimeraCharacter character = ChimeraCharacter.Cast(destroyedEntity);
		if (character && character.IsInVehicle())
		{
			CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
			if (compartmentAccess)
			{
				BaseCompartmentSlot compartmentSlot = compartmentAccess.GetCompartment();
				if (compartmentSlot)
				{
					IEntity vehicle = compartmentSlot.GetOwner();
					excludedEntities.Insert(vehicle);
				}
			}
		}
		
		SCR_TerrainHelper.SnapToGeometry(position, m_ItemEntity.GetOrigin(), excludedEntities, m_ItemEntity.GetWorld());
		
		m_ItemEntity.SetOrigin(position);
		m_ItemEntity.Update();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDisconnected(int playerID)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!player)
			return;
			
		OnDestroyed(player);
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterPlayer(int playerID, IEntity playerEntity)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!player)
			return;

		SCR_InventoryStorageManagerComponent inventoryComponent = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryComponent)
			return;

		inventoryComponent.m_OnItemAddedInvoker.Insert(OnItemPossessed);
		inventoryComponent.m_OnItemRemovedInvoker.Insert(OnItemDropped);
			
		EventHandlerManagerComponent eventHandlerMgr = EventHandlerManagerComponent.Cast(player.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerMgr)
			eventHandlerMgr.RegisterScriptHandler("OnDestroyed", this, OnDestroyed);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!ValidateInputEntity(object, m_Getter, m_ItemEntity))
			return;

		InventoryItemComponent invComp = InventoryItemComponent.Cast(m_ItemEntity.FindComponent(InventoryItemComponent));
		if (!invComp)
			return;
		
		invComp.m_OnParentSlotChangedInvoker.Insert(OnItemCarrierChanged);
			
		array<int> aPlayerIDs = {};
		int iNrOfPlayersConnected = GetGame().GetPlayerManager().GetPlayers(aPlayerIDs); 
					
		foreach (int playerID : aPlayerIDs)
		{
			RegisterPlayer(playerID, null);
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		gameMode.GetOnPlayerSpawned().Insert(RegisterPlayer);
		gameMode.GetOnPlayerDisconnected().Insert(OnDisconnected);
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		array<ref SCR_ScenarioFrameworkActionBase> m_aSubActions = {};
		if (m_aActionsOnItemDropped)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnItemDropped)
			{
				m_aSubActions.Insert(action);
			}
		}
		
		if (m_aActionsOnItemPossessed)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnItemPossessed)
			{
				m_aSubActions.Insert(action);
			}
		}
		
		return m_aSubActions;
	}
}