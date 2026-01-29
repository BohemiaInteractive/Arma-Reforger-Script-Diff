[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkDamageContextConditionProjectilePrefab : SCR_ScenarioFrameworkDamageContextConditionBase
{
	[Attribute(defvalue: "", desc: "Projectile Prefab Name", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_sProjectilePrefabName;
	
	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(BaseDamageContext damageContext)
	{
		IEntity damageSource = damageContext.damageSource;
		if (!damageSource)
			return false;
		
		EntityPrefabData prefabData = damageSource.GetPrefabData();
		return prefabData && prefabData.GetPrefabName() == m_sProjectilePrefabName;
	}
}