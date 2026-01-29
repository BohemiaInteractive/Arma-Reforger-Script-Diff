/*
This node analyzes combat move state and issues new requests for combat move tree.
*/

//! Base class for combat movement. This is the main piece of logic for combat movement during combat behavior and suppression behavior.
//! But in this class the specific type of target is not yet used, and there are virtual methods which must be overridden in derived classes.
class SCR_AICombatMoveLogicBase : AITaskScripted
{
	protected SCR_AICombatMoveState m_State;
	protected SCR_AIUtilityComponent m_Utility;
	protected SCR_AICombatComponent m_CombatComp;
	protected IEntity m_MyEntity;
	protected CharacterControllerComponent m_CharacterController;
	
	// Values updated on each update, to avoid passing them through calls
	protected EAIThreatState m_eThreatState;
	protected ECharacterStance m_eStance;
	protected EWeaponType m_eWeaponType;
	protected float m_fTargetDist;
	protected float m_fWeaponMinDist;
	protected bool m_bCloseRangeCombat;	// True if we consider it's a close range fight
	protected bool m_bVeryLongRangeCombat; // True if we consider it's a very long range fight
	
	protected float m_fNextUpdate_ms;
	[Attribute("500")]
	protected float m_fUpdateInterval_ms;
	
	// Half-angle of cover query sector when directed cover search is used
	protected const float COVER_QUERY_SECTOR_ANGLE_RAD = 0.3 * Math.PI;
	
	//--------------------------------------------------------------------------------------------
	// These methods must be overriden in derived classes.
	protected bool OnUpdate(AIAgent owner, float dt); // This gets called before the rest of the logic. Here you should read data from ports, return true on success.
	protected vector ResolveRequestTargetPos();
	protected bool ResolveFailMoveIfNoCover(); // Gets called from PushRequestMove, this should return value of request.m_bFailIfNoCover
	protected float ResolveStoppedWaitTime(bool inCover, EAIThreatState threat, EWeaponType weaponType);
	protected vector GetTargetPosition();
	protected float GetTargetDistance();
	protected bool MoveToNextPosCondition();
	protected vector GetAvoidStraightPathDir() { return vector.Zero; }; // Must return vector for straight path avoidance (flanking)
	//--------------------------------------------------------------------------------------------
	
	
	//--------------------------------------------------------------------------------------------
	protected override void OnInit(AIAgent owner)
	{
		m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (m_Utility)
			m_State = m_Utility.m_CombatMoveState;
		
		m_MyEntity = owner.GetControlledEntity();
		
		if (m_MyEntity)
		{
			m_CharacterController = CharacterControllerComponent.Cast(m_MyEntity.FindComponent(CharacterControllerComponent));
			m_CombatComp = SCR_AICombatComponent.Cast(m_MyEntity.FindComponent(SCR_AICombatComponent));
		}
	}
	
	protected override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		float currentTime_ms = GetGame().GetWorld().GetWorldTime();
		if (currentTime_ms < m_fNextUpdate_ms)
			return ENodeResult.RUNNING;
		m_fNextUpdate_ms = currentTime_ms + m_fUpdateInterval_ms;
		
		if (!OnUpdate(owner, dt))
			return ENodeResult.FAIL;
		
		if (!m_State || !m_MyEntity || !m_Utility || !m_CombatComp || !m_CharacterController)
			return ENodeResult.FAIL;
		
		// Don't run combat movement logic if CombatMove BT is not used now (like in turret)
		SCR_AIBehaviorBase executedBehavior = SCR_AIBehaviorBase.Cast(m_Utility.GetExecutedAction());
		if (executedBehavior && !executedBehavior.m_bUseCombatMove)
			return ENodeResult.RUNNING;
		
