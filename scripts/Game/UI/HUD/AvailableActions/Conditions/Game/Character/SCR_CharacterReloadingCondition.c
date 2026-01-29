//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_CharacterReloadingCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		float result = data.GetIsCharacterReloading();

		return GetReturnResult((int)result);
	}
};
