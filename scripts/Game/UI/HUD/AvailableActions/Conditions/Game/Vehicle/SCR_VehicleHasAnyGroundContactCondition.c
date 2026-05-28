//------------------------------------------------------------------------------------------------
//! Returns true if vehicle is colliding with ground
[BaseContainerProps()]
class SCR_VehicleHasAnyGroundContactCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		VehicleControllerComponent controller = VehicleControllerComponent.Cast(data.GetCurrentVehicleController());
		if (!controller)
			return GetReturnResult(false);

		VehicleBaseSimulation simulation = controller.GetBaseSimulation();
		return GetReturnResult(simulation && simulation.HasAnyGroundContact());
	}
}
