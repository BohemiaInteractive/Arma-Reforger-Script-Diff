[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetEntityByNameFromVariable : SCR_ScenarioFrameworkGet
{
	[Attribute()]
	string 		m_sVariableName;

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return null;
		
		string value;
		if (!scenarioFrameworkSystem.GetVariable(m_sVariableName, value))
			return null;
				
		return new SCR_ScenarioFrameworkParam<IEntity>(FindEntityByName(value));
	}
}