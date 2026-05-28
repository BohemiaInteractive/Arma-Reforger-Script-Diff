[BaseContainerProps()]
class SCR_CharacterHasGadgetInHandCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] data
	//! \return true when the currently controlled entity has a gadget in hand, false otherwise
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(data.GetHeldGadget() != null);
	}
}
