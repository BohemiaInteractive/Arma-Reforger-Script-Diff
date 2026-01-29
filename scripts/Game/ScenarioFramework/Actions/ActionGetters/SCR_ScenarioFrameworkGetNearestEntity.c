[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetNearestEntity : SCR_ScenarioFrameworkGet
{
	[Attribute(desc: "Center entity getter. ")]
	ref SCR_ScenarioFrameworkGet m_CenterEntityGetter;
	
	[Attribute(defvalue: "0", desc: "Radius", params: "0 inf 0.01")]
	float m_fRadius;
	
	[Attribute(defvalue: "", desc: "Searched Entity", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et xob")]
	ResourceName m_sSearchedEntityResourceName;
	
	protected IEntity m_Entity;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (!m_CenterEntityGetter)
			return null;
		
		SCR_ScenarioFrameworkParam<IEntity> centerEntityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_CenterEntityGetter.Get());
		if (!centerEntityWrapper)
			return null;
		
		IEntity centerEntity = centerEntityWrapper.GetValue();
		if (!centerEntity)
			return null;
		
		if (m_sSearchedEntityResourceName.Contains(".xob"))
			GetGame().GetWorld().QueryEntitiesBySphere(centerEntity.GetOrigin(), m_fRadius, QueryAddEntityXOB);
		else
			GetGame().GetWorld().QueryEntitiesBySphere(centerEntity.GetOrigin(), m_fRadius, QueryAddEntity);
		
		
		return new SCR_ScenarioFrameworkParam<IEntity>(m_Entity);
	}
	
	//------------------------------------------------------------------------------------------------
	bool QueryAddEntity(notnull IEntity entity)
	{
		if (entity.GetPrefabData().GetPrefabName() == m_sSearchedEntityResourceName)
		{
			m_Entity = entity;
			return false;
		}		
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool QueryAddEntityXOB(notnull IEntity entity)
	{
		VObject vobject = entity.GetVObject();
		if (vobject && vobject.GetResourceName() == m_sSearchedEntityResourceName)
		{
			m_Entity = entity;
			return false;
		}
		
		return true;
	}
}