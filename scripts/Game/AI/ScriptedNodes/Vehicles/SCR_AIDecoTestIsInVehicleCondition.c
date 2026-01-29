class SCR_AIDecoTestIsInVehicleCondition : DecoratorTestScripted
{
	// this tests SCR_BoardingWaypoint waypoint completion condition: either characters are in vehicles or group usable vehicles are fully occupied
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		SCR_AIBoardingParameters allowance = new SCR_AIBoardingParameters();
		EAIWaypointCompletionType completionType;
		AIWaypoint wp = AIWaypoint.Cast(controlled);
		if (!wp)
			return false;
		completionType = wp.GetCompletionType();
		SCR_BoardingWaypoint bwp = SCR_BoardingWaypoint.Cast(controlled);
		if (bwp)
			allowance = bwp.GetAllowance();
		else
		{
			allowance.m_bIsCargoAllowed = true;
			allowance.m_bIsGunnerAllowed = true;
			allowance.m_bIsDriverAllowed = true;
		}
		
		SCR_AIGroup group = SCR_AIGroup.Cast(agent);
		if (!group)
		{
			Debug.Error("Running on AIAgent that is not a SCR_AIGroup group!");
			return false;
		}
		
		
		
		array<AIAgent> agents = {};
		
		ref array<IEntity> vehicles = {};
		group.GetGroupUtilityComponent().m_VehicleMgr.GetAllVehicleEntities(vehicles);
		bool noAvailableVehicles = true;
		foreach (IEntity vehicle: vehicles)
		{
			if (vehicle && VehicleHasEmptyCompartments(vehicle, allowance))
			{
				noAvailableVehicles = false;
				break;
			}
		};
		
		switch (completionType)
		{
			case EAIWaypointCompletionType.All :
			{
				group.GetAgents(agents);
				bool completeWaypoint = true;
				foreach (AIAgent a: agents)
				{
					ChimeraCharacter character = ChimeraCharacter.Cast(a.GetControlledEntity());
					if (character && !character.IsInVehicle())
					{
						if (vehicles.IsEmpty()) // i have character outside a vehicle, no known vehicles to use -> not complete waypoint
							return false;
						completeWaypoint = noAvailableVehicles; // i have character outside the vehicle, no available vehicles to use --> complete waypoint
						break;
					}
				}
				return completeWaypoint;
			}
			case EAIWaypointCompletionType.Leader :
			{
				ChimeraCharacter character = ChimeraCharacter.Cast(group.GetLeaderEntity());
				if (!character)
					return false;
				if (character.IsInVehicle()) // leader is inside the vehicle --> complete waypoint
					return true;
				if (vehicles.IsEmpty()) // leader is outside, no known vehicles --> not complete waypoint
					return false;
				return noAvailableVehicles;
			}
			case EAIWaypointCompletionType.Any :
			{
				group.GetAgents(agents);
				foreach (AIAgent a: agents)
				{
					ChimeraCharacter character = ChimeraCharacter.Cast(a.GetControlledEntity());
					if (!character) // same logic as leader's
						return false;
					if (character.IsInVehicle())
						return true;
					if (vehicles.IsEmpty())
						return false;
					return noAvailableVehicles;
				}
				return false;
			}
		}
		return false;
	}
	
	// returns true if there is compartment of required type that is yet unoccupied
	bool VehicleHasEmptyCompartments(notnull IEntity vehicle, SCR_AIBoardingParameters allowedCompartmentTypes) 
	{
		BaseCompartmentManagerComponent compComp = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
		if (!compComp)
			return false;
		
		array<BaseCompartmentSlot> compartmentSlots = {};
		compComp.GetCompartments(compartmentSlots);
		ECompartmentType compartmentType;
		foreach (BaseCompartmentSlot slot : compartmentSlots)
		{
			if (allowedCompartmentTypes.m_bIsDriverAllowed && PilotCompartmentSlot.Cast(slot))
				compartmentType = ECompartmentType.PILOT;
			else if (allowedCompartmentTypes.m_bIsGunnerAllowed && TurretCompartmentSlot.Cast(slot))
				compartmentType = ECompartmentType.TURRET;
			else if (allowedCompartmentTypes.m_bIsCargoAllowed && CargoCompartmentSlot.Cast(slot))
				compartmentType = ECompartmentType.CARGO;
			else 
				continue;
			
			if (!slot.IsOccupied())
			{
				// PrintFormat("Found empty compartment %1 of type %2", slot, compartmentType);
				return true;
			}
		}
		
		return false; 
	}
};