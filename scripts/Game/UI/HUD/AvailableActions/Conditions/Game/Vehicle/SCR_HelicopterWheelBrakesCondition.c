//------------------------------------------------------------------------------------------------
//! Returns true if persistent wheel brakes are available and are enabled
[BaseContainerProps()]
class SCR_HelicopterWheelBrakesCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		HelicopterControllerComponent controller = HelicopterControllerComponent.Cast(data.GetCurrentVehicleController());
		if (!controller)
			return false;

		VehicleHelicopterSimulation simulation = VehicleHelicopterSimulation.Cast(controller.GetBaseSimulation());
		if (!simulation)
			return false;

		if (simulation.LandingGearGetType() != LandingGearType.WHEEL)
			return false;

		return GetReturnResult(controller.GetPersistentWheelBrake());
	}
}
