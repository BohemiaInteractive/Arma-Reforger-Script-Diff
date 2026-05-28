[BaseContainerProps()]
class SCR_GadgetSelectionCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.IsGadgetSelection());
	}
}