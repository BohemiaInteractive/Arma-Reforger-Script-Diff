[BaseContainerProps()]
class SCR_AIDangerReaction_GrenadeLanding : SCR_AIDangerReaction
{
	static const float GRENADE_AVOID_RADIUS = 13; // Max distance for agent to react. Must be same as in behavior tree!
	static const float GRENADE_AVOID_RADIUS_MIN = 2; // Distance below which the reaction will be almost instantaneous (GRENADE_AVOID_REACT_MIN_TIME_MS)
	
	static const float GRENADE_AVOID_REACT_MIN_TIME_MS = 50; // Minimal possible delay time for grenate reaction in ms
	static const float GRENADE_AVOID_DIST_REACT_TIME_MS = 2000; // Worst possible reaction time based on distance (GRENADE_AVOID_RADIUS)
	
	static const float GRENADE_AVOID_VIS_REACT_TIME_MS = 1100; // Worst possible reaction time based on grenade visiblity (behind the agent's back)

	static const float GRENADE_AVOID_DIST_CURVE_PARAM = GRENADE_AVOID_DIST_REACT_TIME_MS / Math.Pow(GRENADE_AVOID_RADIUS - GRENADE_AVOID_RADIUS_MIN, 2); // Constant for quadratic curve
	
	//-------------------------------------------------------------------------------------------
	// Returns grenade visibility factor as float in range 0-1 based on agent body direction in relation to grenade
	float GetGrenadeVisibilityFactor(GenericEntity entity, vector grenadePos)
	{
		vector agentTransform[3];
		entity.GetTransform(agentTransform);
		vector agentDir = agentTransform[2];	
		
		vector dirToGrenade = vector.Direction(entity.GetOrigin(), grenadePos);
		dirToGrenade.Normalize();
		
		float angle = vector.Dot(dirToGrenade, agentDir);
		
		return Math.Map(angle, -1, 1, 0, 1); 
	}

	//-------------------------------------------------------------------------------------------
	// Returns reaction time in ms based on agent threat level
	float GetThreatLevelReactionTime(notnull SCR_AIThreatSystem threatSystem)
	{
		EAIThreatState threatState = threatSystem.GetState();
			
		switch (threatState)
		{
			case EAIThreatState.SAFE:
			{
				return 900;
			}
			break;
			case EAIThreatState.VIGILANT:
			{
				return 200;
			}
			break;
			case EAIThreatState.ALERTED:
			{
				return 100;
			}
			break;
			case EAIThreatState.THREATENED:
			{
				return 370;
			}
			break;
		}
		
		return 1.0;
	}
	
	//-------------------------------------------------------------------------------------------
	// Returns reaction time in ms based on agent sq distance to grenade
	float GetDistanceSQReactionTime(float distanceToGrenade)
	{
		// Quadratic curve from min to max distance
		return GRENADE_AVOID_DIST_CURVE_PARAM * (
			distanceToGrenade * distanceToGrenade - 
			2 * GRENADE_AVOID_RADIUS_MIN * distanceToGrenade +
			GRENADE_AVOID_RADIUS_MIN * GRENADE_AVOID_RADIUS_MIN
		);
	}
	
	//-----------------------------------------------------------------------------------------------------
	// Return utility owner faction
	Faction GetOwnerFaction(SCR_AIUtilityComponent utility)
	{
		SCR_ChimeraCharacter ownerCharacter = SCR_ChimeraCharacter.Cast(utility.m_OwnerEntity);
		if (!ownerCharacter)
			return null;
		
		return ownerCharacter.GetFaction();
	}
	
	//-----------------------------------------------------------------------------------------------------
	// Return character of grenade instigator
	SCR_ChimeraCharacter GetShooterCharacter(notnull IEntity grenadeObject)
	{
		GrenadeMoveComponent grenadeComp = GrenadeMoveComponent.Cast(grenadeObject.FindComponent(GrenadeMoveComponent));
		if (!grenadeComp)
			return null;
		
		Instigator shooterInst = grenadeComp.GetInstigator();
		if (!shooterInst)
			return null;
		
		return SCR_ChimeraCharacter.Cast(shooterInst.GetInstigatorEntity());
	}
	
	//-----------------------------------------------------------------------------------------------------
	// Adds grenade instigator position to group perception for potentatial investigation later
	void AddToGroupPerception(SCR_AIUtilityComponent utility, IEntity shooter, vector observePosition, Faction faction)
	{		
		AIGroup group = utility.GetOwner().GetParentGroup();
		if (!group)
			return;
		
		SCR_AIGroupUtilityComponent groupUtilityComp = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
		if (!groupUtilityComp)
			return;
		
		PerceptionManager perceptionMan = GetGame().GetPerceptionManager();
		if (!perceptionMan)
			return;

		groupUtilityComp.m_Perception.AddOrUpdateGunshot(shooter, observePosition, faction, perceptionMan.GetTime(), true);
	}
	
	//-------------------------------------------------------------------------------------------
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount)
	{
		IEntity grenadeObject = dangerEvent.GetObject();
		if (grenadeObject)
		{
			vector grenadePos = grenadeObject.GetOrigin();
			vector agentPos = utility.GetOrigin();
			float distanceToGrenade = vector.Distance(utility.GetOrigin(), grenadePos);
			if (distanceToGrenade < GRENADE_AVOID_RADIUS)
			{
				vector observePosition = vector.Zero;
				SCR_ChimeraCharacter shooter = GetShooterCharacter(grenadeObject);
				Faction ownerFaction = GetOwnerFaction(utility);
				Faction shooterFaction = shooter.GetFaction();
				
				// Filter out friendlies
				if (ownerFaction && shooter && !ownerFaction.IsFactionFriendly(shooterFaction))
					AddToGroupPerception(utility, shooter, shooter.GetOrigin(), shooterFaction);

				float reactionDelay = GRENADE_AVOID_REACT_MIN_TIME_MS;
				
				if (distanceToGrenade > GRENADE_AVOID_RADIUS_MIN)
				{					
					float visibilityFactor = GetGrenadeVisibilityFactor(utility.m_OwnerEntity, grenadePos);
					float visbilityReactionTime = GRENADE_AVOID_VIS_REACT_TIME_MS * (1 - visibilityFactor);
					float threatLevelReactionTime = GetThreatLevelReactionTime(threatSystem);
					float distanceReactionTime = GetDistanceSQReactionTime(distanceToGrenade);
					
					reactionDelay += visbilityReactionTime + threatLevelReactionTime + distanceReactionTime;
										
					#ifdef AI_DEBUG
					AddDebugMessage(utility, string.Format("Delayed grenade reaction (visFact: %1 visReactTime: %2ms ThreatLvlTime: %3ms distReactTime %4ms distanceToGrenade %5)", visibilityFactor, visbilityReactionTime, threatLevelReactionTime, distanceReactionTime, distanceToGrenade));
					#endif					
				} else {
					#ifdef AI_DEBUG
					AddDebugMessage(utility, string.Format("Grenade distance below min treshold, quick reaction (distanceToGrenade: %1)", distanceToGrenade));
					#endif
				}
				
				SCR_AIMoveFromDangerBehavior behavior = new SCR_AIMoveFromGrenadeBehavior(utility, null, vector.Zero, grenadeObject, reactionDelay, observePosition);
				utility.AddAction(behavior);
				return true;
			}
		}
		return false;
	}
};