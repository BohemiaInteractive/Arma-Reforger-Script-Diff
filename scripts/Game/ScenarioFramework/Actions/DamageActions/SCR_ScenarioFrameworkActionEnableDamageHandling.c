[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionEnableDamageHandling : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Specific entity to disable damage handling for.")]
	ref SCR_ScenarioFrameworkGet m_EntityGetter;
	
	[Attribute(defvalue: "1", desc: "Enable damage handling")]
	bool m_bEnableDamage;
	
	[Attribute(desc: "Entity search radius. Any entity in it will have its damage handling disabled.", params: "0 inf 0.01")]
	float m_fSearchRadius;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (ValidateInputEntity(object, m_EntityGetter, entity))
			QueryEntity(entity);
		
		if (m_fSearchRadius > 0)
			GetGame().GetWorld().QueryEntitiesBySphere(object.GetOrigin(), m_fSearchRadius, QueryEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool QueryEntity(notnull IEntity ent)
	{
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(ent.FindComponent(SCR_DamageManagerComponent));
		if (damageManager)
			damageManager.EnableDamageHandling(m_bEnableDamage);
		
		return true;
	}
}