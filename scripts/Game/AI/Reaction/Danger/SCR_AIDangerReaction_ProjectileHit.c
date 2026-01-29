[BaseContainerProps()]
class SCR_AIDangerReaction_ProjectileHit : SCR_AIDangerReaction
{
	protected static const float BULLET_IMPACT_DISTANCE_MAX_SQ = 10*10;
	
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount)
	{
		float distanceSq = vector.DistanceSq(utility.GetOrigin(), dangerEvent.GetPosition());
		if (distanceSq > BULLET_IMPACT_DISTANCE_MAX_SQ)
			return false;

		threatSystem.ThreatBulletImpact(dangerEventCount);
		
		return true;
	}	
};
