[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetAreaTrigger : SCR_ScenarioFrameworkGet
{
	[Attribute()]
	string 		m_sAreaName;

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity = FindEntityByName(m_sAreaName);
		if (!entity)
			return null;

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
		if (!area)
			return null;

		SCR_ScenarioFrameworkTriggerEntity trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(area.GetTrigger());
		if (!trigger)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(entity);
	}
}