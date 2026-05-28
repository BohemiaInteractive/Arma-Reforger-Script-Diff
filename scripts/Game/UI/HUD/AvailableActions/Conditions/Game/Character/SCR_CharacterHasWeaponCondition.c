//! Returns true if character has weapon equipped
[BaseContainerProps()]
class SCR_CharacterHasWeaponCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity has a weapon equipped
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.GetCurrentWeaponEntity() != null);
	}
}
