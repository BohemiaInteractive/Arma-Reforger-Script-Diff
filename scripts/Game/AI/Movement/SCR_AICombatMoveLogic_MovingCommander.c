/*
This issues turret operator requests to move to driver that is performing the move itself.
*/

//! Base class for combat movement logic executed by turret operator of vehicle.
class SCR_AICombatMoveLogicVehicleGunnerBase : AITaskScripted
{
	protected SCR_AIUtilityComponent m_Utility;
	protected SCR_AICombatComponent m_CombatComp;
	protected SCR_CompartmentAccessComponent m_CompartmentAccessComponent;
	protected BaseWeaponManagerComponent m_WeaponManagerComponent;
	protected TurretComponent m_TurretComponent;
	protected IEntity m_MyEntity;
	protected Vehicle m_MyVehicle;
	
	protected SCR_AICombatMoveState m_DriverState;
	protected SCR_AIUtilityComponent m_DriverUtility;
	
	[Attribute("500", UIWidgets.EditBox, "Update interval of the node")]
	protected float m_fUpdateInterval_ms;
	protected float m_fNextUpdate_ms;
	
	//--------------------------------------------------------------------------------------------
	protected override void OnInit(AIAgent owner)
	{
		m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		m_MyEntity = owner.GetControlledEntity();
		
		m_CompartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(m_MyEntity.FindComponent(SCR_CompartmentAccessComponent));
		m_CombatComp = SCR_AICombatComponent.Cast(m_MyEntity.FindComponent(SCR_AICombatComponent));
		
		if (m_CompartmentAccessComponent && m_CompartmentAccessComponent.IsInCompartment())
		{
			IEntity turretEnt = m_CompartmentAccessComponent.GetCompartment().GetOwner();
			if (turretEnt)
			{
				TurretControllerComponent contr = TurretControllerComponent.Cast(turretEnt.FindComponent(TurretControllerComponent));
				if (contr)
					m_TurretComponent = contr.GetTurretComponent();
				m_WeaponManagerComponent = BaseWeaponManagerComponent.Cast(turretEnt.FindComponent(BaseWeaponManagerComponent));
			}
			m_MyVehicle = Vehicle.Cast(m_CompartmentAccessComponent.GetVehicle());
		}
	}
	
	//--------------------------------------------------------------------------------------------
	//! Applies combat move request to driver's mind
	protected void ApplyNewRequest(notnull SCR_AICombatMoveRequestBase rq)
	{
		rq.m_eUnitType = SCR_EAICombatMoveUnitType.GROUND_VEHICLE;
		m_DriverState.ApplyNewRequest(rq);
	}
	
	//--------------------------------------------------------------------------------------------
	protected override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		// Timer logic
		float currentTime_ms = GetGame().GetWorld().GetWorldTime();
		if (currentTime_ms < m_fNextUpdate_ms)
			return ENodeResult.RUNNING;
		m_fNextUpdate_ms = currentTime_ms + m_fUpdateInterval_ms;
		
		// Update variables for driver, turret, ...
		if (!UpdateDriver(owner))
			return ENodeResult.FAIL;
		
		// Verify variables
		if (!m_DriverState || !m_Utility || !m_TurretComponent || !m_MyVehicle)
			return ENodeResult.FAIL;
		
		// Is driver awake?
		if (m_DriverUtility.m_AIInfo.HasUnitState(EUnitState.UNCONSCIOUS))
			return ENodeResult.FAIL;
		
		// Can driver execute our requests?
		SCR_AIBehaviorBase executedBehavior = SCR_AIBehaviorBase.Cast(m_DriverUtility.GetExecutedAction());
		if (executedBehavior && !executedBehavior.m_bUseCombatMove)
			return ENodeResult.FAIL;
		
		// Run child class logic
		bool result = UpdateCombatMoveLogic();
		
