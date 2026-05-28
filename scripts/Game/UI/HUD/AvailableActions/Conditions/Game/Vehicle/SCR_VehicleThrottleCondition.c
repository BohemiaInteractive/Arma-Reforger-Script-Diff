//! Returns true if vehicle's throttle is over specified threshold
[BaseContainerProps()]
class SCR_VehicleThrottleCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: "0.05", uiwidget: UIWidgets.Auto, desc: "Minimum throttle to pass this conditon")]
	protected float m_fMinimumThrottle;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current gear matches the condition
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		CarControllerComponent controller = CarControllerComponent.Cast(data.GetCurrentVehicleController());
		if (!controller)
			return GetReturnResult(false);

		VehicleWheeledSimulation simulation = controller.GetSimulation();
		return GetReturnResult(simulation && simulation.GetThrottle() >= m_fMinimumThrottle);
	}
}
