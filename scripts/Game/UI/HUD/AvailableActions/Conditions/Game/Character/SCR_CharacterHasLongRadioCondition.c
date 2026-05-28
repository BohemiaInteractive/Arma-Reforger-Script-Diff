//! Returns true if character is transmiting with radio
[BaseContainerProps()]
class SCR_CharacterHasLongRadioCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.m_bOwnsLongRangeRadio);
	}
}
