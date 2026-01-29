[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetArrayOfLayerBases : SCR_ScenarioFrameworkGet
{
	[Attribute()]
	ref array<string> m_aLayerBaseNames;

	ref array<IEntity> m_aEntities = {};
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity;
		SCR_ScenarioFrameworkLayerBase layer;
		foreach (string name : m_aLayerBaseNames)
		{
			entity = FindEntityByName(name);
			if (!entity)
				continue;
			
			layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
				continue;
			
			if (!m_aEntities.Contains(entity))
				m_aEntities.Insert(entity);
		}
		
		return new SCR_ScenarioFrameworkParam<array<IEntity>>(m_aEntities);
	}
}