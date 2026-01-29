//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_WeaponSightHasZeroingCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Return true when current zeroing on weapon is not 0
	//! That means that weapon has no zeroing, cause weapon with zeroing has zeroing alway > 0
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		BaseWeaponComponent currentweapon = data.GetCurrentWeapon();
		if (!currentweapon)
			return false;

		float zeroing = currentweapon.GetCurrentSightsZeroing();
		return GetReturnResult(zeroing > 0);
	}
};
