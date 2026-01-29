[BaseContainerProps()]
class SCR_VehicleConditionCheck
{
	[Attribute()]
	protected string m_sConditionName;

	[Attribute()]
	protected ref SCR_UIInfo m_FailedConditionUIInfo;

	//------------------------------------------------------------------------------------------------
	//! Checks if provided Vehicle object is not null.
	//! \param[in] vehicle inspected by method to determine if it is valid for further processing.
	//! \return true if vehicle is not null, false otherwise.
	bool CheckCondition(Vehicle vehicle)
	{
		return vehicle != null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_UIInfo GetFailedConditionUIInfo()
	{
		return m_FailedConditionUIInfo;
	}

	//------------------------------------------------------------------------------------------------
	void SetFailedConditionUIInfo(SCR_UIInfo uiInfo)
	{
		m_FailedConditionUIInfo = uiInfo;
	}
}
