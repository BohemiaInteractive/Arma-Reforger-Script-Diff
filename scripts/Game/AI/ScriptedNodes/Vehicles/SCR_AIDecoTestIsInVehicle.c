class SCR_AIDecoTestIsInVehicle : DecoratorTestScripted
{
	// this tests if Entity is in vehicle (to be added to other tests - for sole node use "DecoIsInVehicle" node)
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		if (!controlled)
			controlled = agent.GetControlledEntity();
		ChimeraCharacter character = ChimeraCharacter.Cast(controlled);
		if (!character)
			return false;
		return character.IsInVehicle();			
	}	
};
