/*
Logic common to all combat move request types.
*/

class SCR_AIProcessCombatMoveRequest : AITaskScripted
{
	// Inputs
	protected static const string PORT_REQUEST = "Request";
	
	// Outputs
	protected static const string PORT_TYPE = "Type";
	
	protected SCR_AICombatMoveState m_State;
	
	//--------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		SCR_AIUtilityComponent utilityComp = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (utilityComp)
			m_State = utilityComp.m_CombatMoveState;
	}
	
	//---------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_State)
			return ENodeResult.FAIL;
		
		SCR_AICombatMoveRequestBase rq;
		if (!GetVariableIn(PORT_REQUEST, rq))
			return ENodeResult.FAIL;
		
		SetVariableOut(PORT_TYPE, rq.m_eType);
		
		// Common logic for all request
		m_State.m_bAimAtTarget = rq.m_bAimAtTarget;
		rq.m_eState = SCR_EAICombatMoveRequestState.EXECUTING;
		rq.m_eFailReason = SCR_EAICombatMoveRequestFailReason.NONE;
		
		return ENodeResult.SUCCESS;
	}
	
	
	//---------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_TYPE
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_REQUEST
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	static override bool VisibleInPalette() { return true; }
}