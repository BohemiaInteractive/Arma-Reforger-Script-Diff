[BaseContainerProps()]
class SCR_WeaponSwitchAvailableCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.IsQuickSlotShown());
	}
}