		// Update cached variables
		m_fTargetDist = GetTargetDistance();
		m_bCloseRangeCombat = m_fTargetDist < SCR_AICombatMoveUtils.CLOSE_RANGE_COMBAT_DIST;
		m_bVeryLongRangeCombat = m_fTargetDist > SCR_AICombatMoveUtils.VERY_LONG_RANGE_COMBAT_DIST;
		m_eThreatState = m_Utility.m_ThreatSystem.GetState();
		m_eStance = m_CharacterController.GetStance();
		m_fWeaponMinDist = m_CombatComp.GetSelectedWeaponMinDist();
		m_eWeaponType = m_CombatComp.GetSelectedWeaponType();
		
		
		/*		
		//------------------------------------------------------------------------------------
		Combat movement logic
		
		Conditions represent states inside which we want to remain.
		
		Conditions are organized based on their priority, highest first.
		
		Within each state there can be extra logic which decides if it's worth to
		send a new request, because even though we have selected a state, we should avoid
		spamming same request over and over.
		
		Conditions for states mostly depend on Combat Move State and its timers.
		
		It is important to write logic in such a way that it doesn't depend on state
		of this node. In this case the state flow also doesn't depend on it, and AI
		does movement is more fluent when switching to a new behavior which also utilizes
		combat movement, including attacking a different target.
		*/
		
		if (SuppressedInCoverCondition())
		{
			SuppressedInCoverLogic();
		}
		else if (MoveFromTargetCondition())
		{
			// Too close to target
			// Step away
			if (MoveFromTargetNewRequestCondition())
				PushRequestMoveFromTarget();
		}
		else if (CurrentCoverUselessCondition())
		{
			// Current cover has been compromised, it's not directed at enemy any more
			// Find a new cover nearby
			PushRequestLeaveUselessCover();
		}
		else if (m_State.m_bInCover && m_CharacterController.IsReloading())
		{
			// We're reloading and can't do much else now
			// Hide in cover
			if (m_State.m_bExposedInCover)
				m_State.ApplyRequestChangeStanceInCover(false);
		}
		else if (m_State.m_bInCover && !m_State.m_bExposedInCover)
		{
			// We're in cover but we are still hiding in it, unhide
			m_State.ApplyRequestChangeStanceInCover(true);
		}
		else if (FFAvoidanceCondition())
		{
			if (FFAvoidanceNewRequestCondition())
				PushRequestFFAvoidance();
		}
		else if (MoveToNextPosCondition())
		{
			// We've waited here too long, move to next place
			PushRequestMove();
		}
		else if (!m_State.IsExecutingRequest() && !m_State.m_bInCover)
		{
			// We are stopped and not in cover, manage our stance
			ECharacterStance newStance = ResolveStanceOutsideCover(m_bCloseRangeCombat, m_eThreatState);
			if (newStance > m_eStance)
			{
				// Only let stance go down, no need to get back up
				m_State.ApplyRequestChangeStanceOutsideCover(newStance);
			}
		}
		
