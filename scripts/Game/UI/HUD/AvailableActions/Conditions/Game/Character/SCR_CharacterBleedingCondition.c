[BaseContainerProps()]
class SCR_CharacterBleedingCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true if character is bleeding for certain time
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.GetIsCharacterBleeding());
	}
}