		if (result)
			return ENodeResult.RUNNING;
		else
			return ENodeResult.FAIL;
	}
	
	//--------------------------------------------------------------------------------------------
	protected bool UpdateDriver(AIAgent owner)
	{
		if (!m_MyVehicle)
			return false;
		
		IEntity driverEntity = m_MyVehicle.GetPilot();
		if (!driverEntity)
			return false;
		
		AIControlComponent driverControlComp = AIControlComponent.Cast(driverEntity.FindComponent(AIControlComponent));
		
		if (!driverControlComp)
			return false;
		
		AIAgent driverAgent = driverControlComp.GetAIAgent();
		
		if (!driverAgent)
			return false;
		
		m_DriverUtility = SCR_AIUtilityComponent.Cast(driverAgent.FindComponent(SCR_AIUtilityComponent));
		
		if (!m_DriverUtility)
			return false;
		
		m_DriverState = m_DriverUtility.m_CombatMoveState;
		
		return true;
	}
	
	//--------------------------------------------------------------------------------------------
	//! Logic of the child class
	bool UpdateCombatMoveLogic();
	
	//--------------------------------------------------------------------------------------------
	// Returns true if it's first of combat movement logic. Doesn't mean first execution of this node.
	protected bool IsFirstExecution()
	{
		return !m_DriverState.GetRequest();
	}
	
	//--------------------------------------------------------------------------------------------
	//! Checks if target position is within safe aiming limits of the turret.
	//! Those limits are not exactly turret's limits, but smaller than that,
	//! because we want this to be triggered before actual physical limits are reached.
	protected bool TargetWithinTurretSafeHorizontalLimits(vector targetPos)
	{
		// Check turret horizontal limits
		vector turretLimitsHoriz_Rad, turretLimitsVert_Rad;
		m_TurretComponent.GetAimingLimits(turretLimitsHoriz_Rad, turretLimitsVert_Rad); // Initially this returns in degrees, we convert it later
		if (m_TurretComponent.HasMoveableBase())
			turretLimitsHoriz_Rad = Vector(-180, 180, 0);
		turretLimitsHoriz_Rad = Math.DEG2RAD * turretLimitsHoriz_Rad;
		turretLimitsVert_Rad = Math.DEG2RAD * turretLimitsVert_Rad;
		
		vector targetPosLocal = m_TurretComponent.GetOwner().CoordToLocal(targetPos); // Target pos in turret space
		vector dirToTargetLocalXZ = targetPosLocal; // Dir to target in turret space, projected on XZ plane.
		dirToTargetLocalXZ[1] = 0;
		dirToTargetLocalXZ.Normalize();
		float angleDirToTarget_Rad = Math.Atan2(dirToTargetLocalXZ[0], 1.0); // Angle of direction to target, in turret space
		
		// We want to start rotating car sooner than target reaches limits of the turret, that's why we shrink the horizontal limits.
		vector safeTurretLimitsHoriz_Rad = 0.5 * turretLimitsHoriz_Rad;
		return (angleDirToTarget_Rad > safeTurretLimitsHoriz_Rad[0]) && (angleDirToTarget_Rad < safeTurretLimitsHoriz_Rad[1]);
	}
	
	//--------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
}

//! Combat move logic for Attack Behavior of gunner. It receives BaseTarget.
class SCR_AICombatMoveLogicVehicleGunner_Attack : SCR_AICombatMoveLogicVehicleGunnerBase
{
	// Inputs
	protected static const string PORT_BASE_TARGET = "BaseTarget";
	
	protected BaseTarget m_Target;
	
	protected const float WEAPON_MIN_DIST = 2.0;
	
	// minimal distance Driver should be from enemy
	protected const float MIN_ENGAGEMENT_DISTANCE_TO_TARGET_SQ = 60.0 * 60.0;	// when too close to target, try to move backwards
	protected const float MAX_MOVE_DURATION_TO_TARGET_S = 9; 					// max step to move towards target
	protected const float MAX_MOVE_DURATION_TO_TARGET_THREATENED_S = 5; 		// move less under threat
	protected const float REVERSE_MOVE_DURATION_S = 1.2; 						// step to move from target. This must be consistent with reverse distance in car movement component, so that car moves backwards.
	
	// Values updated on each update, to avoid passing them through calls
	protected EAIThreatState m_eThreatState;
	protected float m_fTargetDist;
	protected float m_fWeaponMinDist = WEAPON_MIN_DIST;
	
