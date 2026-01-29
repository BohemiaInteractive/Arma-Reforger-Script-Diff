//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_CharacterCanFireWeaponCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		return GetReturnResult(data.GetCanCharacterFireWeapon());
	}
};
