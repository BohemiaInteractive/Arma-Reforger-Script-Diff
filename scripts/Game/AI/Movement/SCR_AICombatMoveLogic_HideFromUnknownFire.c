class SCR_AICombatMoveLogic_HideFromUnknownFire : AITaskScripted
{
	protected static const float COVER_SEARCH_DIST_MAX = 10.0;
	
	// Inputs
	protected static const string PORT_POSITION = "Position";
	
	// Outputs
	protected static const string PORT_COMPLETE_ACTION = "CompleteAction";
	
	protected SCR_AICombatMoveState m_State;
	protected SCR_AIUtilityComponent m_Utility;
	protected CharacterControllerComponent m_CharacterController;
	
	protected bool m_bPushedMoveRequest = false;
	
	//--------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity myEntity = owner.GetControlledEntity();
		
		m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (m_Utility)
			m_State = m_Utility.m_CombatMoveState;
		
		if (myEntity)
			m_CharacterController = CharacterControllerComponent.Cast(myEntity.FindComponent(CharacterControllerComponent));
	}
	
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity myEntity = owner.GetControlledEntity();
		
		if (!m_State || !myEntity)
			return ENodeResult.FAIL;
		
		
		vector threatPos;
		GetVariableIn(PORT_POSITION, threatPos);
		if (threatPos == vector.Zero)
			return ENodeResult.FAIL;
		
		float distToThreat = vector.Distance(myEntity.GetOrigin(), threatPos);
		CombatMoveLogic(threatPos, distToThreat);
		
		//SetVariableOut(PORT_COMPLETE_ACTION);
		
		return ENodeResult.SUCCESS;
	}
	
	void CombatMoveLogic(vector threatPos, float distToThreat)
	{
		if (!m_State)
			return;
		
		if (!m_State.IsMoving() && !m_State.IsInValidCover() && !m_bPushedMoveRequest)
		{
			// Standing not in cover
			
			SCR_AICombatMoveRequest_Move rq = new SCR_AICombatMoveRequest_Move();
			
			rq.m_eReason = SCR_EAICombatMoveReason.MOVE_FROM_DANGER;
			
			rq.m_vTargetPos = threatPos;
			rq.m_vMovePos = rq.m_vTargetPos;
			rq.m_bTryFindCover = true;
			rq.m_bUseCoverSearchDirectivity = false;
			rq.m_bCheckCoverVisibility = false;
			rq.m_bFailIfNoCover = false;
			rq.m_eStanceMoving = ECharacterStance.STAND;
			rq.m_eStanceEnd = ECharacterStance.CROUCH;
			rq.m_eMovementType = EMovementType.SPRINT;
			rq.m_fCoverSearchDistMax = COVER_SEARCH_DIST_MAX;
			rq.m_fCoverSearchDistMin = 0;
			rq.m_fMoveDuration_s = Math.RandomFloat(1.0, 1.5) * COVER_SEARCH_DIST_MAX / SCR_AICombatMoveUtils.CHARACTER_SPEED_STAND_SPRINT;
			rq.m_eDirection = SCR_EAICombatMoveDirection.ANYWHERE;
			// rq.m_fCoverSearchSectorHalfAngleRad - not needed since direction is ANYWHERE
			
			rq.m_bAimAtTarget = false; // Don't aim while running
			rq.m_bAimAtTargetEnd = true;
			
			m_State.ApplyNewRequest(rq);
			m_bPushedMoveRequest = true;
		}
		else if (!m_State.IsMoving())
		{
			// We are not moving, manage our stance based on threat and range
			EAIThreatState threat = m_Utility.m_ThreatSystem.GetState();
			
			bool closeRangeCombat = distToThreat < SCR_AICombatMoveUtils.CLOSE_RANGE_COMBAT_DIST;
			
			if (closeRangeCombat)
			{
				if (m_State.m_bInCover)
				{
					bool newExposedInCover = threat != EAIThreatState.THREATENED;
					
					if (m_State.m_bExposedInCover != newExposedInCover)
						PushRequesChangeStanceInCover(newExposedInCover);
				}
				else
				{
					ECharacterStance newStance;
				
					if (threat == EAIThreatState.THREATENED)
						newStance = ECharacterStance.CROUCH;
					else
						newStance = ECharacterStance.STAND;
					
					if (newStance != m_CharacterController.GetStance())
						m_State.ApplyRequestChangeStanceOutsideCover(newStance);
				}
			}
			else
			{
				ECharacterStance newStance;
				
				if (threat == EAIThreatState.THREATENED)
					newStance = ECharacterStance.PRONE;
				else
					newStance = ECharacterStance.CROUCH;
				
				if (newStance != m_CharacterController.GetStance())
						m_State.ApplyRequestChangeStanceOutsideCover(newStance);
			}
		}
	}
	
	//------------------------------------------------------------------------------------
	void PushRequesChangeStanceInCover(bool exposed)
	{
		SCR_AICombatMoveRequest_ChangeStanceInCover rq = new SCR_AICombatMoveRequest_ChangeStanceInCover();
		
		rq.m_bExposedInCover = exposed;
		rq.m_bAimAtTarget = true;
		rq.m_bAimAtTargetEnd = true;
		
		m_State.ApplyNewRequest(rq);
	}
	
	//------------------------------------------------------------------------------------
	void PushRequestChangeStanceOutsideCover(ECharacterStance stance)
	{
		SCR_AICombatMoveRequest_ChangeStance rq = new SCR_AICombatMoveRequest_ChangeStance();
		
		rq.m_eStance = stance;
		rq.m_bAimAtTarget = true;
		rq.m_bAimAtTargetEnd = true;
		
		m_State.ApplyNewRequest(rq);
	}
	
	//------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = { PORT_POSITION };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }

	//protected static ref TStringArray s_aVarsOut = { PORT_COMPLETE_ACTION };
	//override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
}