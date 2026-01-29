[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkSetVariable : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Name of the variable")]
	string m_sVariableName;

	[Attribute(desc: "Value of the variable")]
	string m_sVariableValue;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;	
		
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;
		
		scenarioFrameworkSystem.SetVariableValue(m_sVariableName, m_sVariableValue);
	}
}