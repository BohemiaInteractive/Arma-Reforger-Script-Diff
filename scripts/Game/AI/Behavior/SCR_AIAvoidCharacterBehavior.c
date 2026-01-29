class SCR_AIAvoidCharacterBehavior : SCR_AIBehaviorBase
{
	protected vector m_vTargetPos;
	protected vector m_vTargetVelocity;
	
	protected const float MOVE_DURATION_S = 0.5;
	
	//--------------------------------------------------------------------------------------------------------------
	void SCR_AIAvoidCharacterBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector targetPos, vector targetVelocity)
	{
		m_bAllowLook = false;
		m_bUseCombatMove = true;
		m_vTargetPos = targetPos;
		m_vTargetVelocity = targetVelocity;
		SetPriority(PRIORITY_BEHAVIOR_AVOID_CHARACTER);
		m_sBehaviorTree = "{A75A34B4B237851F}AI/BehaviorTrees/Chimera/Soldier/AvoidCharacter.bt";
	}
	
	//--------------------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		SCR_AICombatMoveRequest_Move rq = new SCR_AICombatMoveRequest_Move();
		
		SCR_CharacterControllerComponent characterController = m_Utility.GetCharacterController();
		
		if (!characterController)
		{
			Fail();
			return;
		}
		
		// Just move a bit backwards from the position of offender. Don't use covers.
		
		rq.m_bAimAtTarget = true;
		rq.m_bAimAtTargetEnd = true;
		rq.m_eReason = SCR_EAICombatMoveReason.CHARACTER_AVOIDANCE;
		rq.m_vTargetPos = m_vTargetPos;								// Position of target (enemy) - must be useable for cover search, should be muzzle position or head position.
		rq.m_vMovePos = m_vTargetPos;								// Move position relative to which m_eDirection works
		rq.m_bTryFindCover = false;									// Try to find cover or not?
		rq.m_bFailIfNoCover = false;								// If m_bTryFindCover is true, but cover was not found, fail or move anyway?
		//rq.m_bCheckCoverVisibility = false;						// (only m_bTryFindCover) Do we need to check target pos. visibility from that cover or not?
		rq.m_eStanceMoving = characterController.GetStance();		// Stance used for movement
		rq.m_eStanceEnd = rq.m_eStanceMoving;						// Stance used at the end (might be overridden by found cover)
		rq.m_eMovementType = EMovementType.RUN;						//
		//float m_fCoverSearchDistMin;								// (only m_bTryFindCover)
		//float m_fCoverSearchDistMax;								// (only m_bTryFindCover)
		//float m_fCoverSearchSectorHalfAngleRad = Math.PI;			// (only m_bTryFindCover) Half-angle (in radians) of cover query sector. Pi is full circle, Pi/2 is -90deg...+90deg sector.
		rq.m_fMoveDuration_s = MOVE_DURATION_S;						// Movement duration for movement to non-cover position
		//bool m_bUseCoverSearchDirectivity;						// If trying to find cover, prefer covers in given direction or not. Affects cover scoring.
		rq.m_eDirection = ResolveMoveDirection();					// Direction - Where we want to move !!! relative to m_vMovePos
		//vector m_vAvoidStraightPathDir;							// When not 0,0,0, changes pathfinding to try to flank and avoid the path aligned same direction. Makes sense mostly for m_eDirection = CUSTOM_POS.
		
		rq.GetOnCompleted().Insert(OnMovementCompleted);
		rq.GetOnFailed().Insert(OnMovementFailed);
		
		m_Utility.m_CombatMoveState.ApplyNewRequest(rq);
		
		// For safety, in case that move request gets stuck, make a timer with duration larger than duration of move request, and complete the action.
		m_Utility.GetCallqueue().CallLater(OnTimeout, 1500*MOVE_DURATION_S);
	}
	
	//--------------------------------------------------------------------------------------------------------------
	SCR_EAICombatMoveDirection ResolveMoveDirection()
	{
		if (m_vTargetVelocity == vector.Zero)
			return SCR_EAICombatMoveDirection.BACKWARD;
		
		vector dirToMeXZ = (m_Utility.m_OwnerEntity.GetOrigin() - m_vTargetPos);
		dirToMeXZ[1] = 0;
		dirToMeXZ.Normalize();
		
		vector dirVelocityXZ = m_vTargetVelocity;
		dirVelocityXZ[1] = 0;
		dirVelocityXZ.Normalize();
		
		vector cross = dirToMeXZ * dirVelocityXZ;
		float sinAngle = cross[1];
		
		if (Math.AbsFloat(sinAngle) < 0.5) // Sin 30
			return SCR_EAICombatMoveDirection.BACKWARD;
		
		if (sinAngle > 0)
			return SCR_EAICombatMoveDirection.RIGHT;
		else
			return SCR_EAICombatMoveDirection.LEFT;
	}
	
	//--------------------------------------------------------------------------------------------------------------
	void OnMovementCompleted(SCR_AIUtilityComponent utility, SCR_AICombatMoveRequestBase request)
	{
		Complete();
	}
	
	//--------------------------------------------------------------------------------------------------------------
	void OnMovementFailed(SCR_AIUtilityComponent utility, SCR_AICombatMoveRequestBase request, SCR_EAICombatMoveRequestFailReason failReason)
	{
		Complete();
	}
	
	//--------------------------------------------------------------------------------------------------------------
	void OnTimeout()
	{
		if (GetActionState() != EAIActionState.FAILED || GetActionState() != EAIActionState.COMPLETED)
			Complete();
	}
	
	//--------------------------------------------------------------------------------------------------------------
	override SCR_EAIBehaviorCause GetCause()
	{
		return SCR_EAIBehaviorCause.SAFE;
	}
}