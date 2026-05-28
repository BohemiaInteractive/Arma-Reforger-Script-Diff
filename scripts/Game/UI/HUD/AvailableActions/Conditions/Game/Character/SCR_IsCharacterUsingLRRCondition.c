//! Returns true if character is transmitting over long range radio 
[BaseContainerProps()]
class SCR_IsCharacterUsingLRRCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.m_bIsRadioLongRange);
	}
}