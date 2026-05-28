[BaseContainerProps()]
class SCR_WeaponSightHasZeroingCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Return true when current zeroing on weapon is not 0
	//! That means that weapon has no zeroing, cause weapon with zeroing has zeroing alway > 0
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		BaseWeaponComponent currentweapon = data.GetCurrentWeapon();
		return GetReturnResult(currentweapon && currentweapon.GetCurrentSightsZeroing() > 0);
	}
}
