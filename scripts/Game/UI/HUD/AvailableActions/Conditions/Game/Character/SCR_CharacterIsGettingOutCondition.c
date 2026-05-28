//! Returns true if character is in a vehicle
[BaseContainerProps()]
class SCR_CharacterIsGettingOutCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity is in vehicle
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.GetIsCharacterGettingOut());
	}
}
