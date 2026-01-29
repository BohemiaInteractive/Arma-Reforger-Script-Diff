//------------------------------------------------------------------------------------------------
// TARGET REACTION BASE
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_AITargetReactionBase
{
	void PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, BaseTarget baseTarget, vector lastSeenPosition) {}
};

//! This reaction is called every time selected target changes from anything to anything, including null
[BaseContainerProps()]
class SCR_AITargetReaction_SelectedTargetChangedBase
{
	void PerformReaction(notnull SCR_AIUtilityComponent utility, BaseTarget prevTarget, BaseTarget newTarget);
}

//------------------------------------------------------------------------------------------------
// TARGET REACTIONS
//------------------------------------------------------------------------------------------------


[BaseContainerProps()]
class SCR_AITargetReaction_Unknown : SCR_AITargetReactionBase
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, BaseTarget baseTarget, vector lastSeenPosition)
	{
		IEntity targetEntity = baseTarget.GetTargetEntity();
		if (targetEntity)
			utility.m_LookAction.LookAt(targetEntity, SCR_AILookAction.PRIO_UNKNOWN_TARGET, 2.0);
	}
};

[BaseContainerProps()]
class SCR_AITargetReaction_RetreatFromEnemy : SCR_AITargetReactionBase
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, BaseTarget baseTarget, vector lastSeenPosition)
	{
		if (!baseTarget)
			return;
		
		// Find if we already have a retreat action from that target
		utility.SetStateAllActionsOfType(SCR_AIRetreatFromTargetBehavior, EAIActionState.COMPLETED, true);
		
		// Don't retreat if we are a car driver
		if (utility.m_AIInfo.HasUnitState(EUnitState.PILOT))
			return;
		
		SCR_AIRetreatFromTargetBehavior behavior = new SCR_AIRetreatFromTargetBehavior(utility, null, baseTarget);
		utility.AddAction(behavior);
		
		// Cancel all investigations, if we have discovered something which we can't attack, it makes no sense to push further
		utility.SetStateAllActionsOfType(SCR_AIMoveAndInvestigateBehavior, EAIActionState.FAILED);
	}
}

//! Performs reporting of selected target if it changes
[BaseContainerProps()]
class SCR_AITargetReaction_SelectedTargetChanged : SCR_AITargetReaction_SelectedTargetChangedBase
{
	//--------------------------------------------------------------------------------------------------
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, BaseTarget prevTarget, BaseTarget newTarget)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(utility, string.Format("Prev Target: %1, New Target: %2", prevTarget, newTarget));
		#endif
		
		if (prevTarget)
		{
			ReportPreviousTarget(utility, prevTarget, newTarget);
		}
		
		utility.SetStateAllActionsOfType(SCR_AIAttackBehavior, EAIActionState.COMPLETED, true);
		
		if (newTarget)
		{
			utility.SetStateAllActionsOfType(SCR_AIRetreatFromTargetBehavior, EAIActionState.COMPLETED, true);
			CreateAttackActionForTarget(utility, newTarget, prevTarget);
			
			IEntity targetEntity = newTarget.GetTargetEntity();
			if (targetEntity)
				utility.m_LookAction.LookAt(targetEntity, SCR_AILookAction.PRIO_ENEMY_TARGET);
		}
	}
	
	//--------------------------------------------------------------------------------------------------
	protected void CreateAttackActionForTarget(notnull SCR_AIUtilityComponent utility, notnull BaseTarget target, BaseTarget prevTarget)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(utility, "CreateAttackActionForTarget()");
		#endif
				
		IEntity targetEntity = target.GetTargetEntity();
		if (!targetEntity)
			return;
		
		// Create new attack behavior
		auto behavior = new SCR_AIAttackBehavior(utility, null, target, prevTarget);
		
		// AddAction must be used here, not AddActionIfMissing!
		utility.AddAction(behavior);
		
		// this is now disabled since group commands when we dismount
		// If passenger and not in turret, dismount vehicle
		//if (!utility.m_AIInfo.HasUnitState(EUnitState.IN_TURRET))
		//	utility.WrapBehaviorOutsideOfVehicle(behavior);
	}
	
	//--------------------------------------------------------------------------------------------------
	//! Reports what happened with the previous target: was lost or destroyed
	protected void ReportPreviousTarget(notnull SCR_AIUtilityComponent utility, notnull BaseTarget prevTarget, BaseTarget newTarget)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(utility, "ReportPreviousTarget()");
		#endif
		
		IEntity targetEntity = prevTarget.GetTargetEntity();
		ETargetCategory targetCategory = prevTarget.GetTargetCategory();
		DamageManagerComponent damageMgr = prevTarget.GetDamageManagerComponent();
		
		if (targetCategory == ETargetCategory.FRIENDLY)
		{
			#ifdef AI_DEBUG
			AddDebugMessage(utility, "  Target is now friendly");
			#endif
		}
		else if (!targetEntity)
		{
			#ifdef AI_DEBUG
			AddDebugMessage(utility, "  Target entity is null, not reporting");
			#endif
		}
		else if (prevTarget.IsDisarmed() || (damageMgr && damageMgr.IsDestroyed()))
		{
			// Target is destroyed or unconscious/disarmed (we pretend it's destroyed)
			#ifdef AI_DEBUG
			AddDebugMessage(utility, "  Target is destroyed or disarmed");
			#endif
			
			// Report target destroyed
			if (!utility.m_CommsHandler.CanBypass())
			{
				SCR_AITalkRequest rq = new SCR_AITalkRequest(ECommunicationType.REPORT_TARGET_DOWN, null, vector.Zero, 0, transmitIfNoReceivers: false, transmitIfPassenger: false, preset: SCR_EAITalkRequestPreset.IRRELEVANT_IMMEDIATE);
				utility.m_CommsHandler.AddRequest(rq);
			}
		}
		else if (!newTarget &&
			(prevTarget.GetTimeSinceSeen() > SCR_AICombatComponent.TARGET_MAX_LAST_SEEN) &&
			(prevTarget.GetUnitType() & utility.m_CombatComponent.GetUnitTypesCanAttack() != 0)) // We switched from this target not because we can't attack it any more
		{
			// We haven't seen the target for too long
			#ifdef AI_DEBUG
			AddDebugMessage(utility, string.Format("  Target is lost, time since seen: %1", prevTarget.GetTimeSinceSeen()));
			#endif
			
			// Report target lost
			if (!utility.m_CommsHandler.CanBypass())
			{
				SCR_AITalkRequest rq = new SCR_AITalkRequest(ECommunicationType.REPORT_TARGET_LOST, null, vector.Zero, 0, transmitIfNoReceivers: false, transmitIfPassenger: false, preset: SCR_EAITalkRequestPreset.IRRELEVANT_IMMEDIATE);
				utility.m_CommsHandler.AddRequest(rq);
			}				
		}
	}
	
	#ifdef AI_DEBUG
	protected void AddDebugMessage(notnull SCR_AIUtilityComponent utility, string str, LogLevel logLevel = LogLevel.NORMAL)
	{
		SCR_AIInfoBaseComponent infoComp = utility.m_AIInfo;
		infoComp.AddDebugMessage("SCR_AITargetReaction_SelectedTargetChanged: " + str, msgType: EAIDebugMsgType.COMBAT, logLevel);
	}
	#endif
}