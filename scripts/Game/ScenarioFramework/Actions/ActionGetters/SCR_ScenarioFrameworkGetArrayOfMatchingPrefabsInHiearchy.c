[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetArrayOfMatchingPrefabsInHiearchy : SCR_ScenarioFrameworkGet
{
	[Attribute(desc: "Getter for entity to search its hiearchy")];
	ref SCR_ScenarioFrameworkGetLayerBase m_EntityGetter;
	
	[Attribute(desc: "Prefab resource names")]
	ref array<ref ResourceName> m_aPrefabResourceNames;
	
	ref array<IEntity> m_aEntities = {};
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (!m_EntityGetter || !m_aPrefabResourceNames)
			return null;
		
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_EntityGetter.Get());
		if (!entityWrapper)
			return null;
		
		IEntity entity = entityWrapper.GetValue();
		if (!entity)
			return null;
		
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return null;
		
		ProcessChildren(layer.GetSpawnedEntity());
		
		return new SCR_ScenarioFrameworkParam<array<IEntity>>(m_aEntities);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ProcessChildren(IEntity ent)
	{
		if (m_aPrefabResourceNames.Find(ent.GetPrefabData().GetPrefabName()))
			m_aEntities.Insert(ent);
		
		IEntity children = ent.GetChildren();
		while (children)
		{
			ProcessChildren(children);
			children = children.GetSibling();
		}
	}
}