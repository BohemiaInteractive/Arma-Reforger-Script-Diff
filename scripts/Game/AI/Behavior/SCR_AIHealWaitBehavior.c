/*!
Behavior for waiting until AI is healed by someone else.
*/

class SCR_AIHealWaitBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParam<IEntity> m_HealProvider = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.DANGER_MEDIUM;
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void InitParameters(IEntity healProvider)
	{
		m_HealProvider.Init(this, healProvider);
	}
	
	//-------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIHealWaitBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, IEntity healProvider, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(healProvider);
		SetPriority(PRIORITY_BEHAVIOR_HEAL_WAIT);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_sBehaviorTree = "{AAB70A7FFF8BB63C}AI/BehaviorTrees/Chimera/Soldier/Behavior_HealWait.bt";
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Unit healed externally", EAIDebugCategory.INFO, 5);
#endif
	}
};