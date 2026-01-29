[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkSetSignal : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to set the signal on (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Signal to set")]
	string m_sSignal;
	
	[Attribute(desc: "Value to set")]
	int m_iValue;

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

		scenarioFrameworkSystem.SetSignalValue(entity, m_sSignal, m_iValue);
	}
}