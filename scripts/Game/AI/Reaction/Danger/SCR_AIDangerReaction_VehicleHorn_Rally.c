//! Rally bystander reaction to vehicle horn. Assign only in the bystander agent prefab.
[BaseContainerProps()]
class SCR_AIDangerReaction_VehicleHorn_Rally : SCR_AIDangerReaction_VehicleHorn
{
	static const float REACTION_RALLY_FRIENDLY_DIST_SQ = 8*8;

	//----------------------------------------------------------------------------------------------------------------------------------------------------------
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount)
	{
		IEntity vehicleObject = dangerEvent.GetVictim();
		if (!vehicleObject)
			return false;

		// Ignore if we already have an action to move from that vehicle
		if (SCR_AIMoveFromDangerBehavior.ExistsBehaviorForEntity(utility, vehicleObject))
			return false;

		//Check distance
		vector vehiclePos 	= vehicleObject.GetOrigin();
		vector agentPos 	= utility.GetOrigin();
		float distSq = vector.DistanceSq(vehiclePos, agentPos);

		if (distSq >= REACTION_ENEMY_DIST_SQ)
			return false;

		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
		//Enemy car
		if (agent && agent.IsPerceivedEnemy(vehicleObject))
		{
			//Orientate towards the sound origin
			utility.m_LookAction.LookAt(dangerEvent.GetPosition(), utility.m_LookAction.PRIO_DANGER_EVENT);
			return true;
		}
		//Ally car
		else
		{
			if (distSq <= REACTION_RALLY_FRIENDLY_DIST_SQ)
			{
				//Move away from danger - cancel gestures for rally character
				SCR_ChimeraAIBystander rallyAgent = SCR_ChimeraAIBystander.Cast(agent);
				if (rallyAgent)
					rallyAgent.InvokeRelevantInvokerToEndGesture(true, true);

				SCR_AIMoveFromDangerBehavior behavior = new SCR_AIMoveFromVehicleHornBehavior(utility, null, vector.Zero, dangerEntity: vehicleObject);
				utility.AddAction(behavior);
				return true;
			}
		}
		return false;
	}
};