	//------------------------------------------------------------------------------------
	override bool UpdateCombatMoveLogic()
	{
		// Read target data
		GetVariableIn(PORT_BASE_TARGET, m_Target);
		if (!m_Target || !m_Target.GetTargetEntity())
			return false;
		
		// Update cached variables
		m_fTargetDist = GetTargetDistance();		
		m_eThreatState = m_Utility.m_ThreatSystem.GetState();
		m_fWeaponMinDist = 2.0;
		
		// Conditions and states of combat movement
		if (MoveFromTargetCondition())
		{
			// Target out of reach for turret or too close
			// Step backwards
			if (MoveFromTargetNewRequestCondition())
				PushRequestMoveFromTarget();
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
		else if (!m_DriverState.IsExecutingRequest())
		{
			// TODO: We are stopped keep distance, scan the perimeter			
		}
		
		return true;
	}
	
	//--------------------------------------------------------------------------------------------
	//! decides if we should move backwards from target 
	protected bool MoveFromTargetCondition()
	{
		if (!m_WeaponManagerComponent)
			return false;
		
		vector mat[4];
		m_WeaponManagerComponent.GetCurrentMuzzleTransform(mat);
		vector muzzlePos = mat[3];
		vector muzzleDir = mat[2].Normalized();
		vector targetPos = m_Target.GetLastSeenPosition();
		vector targetDir = (targetPos - muzzlePos).Normalized();
		
		// Target is too close in front -> we move backward
		if (vector.DistanceSq(targetPos, muzzlePos) < MIN_ENGAGEMENT_DISTANCE_TO_TARGET_SQ)
			return true;
		
		// Target is behind vehicle -> we move backward
		//vector targetPosVehicleSpaceLocal = m_MyVehicle.CoordToLocal(targetPos); // Target pos in vehicle's space
		//if (targetPosVehicleSpaceLocal[2] < 0)
		//	return true;
		
		// Check turret horizontal limits
		if (!TargetWithinTurretSafeHorizontalLimits(targetPos))
			return true;
		
		return false;
	}
	
	//--------------------------------------------------------------------------------------------
	protected bool MoveFromTargetNewRequestCondition()
	{
		if (!m_DriverState.IsExecutingRequest())
			return true;
		
		// Still executing ...
		// Send new request only if we are executing NOT move_from_target
		SCR_AICombatMoveRequest_Move rq = SCR_AICombatMoveRequest_Move.Cast(m_DriverState.GetRequest());
		if (!rq)
			return true;
		
		return rq.m_eReason != SCR_EAICombatMoveReason.MOVE_FROM_TARGET;
	}
	
	//--------------------------------------------------------------------------------------------
	protected void PushRequestMoveFromTarget()
	{
		SCR_AICombatMoveRequest_Move rq = new SCR_AICombatMoveRequest_Move();
		
		rq.m_eReason = SCR_EAICombatMoveReason.MOVE_FROM_TARGET;
		
		rq.m_vMovePos = ResolveRequestTargetPos();
		rq.m_eDirection = SCR_EAICombatMoveDirection.BACKWARD;
		rq.m_fMoveDuration_s = REVERSE_MOVE_DURATION_S;
		rq.m_bAimAtTarget = false;
		rq.m_bAimAtTargetEnd = false;
		
		ApplyNewRequest(rq);
	}
	
	//--------------------------------------------------------------------------------------------
	// Movement
	
	protected void PushRequestMove()
	{		
		SCR_AICombatMoveRequest_Move rq = new SCR_AICombatMoveRequest_Move();
		
		rq.m_eReason = SCR_EAICombatMoveReason.STANDARD;
		
		// Common values
		rq.m_vTargetPos = ResolveRequestTargetPos();
		ResolveMoveRequestMovePosAndDir(rq.m_vTargetPos, rq.m_vMovePos, rq.m_eDirection);
		rq.m_bTryFindCover = false;
		rq.m_bUseCoverSearchDirectivity = false;
		rq.m_bCheckCoverVisibility = false;
		
		
		float moveDurationMax = MAX_MOVE_DURATION_TO_TARGET_S;
		
		// Long range combat
		
		switch (m_eThreatState)
		{
			case EAIThreatState.THREATENED:
			{
				moveDurationMax = MAX_MOVE_DURATION_TO_TARGET_THREATENED_S;				
				break;
			}
			default:
			{
				moveDurationMax = MAX_MOVE_DURATION_TO_TARGET_S;				
				break;
			}
		}
		
		rq.m_bAimAtTarget = false; 
		rq.m_bAimAtTargetEnd = false; // turn towards the target should be true!
		rq.m_bFailIfNoCover = false;
		
		rq.m_fMoveDuration_s = Math.RandomFloat(0.5, 1.0) * moveDurationMax; // Move distance randomized
		
		// Subscribe to events
		// We will pronounce voice lines once we start or end moving
		rq.GetOnMovementStarted().Insert(OnMovementStarted);
		rq.GetOnCompleted().Insert(OnMovementCompleted);
		
		ApplyNewRequest(rq);
	}
	
	//--------------------------------------------------------------------------------------------
	// Resolves which move pos and dir. we should use for _MOVE_ request
	// By now rq.m_vTargetPos must be already calculated!
	protected void ResolveMoveRequestMovePosAndDir(vector targetPos, out vector outMovePos, out SCR_EAICombatMoveDirection outDirection)
	{	
		AIWaypoint wp = null;
		AIAgent agent = m_DriverUtility.GetAIAgent();
		AIGroup group = agent.GetParentGroup();
		if (group)
			wp = group.GetCurrentWaypoint();
		
		vector movePos;
		SCR_EAICombatMoveDirection eDirection;		
		
		if (!wp)
		{
			// No waypoint, standard move logic
			eDirection = SCR_EAICombatMoveDirection.FORWARD;			
			movePos = targetPos;						
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
			}
			else if (myDistToWp > 0.5 * wpRadius)
			{
				// We are between 50% and 100% of wp radius
				
				if (tgtInWaypoint)
				{
					// Towards target
					movePos = targetPos;
					eDirection = SCR_EAICombatMoveDirection.FORWARD;					
				}
				else
				{
					// Move around current pos.
					movePos = targetPos;
					eDirection = SCR_EAICombatMoveDirection.ANYWHERE;					
				}
			}
			else
			{
				// We are within 50% radius of wp,
				// Move towards tgt, regardless where tgt is
				movePos = targetPos;
				eDirection = SCR_EAICombatMoveDirection.FORWARD;								
			}
		}
		
		outMovePos = movePos;
		outDirection = eDirection;		
	}
	
