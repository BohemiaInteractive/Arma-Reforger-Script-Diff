//------------------------------------------------------------------------------------------------
//! Returns true if character has provided gadget in their inventory
//! Set m_eConsumableType to filter for a specific item
[BaseContainerProps()]
class SCR_CharacterIsUsingItem : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity has specified gadget
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		bool result = data.GetIsCharacterUsingItem();
		return GetReturnResult(result);
	}
};
