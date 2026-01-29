class SCR_AIGetTargetInfoProperties : AITaskScripted
{
	protected static const string TARGET_INFO_PORT = "TargetInfo";
	
	protected static const string ENTITY_PORT = "Entity";
	protected static const string LAST_SEEN_POSITION_PORT = "LastSeenPosition";
	protected static const string TIME_SINCE_SEEN_PORT = "TimeSinceSeen";	
	
	protected ref TStringArray s_aVarsIn = {TARGET_INFO_PORT};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected ref TStringArray s_aVarsOut = {
		ENTITY_PORT,
		LAST_SEEN_POSITION_PORT,
		TIME_SINCE_SEEN_PORT,		
	};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
	
	static override string GetOnHoverDescription() { return "Returns properties of provided BaseTarget or TargetInfo"; };
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		BaseTarget baseTarget;
		SCR_AITargetInfo targetInfo;
		
		if(!GetVariableIn(TARGET_INFO_PORT, targetInfo))
			return ENodeResult.FAIL;
		
		if (targetInfo)
		{
			SetVariableOut(ENTITY_PORT, targetInfo.m_Entity);
			SetVariableOut(LAST_SEEN_POSITION_PORT, targetInfo.m_vWorldPos);
			SetVariableOut(TIME_SINCE_SEEN_PORT, targetInfo.m_fTimestamp);			
			return ENodeResult.SUCCESS;
		}
		return ENodeResult.FAIL;
	}
}