		return ENodeResult.RUNNING;
	}
	
	
	//--------------------------------------------------------------------------------------------
	// Friendly fire avoidance
	
	// Friendly fire avoidance condition
	protected bool FFAvoidanceCondition()
	{
		// Should we deal with friendly fire avoidance?
		
		// Ignore if we don't want to aim at all
		if (!m_State.m_bAimAtTarget)
			return false;
		
		// True only when not in cover
		if (m_State.m_bInCover)
			return false;
		
		// True when not moving to cover
		if (m_State.IsMovingToCover())
			return false;
		
		// True when friendly is in aim
		return m_Utility.m_CombatComponent.IsFriendlyInAim();
	}
	
	// Friendly fire avoidance condition for pushing new request
	protected bool FFAvoidanceNewRequestCondition()
	{
		if (!m_State.IsExecutingRequest())
			return true;
		
		// Still executing ...
		// Send new request only if we are executing NOT side-step
		SCR_AICombatMoveRequest_Move rq = SCR_AICombatMoveRequest_Move.Cast(m_State.GetRequest());
		if (!rq)
			return true;
		
		return rq.m_eReason != SCR_EAICombatMoveReason.FF_AVOIDANCE;
	}
	
	protected void PushRequestFFAvoidance()
	{
		SCR_AICombatMoveRequest_Move rq = new SCR_AICombatMoveRequest_Move();
		
		rq.m_eReason = SCR_EAICombatMoveReason.FF_AVOIDANCE;
		
		// If prev. request was FF avoidance too, keep direction.
		// Otherwise choose a new direction.
		SCR_AICombatMoveRequest_Move prevRequest = SCR_AICombatMoveRequest_Move.Cast(m_State.GetRequest());
		if (prevRequest && prevRequest.m_eReason == SCR_EAICombatMoveReason.FF_AVOIDANCE)
		{
			rq.m_eDirection = prevRequest.m_eDirection;
		}
		else
		{
			if (Math.RandomIntInclusive(0, 1) == 1)
				rq.m_eDirection = SCR_EAICombatMoveDirection.RIGHT;
			else
				rq.m_eDirection = SCR_EAICombatMoveDirection.LEFT;
		}
		
		rq.m_eStanceMoving = m_CharacterController.GetStance(); // Don't change stance
		rq.m_eStanceEnd = rq.m_eStanceMoving;
		rq.m_vMovePos = ResolveRequestTargetPos();
		rq.m_eMovementType = EMovementType.WALK;
		rq.m_fMoveDuration_s = 1.0;
		rq.m_bAimAtTarget = SCR_AICombatMoveUtils.IsAimingAndMovementPossible(rq.m_eStanceMoving, rq.m_eMovementType);
		rq.m_bAimAtTargetEnd = true;
		
		m_State.ApplyNewRequest(rq);
	}
	
	
	//--------------------------------------------------------------------------------------------
	// Movement
	
	protected void PushRequestMove()
	{		
		SCR_AICombatMoveRequest_Move rq = new SCR_AICombatMoveRequest_Move();
		
		rq.m_eReason = SCR_EAICombatMoveReason.STANDARD;
		
		// Common values
		rq.m_vTargetPos = ResolveRequestTargetPos();
		ResolveMoveRequestMovePosAndDir(rq.m_vTargetPos, rq.m_vMovePos, rq.m_vAvoidStraightPathDir, rq.m_eDirection, rq.m_fCoverSearchSectorHalfAngleRad);
		rq.m_bTryFindCover = true;
		rq.m_bUseCoverSearchDirectivity = true;
		rq.m_bCheckCoverVisibility = true;

		float coverSearchDistMin = 0;
		float coverSearchDistMax = 30;
		float moveDurationMax = 6;
		if (m_bCloseRangeCombat)
		{
			// Close range combat
			
			switch (m_eThreatState)
			{
				case EAIThreatState.THREATENED:
				{
					rq.m_eStanceMoving = ECharacterStance.CROUCH;
					rq.m_eStanceEnd = ECharacterStance.CROUCH;
					coverSearchDistMin = 2.0;
					coverSearchDistMax = 10.0;
					moveDurationMax = 2;
					break;
				}
				default:
				{
					rq.m_eStanceMoving = ECharacterStance.STAND;
					rq.m_eStanceEnd = ECharacterStance.CROUCH;
					coverSearchDistMin = 5.0;
					coverSearchDistMax = 15.0;
					moveDurationMax = 3;
					break;
				}
			}
			
			rq.m_eMovementType = EMovementType.RUN;
			rq.m_bAimAtTarget = SCR_AICombatMoveUtils.IsAimingAndMovementPossible(rq.m_eStanceMoving, rq.m_eMovementType) &&
								IsAimingAndMovingAllowedForWeapon(m_eWeaponType);
			rq.m_bAimAtTargetEnd = true;
		}
		else
		{
			// Long range combat
			
			switch (m_eThreatState)
			{
				case EAIThreatState.THREATENED:
				{
					coverSearchDistMin = 2.0;
					coverSearchDistMax = 20.0;
					moveDurationMax = 2.5;
					rq.m_eStanceMoving = ECharacterStance.CROUCH;
					rq.m_eStanceEnd = ECharacterStance.PRONE;
					break;
				}
				default:
				{
					coverSearchDistMin = 10.0;
					coverSearchDistMax = 30.0;
					moveDurationMax = 4; // Shouldn't be so large because we are sprinting and can't shoot
					rq.m_eStanceMoving = ECharacterStance.CROUCH;
					rq.m_eStanceEnd = ECharacterStance.CROUCH;
					break;
				}
			}
			
			rq.m_eMovementType = EMovementType.SPRINT;
			rq.m_bAimAtTarget = false; // Can't aim at tgt while sprinting
			rq.m_bAimAtTargetEnd = true;
		}
		
		rq.m_bFailIfNoCover = ResolveFailMoveIfNoCover();
		
		// If we are not in cover, min cover search distance is overridden to 0, we should find any cover ASAP
		if (!m_State.m_bInCover)
			coverSearchDistMin = 0;
		
		rq.m_fCoverSearchDistMin = coverSearchDistMin;
		rq.m_fCoverSearchDistMax = coverSearchDistMax;
		rq.m_fMoveDuration_s = Math.RandomFloat(0.5, 1.0) * moveDurationMax;
		
		// Subscribe to events
		// We will pronounce voice lines once we start or end moving
		rq.GetOnMovementStarted().Insert(OnMovementStarted);
		rq.GetOnCompleted().Insert(OnMovementCompleted);
		
		m_State.ApplyNewRequest(rq);
	}
	
	protected static void OnMovementStarted(SCR_AIUtilityComponent utility, SCR_AICombatMoveRequest_Move rq, vector pos, bool destinationIsCover)
	{
		if (!utility.m_CommsHandler.CanBypass())
		{
			SCR_AITalkRequest talkRq = new SCR_AITalkRequest(ECommunicationType.REPORT_MOVING, null, vector.Zero, 0, false, false, SCR_EAITalkRequestPreset.IRRELEVANT_IMMEDIATE);
			utility.m_CommsHandler.AddRequest(talkRq);
		}
	}
	
	protected static void OnMovementCompleted(SCR_AIUtilityComponent utility, SCR_AICombatMoveRequestBase rq)
	{
		if (!utility.m_CommsHandler.CanBypass())
		{
			SCR_AITalkRequest talkRq = new SCR_AITalkRequest(ECommunicationType.REPORT_COVERING, null, vector.Zero, 0, false, false, SCR_EAITalkRequestPreset.IRRELEVANT_IMMEDIATE);
			utility.m_CommsHandler.AddRequest(talkRq);
		}
	}
	
	// Resolves which move pos and dir. we should use for _MOVE_ request
	// By now rq.m_vTargetPos must be already calculated!
	protected void ResolveMoveRequestMovePosAndDir(vector targetPos, out vector outMovePos, out vector outAvoidStraightPathDir, out SCR_EAICombatMoveDirection outDirection, out float outCoverSearchSectorHalfAngleRad)
	{	
		AIWaypoint wp = null;
		AIAgent agent = m_Utility.GetAIAgent();
		AIGroup group = agent.GetParentGroup();
		if (group)
			wp = group.GetCurrentWaypoint();
		
		vector movePos;
		SCR_EAICombatMoveDirection eDirection;
		float coverSearchSectorHalfAngleRad;
		vector avoidStraightPathDir;
		
		if (!wp || SCR_EntityWaypoint.Cast(wp))
		{
			// No waypoint, or it's an entity-associated waypoint, like Follow waypoint.
			// Therefore use standard movement logic.
			// Otherwise they will want to run towards position where the waypoint is placed, which makes no sense.
			movePos = targetPos;
			eDirection = SCR_EAICombatMoveDirection.CUSTOM_POS; // Move to target
			avoidStraightPathDir = GetAvoidStraightPathDir(); // Use flanking
			
			if (IsFirstExecution())
					coverSearchSectorHalfAngleRad = Math.PI; // Full circle, on first run we just want any cover if possible
				else
					coverSearchSectorHalfAngleRad = COVER_QUERY_SECTOR_ANGLE_RAD;
		}
		else
		{
			vector wpPos = wp.GetOrigin();
			float wpRadius = wp.GetCompletionRadius();
			bool tgtInWaypoint = vector.DistanceXZ(wpPos, targetPos) < wpRadius;
			float myDistToWp = vector.DistanceXZ(wpPos, m_MyEntity.GetOrigin());
			
			if (myDistToWp > wpRadius)
			{
				// We are outside WP, move towards center
				movePos = wpPos;
				eDirection = SCR_EAICombatMoveDirection.CUSTOM_POS;
				coverSearchSectorHalfAngleRad = COVER_QUERY_SECTOR_ANGLE_RAD;
				avoidStraightPathDir = vector.Zero; // Go straight
			}
			else if (myDistToWp > 0.5 * wpRadius)
			{
				// We are between 50% and 100% of wp radius
				
				if (tgtInWaypoint)
				{
					// Towards target
					movePos = targetPos;
					eDirection = SCR_EAICombatMoveDirection.CUSTOM_POS;
					avoidStraightPathDir = GetAvoidStraightPathDir(); // Use flanking
					coverSearchSectorHalfAngleRad = COVER_QUERY_SECTOR_ANGLE_RAD;
				}
				else
				{
					// Move around current pos.
					movePos = targetPos;
					eDirection = SCR_EAICombatMoveDirection.ANYWHERE;
					avoidStraightPathDir = vector.Zero;
					coverSearchSectorHalfAngleRad = -1.0;
				}
			}
			else
			{
				// We are within 50% radius of wp,
				// Move towards tgt, regardless where tgt is
				movePos = targetPos;
				eDirection = SCR_EAICombatMoveDirection.CUSTOM_POS;
				avoidStraightPathDir = GetAvoidStraightPathDir();
				
				coverSearchSectorHalfAngleRad = COVER_QUERY_SECTOR_ANGLE_RAD;
			}
		}
		
		outMovePos = movePos;
		outDirection = eDirection;
		outAvoidStraightPathDir = avoidStraightPathDir;
		outCoverSearchSectorHalfAngleRad = coverSearchSectorHalfAngleRad;
	}
	
	//--------------------------------------------------------------------------------------------
	// Hide in cover when suppressed
	
	protected bool SuppressedInCoverCondition()
	{
		return m_State.m_bInCover && m_eThreatState == EAIThreatState.THREATENED;
	}
	
	protected void SuppressedInCoverLogic()
	{
		// We're pinned in cover and can't do much else now
		// Alternate hiding in cover and unhiding
		
		// How long to wait here? Depends on timer value from previous request.
		
		float waitTime_s;
		SCR_AICombatMoveRequestBase rq = m_State.GetRequest();
		if (SCR_AICombatMoveRequest_ChangeStanceInCover.Cast(rq) && rq.m_eReason == SCR_EAICombatMoveReason.SUPPRESSED_IN_COVER)
			waitTime_s = rq.m_f_UserTimer_s;
		else
		{
			if (m_State.m_bExposedInCover)
				waitTime_s = 1.7;
			else
				waitTime_s = 3.0;
		}
		
		if (m_State.m_bExposedInCover && m_State.m_fTimerRequest_s > waitTime_s)
		{
			float newWaitTime = Math.RandomFloat(3.5, 7.0); // Hide in cover for this time
			PushRequestChangeStanceInCover(false, SCR_EAICombatMoveReason.SUPPRESSED_IN_COVER, newWaitTime);
		}
		else if (!m_State.m_bExposedInCover && m_State.m_fTimerRequest_s > waitTime_s)
		{
			float newWaitTime = Math.RandomFloat(1.7, 2.5); // Expose out of cover for this time
			PushRequestChangeStanceInCover(true, SCR_EAICombatMoveReason.SUPPRESSED_IN_COVER, newWaitTime);
		}
	}
	
	protected void PushRequestChangeStanceInCover(bool exposed, SCR_EAICombatMoveReason reason, float waitTime)
	{
		SCR_AICombatMoveRequest_ChangeStanceInCover rq = new SCR_AICombatMoveRequest_ChangeStanceInCover();
		
		rq.m_bExposedInCover = exposed;
		rq.m_bAimAtTarget = true;
		rq.m_bAimAtTargetEnd = true;
		rq.m_f_UserTimer_s = waitTime;
		rq.m_eReason = reason;
		
		m_State.ApplyNewRequest(rq);
	}
	
	//--------------------------------------------------------------------------------------------
	// Cover not useful any more
	// It means that it doesn't provide cover in direction of enemy
	
	protected bool CurrentCoverUselessCondition()
	{
		if (!m_State.m_bInCover || !m_State.IsAssignedCoverValid())
			return false;
		
		vector tgtPos = GetTargetPosition();
		
		float cosAngle = m_State.GetAssignedCover().CosAngleToThreat(tgtPos);
		return (cosAngle < 0.5); // cos 60 deg = 0.5
	}
	
	protected void PushRequestLeaveUselessCover()
	{
		SCR_AICombatMoveRequest_Move rq = new SCR_AICombatMoveRequest_Move();
		
		rq.m_eReason = SCR_EAICombatMoveReason.STANDARD;
		
		rq.m_vTargetPos = ResolveRequestTargetPos();
		rq.m_vMovePos = rq.m_vTargetPos;
		rq.m_bTryFindCover = true;
		rq.m_bUseCoverSearchDirectivity = true;
		rq.m_bCheckCoverVisibility = true;
		rq.m_bFailIfNoCover = true;
		rq.m_eStanceMoving = ECharacterStance.CROUCH;
		rq.m_eStanceEnd = ECharacterStance.CROUCH;
		rq.m_eMovementType = EMovementType.RUN;
		rq.m_eDirection = SCR_EAICombatMoveDirection.BACKWARD; // Move back from target
		rq.m_fCoverSearchSectorHalfAngleRad = -1.0;
		rq.m_bAimAtTarget = SCR_AICombatMoveUtils.IsAimingAndMovementPossible(rq.m_eStanceMoving, rq.m_eMovementType);
		rq.m_bAimAtTargetEnd = true;
		
		rq.m_fCoverSearchDistMin = 0;
		rq.m_fCoverSearchDistMax = 20;
		
		m_State.ApplyNewRequest(rq);
	}
	
	
	//--------------------------------------------------------------------------------------------
	// Moving away from target if we are too close
	// This is meant for very stepping backwards a very short distance
	
	protected bool MoveFromTargetCondition()
	{
		float weaponMinDist = Math.Max(3.0, m_fWeaponMinDist);
		
		return m_fTargetDist < weaponMinDist;
	}
	
	protected bool MoveFromTargetNewRequestCondition()
	{
		if (!m_State.IsExecutingRequest())
			return true;
		
		// Still executing ...
		// Send new request only if we are executing NOT side-step
		SCR_AICombatMoveRequest_Move rq = SCR_AICombatMoveRequest_Move.Cast(m_State.GetRequest());
		if (!rq)
			return true;
		
		return rq.m_eReason != SCR_EAICombatMoveReason.MOVE_FROM_TARGET;
	}
	
	protected void PushRequestMoveFromTarget()
	{
		SCR_AICombatMoveRequest_Move rq = new SCR_AICombatMoveRequest_Move();
		
		rq.m_eReason = SCR_EAICombatMoveReason.MOVE_FROM_TARGET;
		
		rq.m_vMovePos = ResolveRequestTargetPos();
		rq.m_eMovementType = EMovementType.RUN;
		rq.m_eStanceMoving = ECharacterStance.STAND;
		rq.m_eStanceEnd = ECharacterStance.STAND;
		rq.m_eDirection = SCR_EAICombatMoveDirection.BACKWARD;
		rq.m_fMoveDuration_s = 2.0;
		rq.m_bAimAtTarget = SCR_AICombatMoveUtils.IsAimingAndMovementPossible(rq.m_eStanceMoving, rq.m_eMovementType);
		rq.m_bAimAtTargetEnd = true;
		
		m_State.ApplyNewRequest(rq);
	}
	
	//--------------------------------------------------------------------------------------------
	// Returns stance when stopped outside cover
	protected static ECharacterStance ResolveStanceOutsideCover(bool closeRange, EAIThreatState threat)
	{
		if (closeRange)
		{
			// Close range combat
			return ECharacterStance.CROUCH;
		}
		else
		{
			// Long range combat
			switch (threat)
			{
				case EAIThreatState.THREATENED:
					return ECharacterStance.PRONE;
				default:
					return ECharacterStance.CROUCH;
			}
		}
		return ECharacterStance.STAND;
	}
	
	//--------------------------------------------------------------------------------------------
	// Returns 'optimal' distance
	// If we are between weaponMinDist and 'optimal' dist, we don't need to move closer to tgt
	protected static float ResolveOptimalDistance(float weaponMinDist)
	{
		return Math.Max(weaponMinDist + 5.0, 15.0);
	}
	
	//--------------------------------------------------------------------------------------------
	// Returns true if we are allowed to aim and move with that weapon
	protected static bool IsAimingAndMovingAllowedForWeapon(EWeaponType weaponType)
	{
		switch (weaponType)
		{
			// Most common case is fast
			case EWeaponType.WT_RIFLE:
				return true;
			
			case EWeaponType.WT_ROCKETLAUNCHER:
			case EWeaponType.WT_GRENADELAUNCHER:
				return false;
			
			// Everything else
			default:
				return true;
		}
		
		return true;
	}
	
	//--------------------------------------------------------------------------------------------
	// Returns true if it's first of combat movement logic. Doesn't mean first execution of this node.
	protected bool IsFirstExecution()
	{
		return !m_State.GetRequest();
	}
	
	//--------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
}

