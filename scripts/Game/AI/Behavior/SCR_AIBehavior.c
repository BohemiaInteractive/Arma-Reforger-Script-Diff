enum SCR_EAIBehaviorCause
{
	SAFE = 10,			// There's nothing better to do and we are safe
	SELF_AID = 20,		// Minor actions related to caring about ourselves
	GROUP_GOAL = 30,	// Doing something on behalf of group
	DANGER_LOW = 40,	// Reacting to dangers of not immediate potential death, like gunshots
	COMBAT = 50,		// Combat related behaviors
	DANGER_MEDIUM = 60,	// Avoiding non-immediate certain death (healing injury)
	DANGER_HIGH = 70,	// Avoiding immediate certain death (car, grenade)
	ALWAYS = 1000 		// Special value for settings to run in all behaviors
}

class SCR_AIBehaviorBase : SCR_AIActionBase
{
	SCR_AIUtilityComponent m_Utility;
	
	float m_fThreat = 0.0; // This threat value will be added to the calculated threat level in threat system
	bool m_bAllowLook = true;
	bool m_bResetLook = false;
	bool m_bUseCombatMove = false;
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIBehaviorBase(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity)
	{
		m_Utility = utility;
		SetRelatedGroupActivity(groupActivity);
		if (groupActivity)
			groupActivity.OnChildBehaviorCreated(this);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	SCR_AIActivityBase GetGroupActivityContext()
	{
		return SCR_AIActivityBase.Cast(GetRelatedGroupActivity());
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		if (m_bResetLook)
			m_Utility.m_LookAction.Cancel();
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	#ifdef AI_DEBUG
    override protected void AddDebugMessage(string str)
	{
		if (!m_Utility)
			return;
		
		AIAgent ownerAgent = m_Utility.GetOwner();
		if (!ownerAgent)
			return;
		
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(ownerAgent.FindComponent(SCR_AIInfoBaseComponent));
		if (!infoComp)
			return;
		
		infoComp.AddDebugMessage(string.Format("%1: %2", this, str), msgType: EAIDebugMsgType.ACTION);
	}
	#endif
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		SCR_AIActivityBase relatedActivity = SCR_AIActivityBase.Cast(GetRelatedGroupActivity());
		if (relatedActivity)
			relatedActivity.OnChildBehaviorFinished(this);
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		SCR_AIActivityBase relatedActivity = SCR_AIActivityBase.Cast(GetRelatedGroupActivity());
		if (relatedActivity)
			relatedActivity.OnChildBehaviorFinished(this);
	}
	
};

//---------------------------------------------------------------------------------------------------------------------------------
class SCR_AIWaitBehavior : SCR_AIBehaviorBase
{	
	void SCR_AIWaitBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Wait.bt";
		SetPriority(PRIORITY_BEHAVIOR_WAIT);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.SAFE;
	}	
};

//---------------------------------------------------------------------------------------------------------------------------------
class SCR_AIIdleBehavior : SCR_AIBehaviorBase
{
	void SCR_AIIdleBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity)
	{
		if (!utility)
			return;
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Idle.bt";
		SetPriority(PRIORITY_BEHAVIOR_IDLE);
	}
	
	override void OnActionSelected()
	{
		super.OnActionSelected();
		
		// Temporary solution to make AI select some generic weapon suitable against infantry, like a rifle.
		m_Utility.m_CombatComponent.SetExpectedEnemyType(EAIUnitType.UnitType_Infantry);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.SAFE;
	}	
};
