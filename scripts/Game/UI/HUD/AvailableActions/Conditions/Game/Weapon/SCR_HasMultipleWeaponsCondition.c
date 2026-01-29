[BaseContainerProps()]
class SCR_HasMultipleWeaponsCondition : SCR_AvailableActionCondition
{
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return GetReturnResult(false);

		BaseWeaponManagerComponent weaponMgr;
		if (data.GetIsCharacterInVehicle())
		{
			TurretControllerComponent turretController = data.GetCurrentTurretController();
			if (!turretController)
				return GetReturnResult(false);

			weaponMgr = turretController.GetWeaponManager();
		}
		else
		{
			CharacterControllerComponent controller = data.GetCharacterController();
			if (!controller)
				return GetReturnResult(false);

			weaponMgr = controller.GetWeaponManagerComponent();
		}

		return GetReturnResult(weaponMgr && weaponMgr.HasAnotherSelectableWeapon());
	}
}