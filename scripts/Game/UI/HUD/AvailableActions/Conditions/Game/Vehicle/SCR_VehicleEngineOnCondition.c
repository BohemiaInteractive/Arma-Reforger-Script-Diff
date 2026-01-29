//------------------------------------------------------------------------------------------------
//! Returns true if current vehicle engine is on
[BaseContainerProps()]
class SCR_VehicleEngineOnCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		VehicleControllerComponent controller = VehicleControllerComponent.Cast(data.GetCurrentVehicleController());
		if (!controller)
			return false;

		bool result = controller.IsEngineOn();
		return GetReturnResult(result);
	}
};
