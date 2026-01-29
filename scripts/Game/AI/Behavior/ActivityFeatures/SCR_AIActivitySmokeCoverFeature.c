enum SCR_AIActivitySmokeCoverFeatureProperties
{
	NONE = 						0,
	PROTECT_POS = 				1,
	PROTECT_FROM_CLUSTERS = 	2,
};

// Class used for weighting of target clusters for smoke cover
class SCR_AIActivitySmokeCoverFeatureCluster : Managed
{
	SCR_AIGroupTargetCluster m_Cluster;
	vector m_vClusterCenterPos;

	// More weight, less likely to be considered
	[SortAttribute()]
	float m_fWeight = 0;
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIActivitySmokeCoverFeatureCluster(SCR_AIGroupTargetCluster cluster, vector targetPos)
	{
		m_Cluster = cluster;
		m_vClusterCenterPos = m_Cluster.m_State.GetCenterPosition();		
		m_fWeight = vector.DistanceSq(targetPos, m_vClusterCenterPos);
		
		// More targets in a cluster, bigger probability we'll consider it
		m_fWeight -= m_Cluster.m_aTargets.Count() * 16;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetCenterPosition()
	{
		return m_vClusterCenterPos;
	}
}

// Class used for weighting of group agents for smoke cover
class SCR_AIActivitySmokeCoverFeatureAgent : Managed
{
	static const float CLOSE_DISTANCE_TRESHOLD_SQ = 11*11;
	static const float MAX_CLOSE_DISTANCE_WEIGHT = 256;
	
	SCR_ChimeraAIAgent m_Agent;
	
	// More weight, less likely to get picked
	[SortAttribute()]
	float m_fWeight = 0;
	
	//------------------------------------------------------------------------------------------------
	float GetCloseTargetPenalty(float distanceToTargetSq) {
		return ((-1 * MAX_CLOSE_DISTANCE_WEIGHT / (CLOSE_DISTANCE_TRESHOLD_SQ * CLOSE_DISTANCE_TRESHOLD_SQ)) * (distanceToTargetSq * distanceToTargetSq)) + MAX_CLOSE_DISTANCE_WEIGHT;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIActivitySmokeCoverFeatureAgent(SCR_ChimeraAIAgent agent, float distanceToTargetSq, bool avoid)
	{
		m_Agent = agent;
		m_fWeight = distanceToTargetSq;
		SCR_AIInfoComponent infoComp = agent.m_InfoComponent;
		
		if (infoComp.HasRole(EUnitRole.MEDIC))
			m_fWeight += 64;
		
		if (infoComp.HasRole(EUnitRole.MACHINEGUNNER))
			m_fWeight += 256;
		
		if (infoComp.HasUnitState(EUnitState.WOUNDED))
			m_fWeight += 256;
		
		if (avoid)
			m_fWeight += 512;
		
		if (distanceToTargetSq < CLOSE_DISTANCE_TRESHOLD_SQ)
			m_fWeight += GetCloseTargetPenalty(distanceToTargetSq);
	}
	
	//------------------------------------------------------------------------------------------------
	void SendMessage(AICommunicationComponent comms, SCR_AIActivityBase activity, vector position, float delay)
	{		
		SCR_AIMessage_ThrowGrenadeTo msg = SCR_AIMessage_ThrowGrenadeTo.Create(position, EWeaponType.WT_SMOKEGRENADE, delay);
		msg.m_RelatedGroupActivity = activity;
		msg.m_fPriorityLevel = SCR_AIActionBase.PRIORITY_BEHAVIOR_THROW_GRENADE;
		msg.SetReceiver(m_Agent);
		comms.RequestBroadcast(msg, m_Agent);
	}
}

class SCR_AIActivitySmokeCoverFeature: SCR_AIActivityFeatureBase
{
	static const int MAX_DISTANCE_TO_TARGET_POS_SQ = 40*40;
	static const int SMOKE_WALL_GAPS_SIZE = 5; // Width in meters of gaps between smokes in smoke walls
	
