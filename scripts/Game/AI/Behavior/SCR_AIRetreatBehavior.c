class SCR_AIRetreatWhileLookAtBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParamAssignable<vector> m_Target = new SCR_BTParamAssignable<vector>("RetreatToPoint");
	ref SCR_BTParamAssignable<vector> m_LookAt = new SCR_BTParamAssignable<vector>("LookAt");
	
	bool m_TimeSet = false;
	float m_Timestamp = 0;
	const float m_MaxTime = 3;
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIRetreatWhileLookAtBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		vector tempPos;
		m_Target.Init(this, tempPos);
		m_LookAt.Init(this, tempPos);
		 
		m_sBehaviorTree = "{9673533977BD9950}AI/BehaviorTrees/Chimera/Soldier/RetreatWhileLookAt.bt";
		SetPriority(PRIORITY_BEHAVIOR_RETREAT_MELEE);
		m_fPriorityLevel.m_Value = priorityLevel;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.COMBAT;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		if(!m_TimeSet)
		{
			m_Timestamp = GetGame().GetWorld().GetWorldTime(); 
			m_TimeSet = true;
		}
		
		//get timeslice
		float current = GetGame().GetWorld().GetWorldTime();
		
		float secondsPassed = (current - m_Timestamp)/1000;
		
		//if the timer has passed, call Fail()
		if(secondsPassed >= m_MaxTime)
		{
			Fail();
		}
		
		return GetPriority();
	}
};



//----------------------------------------------------------------------------------------------------------------------------------------------------------
class SCR_AIRetreatFromTargetBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParamRef<BaseTarget> m_RetreatFromTarget = new SCR_BTParamRef<BaseTarget>("RetreatFromTarget");
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIRetreatFromTargetBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, notnull BaseTarget retreatFromTarget, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_RetreatFromTarget.Init(this, retreatFromTarget);
		m_sBehaviorTree = "{91B8D5FDB60C1C80}AI/BehaviorTrees/Chimera/Soldier/RetreatFromTarget.bt";
		SetPriority(PRIORITY_BEHAVIOR_RETREAT_FROM_TARGET);
		m_fPriorityLevel.m_Value = priorityLevel;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.COMBAT;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		if (m_RetreatFromTarget.m_Value.GetTargetCategory() != ETargetCategory.ENEMY)
		{
			Complete();
			return 0;
		}
		else
			return GetPriority();
	}
}

class SCR_AIGetRetreatFromTargetBehaviorParameters: SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIRetreatFromTargetBehavior(null, null, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};
