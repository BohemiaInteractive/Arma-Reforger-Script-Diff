[BaseContainerProps()]
class SCR_WeaponSwitchAvailableCondition : SCR_AvailableActionCondition
{
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.IsQuickSlotShown());
	}
};