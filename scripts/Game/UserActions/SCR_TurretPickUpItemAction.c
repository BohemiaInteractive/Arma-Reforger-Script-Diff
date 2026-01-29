class SCR_TurretPickUpItemAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if(!IsTurretDisassembled())
			return false;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool IsTurretDisassembled()
	{
		/*
		GenericEntity genOwner = GenericEntity.Cast(m_Owner);

		// Check if turret has an occupant
		auto controller =TurretControllerComponent.Cast(genOwner.FindComponent(TurretControllerComponent));
		if(!controller)
			return false;
		auto compartment = controller.GetCompartmentSlot();
		if(compartment && compartment.GetOccupant())
			return false;

		// Check if destroyed
		auto damageManagerComponent = DamageManagerComponent.Cast(genOwner.FindComponent(DamageManagerComponent));
		if (damageManagerComponent && damageManagerComponent.GetState() != EDamageState.ALIVE)
			return false;

		// Check WeaponSlot
		auto genericWeaponSlotComponent = genOwner.FindComponent(WeaponSlotComponent);
		if (!genericWeaponSlotComponent)
			return false;
		auto weaponSlotComponent = WeaponSlotComponent.Cast(genericWeaponSlotComponent);
		IEntity weaponEntityInSlot = weaponSlotComponent.GetWeaponEntity();
		if(weaponEntityInSlot)
			return false;
		*/
		return true;
	}
};