	//-------------------------------------------------------------------------------------
	protected void GetConsideredAgents(SCR_AIGroupUtilityComponent groupUtility, vector targetPosition, array<AIAgent> avoidAgents, array<AIAgent> excludeAgents,
	notnull array<ref SCR_AIActivitySmokeCoverFeatureAgent> outConsideredAgents, out int combatReadyAgentsCount)
	{
		combatReadyAgentsCount = 0;
		
		foreach (SCR_AIInfoComponent infoComp: groupUtility.m_aInfoComponents)
		{
			SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(infoComp.GetOwner());
			if (!agent || excludeAgents.Contains(agent))
				continue;
			
			// Ignore agents that are not "available", considered combat ready
			if (!IsAgentAvailable(agent))
				continue;
			
			// Count how many agents in combat-ready condition group currently has
			combatReadyAgentsCount++;
			
			IEntity controlledEntity = agent.GetControlledEntity();
			// Don't consider agents that are not capable of throwing smoke grenades
			if (!controlledEntity || !infoComp.HasRole(EUnitRole.HAS_SMOKE_GRENADE))
				continue;
			
			// Don't consider agents that are too far or too close to target position
			float distanceToTargetSq = vector.DistanceSq(targetPosition, controlledEntity.GetOrigin());
			if (distanceToTargetSq > MAX_DISTANCE_TO_TARGET_POS_SQ)
				continue;
			
			outConsideredAgents.Insert(new SCR_AIActivitySmokeCoverFeatureAgent(agent, distanceToTargetSq, avoidAgents.Contains(agent)));
		}
	}
	
