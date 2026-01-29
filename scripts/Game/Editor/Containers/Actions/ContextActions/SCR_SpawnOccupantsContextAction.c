//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_SpawnOccupantsContextAction : SCR_SelectedEntitiesContextAction
{	
	[Attribute(desc: "Compartments Types To fill with characters within vehicle.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECompartmentType))]
	protected ref array<ECompartmentType> m_aCompartmentsTypes;
	
	//~ Store vehicles to spawn occupants in
	protected ref map<SCR_BaseCompartmentManagerComponent, SCR_EditableVehicleComponent> m_VehiclesToOccupyQueue = new map<SCR_BaseCompartmentManagerComponent, SCR_EditableVehicleComponent>();
	
	protected bool m_bHasSendNotEnoughBudgetNotification = false;
	protected bool m_bIsSpawningOccupants = false;
	
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{		
		//~ Disabled until the system is reworked. HasEnoughBudgetForDefaultOccupants should be called here or at least in the budget component and the player Id needs to be send over to get the contentbrowser!
		return false;
		
		SCR_EditableVehicleUIInfo uiInfo = SCR_EditableVehicleUIInfo.Cast(selectedEntity.GetInfo());
		if (!uiInfo || (uiInfo && !uiInfo.CanFillWithGivenTypes(m_aCompartmentsTypes)))
			return false;
		
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(selectedEntity.GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));

		return compartmentManager && compartmentManager.HasDefaultOccupantsDataForTypes(m_aCompartmentsTypes);
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		//~ Disabled until the system is reworked. HasEnoughBudgetForDefaultOccupants should be called here or at least in the budget component and the player Id needs to be send over to get the contentbrowser!
		return false;
		
		SCR_EditableVehicleComponent vehicle = SCR_EditableVehicleComponent.Cast(selectedEntity);
		return vehicle && vehicle.CanOccupyVehicleWithCharacters(m_aCompartmentsTypes, -1, false, true, true, true, true);
	}
	
	//~ Makes sure CanBePerformed is not executed twice
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		if (!InitPerform()) return;
		
		foreach	(SCR_EditableEntityComponent entity : selectedEntities)
			Perform(entity, cursorWorldPosition);
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{		
		//~ Disabled until the system is reworked. HasEnoughBudgetForDefaultOccupants should be called here or at least in the budget component and the player Id needs to be send over to get the contentbrowser!
		return;
		
		SCR_EditableVehicleComponent vehicle = SCR_EditableVehicleComponent.Cast(selectedEntity);
		if (!vehicle)
			return;
	
		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(vehicle.GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return;
		
		//~ Already being spawned
		if (m_VehiclesToOccupyQueue.Contains(compartmentManager))
			return;
		
		//~ Add to spawn que
		m_VehiclesToOccupyQueue.Insert(compartmentManager, vehicle);
		
		//~ If not yet spawning start spawn logic
		if (!m_bIsSpawningOccupants)
		{
			m_bIsSpawningOccupants = true;
			
			//~ Start spawning by calling spawn next
			SpawnNextOccupantsInEntity();
		}
	}
	
	//~ When done spawning characters in entity
	protected void SpawnNextOccupantsInEntity(SCR_BaseCompartmentManagerComponent compartmentManager = null, array<IEntity> spawnedCharacters = null, bool wasCanceled = false)
	{
		//~ Remove listener
		if (compartmentManager)
		{
			compartmentManager.GetOnDoneSpawningDefaultOccupants().Remove(SpawnNextOccupantsInEntity);
			m_VehiclesToOccupyQueue.Remove(compartmentManager);
		}

		//~ Nothing more to spawn so set spawning done
		if (m_VehiclesToOccupyQueue.IsEmpty())
		{
			SetSpawningDone();
			return;
		}
			
		SCR_EditableVehicleComponent vehicleToOccupy = m_VehiclesToOccupyQueue.GetElement(0); 
		
		//~ Entry is empty meaning that it was deleted so call the next in entry
		if (!vehicleToOccupy)
		{
			m_VehiclesToOccupyQueue.RemoveElement(0);
			SpawnNextOccupantsInEntity(null, null, false);
			return;
		}
		
		//~ Grab the new compartment
		compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(vehicleToOccupy.GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		
		//~ Check if compartment exists (which should always be the cause). But try spawn next if it does not
		if (!compartmentManager)
		{
			m_VehiclesToOccupyQueue.RemoveElement(0);
			SpawnNextOccupantsInEntity(null, null, false);
			return;
		}
		
		//~ No longer valid to spawn
		if (!vehicleToOccupy.CanOccupyVehicleWithCharacters(m_aCompartmentsTypes, -1, true, checkEditorBudget: false, checkForFreeDefaultCompartments: true))
		{
			m_VehiclesToOccupyQueue.RemoveElement(0);
			SpawnNextOccupantsInEntity(null, null, false);
			return;
		}
		
		//~ No longer valid budget
		if (!vehicleToOccupy.CanOccupyVehicleWithCharacters(m_aCompartmentsTypes, -1, false, true, false, false, true))
		{
			//~ Todo get player ID to send notification
			/*if (!m_bHasSendNotEnoughBudgetNotification)
			{
				m_bHasSendNotEnoughBudgetNotification = true;
				SCR_NotificationsComponent.SendToPlayer(playerID, ENotification.EDITOR_PLACING_BUDGET_MAX_FOR_VEHICLE_OCCUPANTS);
			}*/
			
			m_VehiclesToOccupyQueue.RemoveElement(0);
			SpawnNextOccupantsInEntity(null, null, false);
			return;
		}
		
		//~ Fill next vehicle in queue
		compartmentManager.GetOnDoneSpawningDefaultOccupants().Insert(SpawnNextOccupantsInEntity);
		vehicleToOccupy.OccupyVehicleWithDefaultCharacters(m_aCompartmentsTypes);
	}
	
	protected void SetSpawningDone()
	{
		m_VehiclesToOccupyQueue.Clear();
		m_bIsSpawningOccupants = false;
		m_bHasSendNotEnoughBudgetNotification = false;
	}
};