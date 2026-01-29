[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetChildEntityByPrefabName : SCR_ScenarioFrameworkGet
{
	[Attribute()]
	string 		m_sLayerName;
	
	[Attribute(defvalue: "", desc: "Prefab Name", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_sPrefabName;

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
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
		
		if (ent.GetPrefabData().GetPrefabName() == m_sPrefabName)
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