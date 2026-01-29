[BaseContainerProps()]
class SCR_DetonatorHasConnectedChargesCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when currently controlled entity holds in his hand the detonator which has connected charges
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return GetReturnResult(false);

		SCR_GadgetComponent gadgetComp = data.GetHeldGadgetComponent();
		if (!gadgetComp)
			return GetReturnResult(false);

		if (gadgetComp.GetType() != EGadgetType.DETONATOR)
			return GetReturnResult(false);

		SCR_DetonatorGadgetComponent detonatorComp = SCR_DetonatorGadgetComponent.Cast(gadgetComp);
		if (!detonatorComp)
			return GetReturnResult(false);

		return GetReturnResult(detonatorComp.GetNumberOfConnectedCharges() > 0);
	}
}
