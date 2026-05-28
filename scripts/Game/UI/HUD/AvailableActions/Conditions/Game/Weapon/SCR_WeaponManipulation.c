[BaseContainerProps()]
class SCR_WeaponManipulationCondition : SCR_AvailableActionCondition
{
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.IsWeaponManipulation());
	}
}