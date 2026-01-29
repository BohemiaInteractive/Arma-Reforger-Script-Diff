class SCR_AIBaseTargetToTargetInfo : AITaskScripted
{
	protected static const string TARGET_INFO_PORT = "TargetInfoOut";
	
	protected static const string BASE_TARGET = "BaseTargetIn";
	
	protected ref TStringArray s_aVarsOut = {TARGET_INFO_PORT};
	protected ref TStringArray s_aVarsIn = {BASE_TARGET};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	ref SCR_AITargetInfo m_TargetInfo;
		
	static override bool VisibleInPalette() { return true; }
	
	static override string GetOnHoverDescription() { return "Converts baseTarget object instance to SCR_AITarget info object instance"; };
	
	//---------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		BaseTarget baseTarget;
		
		if (!GetVariableIn(BASE_TARGET, baseTarget))
			return NodeError(this, owner, "No base target provided!");
		m_TargetInfo = new SCR_AITargetInfo();
		m_TargetInfo.InitFromBaseTarget(baseTarget);
		SetVariableOut(TARGET_INFO_PORT, m_TargetInfo);
		return ENodeResult.SUCCESS;
	}
}