//! Combat movement node for attack behavior, which is aimed at BaseTarget
class SCR_AICombatMoveLogic_Attack : SCR_AICombatMoveLogicBase
{
	// Inputs
	protected static const string PORT_BASE_TARGET = "BaseTarget";
	protected static const string PORT_AVOID_STRAIGHT_PATH_DIR = "AvoidStraightPathDir";
	
	protected BaseTarget m_Target;
	protected vector m_vAvoidStraightPathDir;
	
	//--------------------------------------------------------------------------------------------
	protected override bool OnUpdate(AIAgent owner, float dt)
	{
		GetVariableIn(PORT_BASE_TARGET, m_Target);
		GetVariableIn(PORT_AVOID_STRAIGHT_PATH_DIR, m_vAvoidStraightPathDir);
		
		if (!m_Target || !m_Target.GetTargetEntity())
			return false;
		
		return true;
	}
	
	//--------------------------------------------------------------------------------------------
	protected override float GetTargetDistance()
	{
		return m_Target.GetDistance();
	}
	
	//--------------------------------------------------------------------------------------------
	protected override vector GetTargetPosition()
	{
		return m_Target.GetLastSeenPosition();
	}
	
	protected override vector GetAvoidStraightPathDir()
	{
		return m_vAvoidStraightPathDir;
	}
	
