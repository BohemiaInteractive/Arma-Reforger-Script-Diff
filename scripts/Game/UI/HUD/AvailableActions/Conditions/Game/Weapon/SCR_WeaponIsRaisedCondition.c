[BaseContainerProps()]
class SCR_WeaponIsRaisedCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.GetIsCharacterWeaponRaised());
	}
}
