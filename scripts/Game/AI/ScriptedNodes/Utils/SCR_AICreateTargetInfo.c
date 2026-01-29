class SCR_AICreateTargetInfo : AITaskScripted
{
	protected static const string TARGET_INFO_PORT = "TargetInfo";
	
	protected static const string ENTITY_PORT = "Entity";
	protected static const string LAST_SEEN_POSITION_PORT = "LastSeenPosition";
	protected static const string TIME_SINCE_SEEN_PORT = "TimeSinceSeen";
	
	ref SCR_AITargetInfo m_TargetInfo;
	
	protected ref TStringArray s_aVarsIn = {
		ENTITY_PORT,
		LAST_SEEN_POSITION_PORT,
		TIME_SINCE_SEEN_PORT,
	};
	protected ref TStringArray s_aVarsOut = {TARGET_INFO_PORT};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	override TStringArray GetVariablesOut() { return s_aVarsOut; }	
	
	static override bool VisibleInPalette() { return true; }
	
	static override string GetOnHoverDescription() { return "Create SCR_TargetInfo object from its member variables"; };
	
	//----------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		m_TargetInfo = new SCR_AITargetInfo();
		
		if(!GetVariableIn(ENTITY_PORT, m_TargetInfo.m_Entity))
			return NodeError(this, owner, "No entity provided!");
		
		GetVariableIn(LAST_SEEN_POSITION_PORT, m_TargetInfo.m_vWorldPos);
		GetVariableIn(TIME_SINCE_SEEN_PORT, m_TargetInfo.m_fTimestamp);
		SetVariableOut(TARGET_INFO_PORT, m_TargetInfo);
		return ENodeResult.SUCCESS;
	}
}