	//--------------------------------------------------------------------------------------------
	protected override vector ResolveRequestTargetPos()
	{
		IEntity tgtEntity = m_Target.GetTargetEntity();
		if (tgtEntity && m_CombatComp.IsTargetVisible(m_Target))
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(tgtEntity);
			if (character)
			{
				vector eyePos = character.EyePosition();
				return eyePos;
			}
			
			// It's a vehicle
			vector pos = tgtEntity.GetOrigin();
			pos = pos + Vector(0, 2.0, 0);
			return pos;
		}
		
		// Target is either not visible or tgtEntity is null, use last seen position
		vector lastSeenPos = m_Target.GetLastSeenPosition();
		lastSeenPos = lastSeenPos + Vector(0, 1.8, 0);
		return lastSeenPos;		
	}
	
	//--------------------------------------------------------------------------------------------
	protected override bool ResolveFailMoveIfNoCover()
	{
		if (m_bCloseRangeCombat)
		{
			return false;
		}
		else
		{
			// Long range combat
			if (IsFirstExecution())
				return true; // On first run we want to move to cover, or stay where we are if there is no cover, and shoot.
			else
				return m_State.m_bInCover; // Don't leave cover if there is no next cover
		}
	}
	
	//--------------------------------------------------------------------------------------------
	protected override float ResolveStoppedWaitTime(bool inCover, EAIThreatState threat, EWeaponType weaponType)
	{
		float waitTime;
		
		if (inCover)
		{
			// In cover
			switch (threat)
			{
				case EAIThreatState.THREATENED:
					waitTime = 20.0;	// Stay in cover for a long time, until we are not suppressed any more
					break;
				default:
					waitTime = 5.0;
			}
		}
		else
		{
			// Not in cover
			switch (threat)
			{
				case EAIThreatState.THREATENED:
					waitTime = 6.0;
					break;
				default:
					waitTime = 3.0;
					break;
			}
		}
		
		// When using those weapons we want to move much less
		bool longWaitTime = false;
		switch (weaponType)
		{
			case EWeaponType.WT_MACHINEGUN:
			case EWeaponType.WT_ROCKETLAUNCHER:
			case EWeaponType.WT_GRENADELAUNCHER:
			case EWeaponType.WT_SNIPERRIFLE:
				longWaitTime = true;
		}
		
		// Note: it's important to let bots enough time to aim at very long range
		// Stop time should be more than just a few seconds.
		if (m_bVeryLongRangeCombat)
			waitTime *= 2.0;
		
		return waitTime;
	}
	
	//--------------------------------------------------------------------------------------------
	protected override bool MoveToNextPosCondition()
	{
		// Don't get any more closer
		// Except we should still move closer if we haven't seen target for a long time
		float optimalDist = ResolveOptimalDistance(m_fWeaponMinDist);
		if (m_fTargetDist < optimalDist && m_Target.GetTimeSinceSeen() < 15)
			return false;
			
		if (m_State.IsExecutingRequest())
			return false;
		
		// If it's first run, ignore timers, only if:
		// - If we are not in cover.
		if (IsFirstExecution() && !m_State.m_bInCover)
			return true;
		
		// If we should keep formation, don't move too far away
		if (m_Utility.ShouldKeepFormation() || m_CombatComp.GetCombatMode() == EAIGroupCombatMode.HOLD_FIRE)
		{
			// In this case we move only once
			if (!IsFirstExecution())
				return false;
		}
		
		float stoppedWaitTime = ResolveStoppedWaitTime(m_State.m_bInCover, m_eThreatState, m_eWeaponType);	
		return m_State.m_fTimerStopped_s > stoppedWaitTime;
	}
	
	//--------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_BASE_TARGET,
		PORT_AVOID_STRAIGHT_PATH_DIR
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
}

