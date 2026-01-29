class SCR_AIDecoTestIsOutOfVehicle : DecoratorTestScripted
{
	// this test either test group / character out of (any) vehicle or group / character out of specific vehicle given in controlled
	// if controlled provided -> it returns TRUE if somebody is inside a different vehicle than the controlled!	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		bool onlySingleVehicleTest = controlled != null;
		ChimeraCharacter character;
		CompartmentAccessComponent compAccComp;
		
		AIGroup gr = AIGroup.Cast(agent);
		if (gr)
		{
			array<AIAgent> agents = new array<AIAgent>();
			gr.GetAgents(agents);
			
			foreach (AIAgent ag : agents)
			{
				character = ChimeraCharacter.Cast(ag.GetControlledEntity());
				compAccComp = character.GetCompartmentAccessComponent();
				if (!character || !compAccComp)
					continue;
				if (character.IsInVehicle() && onlySingleVehicleTest && compAccComp.GetVehicleIn(character) == controlled)
					return false;
				else if (character.IsInVehicle() && !onlySingleVehicleTest)
					return false;
			}
			return true;
		}
		// agent is not group, we must be provided with vehicle we check
		character = ChimeraCharacter.Cast(agent.GetControlledEntity());
		compAccComp = character.GetCompartmentAccessComponent();
		if (!character || !compAccComp)
			return false;
		
		if (onlySingleVehicleTest && compAccComp.GetVehicleIn(character) != controlled) // inside different vehicle than tested against
			return true;
		else if (!onlySingleVehicleTest)
			return character.IsInVehicle();
		return false;
	}
};