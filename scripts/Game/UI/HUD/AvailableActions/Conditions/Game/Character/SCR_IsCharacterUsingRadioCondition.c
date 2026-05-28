//! Returns true if character is transmiting with radio
[BaseContainerProps()]
class SCR_IsCharacterUsingRadioCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.GetIsCharacterUsingRadio());
	}
}
