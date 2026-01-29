class SCR_LootDeadBodyAction : SCR_LootAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetOwner());
		if (!char)
			return false;
		
		// Disallow looting when alive
		CharacterControllerComponent contr = char.GetCharacterController();
		if (!contr)
			return false;
		
		if (contr.GetLifeState() == ECharacterLifeState.ALIVE)
			return false;

		// Disallow looting when in vehicle
		IEntity userVeh = CompartmentAccessComponent.GetVehicleIn(char);
		if (userVeh)
			return false;

		return super.CanBePerformedScript(user);
	}
}
