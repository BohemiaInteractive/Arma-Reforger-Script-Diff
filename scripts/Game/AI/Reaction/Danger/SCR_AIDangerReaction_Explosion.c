[BaseContainerProps()]
class SCR_AIDangerReaction_Explosion : SCR_AIDangerReaction
{
	private static const float EXPLOSION_OBSERVE_DISTANCE = 220; // Maximal distance from explosion to trigger observe behavior
	
	//------------------------------------------------------------------------------------------------
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount)
	{
		IEntity ownerEntity = utility.m_OwnerEntity;
		
		if (!ownerEntity)
			return false;
		
		vector position = dangerEvent.GetPosition();
		float distance = vector.Distance(ownerEntity.GetOrigin(), position);
		
		if (distance > SCR_AIThreatSystem.EXPLOSION_MAX_DISTANCE)
			return false;
		
		// Ignore if friendly
		IEntity instigatorRoot = dangerEvent.GetObject();
		if (instigatorRoot)
		{
			instigatorRoot = instigatorRoot.GetRootParent();
			bool isMilitary = utility.IsMilitary();
			SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
			if (isMilitary && (!agent || !agent.IsPerceivedEnemy(instigatorRoot)))
				return false;
		}
		
		// Increase threat level
		threatSystem.ThreatExplosion(distance);
		utility.m_SectorThreatFilter.OnExplosion(position);
		
		return true;
	}
};