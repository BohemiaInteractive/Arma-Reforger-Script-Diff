[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGet
{
	SCR_ScenarioFrameworkParamBase m_Value;

	//------------------------------------------------------------------------------------------------
	//! \param[in] value
	void SetValue(SCR_ScenarioFrameworkParamBase value)
	{
		m_Value = value;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ScenarioFrameworkParamBase Get()
	{
		return m_Value;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] name
	//! \return
	IEntity FindEntityByName(string name)
	{
		IEntity entity = GetGame().GetWorld().FindEntityByName(name);
		if (!entity)
			return null;

		return entity;
	}
}