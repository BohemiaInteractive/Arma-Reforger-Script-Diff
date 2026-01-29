class SCR_AIGetBaseTargetProperties : AITaskScripted
{
	protected static const string BASE_TARGET_PORT = "BaseTarget";
	
	protected static const string ENTITY_PORT = "Entity";
	protected static const string LAST_SEEN_POSITION_PORT = "LastSeenPosition";
	protected static const string TIME_SINCE_SEEN_PORT = "TimeSinceSeen";
	protected static const string ENDANGERING_PORT = "IsEndangering";
	
	protected ref TStringArray s_aVarsIn = {BASE_TARGET_PORT};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected ref TStringArray s_aVarsOut = {
		ENTITY_PORT,
		LAST_SEEN_POSITION_PORT,
		TIME_SINCE_SEEN_PORT,
		ENDANGERING_PORT
	};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
	
	static override string GetOnHoverDescription() { return "Returns properties of provided BaseTarget"; };
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		BaseTarget baseTarget;
		
		if (!GetVariableIn(BASE_TARGET_PORT, baseTarget))
			return ENodeResult.FAIL;
		
		if (!baseTarget)
			return ENodeResult.FAIL;
		
		SetVariableOut(ENTITY_PORT, baseTarget.GetTargetEntity());
		SetVariableOut(LAST_SEEN_POSITION_PORT, baseTarget.GetLastSeenPosition());
		SetVariableOut(TIME_SINCE_SEEN_PORT, baseTarget.GetTimeSinceSeen());
		SetVariableOut(ENDANGERING_PORT, baseTarget.IsEndangering());
		return ENodeResult.SUCCESS;
	}
}