//------------------------------------------------------------------------------------------------
//! Returns true if character is in a vehicle
[BaseContainerProps()]
class SCR_CharacterIsGettingInCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity is in vehicle
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		return GetReturnResult(data.GetIsCharacterGettingIn());
	}
};
