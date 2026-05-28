//! Returns true if current vehicle engine is on
[BaseContainerProps()]
class SCR_VehicleEngineOnCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		VehicleControllerComponent controller = VehicleControllerComponent.Cast(data.GetCurrentVehicleController());
		return GetReturnResult(controller && controller.IsEngineOn());
	}
}
