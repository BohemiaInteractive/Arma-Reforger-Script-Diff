// AI healing behaviour
// TODO: You have to handle situation, in which movement can be disabled (in AIConfig component)

class SCR_AIHealBehavior : SCR_AIBehaviorBase
{
	IEntity m_EntityToHeal;
	
	SCR_AIInfoComponent m_AIInfo;
	SCR_DamageManagerComponent m_DamageManager;
	
	protected const float PRIORITY_DELAY_MIN_MS = 800.0; // Delay until action returns its max priority
	protected const float PRIORITY_DELAY_MAX_MS = 1200.0;
	
	protected const float MAX_TIME_TO_UNCON_HIGH_PRIORITY_S = 30; // Time in seconds to losing consciousness below which we treat healing ourselves as high priority
	
	protected float m_fTimeCreated_ms;
	protected float m_fPriorityDelay_ms;
	
	//-------------------------------------------------------------------------------------------------------------------------------
	//! priority should be between normal Move and AttackMove and replaced by enum later, maybe evaluated dynamicly
	void SCR_AIHealBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, IEntity entityToHeal, bool allowHealMove, float priority = PRIORITY_BEHAVIOR_HEAL, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Heal.bt";
		m_EntityToHeal = entityToHeal;
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_fTimeCreated_ms = GetGame().GetWorld().GetWorldTime();
		m_fPriorityDelay_ms = Math.RandomFloat(PRIORITY_DELAY_MIN_MS, PRIORITY_DELAY_MAX_MS);
		
		SCR_AIUtilityComponent aiUtilComp = utility;
		if (!aiUtilComp)
			return;
		
		IEntity owner =	aiUtilComp.m_OwnerEntity;
		if (owner)
		{
			m_DamageManager = SCR_DamageManagerComponent.Cast(owner.FindComponent(SCR_DamageManagerComponent));
			m_AIInfo = aiUtilComp.m_AIInfo;
		}
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.DANGER_MEDIUM;
	}	
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Unit healed", EAIDebugCategory.INFO, 5);
#endif
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		if (!(GetGame().GetWorld().GetWorldTime() - m_fTimeCreated_ms > m_fPriorityDelay_ms))
			return 0;
		
		// We should heal ourselves with high priority if time to uncon is lower then treshold
		if (m_AIInfo && m_AIInfo.GetBleedTimeToUnconscious() < MAX_TIME_TO_UNCON_HIGH_PRIORITY_S)
			return PRIORITY_BEHAVIOR_HEAL_HIGH_PRIORITY;
		
		if (m_Utility.m_ThreatSystem.GetThreatMeasureWithoutInjuryFactor() < SCR_AIThreatSystem.VIGILANT_THRESHOLD)
			return GetPriority();
		
			return 0;
	}
};