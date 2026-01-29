[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetVariableValue : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Name of the variable")]
	string m_sVariableName;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;
		
		string value;
		
		scenarioFrameworkSystem.GetVariable(m_sVariableName, value);
	}
}