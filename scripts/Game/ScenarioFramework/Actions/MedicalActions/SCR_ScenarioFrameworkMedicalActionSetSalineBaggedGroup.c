[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetSalineBaggedGroup : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "{00E36F41CA310E2A}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_01.et", desc: "Resource name of the Saline bag you want to use")]
	ResourceName m_sSalineBagPrefab;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		Resource resource = Resource.Load(m_sSalineBagPrefab);
		if (!resource)
			return;
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		m_DamageManager.GetOwner().GetWorldTransform(spawnParams.Transform);
		spawnParams.TransformMode = ETransformMode.WORLD;
		
		IEntity salineBag = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		if (!salineBag)
			return;
		
		SCR_ConsumableItemComponent consumableItemComp = SCR_ConsumableItemComponent.Cast(salineBag.FindComponent(SCR_ConsumableItemComponent));
		if (!consumableItemComp)
			return;
		
		SCR_ConsumableEffectBase consumableEffect = consumableItemComp.GetConsumableEffect();
		if (!consumableEffect)
			return;
		
		consumableEffect.ApplyEffect(m_DamageManager.GetOwner(), m_DamageManager.GetOwner(), salineBag, null);
	}
}