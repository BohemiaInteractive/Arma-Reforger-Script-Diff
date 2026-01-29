class SCR_AIMoveBehaviorBase : SCR_AIBehaviorBase
{
	ref SCR_BTParam<vector> m_vPosition = new SCR_BTParam<vector>(SCR_AIActionTask.POSITION_PORT);
	
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveBehaviorBase(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector pos, float priority = PRIORITY_BEHAVIOR_MOVE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_vPosition.Init(this, pos);
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.SAFE;
	}
};

//-----------------------------------------------------------------------------------------------------
class SCR_AIMoveInFormationBehavior : SCR_AIBehaviorBase
{
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveInFormationBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/KeepInFormation.bt";
		
		m_bAllowLook = priorityLevel == 0;
		m_bResetLook = true;
		
		m_fPriorityLevel.m_Value = priorityLevel;
	}
	
	//-----------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		if (!m_Utility.IsSubformationLeader() && m_Utility.GetNearSubformationLeader() && !m_Utility.GetSubformationLeaderMoving())
		{
			// Leader is not moving, and we are close -> lower the priority, we can do something else if it bothers us
			return PRIORITY_BEHAVIOR_MOVE_IN_FORMATION_LOW_PRIORITY;
		}
		else
		{
			// We are either far or leader is moving, or we are subformation leader, we should keep moving
			return PRIORITY_BEHAVIOR_MOVE_IN_FORMATION;
		}
	}
};

//-----------------------------------------------------------------------------------------------------
class SCR_AIMoveIndividuallyBehavior : SCR_AIMoveBehaviorBase
{
	ref SCR_BTParamAssignable<IEntity> m_Entity = new SCR_BTParamAssignable<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParamAssignable<float> m_Radius = new SCR_BTParamAssignable<float>(SCR_AIActionTask.RADIUS_PORT);
		
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveIndividuallyBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector pos, float priority = PRIORITY_BEHAVIOR_MOVE_INDIVIDUALLY, float priorityLevel = PRIORITY_LEVEL_NORMAL, IEntity ent = null, float radius = 1.0)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/MoveIndividually.bt";
		m_Entity.Init(this, ent);
		if (ent)
			m_vPosition.m_Value = ent.GetOrigin();
		m_Radius.Init(this, radius);
	}
	
	//-----------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " moving to " + m_vPosition.m_Value.ToString();
	}
};

//-----------------------------------------------------------------------------------------------------
class SCR_AIMoveAndInvestigateBehavior : SCR_AIMoveBehaviorBase
{
	ref SCR_BTParam<bool> m_bIsDangerous = new SCR_BTParam<bool>(SCR_AIActionTask.IS_DANGEROUS_PORT);
	ref SCR_BTParam<float> m_fRadius = new SCR_BTParam<float>(SCR_AIActionTask.RADIUS_PORT);
	ref SCR_BTParam<bool> m_bResetTimer = new SCR_BTParam<bool>(SCR_AIActionTask.RESET_TIMER_PORT);
	ref SCR_BTParam<float> m_fTimeOut = new SCR_BTParam<float>(SCR_AIActionTask.TIMEOUT_PORT);
	ref SCR_BTParam<float> m_fDuration = new SCR_BTParam<float>("Duration"); // How much to investigate once we have arrived
	ref SCR_BTParam<vector> m_vStartPos = new SCR_BTParam<vector>("StartPos"); // Our position when the behavior was created
	
	EAIUnitType m_eTargetUnitType;
	float m_fTimeStamp;													// world time when constructor of behavior is called
	bool m_bCanTimeout = true;											// can timeout when not executed?
	protected static const float INVESTIGATION_TIMEOUT_MS = 20000;		// how long it can take NOT to investigate before the investigation becomes obsolete in ms
	
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveAndInvestigateBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector pos, float priority = PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE, float priorityLevel = PRIORITY_LEVEL_NORMAL, float radius = 10, bool isDangerous = true, EAIUnitType targetUnitType = EAIUnitType.UnitType_Infantry, float duration = 10.0)
	{
		m_bIsDangerous.Init(this, isDangerous);
		m_fRadius.Init(this, radius);
		m_bResetTimer.Init(this, false);
		//m_fTimeOut.Init(this, Math.RandomFloat(20,50));
		m_fTimeOut.Init(this, 5.0 * 60.0); // For now it's a reasonable long enough time
		m_fDuration.Init(this, Math.RandomFloat(0.8*duration, 1.2*duration));
		m_eTargetUnitType = targetUnitType;
		m_vStartPos.Init(this, vector.Zero);
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/MoveAndInvestigate.bt";
		if (m_Utility)
		{
			// marking time of creation of this move and investigate (world time)
			m_fTimeStamp = GetGame().GetWorld().GetWorldTime();
		}
		
		// If target is dangerous, during execution of this action we will increase our threat level
		// Aim of this is to be in alerted state through the action, so that when we encounter enemy again,
		// We are not 'surprised' by enemy and there will be no extra delay added
		if (isDangerous)
			m_fThreat = 1.01 * SCR_AIThreatSystem.VIGILANT_THRESHOLD;
	}
	
	//-----------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		m_bCanTimeout = false;
		m_Utility.m_CombatComponent.SetExpectedEnemyType(m_eTargetUnitType);
		
		// Initialize m_vStartPos value
		if (m_vStartPos.m_Value == vector.Zero)
			m_vStartPos.m_Value = m_Utility.m_OwnerEntity.GetOrigin();
	}
};

//-----------------------------------------------------------------------------------------------------
class SCR_AISetInvestigateBehaviorParameters : SCR_AISetActionParameters
{
	protected static ref TStringArray s_aVarsIn = (new SCR_AIMoveAndInvestigateBehavior(null, null, vector.Zero)).GetPortNames();
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	static override bool VisibleInPalette() { return true; }
};
