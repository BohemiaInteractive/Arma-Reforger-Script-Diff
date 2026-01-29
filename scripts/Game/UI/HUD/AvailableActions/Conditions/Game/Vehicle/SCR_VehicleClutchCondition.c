//------------------------------------------------------------------------------------------------
//! Returns true if vehicle's clutch is at least at specified threshold
[BaseContainerProps()]
class SCR_VehicleClutchCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: "0.05", uiwidget: UIWidgets.Auto, desc: "Minimum clutch to pass this conditon")]
	protected float m_fMinimumClutch;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current gear matches the condition
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		CarControllerComponent controller = CarControllerComponent.Cast(data.GetCurrentVehicleController());
		if (!controller)
			return false;

		VehicleWheeledSimulation simulation = controller.GetSimulation();
		if (!simulation)
			return false;

		bool result = simulation.GetClutch() >= m_fMinimumClutch;

		return GetReturnResult(result);
	}
};
