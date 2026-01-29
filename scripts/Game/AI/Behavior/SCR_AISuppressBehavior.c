/*!
Behavior for AI suppressing an area
*/

class SCR_AISuppressBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParamRef<SCR_AISuppressionVolumeBase> m_SuppressionVolume = new SCR_BTParamRef<SCR_AISuppressionVolumeBase>("SuppressionVolume");
	ref SCR_BTParam<float> m_fSuppressionDuration_s = new SCR_BTParam<float>("SuppressionDuration_s");
	ref SCR_BTParam<float> m_fFireRate = new SCR_BTParam<float>("FireRate");
		
	//-------------------------------------------------------------------------------------------------------------------------------
	void SCR_AISuppressBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, SCR_AISuppressionVolumeBase suppressionVolume, float suppressionDuration, float fireRate, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_SuppressionVolume.Init(this, suppressionVolume);
		m_fSuppressionDuration_s.Init(this, suppressionDuration);		
		m_fFireRate.Init(this, fireRate);
		
		SetPriority(PRIORITY_BEHAVIOR_SUPPRESS);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_sBehaviorTree = "{447C5FFBE68FF9FE}AI/BehaviorTrees/Chimera/Soldier/SuppressBehavior.bt";
		m_bAllowLook = false;
		
		// Use combat movement but only on foot
		if (utility)
			m_bUseCombatMove = !utility.m_AIInfo.HasUnitState(EUnitState.IN_VEHICLE);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		EnableFriendlyFireCheck(true);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		EnableFriendlyFireCheck(false);		
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void EnableFriendlyFireCheck(bool enable)
	{
		if (m_Utility && m_Utility.m_PerceptionComponent)
			m_Utility.m_PerceptionComponent.SetFriendlyFireCheck(enable);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.COMBAT;
	}
};

class SCR_AISuppressGroupClusterBehavior : SCR_AISuppressBehavior
{
	protected const int VOLUME_UPDATE_INTERVAL_MS = 1*500;
	protected const float FIRE_RATE_SCALING_MAX_DISTANCE = 600; // Max distance for fire rate scaling
	
	protected const float THREAT_MAX_INCREASE = 0.25; // Max increase of group threat potential per tick
	protected const float THREAT_MAX_DECREASE = 0.15; // Max decrease of group threat per tick while in decrease period
	protected const float THREAT_POTENTIAL_DECAY = 0.6; // How much per tick group threat potential is decaying while in decay period
	protected const int THREAT_MAX_PEAK_DURATION_MS = 20*1000; // Max peak duration (time since threat reaching 100%), after this time we'll start decaying potential
	protected const int THREAT_MAX_PEAK_REACTION_DURATION_MS = 15*1000; // How long we should react on reaching peak (intensification of fire rate while in peak threat)
	
	protected SCR_ChimeraAIAgent m_Agent;
	protected SCR_AICombatComponent m_CombatComponent;
	protected SCR_AIThreatSystem m_ThreatSystem;
	protected float m_fNextUpdate;
	protected float m_fGroupThreat; 
	protected float m_fGroupThreatPotential;
	protected bool m_bGroupThreatInPeak;
	protected float m_fGroupThreatPeakTimeout;
	protected float m_fGroupThreatPeakDuration;
	protected float m_fGroupThreatPeakReactionTimeout;
	
	protected SCR_AITargetClusterState m_ClusterState; // Weak pointer to react on deletion
	protected ref SCR_AISuppressionVolumeClusterBox m_Volume;
		
