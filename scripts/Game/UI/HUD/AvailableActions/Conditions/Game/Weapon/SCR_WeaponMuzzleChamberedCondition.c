//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_WeaponMuzzleChamberedCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Return true if current weapon has bullet in chamber
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		BaseMuzzleComponent muzzle = data.GetCurrentMuzzle();
		if (!muzzle)
			return false;

		bool result = muzzle.IsCurrentBarrelChambered();

		return GetReturnResult(result);
	}
};
