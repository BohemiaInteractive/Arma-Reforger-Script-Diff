//------------------------------------------------------------------------------------------------
//! Returns true if current vehicle fuel volume matches the condition
[BaseContainerProps()]
class SCR_VehicleFuelCondition : SCR_AvailableActionCondition
{
	[Attribute("3", UIWidgets.ComboBox, "Cond operator", "", ParamEnumArray.FromEnum(SCR_ComparerOperator) )]
	private SCR_ComparerOperator m_eOperator;

	[Attribute("2", UIWidgets.EditBox, "Fuel volume", "")]
	private float m_fValue;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled vehicle fuel volume matches the condition by operator
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		IEntity vehicle = data.GetCurrentVehicle();
		if (!vehicle)
			return false;

		FuelManagerComponent fuelNode = FuelManagerComponent.Cast(vehicle.FindComponent(FuelManagerComponent));
		if (!fuelNode)
			return true;

		int current = fuelNode.GetTotalFuel();

		bool result = false;

		result = SCR_Comparer<int>.Compare(m_eOperator, current, (int)m_fValue);
		return GetReturnResult(result);
	}
};
