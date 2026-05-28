[BaseContainerProps()]
class SCR_CharacterHasItemInHandCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity has any item in hand
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.GetCurrentItemEntity() || data.GetCurrentWeaponEntity());
	}
}
