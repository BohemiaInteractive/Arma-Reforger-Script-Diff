[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetSpawnedEntity : SCR_ScenarioFrameworkGet
{
	[Attribute()]
	string 		m_sLayerName;

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity = FindEntityByName(m_sLayerName);
		if (!entity)
			return null;
		
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(layer.GetSpawnedEntity());
	}
}