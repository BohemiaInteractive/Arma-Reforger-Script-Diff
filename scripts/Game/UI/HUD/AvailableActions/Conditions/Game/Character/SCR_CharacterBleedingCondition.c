[BaseContainerProps()]
class SCR_CharacterBleedingCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true if character is bleeding for certain time
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		return GetReturnResult(data.GetIsCharacterBleeding());
	}
}
