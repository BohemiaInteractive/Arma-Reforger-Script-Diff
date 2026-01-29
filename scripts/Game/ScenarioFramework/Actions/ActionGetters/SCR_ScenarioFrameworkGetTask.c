[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetTask : SCR_ScenarioFrameworkGet
{
	[Attribute()]
	string 		m_sLayerTaskName;

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity = FindEntityByName(m_sLayerTaskName);
		if (!entity)
			return null;
		
		SCR_ScenarioFrameworkLayerTask layer = SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (!layer)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(layer.GetTask());
	}
}