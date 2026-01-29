[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionPlaySound : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Sound to play.")]
	string 			m_sSound;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;

		//After activation of this action, we want to play it with slight delay
		SCR_ScenarioFrameworkCallQueueSystem.GetCallQueueNonPausable().CallLater(scenarioFrameworkSystem.PlaySoundOnEntity, 2000, false, null, m_sSound);
	}
}