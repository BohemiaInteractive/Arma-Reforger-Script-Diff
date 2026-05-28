//! Returns true if current vehicle fuel volume matches the condition
[BaseContainerProps()]
class SCR_VehicleFuelCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: SCR_ComparerOperator.GREATER_THAN.ToString(), desc: "Condition operator", uiwidget: UIWidgets.ComboBox, enumType: SCR_ComparerOperator)]
	private SCR_ComparerOperator m_eOperator;

	[Attribute("2", UIWidgets.EditBox, "Fuel volume", "")]
	private float m_fValue;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled vehicle fuel volume matches the condition by operator
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		IEntity vehicle = data.GetCurrentVehicle();
		if (!vehicle)
			return false;

		FuelManagerComponent fuelNode = FuelManagerComponent.Cast(vehicle.FindComponent(FuelManagerComponent));
		return GetReturnResult(fuelNode && SCR_Comparer<int>.Compare(m_eOperator, fuelNode.GetTotalFuel(), (int)m_fValue));
	}
}
