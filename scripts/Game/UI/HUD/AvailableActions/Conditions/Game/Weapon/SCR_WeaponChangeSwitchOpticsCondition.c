[BaseContainerProps()]
class SCR_WeaponChangeSwitchOpticsCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Return true if currently held weapon has more than 1 scopes
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
	
		IEntity controlledEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!controlledEntity)
			return null;

		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		if (!character)
			return null;

		BaseWeaponManagerComponent weaponManager = character.GetCharacterController().GetWeaponManagerComponent();
		if (!weaponManager)
			return null;

		BaseWeaponComponent currentSights = weaponManager.GetCurrentWeapon();
		if (!currentSights)
			return null;

		bool canSwitchOptics;
		
		if (currentSights.FindAvailableSights() == 0)
			canSwitchOptics = currentSights.CanSetSights(1);
		else if(currentSights.FindAvailableSights() == 1)
			canSwitchOptics = currentSights.CanSetSights(0);

		return canSwitchOptics;
	}
};
