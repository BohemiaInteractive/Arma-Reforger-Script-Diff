[BaseContainerProps()]
class SCR_VehicleConditionCheckUIEntry : SCR_ConditionCheckUIEntry
{
	[Attribute(SCR_EVehicleConditionCheckType.MISSING_VEHICLE.ToString(), UIWidgets.ComboBox, enumType: SCR_EVehicleConditionCheckType)]
	protected SCR_EVehicleConditionCheckType m_eVehicleConditionType;

	//------------------------------------------------------------------------------------------------
	void SCR_VehicleConditionCheckUIEntry()
	{
		m_iConditionType = m_eVehicleConditionType;
	}
}
