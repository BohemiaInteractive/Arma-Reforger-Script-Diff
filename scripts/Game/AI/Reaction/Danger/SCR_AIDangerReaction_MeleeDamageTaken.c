[BaseContainerProps()]
class SCR_AIDangerReaction_MeleeDamageTaken : SCR_AIDangerReaction
{
	static const float REACTION_DIST_SQ = 3.5;
	static const float MIN_DIST_RUN = 10.0;
	static const float MAX_DIST_RUN = 15.0;
	
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount)
	{
		if (dangerEvent.GetVictim() != utility.m_OwnerEntity)
			return false;
		// amount of dmg to threatsystem
				
		vector shooterPos = dangerEvent.GetPosition();
			
		//position is 
		vector myPos = dangerEvent.GetVictim().GetOrigin();
		
		//distance
		float distSq = vector.DistanceSq(myPos, shooterPos);
		if (distSq >= REACTION_DIST_SQ)
			return false;
		
		//Rotate towards it
		utility.m_LookAction.LookAt(shooterPos, utility.m_LookAction.PRIO_DANGER_EVENT);

		
		//get the direction from enemy to me = V
		vector V = myPos - shooterPos;
		V.Normalize();
		
		//take a random distance in range [d,D] = d'
		float d = Math.RandomFloat(MIN_DIST_RUN,MAX_DIST_RUN);
		
		//Calculate the center of the pointer, at d' distance from my position in V direction = C
		//Find correct operator
		myPos += V * d;
		 	
		//Call to the BT (in it, it will get a random point, with center at C, and radius R. Then move to it WHILE looking at shooterPos)
		
		//Use the behaviour tree
		SCR_AIRetreatWhileLookAtBehavior behavior = new SCR_AIRetreatWhileLookAtBehavior(utility, null);
		behavior.m_Target.m_Value = myPos;
		behavior.m_LookAt.m_Value = shooterPos;
		
		utility.AddAction(behavior);
		
		return true;
	}
};