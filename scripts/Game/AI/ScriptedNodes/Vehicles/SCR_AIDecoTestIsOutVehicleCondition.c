class SCR_AIDecoTestIsOutVehicleCondition : DecoratorTestScripted
{
	// this tests AIWaypoint waypoint completion condition: either characters are out of vehicles 
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		SCR_AIBoardingParameters allowance;
		EAIWaypointCompletionType completionType;
		AIWaypoint wp = AIWaypoint.Cast(controlled);
		if (!wp)
			return false;
		completionType = wp.GetCompletionType();
		SCR_BoardingWaypoint bwp = SCR_BoardingWaypoint.Cast(controlled);
		if (bwp)
			allowance = bwp.GetAllowance();
		
		SCR_AIGroup group = SCR_AIGroup.Cast(agent);
		if (!group)
		{
			Debug.Error("Running on AIAgent that is not a SCR_AIGroup group!");
			return false;
		}
		
		array<AIAgent> agents = {};
		
		switch (completionType)
		{
			case EAIWaypointCompletionType.All :
			{
				group.GetAgents(agents);
				foreach (AIAgent a: agents)
				{
					ChimeraCharacter character = ChimeraCharacter.Cast(a.GetControlledEntity());
					if (!character)
						continue;
					CompartmentAccessComponent acc = character.GetCompartmentAccessComponent();
					if (!acc)
						continue;
					BaseCompartmentSlot slot = acc.GetCompartment();
					if (slot && IsCompartmentAllowedToLeave(slot, allowance))
						return false;
				}
				return true;
			}
			case EAIWaypointCompletionType.Leader :
			{
				ChimeraCharacter character = ChimeraCharacter.Cast(group.GetLeaderEntity());
				if (!character)
					return false;
				CompartmentAccessComponent acc = character.GetCompartmentAccessComponent();
				if (!acc)
					return false;
				BaseCompartmentSlot slot = acc.GetCompartment();
				if (!slot)
					return true;
				return !IsCompartmentAllowedToLeave(slot, allowance); // leader is not allowed to leave compartment --> condition is true
			}
			case EAIWaypointCompletionType.Any :
			{
				group.GetAgents(agents);
				foreach (AIAgent a: agents)
				{
					ChimeraCharacter character = ChimeraCharacter.Cast(a.GetControlledEntity());
					if (!character) 
						continue;
					if (!character.IsInVehicle())
						return true;
				}
				return false;
			}
		}
		return false;
	}	
	
	//---------------------------------------------------------------------------------------------------
	protected bool IsCompartmentAllowedToLeave(notnull BaseCompartmentSlot slot, SCR_AIBoardingParameters allowance)
	{
		if(!allowance)
			return true;
		else if (PilotCompartmentSlot.Cast(slot) && allowance.m_bIsDriverAllowed)
			return true;
		else if (TurretCompartmentSlot.Cast(slot) && allowance.m_bIsGunnerAllowed)
			return true;
		else if (CargoCompartmentSlot.Cast(slot) && allowance.m_bIsCargoAllowed)
			return true;
		return false;
	}
};