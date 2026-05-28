//! Returns true if character is in a vehicle
[BaseContainerProps()]
class SCR_VehiclePersistentBrakeEngagedCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity is in vehicle
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		CarControllerComponent carController = CarControllerComponent.Cast(data.GetCurrentVehicleController());
		return GetReturnResult(carController && carController.GetPersistentHandBrake());
	}
}