	//-------------------------------------------------------------------------------------
	protected void GetClusterBasedSmokePositions(array<ref SCR_AIGroupTargetCluster> targetClusters, vector targetPosition, int maxSmokePositions, out array<vector> smokePositions)
	{	
		int clustersCount = targetClusters.Count();
		
		array<ref SCR_AIActivitySmokeCoverFeatureCluster> clusters = {};
		foreach (SCR_AIGroupTargetCluster cluster: targetClusters)
			clusters.Insert(new SCR_AIActivitySmokeCoverFeatureCluster(cluster, targetPosition));
				
		// More clusters than max smoke positions - sort clusters by weight
		if (clustersCount > maxSmokePositions)
			clusters.Sort(); // Sort clusters by weight, ascending
				
		float maxClustersCount = Math.Min(maxSmokePositions, clustersCount);
				
		vector lastDirection;
				
		// Gather positions to cover from clusters
		for (int i = 0; i < maxClustersCount; i++)
		{
			SCR_AIActivitySmokeCoverFeatureCluster cluster = clusters[i];
			vector clusterPos = cluster.GetCenterPosition();
					
			lastDirection = vector.Direction(targetPosition, clusterPos).Normalized();					
			smokePositions.Insert(targetPosition + (lastDirection * (5 + (maxClustersCount * 3))));
		}
				
		int smokesLeft = maxSmokePositions - maxClustersCount;

		// Exit if no more smokes left to be thrown
		if (smokesLeft <= 0)
			return;
		
		// If only one cluster, create a wall oriented torwards the cluster
		if (maxClustersCount == 1)
		{
			float distance = 0;
						
			vector pos = smokePositions[0];
			vector relDir = (lastDirection * vector.Up).Normalized();
			
			for (int i = 0; i < smokesLeft; i++)
			{
				int pair = i % 2;
				if (pair == 0)
					distance += SMOKE_WALL_GAPS_SIZE;
							
				vector relVector = relDir * distance;
							
				if (pair == 0)
					smokePositions.Insert(pos + relVector);
				else
					smokePositions.Insert(pos - relVector);
			}
		
			return;
		}

		// If more clusters, fill gaps between already picked smoke positions
		int maxPositions = Math.Min(maxClustersCount / 2, smokesLeft);
						
		for (int i = 0; i < maxPositions; i++)
		{
			vector pos1 = smokePositions[i];
			vector pos2 = smokePositions[i + 1];
			vector direction = vector.Direction(pos1, pos2).Normalized();
			float distance = vector.Distance(pos1, pos2);
							
			smokePositions.Insert(pos1 + (direction * (distance / 2)));
		}
	}
	
	
	//-------------------------------------------------------------------------------------
	bool Execute(
		notnull SCR_AIGroupUtilityComponent groupUtility,
		vector targetPosition,
		SCR_AIActivitySmokeCoverFeatureProperties smokeCoverProperties,
		notnull array<AIAgent> avoidAgents,
		notnull array<AIAgent> excludeAgents,
		int maxPositionCount = 1,
		SCR_AIActivityBase contextActivity = null
	) {
		// Number of agents that are considered combat-ready. Will be used to calculate how many 
		// grenade throwers can be picked to not impare group's ability to continue fire fight
		int combatReadyAgentsCount;
		// Array of agents that will be considered as smoke grenade throwers
		array<ref SCR_AIActivitySmokeCoverFeatureAgent> consideredAgents = {};		
		
		GetConsideredAgents(groupUtility, targetPosition, excludeAgents, avoidAgents, consideredAgents, combatReadyAgentsCount);
		
		int consideredAgentsCount = consideredAgents.Count();
		
		// Early exit if no agents to consider for throwing
		if (!consideredAgentsCount)
			return false;
			
		
		// Get max count of smoke positions to cover
		// Don't allow more than half of combat-ready agents to throw, other half must cover/fight
		int maxSmokePositions = Math.Min(Math.Floor(combatReadyAgentsCount / 2), Math.Min(maxPositionCount, consideredAgentsCount));
		
		// Early exit if we can't smoke any position
		if (maxSmokePositions <= 0)
			return false;
		
		// Sort considered agents by weights if we have more agents than potential positions
		if (consideredAgentsCount > maxSmokePositions)
			consideredAgents.Sort(); // Sort considered agents by weight, ascending
		
		array<vector> smokePositions = {};
		
		// Smoke to protect position, we won't be throwing exactly at target position but to cover it
		if (smokeCoverProperties & SCR_AIActivitySmokeCoverFeatureProperties.PROTECT_POS)
		{
			array<ref SCR_AIGroupTargetCluster> targetClusters = groupUtility.m_Perception.m_aTargetClusters;
			
			// Pick positions based on target clusters if enabled and available
			if ((smokeCoverProperties & SCR_AIActivitySmokeCoverFeatureProperties.PROTECT_FROM_CLUSTERS) && targetClusters.Count() > 0)
				GetClusterBasedSmokePositions(targetClusters, targetPosition, maxSmokePositions, smokePositions);
			// If not, randomize positions around target position
			else
			{
				float angle =  Math.RandomFloat(0, 360);
				for (int i = 0; i < maxSmokePositions; i++)
				{
					vector direction = {Math.Cos(angle * Math.DEG2RAD), 0, Math.Sin(angle * Math.DEG2RAD)};
					// More smoke positions, greater the distance
					float distance = 1 + maxSmokePositions + Math.RandomFloat(0, maxSmokePositions * 1.5);
					smokePositions.Insert(targetPosition + (direction * distance));
					
					angle += (360 / maxSmokePositions);
					if (angle > 360)
						angle -= 360;
				}
			}
		}
		// Just simply smoke target position
		else
			smokePositions.Insert(targetPosition);
		
		AICommunicationComponent comms = groupUtility.m_Owner.GetCommunicationComponent();
		if (!comms)
			return false;

		int smokePositionsCount = smokePositions.Count();
				
		// Send throw grenade messages to all picked agents
		for (int i = 0; i < smokePositionsCount; i++)
		{
			// Randomize small delay to prevent unnatural perfect sync between throws of multiple soldiers
			float delay = 0.3 + Math.RandomFloat(0, 1.5);
			consideredAgents[i].SendMessage(comms, contextActivity, smokePositions[i], delay);
		}
		
		return true;
	}
	
