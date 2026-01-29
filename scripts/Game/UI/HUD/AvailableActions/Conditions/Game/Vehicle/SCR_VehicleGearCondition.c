enum EVehicleGearboxGear
{
	REVERSE,
	NEUTRAL,
	FORWARD,
	FIRST,
	LAST
};
//------------------------------------------------------------------------------------------------
//! Returns true if character is in a vehicle
[BaseContainerProps()]
class SCR_VehicleGearCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: SCR_Enum.GetDefault(EVehicleGearboxGear.FORWARD), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EVehicleGearboxGear))]
	protected EVehicleGearboxGear m_eVehicleGearboxGear;
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

		int currentGear = simulation.GetGear();
		int gearsCount = simulation.GearboxGearsCount();
		int neutralGear = gearsCount - simulation.GearboxForwardGearsCount() - 1;

		bool result;
		if (m_eVehicleGearboxGear == EVehicleGearboxGear.REVERSE)
			result = currentGear < neutralGear;
		else if (m_eVehicleGearboxGear == EVehicleGearboxGear.NEUTRAL)
			result = currentGear == neutralGear;
		else if (m_eVehicleGearboxGear == EVehicleGearboxGear.FORWARD)
			result = currentGear > neutralGear;
		else if (m_eVehicleGearboxGear == EVehicleGearboxGear.FIRST)
			result = currentGear == neutralGear + 1;
		else if (m_eVehicleGearboxGear == EVehicleGearboxGear.LAST)
			result = currentGear == gearsCount - 1;

		return GetReturnResult(result);
	}
};
