//------------------------------------------------------------------------------------------------
//! Returns true if character is swimming
[BaseContainerProps()]
class SCR_CharacterIsSwimmingCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity is swimming
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		return GetReturnResult(data.GetIsCharacterSwimming());
	}
};
