[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionPlayCommunicationSoundOnEntity : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to play the communication sound on (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "Sound to play.")]
	string 			m_sSound;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;

		SCR_ScenarioFrameworkCallQueueSystem.GetCallQueueNonPausable().CallLater(scenarioFrameworkSystem.PlayCommunicationSoundOnEntity, 2000, false, entity, m_sSound);
	}
}