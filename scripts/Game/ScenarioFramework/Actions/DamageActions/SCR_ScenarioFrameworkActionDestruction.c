[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionDestruction : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Specific building to be destroyed.")]
	ref SCR_ScenarioFrameworkGet m_EntityGetter;
	
	[Attribute(defvalue: "0", desc: "Radius Destruction")]
	bool m_bRadiusDestruction;
	
	[Attribute(defvalue: "0", desc: "Destruction Radius", params: "0 inf 0.01")]
	float m_fDestructionRadius;
	
	protected ref array<SCR_DestructibleBuildingComponent> m_aBuildingComponents;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (m_bRadiusDestruction)
		{
			GetGame().GetWorld().QueryEntitiesBySphere(object.GetOrigin(), m_fDestructionRadius, QueryEntity);
		}
		else
		{
			IEntity entity;
			if (ValidateInputEntity(object, m_EntityGetter, entity))
				QueryEntity(entity);
		}
		
		if (!m_aBuildingComponents || m_aBuildingComponents.IsEmpty())
			return;
		
		foreach (SCR_DestructibleBuildingComponent building : m_aBuildingComponents)
		{
			building.GoToDestroyedStateLoad();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool QueryEntity(notnull IEntity ent)
	{
		SCR_DestructibleBuildingComponent destruction = SCR_DestructibleBuildingComponent.Cast(ent.FindComponent(SCR_DestructibleBuildingComponent));
		if (!destruction)
			return true;
		
		if (!m_aBuildingComponents)
			m_aBuildingComponents = {};
		
		m_aBuildingComponents.Insert(destruction);
		return true;
	}
}