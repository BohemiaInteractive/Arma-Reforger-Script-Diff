class SCR_ScenarioFrameworkParam<Class T> : SCR_ScenarioFrameworkParamBase
{
	T m_Value;

	//------------------------------------------------------------------------------------------------
	//! \return
	T GetValue()
	{
		return m_Value;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] value
	void SCR_ScenarioFrameworkParam(T value)
	{
		m_Value = value;
	}
}