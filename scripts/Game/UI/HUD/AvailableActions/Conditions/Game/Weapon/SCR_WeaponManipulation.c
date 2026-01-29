[BaseContainerProps()]
class SCR_WeaponManipulationCondition : SCR_AvailableActionCondition
{
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.IsWeaponManipulation());
	}
};