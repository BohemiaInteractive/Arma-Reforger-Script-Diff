class SCR_TaskDeliverClass: SCR_ScenarioFrameworkTaskClass
{
};

/*
			  TASK DELIVERY STATES
	-----------------------------------------
	| Delivery		| Item		| Item		|
	| Point exists 	| possesed	| Dropped 	|
	-----------------------------------------
	|	0			|	0		|	0		|	0
	-----------------------------------------
	|	0			|	0		|	1		|	1
	-----------------------------------------
	|	0			|	1		|	0		|	2
	-----------------------------------------
	|	1			|	0		|	0		|	4
	-----------------------------------------
	|	1			|	0		|	1		|	5
	-----------------------------------------
	|	1			|	1		|	0		|	6
	-----------------------------------------
*/

enum SCR_EScenarioFrameworkItemGCState
{
	UNDEFINED,
	NOT_EXCLUDED,
	EXCLUDED
}

class SCR_TaskDeliver : SCR_ScenarioFrameworkTask
{	
	protected string							m_sDeliveryTriggerName;
	
	protected int								m_iObjectState = 0;	//see the table above
	protected  SCR_BaseTriggerEntity			m_TriggerDeliver;
	protected bool								m_bDeliveryItemFound;
	protected bool 								m_bTaskPositionUpdated;
	SCR_EScenarioFrameworkItemGCState m_eGarbageCollectionStatus = SCR_EScenarioFrameworkItemGCState.UNDEFINED;
		
