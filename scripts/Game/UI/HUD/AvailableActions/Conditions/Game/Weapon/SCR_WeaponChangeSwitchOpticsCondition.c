[BaseContainerProps()]
class SCR_WeaponChangeSwitchOpticsCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Return true if currently held weapon has more than 1 scopes
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		// Check turret sights first. 
		if (data.GetCurrentTurretSightCount() > 1)
			return GetReturnResult(true);

		BaseWeaponComponent currentWeapon = data.GetCurrentWeapon();
		if (!currentWeapon)
			return GetReturnResult(false);

		int availableSights = currentWeapon.FindAvailableSights();
		
		if (availableSights == 0 || availableSights == 1)
			return GetReturnResult(currentWeapon.CanSetSights(1 - availableSights)); 

		return GetReturnResult(false);
	}
}
