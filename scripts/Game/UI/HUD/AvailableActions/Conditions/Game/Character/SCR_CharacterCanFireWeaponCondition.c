[BaseContainerProps()]
class SCR_CharacterCanFireWeaponCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.GetCanCharacterFireWeapon());
	}
}
