class SCR_AIGetActionPriorityLevel : SCR_AIActionTask
{
	protected static const string PORT_PRIORITY_LEVEL = "PriorityLevel";
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIActionBase action = GetExecutedAction();
		
		if (!action)
			return ENodeResult.FAIL;
		
		float priorityLevel = action.EvaluatePriorityLevel();
		
		SetVariableOut(PORT_PRIORITY_LEVEL, priorityLevel);
		
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette() { return true; }
	
	protected static ref TStringArray s_aVarsOut = { PORT_PRIORITY_LEVEL };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
};