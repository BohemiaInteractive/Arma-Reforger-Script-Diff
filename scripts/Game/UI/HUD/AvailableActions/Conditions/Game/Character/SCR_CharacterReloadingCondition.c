[BaseContainerProps()]
class SCR_CharacterReloadingCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.GetIsCharacterReloading());
	}
}
