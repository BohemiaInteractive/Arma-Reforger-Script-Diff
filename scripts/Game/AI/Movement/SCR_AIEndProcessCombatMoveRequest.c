/*
Logic which runs after all combat move requests
*/

class SCR_AIEndProcessCombatMoveRequest : AITaskScripted
{
	// Inputs
	protected static const string PORT_REQUEST = "Request";
	
	protected SCR_AIUtilityComponent m_Utility;
	protected SCR_AICombatMoveState m_State;
	
	//--------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (m_Utility)
			m_State = m_Utility.m_CombatMoveState;
	}
	
	//---------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_State)
			return ENodeResult.FAIL;
		
		SCR_AICombatMoveRequestBase rq;
		if (!GetVariableIn(PORT_REQUEST, rq))
			return ENodeResult.FAIL;
		
		// Common logic for all request
		m_State.EnableAiming(rq.m_bAimAtTargetEnd);
		
		// Invoke script invokers
		if (rq.m_eState == SCR_EAICombatMoveRequestState.SUCCESS)
		{
			auto invoker = rq.GetOnCompleted(false);
			if (invoker)
				invoker.Invoke(m_Utility, rq);
		}
		else if (rq.m_eState == SCR_EAICombatMoveRequestState.FAIL)
		{
			auto invoker = rq.GetOnFailed(false);
			if (invoker)
				invoker.Invoke(m_Utility, rq, rq.m_eFailReason);
		}
		
		return ENodeResult.SUCCESS;
	}
	
	
	//---------------------------------------------------------------------------	
	protected static ref TStringArray s_aVarsIn = {
		PORT_REQUEST
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	static override bool VisibleInPalette() { return true; }
}