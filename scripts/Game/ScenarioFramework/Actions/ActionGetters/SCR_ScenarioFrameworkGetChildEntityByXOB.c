[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetChildEntityByXOB : SCR_ScenarioFrameworkGet
{
	[Attribute(desc: "Parent entity getter. ")]
	ref SCR_ScenarioFrameworkGet m_EntityGetter;
	
	[Attribute(defvalue: "", desc: "Prefab Name", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "xob")]
	ResourceName m_sMeshResourceName;

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (!m_EntityGetter)
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

		return new SCR_ScenarioFrameworkParam<IEntity>(FindEntity(layer.GetSpawnedEntity()));
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity FindEntity(IEntity ent)
	{
		if (!ent)
			return null;
		
		VObject vobjekt = ent.GetVObject();
		
		if (vobjekt && vobjekt.GetResourceName() == m_sMeshResourceName)
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