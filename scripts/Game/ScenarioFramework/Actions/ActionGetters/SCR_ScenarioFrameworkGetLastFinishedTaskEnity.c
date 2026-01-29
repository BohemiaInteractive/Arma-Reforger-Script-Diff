[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetLastFinishedTaskEnity : SCR_ScenarioFrameworkGet
{
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(scenarioFrameworkSystem.GetLastFinishedTask());
	}
}