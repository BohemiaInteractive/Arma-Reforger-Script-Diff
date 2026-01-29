class SCR_AIGetCombatMoveRequestParametersBase : AITaskScripted
{
	// Inputs
	protected static const string PORT_REQUEST = "Request";
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_REQUEST
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	static override bool VisibleInPalette() { return true; }
	
	static override  string GetOnHoverDescription() { return "Unpacks data from combat move request";	};
}


class SCR_AIGetCombatMoveRequestParameters_Move : SCR_AIGetCombatMoveRequestParametersBase
{
	protected static const string PORT_TARGET_POS = "TargetPos";
	protected static const string PORT_MOVE_POS = "MovePos";
	protected static const string PORT_TRY_FIND_COVER = "TryFindCover";
	protected static const string PORT_FAIL_IF_NO_COVER = "FailIfNoCover";
	protected static const string PORT_STANCE_MOVING = "StanceMoving";
	protected static const string PORT_STANCE_END = "StanceEnd";
	protected static const string PORT_MOVEMENT_TYPE = "MovementType";
	protected static const string PORT_DIRECTION = "CombatMoveDirection";
	protected static const string PORT_AIM_AT_TARGET = "AimAtTarget";
	protected static const string PORT_MOVE_DURATION_S = "MoveDuration_s";
		
	//---------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AICombatMoveRequestBase rqBase;
		GetVariableIn(PORT_REQUEST, rqBase);
		SCR_AICombatMoveRequest_Move rq = SCR_AICombatMoveRequest_Move.Cast(rqBase);
		if (!rq)
			return SCR_AIErrorMessages.NodeErrorCombatMoveRequest(this, owner, rqBase);
			
		//SetVariableOut(PORT_TARGET_POS, rq.m_vTargetPos);
		//SetVariableOut(PORT_MOVE_POS, rq.m_vMovePos);
		SetVariableOut(PORT_TRY_FIND_COVER, rq.m_bTryFindCover);
		SetVariableOut(PORT_FAIL_IF_NO_COVER, rq.m_bFailIfNoCover);
		SetVariableOut(PORT_STANCE_MOVING, rq.m_eStanceMoving);
		SetVariableOut(PORT_STANCE_END, rq.m_eStanceEnd);
		SetVariableOut(PORT_MOVEMENT_TYPE, rq.m_eMovementType);
		SetVariableOut(PORT_DIRECTION, rq.m_eDirection);
		SetVariableOut(PORT_AIM_AT_TARGET, rq.m_bAimAtTarget);
		SetVariableOut(PORT_MOVE_DURATION_S, rq.m_fMoveDuration_s);
		
		return ENodeResult.SUCCESS;
	}
	
	
	//---------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_TRY_FIND_COVER,
		PORT_FAIL_IF_NO_COVER,
		PORT_STANCE_MOVING,
		PORT_STANCE_END,
		PORT_MOVEMENT_TYPE,
		PORT_DIRECTION,
		PORT_AIM_AT_TARGET,
		PORT_MOVE_DURATION_S
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
}

/*
class SCR_AIGetCombatMoveRequestParameters_Step : SCR_AIGetCombatMoveRequestParametersBase
{
	protected static const string PORT_MOVEMENT_TYPE = "MovementType";
	
	//---------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AICombatMoveRequestBase rqBase;
		GetVariableIn(PORT_REQUEST, rqBase);
		SCR_AICombatMoveRequest_Step rq = SCR_AICombatMoveRequest_Step.Cast(rqBase);
		if (!rq)
			return SCR_AIErrorMessages.NodeErrorCombatMoveRequest(this, owner, rqBase);
			
		SetVariableOut(PORT_MOVEMENT_TYPE, rq.m_eMovementType);
		
		return ENodeResult.SUCCESS;
	}
	
	
	//---------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_MOVEMENT_TYPE
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
}
*/

class SCR_AIGetCombatMoveRequestParameters_ChangeStanceInCover : SCR_AIGetCombatMoveRequestParametersBase
{
	protected static const string PORT_EXPOSED_IN_COVER = "ExposedInCover";
	
	//---------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AICombatMoveRequestBase rqBase;
		GetVariableIn(PORT_REQUEST, rqBase);
		SCR_AICombatMoveRequest_ChangeStanceInCover rq = SCR_AICombatMoveRequest_ChangeStanceInCover.Cast(rqBase);
		if (!rq)
			return SCR_AIErrorMessages.NodeErrorCombatMoveRequest(this, owner, rqBase);
			
		SetVariableOut(PORT_EXPOSED_IN_COVER, rq.m_bExposedInCover);
		
		return ENodeResult.SUCCESS;
	}
	
	//---------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_EXPOSED_IN_COVER
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
}

class SCR_AIGetCombatMoveRequestParameters_ChangeStance : SCR_AIGetCombatMoveRequestParametersBase
{
	protected static const string PORT_STANCE = "Stance";
	
	//---------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AICombatMoveRequestBase rqBase;
		GetVariableIn(PORT_REQUEST, rqBase);
		SCR_AICombatMoveRequest_ChangeStance rq = SCR_AICombatMoveRequest_ChangeStance.Cast(rqBase);
		if (!rq)
			return SCR_AIErrorMessages.NodeErrorCombatMoveRequest(this, owner, rqBase);
			
		SetVariableOut(PORT_STANCE, rq.m_eStance);
		
		return ENodeResult.SUCCESS;
	}
	
	//---------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_STANCE
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
}