//! Combat move logic when doing suppressive fire
class SCR_AICombatMoveLogic_Suppressive : SCR_AICombatMoveLogicBase
{
	// Inputs
	protected static const string PORT_SUPPRESSION_VOLUME = "SuppressionVolume";
	protected static const string PORT_VISIBLE = "Visible";
	protected static const string PORT_TIME_LAST_SEEN = "TimeLastSeen_ms";
	
	// Variables updated from input ports
	protected SCR_AISuppressionVolumeBase m_SuppressionVolume;
	protected bool m_bTargetVisible = false;
	protected float m_fTargetLastSeenTime_ms = 0; // World time
	
	protected bool m_bGoodVision;
	
	protected static const float TIME_SINCE_GOOD_VISIBILITY_MIN_MS = 10000.0;
	
	//--------------------------------------------------------------------------------------------
	protected override bool OnUpdate(AIAgent owner, float dt)
	{
		GetVariableIn(PORT_SUPPRESSION_VOLUME, m_SuppressionVolume);
		
		if (!GetVariableIn(PORT_VISIBLE, m_bTargetVisible))
			return false;
		
		if (!GetVariableIn(PORT_TIME_LAST_SEEN, m_fTargetLastSeenTime_ms))
			return false;
		
		if (!m_SuppressionVolume)
			return false;
		
		// Update m_bGoodVision
		// The timer criteria is to exclude occlusion due to us hiding in cover
		float timeSinceLastSeen_ms = GetGame().GetWorld().GetWorldTime() - m_fTargetLastSeenTime_ms;
		m_bGoodVision = m_bTargetVisible || (timeSinceLastSeen_ms < TIME_SINCE_GOOD_VISIBILITY_MIN_MS);
		
		return true;
	}
	
