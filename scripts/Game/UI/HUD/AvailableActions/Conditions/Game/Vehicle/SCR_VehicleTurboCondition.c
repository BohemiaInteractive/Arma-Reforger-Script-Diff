[BaseContainerProps()]
class SCR_VehicleTurboCondition : SCR_AvailableActionCondition
{
	[Attribute("0", UIWidgets.EditBox, "Checks if turbo was toggled rather than holding turbo button")]
	protected bool m_bCheckPersistentState;

	//------------------------------------------------------------------------------------------------
	//! Returns true if driver is using full throttle feature.
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		CarControllerComponent carController = CarControllerComponent.Cast(data.GetCurrentVehicleController());
		bool result = carController && carController.IsThrottleTurbo();
		result = result && (!m_bCheckPersistentState && data.GetCurrentVehicleTurboTime() != 0 || m_bCheckPersistentState && data.GetCurrentVehicleTurboTime() == 0);

		return GetReturnResult(result);
	}
}
