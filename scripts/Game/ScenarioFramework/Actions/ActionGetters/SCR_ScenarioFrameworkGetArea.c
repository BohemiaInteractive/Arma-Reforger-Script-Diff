[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetArea : SCR_ScenarioFrameworkGet
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

		return new SCR_ScenarioFrameworkParam<IEntity>(entity);
	}
}