	//--------------------------------------------------------------------------------------------
	protected override float GetTargetDistance()
	{
		return vector.Distance(m_MyEntity.GetOrigin(), m_SuppressionVolume.GetCenterPosition());
	}
	
	//--------------------------------------------------------------------------------------------
	protected override vector GetTargetPosition()
	{
		return m_SuppressionVolume.GetCenterPosition();
	}
	
	//--------------------------------------------------------------------------------------------
	protected override vector ResolveRequestTargetPos()
	{
		return 	m_SuppressionVolume.GetCenterPosition();	
	}
	
	//--------------------------------------------------------------------------------------------
	protected override bool ResolveFailMoveIfNoCover()
	{
		// Don't move out of cover if we already have good vision from current cover
		if (m_bGoodVision)
			return true;
		
		return false; // We're allowed to move anywhere, including to coverless position. But our own suppression criteria still apply and run above this.
	}
	
	//--------------------------------------------------------------------------------------------
	protected override float ResolveStoppedWaitTime(bool inCover, EAIThreatState threat, EWeaponType weaponType)
	{
		return 1.0;
	}
	
	//--------------------------------------------------------------------------------------------
	protected override bool MoveToNextPosCondition()
	{	
		if (m_State.IsExecutingRequest())
			return false;
		
		if (m_bGoodVision && m_State.m_bInCover)
		{
			// We have good vision and we are in cover, just stay here
			return false;
		}
		
		// If vision is bad, move out until we have good visibility
		// Here we operate with visibility of the suppression volume, still concept is same as during normal attack.
		// Most important thing is to exit area with poor vision of target, but beyond that we don't need to move.
		if (!m_bGoodVision || (m_bGoodVision && !m_State.m_bInCover) || (m_fTargetLastSeenTime_ms == 0))
		{
			// If it's first run, ignore timers, only if:
			// - If we are not in cover.
			if (IsFirstExecution() && !m_State.m_bInCover)
				return true;
			
			float stoppedWaitTime = ResolveStoppedWaitTime(m_State.m_bInCover, m_eThreatState, m_eWeaponType);	
			return m_State.m_fTimerStopped_s > stoppedWaitTime;
		}
		
		return false;
	}
	
	//--------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_SUPPRESSION_VOLUME,
		PORT_VISIBLE,
		PORT_TIME_LAST_SEEN
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
}