	//--------------------------------------------------------------------------------------------
	// Friendly fire avoidance
	
	// Friendly fire avoidance condition
	protected bool FFAvoidanceCondition()
	{
		return m_Utility.m_CombatComponent.IsFriendlyInAim();
	}
	
	//--------------------------------------------------------------------------------------------
	protected bool FFAvoidanceNewRequestCondition()
	{
		if (!m_DriverState.IsExecutingRequest())
			return true;
		
		// Still executing ...
		// Send new request only if we are executing NOT side-step
		SCR_AICombatMoveRequest_Move rq = SCR_AICombatMoveRequest_Move.Cast(m_DriverState.GetRequest());
		if (!rq)
			return true;
		
		return rq.m_eReason != SCR_EAICombatMoveReason.FF_AVOIDANCE;
	}
	
	//--------------------------------------------------------------------------------------------
	protected void PushRequestFFAvoidance()
	{
		SCR_AICombatMoveRequest_Move rq = new SCR_AICombatMoveRequest_Move();
		
		rq.m_eReason = SCR_EAICombatMoveReason.FF_AVOIDANCE;
		
		// If prev. request was FF avoidance too, keep direction.
		// Otherwise choose a new direction.
		SCR_AICombatMoveRequest_Move prevRequest = SCR_AICombatMoveRequest_Move.Cast(m_DriverState.GetRequest());
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
		
		rq.m_vMovePos = ResolveRequestTargetPos();
		rq.m_fMoveDuration_s = REVERSE_MOVE_DURATION_S;
		rq.m_bAimAtTarget = false;
		rq.m_bAimAtTargetEnd = false;
		
		ApplyNewRequest(rq);
	}
	
