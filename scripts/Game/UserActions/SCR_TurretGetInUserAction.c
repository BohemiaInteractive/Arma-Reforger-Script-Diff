class SCR_TurretGetInUserAction : SCR_GetInUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!IsTurretOperational())
			return false;

		return super.CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!IsTurretOperational())
			return false;

		return super.CanBeShownScript(user);
	}

	//------------------------------------------------------------------------------------------------
	bool IsTurretOperational()
	{
		// Check if destroyed
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(GetOwner().FindComponent(DamageManagerComponent));
		if (damageManager && damageManager.GetState() == EDamageState.DESTROYED)
			return false;

		// Check WeaponSlot
		WeaponSlotComponent weaponSlot = WeaponSlotComponent.Cast(GetOwner().FindComponent(WeaponSlotComponent));
		if (!weaponSlot)
			return false;
		
		if (!weaponSlot.GetWeaponEntity())
			return false;

		return true;
	}
};