[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkDamageContextConditionHitZone : SCR_ScenarioFrameworkDamageContextConditionBase
{
	[Attribute(desc: "Hit Zone Group")]
	ref SCR_ScenarioFrameworkDamageContextHitZoneGroupBase m_HitZoneGroupSelector;
	
	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(BaseDamageContext damageContext)
	{
		if (!m_HitZoneGroupSelector)
			return false;
		
		SCR_HitZone hitZone = SCR_HitZone.Cast(damageContext.struckHitZone);
		if (!hitZone)
			return false;
		
		return hitZone.GetHitZoneGroup() == m_HitZoneGroupSelector.GetHitZoneGroup();
	}
}