	//--------------------------------------------------------------------------------------------
	protected float GetTargetDistance()
	{
		return m_Target.GetDistance();
	}
	
	//--------------------------------------------------------------------------------------------
	protected vector ResolveRequestTargetPos()
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
	protected float ResolveStoppedWaitTime(EAIThreatState threat)
	{
		float waitTime;
		
		// based on threat we wait longer
		switch (threat)
		{
			case EAIThreatState.THREATENED:
				waitTime = 12.0;
				break;
			default:
				waitTime = 6.0;
				break;
		}
				
		return waitTime;
	}
	
	//--------------------------------------------------------------------------------------------
	protected bool MoveToNextPosCondition()
	{
		// Don't get any more closer
		// Except we should still move closer if we haven't seen target for a long time
		float optimalDist = ResolveOptimalDistance(m_fWeaponMinDist);
		if (m_fTargetDist < optimalDist && m_Target.GetTimeSinceSeen() < 15)
			return false;
			
		if (m_DriverState.IsExecutingRequest())
			return false;
		
		// If it's first run, ignore timers
		// TODO: add effect of explosion from threat system, i.e. if BOOM -> move away asap
		if (IsFirstExecution())
			return true;
		
		float stoppedWaitTime = ResolveStoppedWaitTime(m_eThreatState);	
		return m_DriverState.m_fTimerStopped_s > stoppedWaitTime;
	}
	
	//--------------------------------------------------------------------------------------------
	// Returns 'optimal' distance
	// If we are between weaponMinDist and 'optimal' dist, we don't need to move closer to tgt
	protected static float ResolveOptimalDistance(float weaponMinDist)
	{
		return Math.Max(weaponMinDist + 5.0, 200);
	}
	
	//--------------------------------------------------------------------------------------------
	//! Methods to make AI talk when they move or stop
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
	
	//--------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_BASE_TARGET
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
}

// Combat move logic for Suppressive Fire behavior of gunner. It receives Suppression Volume.
class SCR_AICombatMoveLogicVehicleGunner_Suppressive : SCR_AICombatMoveLogicVehicleGunnerBase
{
	protected static const string PORT_SUPPRESSION_VOLUME = "SuppressionVolume";
	
	protected SCR_AISuppressionVolumeBase m_SuppressionVolume;
	
	//-------------------------------------------------------------------------------------------
	override bool UpdateCombatMoveLogic()
	{
		GetVariableIn(PORT_SUPPRESSION_VOLUME, m_SuppressionVolume);
		if (!m_SuppressionVolume)
			return false;
		
		// Logic for suppressive fire is very simple. We only want to rotate the car until we can aim the turret at target.
		if (TargetOutsideLimitsCondition())
		{
			if (TargetOutsideLimitsNewRequestCondition())
			{
				PushRequestRotateToTarget();
			}
		}
		
		return true;
	}
	
	//-------------------------------------------------------------------------------------------
	bool TargetOutsideLimitsCondition()
	{
		vector targetPos = m_SuppressionVolume.GetCenterPosition();
		if (!TargetWithinTurretSafeHorizontalLimits(targetPos))
			return true;
		
		return false;
	}
	
	//-------------------------------------------------------------------------------------------
	bool TargetOutsideLimitsNewRequestCondition()
	{
		return !m_DriverState.IsExecutingRequest();
	}
	
	//-------------------------------------------------------------------------------------------
	void PushRequestRotateToTarget()
	{
		SCR_AICombatMoveRequest_Move rq = new SCR_AICombatMoveRequest_Move();
		
		rq.m_eReason = SCR_EAICombatMoveReason.STANDARD;
		
		rq.m_vMovePos = m_SuppressionVolume.GetCenterPosition();
		rq.m_eDirection = SCR_EAICombatMoveDirection.FORWARD;
		rq.m_fMoveDuration_s = 3;
		rq.m_bAimAtTarget = false;
		rq.m_bAimAtTargetEnd = false;
		
		ApplyNewRequest(rq);
	}
	
	//-------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = { PORT_SUPPRESSION_VOLUME };	
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
}