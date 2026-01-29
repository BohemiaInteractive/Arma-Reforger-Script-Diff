class SCR_AIAttackBehavior : SCR_AIBehaviorBase
{
#ifdef WORKBENCH
	ref Shape m_Shape;
#endif

	// Be careful, here we want to store a ref to BaseTarget
	ref SCR_BTParamRef<BaseTarget> m_Target = new SCR_BTParamRef<BaseTarget>(SCR_AIActionTask.TARGET_PORT);
	
	// Direction to target which is calculated when the behavior is created. This is used to calculate flanking path.
	ref SCR_BTParam<vector> m_vInitialDirToTgt = new SCR_BTParam<vector>("InitialDirToTgt");
	
	// Wait time before we start shooting
	ref SCR_BTParam<float> m_fWaitTime = new SCR_BTParam<float>("WaitTime");
	
    SCR_AIWorld m_AIWorld;
	SCR_AICombatComponent m_CombatComponent;
	
	bool m_bSelected = false;
	bool m_bCloseRange = false;

	// This delay is executed before shooting starts
	protected static const float WAIT_TIME_UNEXPECTED = 0.25;
	protected static const float WAIT_TIME_OVERTHREATENED = 0.8;
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.COMBAT;
	}
	
	//----------------------------------------------------------------------------------
	void InitParameters(BaseTarget target, float waitTime)
	{
		m_Target.Init(this, target);
		m_fWaitTime.Init(this, waitTime);
		m_vInitialDirToTgt.Init(this, vector.Zero);
	}
	
	//----------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		m_bSelected = true;
		EnableFriendlyFireCheck(true);
		
		// Update target selection properties if in close combat
		m_bCloseRange = m_Target.m_Value.GetDistance() < SCR_AICombatComponent.CLOSE_RANGE_COMBAT_DISTANCE;
		m_CombatComponent.SetTargetSelectionProperties(m_bCloseRange);
	}
	
	//----------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		EnableFriendlyFireCheck(false);
		
		// Reset target selection properties
		m_bCloseRange = false;
		m_CombatComponent.SetTargetSelectionProperties(m_bCloseRange);
	}

	//----------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		EnableFriendlyFireCheck(false);
	}
	
	//----------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		EnableFriendlyFireCheck(false);
	}
	
	//----------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " attacking " + m_Target.m_Value.ToString();
	}
	
	//----------------------------------------------------------------------------------	
	void SCR_AIAttackBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, BaseTarget target, BaseTarget prevTarget, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(target, 4);
		if (!utility)
			return;
		
		m_fPriorityLevel.m_Value = priorityLevel;
		SetPriority(PRIORITY_BEHAVIOR_ATTACK_NOT_SELECTED);
		m_fThreat = 1.01 * SCR_AIThreatSystem.VIGILANT_THRESHOLD;
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Attack.bt";
		m_bAllowLook = false;
		m_bResetLook = true;
		SetIsUniqueInActionQueue(false);
		m_AIWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		m_CombatComponent = utility.m_CombatComponent;
		
		// Init wait time, but only if we didn't have target before.
		// Otherwise if we are switching between targets, it should be instant.
		if (!prevTarget)
			InitWaitTime(utility);
		else
			m_fWaitTime.m_Value = 0;
		
		// Init initial direction to target, used for flanking
		vector dirToTgt = target.GetLastSeenPosition() - utility.m_OwnerEntity.GetOrigin();
		dirToTgt.Normalize();
		m_vInitialDirToTgt.m_Value = dirToTgt;
	}
	
	//----------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		BaseTarget baseTarget = m_Target.m_Value;
		
		// Return 0 if it's not current target selected by weapon&target selector
		// Since weapon&target selector selects both weapon and target, if we chose to attack a different target,
		// it might happen that we use a wrong weapon
		if (baseTarget != m_CombatComponent.GetCurrentTarget())
			return 0;
		
		// Update m_bUseCombatMove
		m_bUseCombatMove = true;
		
		if (m_Utility.m_AIInfo.HasUnitState(EUnitState.IN_TURRET))
			m_bUseCombatMove = false;
		else if (m_Utility.m_AIInfo.HasUnitState(EUnitState.IN_VEHICLE)) // inside vehicle we dont attack unless in turret
			return 0;
		
		// Update target selection properties if needed
		bool closeRange = baseTarget.GetDistance() < SCR_AICombatComponent.CLOSE_RANGE_COMBAT_DISTANCE;
		if (closeRange != m_bCloseRange)
		{
			m_bCloseRange = closeRange;
			m_CombatComponent.SetTargetSelectionProperties(m_bCloseRange);
		}
		
		// Combat mode?
		if (m_CombatComponent.GetCombatMode() == EAIGroupCombatMode.HOLD_FIRE)
			return PRIORITY_BEHAVIOR_ATTACK_DISREGARD_THREATS;
		
		float targetScore = m_Utility.m_CombatComponent.m_WeaponTargetSelector.CalculateTargetScore(baseTarget);
		
		if (baseTarget.IsEndangering() || baseTarget.GetTimeSinceEndangered() < SCR_AICombatComponent.TARGET_ENDANGERED_TIMEOUT_S)
			targetScore *= SCR_AICombatComponent.ENDANGERING_TARGET_SCORE_MULTIPLIER;
		
		if (targetScore >= SCR_AICombatComponent.TARGET_SCORE_HIGH_PRIORITY_ATTACK)
			return PRIORITY_BEHAVIOR_ATTACK_HIGH_PRIORITY;
			
		if (m_bSelected)
			return PRIORITY_BEHAVIOR_ATTACK_SELECTED;
		
		return PRIORITY_BEHAVIOR_ATTACK_NOT_SELECTED;
	}
	
	// Sets the delay until shooting starts.
	protected void InitWaitTime(SCR_AIUtilityComponent utility)
	{
		float threatMeasure = utility.m_ThreatSystem.GetThreatMeasure();
		
		// Delay depending on threat
		float threatDelay;
		if (threatMeasure < SCR_AIThreatSystem.ATTACK_DELAYED_THRESHOLD)
			threatDelay = WAIT_TIME_UNEXPECTED;
		else if (threatMeasure < SCR_AIThreatSystem.THREATENED_THRESHOLD)
			threatDelay = 0;
		else
			threatDelay = WAIT_TIME_OVERTHREATENED;
		
		// Delay depending on distance
		// 0m - 0ms
		// 100m - 340ms
		// 300m - 700ms
		// 500m - 870ms
		// 800m - 1018ms
		float distance = vector.Distance(m_Utility.m_OwnerEntity.GetOrigin(), m_Target.m_Value.GetLastSeenPosition());
		float distanceDelay = (1.4 * distance) / (300 + distance);
		
		m_fWaitTime.m_Value = threatDelay + distanceDelay;
	}
	
	void EnableFriendlyFireCheck(bool enable)
	{
		if (m_Utility && m_Utility.m_PerceptionComponent)
			m_Utility.m_PerceptionComponent.SetFriendlyFireCheck(enable);
	}
};