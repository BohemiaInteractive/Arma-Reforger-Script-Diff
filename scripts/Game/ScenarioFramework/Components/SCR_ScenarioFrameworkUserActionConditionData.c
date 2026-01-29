[BaseContainerProps()]
class SCR_ScenarioFrameworkUserActionConditionData
{
	[Attribute(desc: "Name of the variable")]
	string m_sVariableName;

	[Attribute(desc: "Value of the variable")]
	string m_sVariableValue;

	//------------------------------------------------------------------------------------------------
	string GetVariableName()
	{
		return m_sVariableName;
	}

	//------------------------------------------------------------------------------------------------
	string GetVariableValue()
	{
		return m_sVariableValue;
	}
}
