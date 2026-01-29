[BaseContainerProps()]
class SCR_CharacterHasItemInHandCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity has any item in hand
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		return GetReturnResult(data.GetCurrentItemEntity() || data.GetCurrentWeaponEntity());
	}
}
