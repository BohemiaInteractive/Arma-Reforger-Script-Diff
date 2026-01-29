[BaseContainerProps()]
class SCR_CharacterHasGadgetInHandCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] data
	//! \return true when the currently controlled entity has a gadget in hand, false otherwise
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		if (data.GetHeldGadget())
			return GetReturnResult(true);

		return GetReturnResult(false);
	}
}
