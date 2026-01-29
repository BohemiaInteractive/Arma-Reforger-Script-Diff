class SCR_AIDecoTestIsEnteringOrLeavingVehicle : DecoratorTestScripted
{	
	override protected bool TestFunction(AIAgent agent, IEntity controlled)
	{
		ChimeraCharacter chChar = ChimeraCharacter.Cast(agent.GetControlledEntity());
		if (!chChar)
			return false;
		CompartmentAccessComponent compAcc = chChar.GetCompartmentAccessComponent();
		if (!compAcc)
			return false;
		return compAcc.IsGettingIn() || compAcc.IsGettingOut();
	}
};