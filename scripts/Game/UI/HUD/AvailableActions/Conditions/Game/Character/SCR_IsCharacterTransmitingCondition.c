//------------------------------------------------------------------------------------------------
//! Returns true if character is transmiting with radio
[BaseContainerProps()]
class SCR_IsCharacterUsingRadioCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		return GetReturnResult(data.GetIsCharacterUsingRadio());
	}
};
