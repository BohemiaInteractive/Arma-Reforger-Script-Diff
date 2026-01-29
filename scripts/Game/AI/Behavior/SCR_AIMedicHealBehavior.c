// AI healing behaviour
// TODO: You have to handle situation, in which movement can be disabled (in AIConfig component)
class SCR_AIMedicHealBehavior : SCR_AIBehaviorBase
{
	SCR_AIConfigComponent m_Config;
	
	ref SCR_BTParam<IEntity> m_EntityToHeal = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
	ref SCR_AIMoveIndividuallyBehavior m_HealMove;
	
	// Max threat value under which we will consider healing someone
	protected const float MAX_THREAT_THRESHOLD = 0.002;
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIMedicHealBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, IEntity entityToHeal, bool allowHealMove, float priority = PRIORITY_BEHAVIOR_MEDIC_HEAL, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_EntityToHeal.Init(this, entityToHeal);
		m_sBehaviorTree = "{990FE3889BBA5839}AI/BehaviorTrees/Chimera/Soldier/MedicHeal.bt";
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_bAllowLook = false;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.GROUP_GOAL;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.BUSY);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
		
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Unit healed", EAIDebugCategory.INFO, 5);
#endif
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
		
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Failed heal", EAIDebugCategory.INFO, 5);
#endif
	}
};
