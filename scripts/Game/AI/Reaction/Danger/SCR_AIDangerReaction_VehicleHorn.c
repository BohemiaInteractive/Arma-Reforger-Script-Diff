[BaseContainerProps()]
class SCR_AIDangerReaction_VehicleHorn : SCR_AIDangerReaction
{
	static const float REACTION_ENEMY_DIST_SQ = 50*50;
	static const float REACTION_FRIENDLY_DIST_SQ = 12*12;
	
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount)
	{		
		IEntity vehicleObject = dangerEvent.GetVictim();
		if (!vehicleObject)
			return false;
		
		// Ignore if we are in same vehicle
		IEntity parent = utility.m_OwnerEntity.GetParent();
		while(parent)
		{
			if (parent == vehicleObject)
				return false;
			parent = parent.GetParent();
		}
		
		// Ignore if we already have an action to move from that vehicle
		if (SCR_AIMoveFromDangerBehavior.ExistsBehaviorForEntity(utility, vehicleObject))
			return false;
		
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
		
		//Check distance
		vector vehiclePos 	= vehicleObject.GetOrigin();
		vector agentPos 	= utility.GetOrigin();
		float distSq = vector.DistanceSq(vehiclePos, agentPos); 
		
		if (distSq >= REACTION_ENEMY_DIST_SQ)
			return false;
		
		//Enemy car
		if(agent.IsPerceivedEnemy(vehicleObject))
		{
			//Orientate towards the sound origin
			utility.m_LookAction.LookAt(dangerEvent.GetPosition(), utility.m_LookAction.PRIO_DANGER_EVENT);
			return true;
		}
		//Ally car
		else
		{
			if (distSq <= REACTION_FRIENDLY_DIST_SQ)
			{
				//Move away from danger
				SCR_AIMoveFromDangerBehavior behavior = new SCR_AIMoveFromVehicleHornBehavior(utility, null, vector.Zero, dangerEntity: vehicleObject);
				utility.AddAction(behavior);
				return true;
			}
		}
		return false;
	}	
};