	//-------------------------------------------------------------------------------------
	bool ExecuteForActivity(SCR_AIActivityBase activity, int maxPositionCount = 3)
	{
		SCR_AIGroupUtilityComponent groupUtility = activity.m_Utility;
		if (!groupUtility)
			return false;
				
		return Execute(
			groupUtility,
			GetActivityTargetPosition(activity),
			GetActivityProperties(activity),
			GetActivityAvoidedAgents(activity),
			GetActivityExcludedAgents(activity),
			maxPositionCount,
			activity
		);
	}
	
	//-------------------------------------------------------------------------------------
	// Returns position that should be covered by smoke grenades for given activity
	vector GetActivityTargetPosition(SCR_AIActivityBase activity)
	{
		return vector.Zero;
	}
	
	//-------------------------------------------------------------------------------------
	// Returns smoke cover properties for given activity
	SCR_AIActivitySmokeCoverFeatureProperties GetActivityProperties(SCR_AIActivityBase activity)
	{
		return SCR_AIActivitySmokeCoverFeatureProperties.NONE;
	}	
	
	//-------------------------------------------------------------------------------------
	// Returns list of agents that should be avoided during selection for smoke grenade throwing for given activity
	// Those agents still can be picked, we'll just try what we can to avoid it
	array<AIAgent> GetActivityAvoidedAgents(SCR_AIActivityBase activity)
	{		
		return null;
	}	
	
	//-------------------------------------------------------------------------------------
	// Returns list of agents that should be excluded from smoke grenade throwing for given activity
	// Those agents won't be picked, no matter the circumstances
	array<AIAgent> GetActivityExcludedAgents(SCR_AIActivityBase activity)
	{
		return null;
	}
}


class SCR_AIHealActivitySmokeCoverFeature: SCR_AIActivitySmokeCoverFeature
{
	//-------------------------------------------------------------------------------------
	override vector GetActivityTargetPosition(SCR_AIActivityBase activity)
	{
		SCR_AIHealActivity healActivity = SCR_AIHealActivity.Cast(activity);
		
		if (healActivity.m_EntityToHeal.m_Value)
			return healActivity.m_EntityToHeal.m_Value.GetOrigin();
		
		return vector.Zero;
	}
	
	//-------------------------------------------------------------------------------------
	override SCR_AIActivitySmokeCoverFeatureProperties GetActivityProperties(SCR_AIActivityBase activity)
	{
		return SCR_AIActivitySmokeCoverFeatureProperties.PROTECT_POS | SCR_AIActivitySmokeCoverFeatureProperties.PROTECT_FROM_CLUSTERS;
	}
	
	//-------------------------------------------------------------------------------------
	override array<AIAgent> GetActivityAvoidedAgents(SCR_AIActivityBase activity)
	{	
		SCR_AIHealActivity healActivity = SCR_AIHealActivity.Cast(activity);
		
		ChimeraCharacter character = ChimeraCharacter.Cast(healActivity.m_MedicEntity.m_Value);
		if (!character)
			return null;
		
		CharacterControllerComponent charCtrl = character.GetCharacterController();
		if (!charCtrl)
			return null;
		
		AIControlComponent aiCtrl = charCtrl.GetAIControlComponent();
		if (!aiCtrl)
			return null;
		
		return {aiCtrl.GetAIAgent()};
	}
	
	//-------------------------------------------------------------------------------------
	override array<AIAgent> GetActivityExcludedAgents(SCR_AIActivityBase activity)
	{
		SCR_AIHealActivity healActivity = SCR_AIHealActivity.Cast(activity);
		
		ChimeraCharacter character = ChimeraCharacter.Cast(healActivity.m_EntityToHeal.m_Value);
		if (!character)
			return null;
		
		CharacterControllerComponent charCtrl = character.GetCharacterController();
		if (!charCtrl)
			return null;
		
		AIControlComponent aiCtrl = charCtrl.GetAIControlComponent();
		if (!aiCtrl)
			return null;		
		
		return {aiCtrl.GetAIAgent()};
	}
}