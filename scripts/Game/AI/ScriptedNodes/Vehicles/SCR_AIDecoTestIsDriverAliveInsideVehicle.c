class SCR_AIDecoTestDriverAliveInsideVehicle : DecoratorTestScripted
{
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		Vehicle vehicle = Vehicle.Cast(controlled);
		if (!vehicle)
			return false;
		IEntity driver = vehicle.GetPilot();
		if (!driver)
			return false;
		ChimeraCharacter character = ChimeraCharacter.Cast(driver);
		if (!character)
			return false;
		CharacterControllerComponent control = character.GetCharacterController();
		if (!control)
			return false;
		return control.GetLifeState() == ECharacterLifeState.ALIVE;
	}
};