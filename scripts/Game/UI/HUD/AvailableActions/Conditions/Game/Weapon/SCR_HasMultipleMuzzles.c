//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_HasMultipleMuzzles : SCR_AvailableActionCondition
{
	ref array<BaseMuzzleComponent> muzzles = {};

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		BaseWeaponComponent weapon = data.GetCurrentWeapon();
		if (!weapon)
			return false;

		int count = weapon.GetMuzzlesList(muzzles);
		return GetReturnResult(count > 1);
	}
};