	//------------------------------------------------------------------------------------------------
	//! \return Represents current state of task delivery object. 0 if item is not possesed and 1 if possesed (someone has it in his possesion)
	int GetTaskDeliverState()
	{
		return m_iObjectState;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets delivery trigger by name if it exists in the world.
	void SetDeliveryTrigger()
	{
		if (!GetGame().GetWorld())
			return;
		
		if (m_sDeliveryTriggerName.IsEmpty())
		{
			Print("ScenarioFramework: Task Deliver trigger is set with empty attribute SetDeliveryTrigger", LogLevel.ERROR);
			return;
		}	

		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sDeliveryTriggerName);
		if (!entity)
			return;

		SetDeliveryTrigger(SCR_BaseTriggerEntity.Cast(entity));
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] trigger Trigger entity for delivery activation.
	void SetDeliveryTrigger(SCR_BaseTriggerEntity trigger)
	{
		if (!trigger)
			return;

		m_TriggerDeliver = trigger;
		m_TriggerDeliver.GetOnActivate().Remove(OnDeliveryTriggerActivated);
		m_TriggerDeliver.GetOnActivate().Insert(OnDeliveryTriggerActivated);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] triggerName Represents the name of the trigger for delivering an object in the scenario.
	void SetTriggerNameToDeliver(string triggerName)
	{
		if (triggerName.IsEmpty())
		{
			Print("ScenarioFramework: Task Deliver trigger is set with empty attribute SetTriggerNameToDeliver", LogLevel.ERROR);
			return;
		}
		
		m_sDeliveryTriggerName = triggerName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the name of trigger for delivery.
	string GetTriggerNameToDeliver()
	{
		return m_sDeliveryTriggerName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] trigger Trigger activates when entities enter its area, checks if asset is inside, finishes task if found, else checks ch
	void OnDeliveryTriggerActivated(notnull SCR_ScenarioFrameworkTriggerEntity trigger)
	{
		if (!m_Asset)
			return;
		
		array<IEntity> entitiesInside = {};
		array<SCR_ChimeraCharacter> chimeraCharacters = {};
		trigger.GetEntitiesInside(entitiesInside);
		foreach (IEntity entity : entitiesInside)
		{
			if (entity == m_Asset)
			{
				m_TriggerDeliver.GetOnActivate().Remove(OnDeliveryTriggerActivated);
				m_LayerTask.ProcessLayerTaskState(SCR_ETaskState.COMPLETED);
				return;
			}

			if (SCR_ChimeraCharacter.Cast(entity))
				chimeraCharacters.Insert(SCR_ChimeraCharacter.Cast(entity))
		}
		
		foreach (SCR_ChimeraCharacter character : chimeraCharacters)
		{
			InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(character.FindComponent(InventoryStorageManagerComponent));
			if (!inventoryComponent)
				continue;
			
			if (inventoryComponent.Contains(m_Asset))
			{
				m_TriggerDeliver.GetOnActivate().Remove(OnDeliveryTriggerActivated);
				m_LayerTask.ProcessLayerTaskState(SCR_ETaskState.COMPLETED);
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates task title and description if possessed item matches asset, sets delivery item found flag if not, and progresses state if
	//! \param[in] item Checks if possessed item matches task asset, updates title and description if so, sets state to PROGRESSED if delivery
	//! \param[in] pStorageOwner Represents inventory storage component for item possession tracking.
	void OnObjectPossessed(IEntity item, BaseInventoryStorageComponent pStorageOwner)
	{
		if(!item || item != m_Asset)
			return;
		
		UpdateTaskTitleAndDescription(1);
		if (!m_bDeliveryItemFound)
		{
			m_bDeliveryItemFound = true;
			m_LayerTask.ProcessLayerTaskState(SCR_ETaskState.PROGRESSED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates task state and title/description when an item is dropped into inventory.
	//! \param[in] item Item dropped event, checks if dropped item matches assigned asset, updates task state if match found.
	//! \param[in] pStorageOwner Represents inventory storage component of an entity in the method.
	void OnObjectDropped(IEntity item, BaseInventoryStorageComponent pStorageOwner)
	{
		if(!item || item != m_Asset)
			return;

		m_LayerTask.ProcessLayerTaskState(SCR_ETaskState.PROGRESSED);
		UpdateTaskTitleAndDescription(0);
	}

	//------------------------------------------------------------------------------------------------
	//! Updates task title and description based on object state, triggers delivery, and sets task title and description for the support entity
	//! \param[in] iPossessed iPossessed represents the possession state of the task, either 0 (not possessed) or 1 (possessed
	void UpdateTaskTitleAndDescription(int iPossessed = -1)
	{
		if (iPossessed == -1)
		{
			//the request comes from outside (most likely the Delivery point has been created)
			//check if the item has been already possessed or not
			if (m_iObjectState & 0x01)
				iPossessed = 1;
			else 
				iPossessed = 0;
		}
		
		//Decide based on the task state - see the table on top of the class
		if (!m_TriggerDeliver) 
		{
			//delivery point still doesn't exist
			if (iPossessed == 1)
			{
				m_iObjectState = 1;	
				SetTaskUIVisibility(SCR_ETaskUIVisibility.LIST_ONLY);	
			}
			else
			{
				//dropped
				m_iObjectState = 2;	
				if (!m_bTaskPositionUpdated)
					UpdateDroppedTaskMarker();
			}	
		}
		else
		{
			if (iPossessed == 1)
			{
				m_iObjectState = 5;			
				m_LayerTask.m_TaskSystem.MoveTask(this, m_TriggerDeliver.GetOrigin());
			}
			else
			{
				//dropped
				m_iObjectState = 6;
				if (!m_bTaskPositionUpdated)
					UpdateDroppedTaskMarker();
			}
		}
		
		if (!m_LayerTask)
			return;
								
		SCR_ScenarioFrameworkSlotTask subject = m_LayerTask.GetSlotTask();
		if (!subject)
			return;
		
		if (GetTaskName() != subject.GetTaskTitle(m_iObjectState))
			SetTaskName(subject.GetTaskTitle(m_iObjectState));
		
		if (GetTaskDescription() != subject.GetTaskDescription(m_iObjectState))
			SetTaskDescription(subject.GetTaskDescription(m_iObjectState));
		
		if (m_iObjectState != 1)
			SetTaskUIVisibility(SCR_ETaskUIVisibility.ALL);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates dropped task marker position, checks if it's a deliver task, schedules marker position update if it is.
	protected void UpdateDroppedTaskMarker()
	{
		m_bTaskPositionUpdated = true;
		SCR_ScenarioFrameworkLayerTaskDeliver layerTaskDeliver = SCR_ScenarioFrameworkLayerTaskDeliver.Cast(m_LayerTask);
		if (!layerTaskDeliver)
		{
			Print("ScenarioFramework: Task Deliver failed to access LayerTask", LogLevel.ERROR);
			return;
		}
					
		//We want to delay position movement of the Task marker on the map by given time
		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(MoveTaskMarkerPosition, 1000 * layerTaskDeliver.GetIntelMapMarkerUpdateDelay(), false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates task position, moves marker if asset exists, logs error if asset not assigned.
	protected void MoveTaskMarkerPosition()
	{
		m_bTaskPositionUpdated = false;
		if (m_Asset)
			m_LayerTask.m_TaskSystem.MoveTask(this, m_Asset.GetOrigin());
		else
			Print("ScenarioFramework: Task Deliver does not have m_Asset properly assigned for MoveTaskMarkerPosition", LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles item carrier changes, removes/adds event handlers, and updates garbage collection status.
	//! \param[in] oldSlot Represents the previous inventory storage slot of an item before it was moved or changed.
	//! \param[in] newSlot New slot represents the new location where an item is moved in the inventory system.
	protected void OnItemCarrierChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		EventHandlerManagerComponent eventHandlerMgr;
		InventoryItemComponent inventoryComponent;
		IEntity owner;
		IEntity rootOwner;
		if (oldSlot)
		{
			owner = oldSlot.GetOwner();
			rootOwner = oldSlot.GetOwner().GetRootParent();
			
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
			rootOwner = newSlot.GetOwner().GetRootParent();
			
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
		
		if (!m_Asset)
			return;
		
		InventoryItemComponent invComp = InventoryItemComponent.Cast(m_Asset.FindComponent(InventoryItemComponent));
		if (!invComp)
			return;
		
		InventoryStorageSlot parentSlot = invComp.GetParentSlot();
		if (!parentSlot)
			return;
		
		InventoryStorageManagerComponent inventoryComponent = InventoryStorageManagerComponent.Cast(destroyedEntity.FindComponent(InventoryStorageManagerComponent));
		if (!inventoryComponent)
			return;
		
		if (!inventoryComponent.Contains(m_Asset))
			return;	
		
		inventoryComponent.TryRemoveItemFromStorage(m_Asset, parentSlot.GetStorage());
		
		vector position;
		array<IEntity> excludedEntities = {};
		excludedEntities.Insert(destroyedEntity);
		excludedEntities.Insert(m_Asset);
		
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
		
		SCR_TerrainHelper.SnapToGeometry(position, m_Asset.GetOrigin(), excludedEntities, m_Asset.GetWorld());
		
		m_Asset.SetOrigin(position);
		m_Asset.Update();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] iPlayerID iPlayerID represents the unique identifier for the player being disconnected from the server.
	protected void OnDisconnected(int iPlayerID)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(iPlayerID);
		if (!player)
			return;
			
		OnDestroyed(player);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Registers player with inventory and event handlers.
	//! \param[in] iPlayerID Player ID representing the controlled entity in the game.
	//! \param[in] playerEntity Player entity represents an in-game player controlled by a human player.
	protected void RegisterPlayer(int iPlayerID, IEntity playerEntity)
	{
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(iPlayerID);
		if (!player)
			return;

		SCR_InventoryStorageManagerComponent inventoryComponent = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryComponent)
			return;

		inventoryComponent.m_OnItemAddedInvoker.Insert(OnObjectPossessed);
		inventoryComponent.m_OnItemRemovedInvoker.Insert(OnObjectDropped);
			
		EventHandlerManagerComponent EventHandlerMgr = EventHandlerManagerComponent.Cast(player.FindComponent(EventHandlerManagerComponent));
		if (EventHandlerMgr)
			EventHandlerMgr.RegisterScriptHandler("OnDestroyed", this, OnDestroyed);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Rehooks task asset, sets up damage state change listener, checks engine stop after 5 seconds if vehicle.
	//! \param[in] object to be linked to this task.
	override void HookTaskAsset(IEntity object)
	{
		if (!object)
			return;
		
		super.HookTaskAsset(object);
		
		if (!m_Asset)
			return;
			
		InventoryItemComponent invComp = InventoryItemComponent.Cast(m_Asset.FindComponent(InventoryItemComponent));
		if (!invComp)
			return;
		
		invComp.m_OnParentSlotChangedInvoker.Insert(OnItemCarrierChanged);
			
		array<int> aPlayerIDs = {};
		int iNrOfPlayersConnected = GetGame().GetPlayerManager().GetPlayers(aPlayerIDs); 
					
		foreach (int i : aPlayerIDs)
		{
			RegisterPlayer(i, null);
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		gameMode.GetOnPlayerSpawned().Remove(RegisterPlayer);
		gameMode.GetOnPlayerSpawned().Insert(RegisterPlayer);
		gameMode.GetOnPlayerDisconnected().Remove(OnDisconnected);
		gameMode.GetOnPlayerDisconnected().Insert(OnDisconnected);
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets delivery trigger for an event.
	protected void InvokedSetDeliveryTrigger()
	{
		SetDeliveryTrigger();
	}
}
