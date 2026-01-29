class SCR_AISetCombatMoveRequestState : AITaskScripted
{
	protected static const string PORT_REQUEST = "Request";
	
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(SCR_EAICombatMoveRequestState))]
	protected SCR_EAICombatMoveRequestState m_eState;
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AICombatMoveRequestBase rq;
		
		GetVariableIn(PORT_REQUEST, rq);
		if (!rq)
			return ENodeResult.FAIL;
		
		rq.m_eState = m_eState;
		
		return ENodeResult.SUCCESS;
	}
	
	//--------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	override string GetNodeMiddleText()
	{
		return string.Format("State: %1", typename.EnumToString(SCR_EAICombatMoveRequestState, m_eState));
	}
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_REQUEST
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
}

class SCR_AISetCombatMoveRequestFailReason : AITaskScripted
{
	protected static const string PORT_REQUEST = "Request";
	
	[Attribute("0", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EAICombatMoveRequestFailReason))]
	protected SCR_EAICombatMoveRequestFailReason m_eFailReason;
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AICombatMoveRequestBase rq;
		
		GetVariableIn(PORT_REQUEST, rq);
		if (!rq)
			return ENodeResult.FAIL;
		
		rq.m_eFailReason = m_eFailReason;
		
		return ENodeResult.SUCCESS;
	}
	
	//--------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	override string GetNodeMiddleText()
	{
		return string.Format("Fail Reason: %1", typename.EnumToString(SCR_EAICombatMoveRequestFailReason, m_eFailReason));
	}
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_REQUEST
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
}