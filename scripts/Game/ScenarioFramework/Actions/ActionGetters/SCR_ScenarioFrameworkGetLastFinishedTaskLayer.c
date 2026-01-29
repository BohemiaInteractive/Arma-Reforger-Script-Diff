[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetLastFinishedTaskLayer : SCR_ScenarioFrameworkGet
{
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return null;

		return new SCR_ScenarioFrameworkParam<SCR_ScenarioFrameworkLayerTask>(SCR_ScenarioFrameworkLayerTask.Cast(scenarioFrameworkSystem.GetLastFinishedTaskLayer()));
	}
}