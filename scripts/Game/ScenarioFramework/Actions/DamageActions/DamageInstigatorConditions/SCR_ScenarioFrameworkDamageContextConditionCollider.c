[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkDamageContextConditionCollider : SCR_ScenarioFrameworkDamageContextConditionBase
{
	[Attribute("Accepted collider IDs")];
	ref array<int> m_aColliderIDs;
	
	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(BaseDamageContext damageContext)
	{
		foreach (int colliderID : m_aColliderIDs)
		{
			if (colliderID == damageContext.colliderID)
				return true;
		}
		
		return false;
	}
}