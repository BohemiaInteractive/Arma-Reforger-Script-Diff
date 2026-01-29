class SCR_AICombatMoveLogic_MoveFromGrenade : AITaskScripted
{
	// Inputs
	protected static const string PORT_POSITION = "Position";
	
	// Outputs
	protected static const string PORT_COMPLETE_ACTION = "CompleteAction";
	
	protected SCR_AICombatMoveState m_State;
	
	//--------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (utility)
			m_State = utility.m_CombatMoveState;
	}
	
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_State)
			return ENodeResult.FAIL;
		
		vector threatPos;
		GetVariableIn(PORT_POSITION, threatPos);
		if (threatPos == vector.Zero)
			return ENodeResult.FAIL;
		
		bool completeAction = false;
		CombatMoveLogic(threatPos, completeAction);
		
		SetVariableOut(PORT_COMPLETE_ACTION, completeAction);
		
		return ENodeResult.SUCCESS;
	}
	
	//--------------------------------------------------------------------------------------------
	void CombatMoveLogic(vector threatPos, out bool outCompleteAction)
	{
		if (!m_State)
			return;
		
		bool completeAction = false;
		
		if (ConditionPushMoveRequest(threatPos))
		{
			SCR_AICombatMoveRequest_Move rq = new SCR_AICombatMoveRequest_Move();
			
			rq.m_eReason = SCR_EAICombatMoveReason.MOVE_FROM_DANGER;
			
			rq.m_vTargetPos = threatPos;
			rq.m_vMovePos = rq.m_vTargetPos;
			rq.m_bTryFindCover = true;
			rq.m_bUseCoverSearchDirectivity = true;
			rq.m_bCheckCoverVisibility = false;
			rq.m_bFailIfNoCover = false;
			rq.m_eStanceMoving = ECharacterStance.STAND;
			rq.m_eStanceEnd = ECharacterStance.CROUCH;
			rq.m_eMovementType = EMovementType.SPRINT;
			rq.m_fCoverSearchDistMax = 15;
			rq.m_fCoverSearchDistMin = 4;
			rq.m_fMoveDuration_s = rq.m_fCoverSearchDistMax / SCR_AICombatMoveUtils.CHARACTER_SPEED_STAND_SPRINT;
			rq.m_eDirection = SCR_EAICombatMoveDirection.BACKWARD;
			rq.m_fCoverSearchSectorHalfAngleRad = Math.PI_HALF; // 90 deg
			
			rq.m_bAimAtTarget = false;
			rq.m_bAimAtTargetEnd = true;
			
			m_State.ApplyNewRequest(rq);
		}
		else if (m_State.m_bInCover && m_State.IsAssignedCoverValid() && IsCoverSafeAgainstGrenade(m_State.GetAssignedCover(), threatPos))
		{
			// Found good enough cover, complete the action
			completeAction = true;
			return;
		}
		
		outCompleteAction = completeAction;
	}
	
	//--------------------------------------------------------------------------------------------
	bool ConditionPushMoveRequest(vector tgtPos)
	{
		if (m_State.IsMoving())
		{
			if (m_State.IsMovingToCover())
			{
				// Already moving to cover, continue moving there even if it's not cover to hide from this grenade
				return false;
			}
			else
			{
				SCR_AICombatMoveRequest_Move rqMove = SCR_AICombatMoveRequest_Move.Cast(m_State.GetRequest());
				if (rqMove && rqMove.m_eReason == SCR_EAICombatMoveReason.MOVE_FROM_DANGER)
					return false;
				else
				{
					// Moving not to cover, but this is not our request, make a new request
					return true;
				}
			}
		}	
		else if (m_State.m_bInCover && m_State.IsAssignedCoverValid())
		{
			// Not moving, but we are in cover
			if (!IsCoverSafeAgainstGrenade(m_State.GetAssignedCover(), tgtPos))
				return true;
			else
				return false;
		}
		else
		{
			// Not moving, not in cover, run somewhere
			return true;
		}
		
		return false;
	}
	
	//--------------------------------------------------------------------------------------------
	bool IsCoverSafeAgainstGrenade(notnull SCR_AICoverLock coverLock, vector threatPos)
	{
		float coverDistToThreatSq = vector.DistanceSq(threatPos, coverLock.m_vCoverPos);
		
		// So close is not safe
		if (coverDistToThreatSq < 3*3) // TODO make it constant
			return false;
		
		// Far enough, check angle
		float cosAngleCoverToTgt = m_State.GetAssignedCover().CosAngleToThreat(threatPos);
		return cosAngleCoverToTgt > 0.707; // This should match or be close to value in cover query properties
	}
	
	//------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = { PORT_POSITION };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }

	protected static ref TStringArray s_aVarsOut = { PORT_COMPLETE_ACTION };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
}