	//-------------------------------------------------------------------------------------------------------------------------------
	void SCR_AISuppressGroupClusterBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, SCR_AISuppressionVolumeBase suppressionVolume, float suppressionDuration, float fireRate, float priorityLevel = PRIORITY_LEVEL_NORMAL, SCR_AITargetClusterState clusterState = null)
	{
		m_ThreatSystem = m_Utility.m_ThreatSystem;
		m_CombatComponent = utility.m_CombatComponent;
		if (m_CombatComponent)
			m_Agent = m_CombatComponent.GetAiAgent();
		
		if (clusterState)
		{
			m_ClusterState = clusterState;
			m_Volume = new SCR_AISuppressionVolumeClusterBox(vector.Zero, vector.Zero);
			m_SuppressionVolume.m_Value = m_Volume;
		}
		else
		{
			Print("SCR_AISuppressGroupClusterBehavior failed, no cluster state provided", LogLevel.ERROR);
			Fail();
		}
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	SCR_AITargetClusterState GetClusterState() {
		return m_ClusterState;
	}
		
	//-------------------------------------------------------------------------------------------------------------------------------
	protected float GetDistance(notnull SCR_AITargetClusterState clusterState)
	{
		if (!m_Utility || !m_Utility.m_OwnerEntity)
			return 1;
	
		return vector.Distance(m_Utility.m_OwnerEntity.GetOrigin(), clusterState.GetCenterPosition());
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	protected float GetFireRate(float distance, float timeSinceLastInfoS, float soldierThreat, float groupThreat, float peakReactionFactor)
	{	
		float midDist = SCR_AICombatComponent.CLOSE_RANGE_COMBAT_DISTANCE + SCR_AICombatComponent.LONG_RANGE_COMBAT_DISTANCE / 2;
		
		// Relation between group and soldier threat depends on distance
		float soldierThreatFactor = Math.Clamp(Math.Map(distance, 0, FIRE_RATE_SCALING_MAX_DISTANCE, 0.7, 0.3), 0.3, 0.7);
		float groupThreatFactor = 1 - soldierThreatFactor;
		
		// Get base fire rate
		float fireRate = 1.75 * (soldierThreat * soldierThreatFactor + groupThreat * groupThreatFactor);
		
		// Apply peak factor
		if (peakReactionFactor > 0)
		{
			float peakFireRate = fireRate * 1.2 * peakReactionFactor;
			fireRate += Math.Max(0, Math.AbsFloat(fireRate - peakFireRate));
		}
		
		// Units at longer ranges fire slower for aiming, we're increasing fire rate because for suppression we don't need to be so accurate
		if (distance > midDist)
			fireRate *= Math.Map(distance, midDist, FIRE_RATE_SCALING_MAX_DISTANCE, 1, 1.25);
		
		// If info is considered old, we scale down fire rate
		if (timeSinceLastInfoS > SCR_AIGroupUtilityComponent.SUPPRESS_OLD_CLUSTER_INFO_AGE_S)
			fireRate *= Math.Map(timeSinceLastInfoS, 0, SCR_AIGroupUtilityComponent.SUPPRESS_MAX_CLUSTER_INFO_AGE_S, 1, 0.3);
		
		// Remove clamp!
		return Math.Clamp(fireRate, 0.05, 2);
	}
	
	
	//-------------------------------------------------------------------------------------------------------------------------------
	protected float GetGroupThreat()
	{
		AIGroup group = m_Agent.GetParentGroup();
		if (group)
		{
			SCR_AIGroupUtilityComponent groupUtil = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
			if (groupUtil)
				return groupUtil.GetThreatMeasure();
		}
		
		return 0;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{	
		// Complete if no cluster to suppress
		if (!m_ClusterState || !m_Volume)
		{
			Complete();
			return 0;
		}
		float time = GetGame().GetWorld().GetWorldTime();
		if (time > m_fNextUpdate)
		{
			// Update volume
			m_Volume.SetClusterState(m_ClusterState);
			
			BaseTarget target = m_CombatComponent.GetCurrentTarget();
			
			float soldierThreat;
			if (m_ThreatSystem)
				soldierThreat = m_ThreatSystem.GetThreatMeasure();
			
			float groupThreat = GetGroupThreat();
			bool inDecay = m_bGroupThreatInPeak && time > m_fGroupThreatPeakTimeout;
			float peakReactionFactor = 0;
			
			// Check if we're in peak reaction
			if (m_bGroupThreatInPeak && time < m_fGroupThreatPeakReactionTimeout)
			{
				// Calculate peak reaction fire rate multiplier
				float timeSincePeak = m_fGroupThreatPeakReactionTimeout - time;
				float peakTime = THREAT_MAX_PEAK_REACTION_DURATION_MS / 2;
			    float diff = Math.AbsFloat(peakTime - timeSincePeak);			
				peakReactionFactor = Math.Map(diff, 0, peakTime, 1, 0);
			}
			
			// Don't increase potential while in decay period
			if (!inDecay)
				m_fGroupThreatPotential += groupThreat;
			
			if (m_fGroupThreatPotential > 0)
			{
				float change = Math.Min(m_fGroupThreatPotential, THREAT_MAX_INCREASE);
				m_fGroupThreatPotential -= change;
				
				// Are we in peak
				if (m_fGroupThreat == 1)
				{
					// Start new peak
					if (!m_bGroupThreatInPeak)
					{	
						m_bGroupThreatInPeak = true;
						m_fGroupThreatPeakTimeout = time + THREAT_MAX_PEAK_DURATION_MS;
						m_fGroupThreatPeakReactionTimeout = time + THREAT_MAX_PEAK_REACTION_DURATION_MS;
					}
					
					// Decay potential if we're in decay period
					if (inDecay)
						m_fGroupThreatPotential -= Math.Min(m_fGroupThreatPotential, THREAT_POTENTIAL_DECAY);
				}
				// We're not in peak, increase group threat
				else
					m_fGroupThreat = Math.Clamp(m_fGroupThreat + change, 0, 1);
			}

			// End of threat peak
			if (m_bGroupThreatInPeak && m_fGroupThreat < 0.5)
				m_bGroupThreatInPeak = false;			
						
			// Potential depleted, decrease group threat
			if (m_fGroupThreatPotential == 0 && m_fGroupThreat > 0)
				m_fGroupThreat -= Math.Min(m_fGroupThreat, THREAT_MAX_DECREASE);
			
			float distance = GetDistance(m_ClusterState);			
			float timeSinceLastInfo = m_ClusterState.GetTimeSinceLastNewInformation();
			float fireRate = GetFireRate(distance, timeSinceLastInfo, soldierThreat, m_fGroupThreat, peakReactionFactor);
			
			m_fFireRate.m_Value = fireRate;
			m_fNextUpdate = time + VOLUME_UPDATE_INTERVAL_MS;
		}
		
		return PRIORITY_BEHAVIOR_SUPPRESS;
	}
}
