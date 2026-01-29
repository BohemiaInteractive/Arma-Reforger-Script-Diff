[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetChildEntityByClassName : SCR_ScenarioFrameworkGet
{
	[Attribute()]
	string m_sLayerName;
	
	[Attribute(defvalue: "", desc: "Class Name")]
	string m_sClassName;
	
	typename m_ClassToSearch;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		m_ClassToSearch = m_sClassName.ToType();
		
		IEntity entity = FindEntityByName(m_sLayerName);
		if (!entity)
			return null;
		
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(FindEntity(layer.GetSpawnedEntity()));
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity FindEntity(IEntity ent)
	{
		if (!ent)
			return null;
		
		if (ent.Type() == m_ClassToSearch)
			return ent;

		IEntity children = ent.GetChildren();
		while (children)
		{
			ent = FindEntity(children);
			if (ent)
				return ent;
			
			children = children.GetSibling();
		}
		
		return null;
	}
}