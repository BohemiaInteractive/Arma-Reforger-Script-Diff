class SCR_AIDefendBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParam<AIWaypoint> m_RelatedWaypoint = new SCR_BTParam<AIWaypoint>(SCR_AIActionTask.WAYPOINT_PORT);
	ref SCR_BTParam<vector> m_fDefendLocation = new SCR_BTParam<vector>(SCR_AIActionTask.DEFEND_LOCATION_PORT);
	ref SCR_BTParam<float> m_fDefendAngularRange = new SCR_BTParam<float>(SCR_AIActionTask.ANGULAR_RANGE_PORT);
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.COMBAT;
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	void InitParameters(AIWaypoint relatedWaypoint, vector defendLocation, float defendAngularRange)
	{
		m_RelatedWaypoint.Init(this, relatedWaypoint);
		m_fDefendLocation.Init(this, defendLocation);
		m_fDefendAngularRange.Init(this, defendAngularRange);
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIDefendBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, AIWaypoint relatedWaypoint, vector location, float angularRange, float priority = PRIORITY_BEHAVIOR_DEFEND, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Defend.bt";
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
		InitParameters(relatedWaypoint, location, angularRange);
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " defending " + m_RelatedWaypoint.m_Value.ToString();
	}
};
