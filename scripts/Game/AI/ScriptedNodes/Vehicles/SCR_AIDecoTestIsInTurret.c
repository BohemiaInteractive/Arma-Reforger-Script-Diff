class SCR_AIDecoTestIsInTurret : DecoratorTestScripted
{
	// this tests if Entity is in turret
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		if (!controlled)
			controlled = agent.GetControlledEntity();
		ChimeraCharacter character = ChimeraCharacter.Cast(controlled);
		if (!character)
			return false;
		SCR_CompartmentAccessComponent compAcc = SCR_CompartmentAccessComponent.Cast(character.GetCompartmentAccessComponent());
		if (!compAcc)
			return false;
		if (!compAcc.IsInCompartment())
			return false;
		return TurretCompartmentSlot.Cast(compAcc.GetCompartment());
	}	
};