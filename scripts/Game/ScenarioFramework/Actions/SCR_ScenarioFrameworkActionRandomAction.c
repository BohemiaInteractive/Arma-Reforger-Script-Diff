[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionRandomAction : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Upon activation, one random Action will be selected from these Actions")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_aActions || m_aActions.IsEmpty())
			return;

		SCR_ScenarioFrameworkActionBase randomAction = m_aActions.GetRandomElement();
		if (randomAction)
			randomAction.Init(object);
	}

	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActions;
	}
}