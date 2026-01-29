class SCR_AIPerformActionBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParam<SCR_AISmartActionComponent> m_SmartActionComponent = new SCR_BTParam<SCR_AISmartActionComponent>(SCR_AIActionTask.SMARTACTION_PORT);
		
	//------------------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIPerformActionBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, SCR_AISmartActionComponent smartActionComponent, float priority = PRIORITY_BEHAVIOR_PERFORM_ACTION, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/PerformAction.bt";
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_SmartActionComponent.Init(this, smartActionComponent);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.SAFE;
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " using smart action " + m_SmartActionComponent.m_Value.ToString();
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		if (m_SmartActionComponent.m_Value)
			m_SmartActionComponent.m_Value.ReleaseAction();
	}
};
