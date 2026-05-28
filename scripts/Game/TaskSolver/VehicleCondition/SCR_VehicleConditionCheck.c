[EnumLinear()]
enum SCR_EVehicleConditionCheckType
{
	MISSING_VEHICLE,
	INOPERABLE_VEHICLE,
	VEHICLE_PILOT_TAKEN
}

[BaseContainerProps()]
class SCR_VehicleConditionCheck
{
	protected SCR_EVehicleConditionCheckType m_eConditionType = SCR_EVehicleConditionCheckType.MISSING_VEHICLE;

	//------------------------------------------------------------------------------------------------
	SCR_EVehicleConditionCheckType GetConditionType()
	{
		return m_eConditionType;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if provided Vehicle object is not null.
	//! \param[in] vehicle inspected by method to determine if it is valid for further processing.
	//! \return true if vehicle is not null, false otherwise.
	bool CheckCondition(Vehicle vehicle)
	{
		return vehicle != null;
	}
}
