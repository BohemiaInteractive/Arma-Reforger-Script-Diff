[BaseContainerProps()]
class SCR_WeaponMuzzleChamberedCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Return true if current weapon has bullet in chamber
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		BaseMuzzleComponent muzzle = data.GetCurrentMuzzle();
		if (!muzzle)
			return false;

		return GetReturnResult(muzzle.IsCurrentBarrelChambered());
	}
}
