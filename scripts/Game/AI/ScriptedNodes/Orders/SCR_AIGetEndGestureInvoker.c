class SCR_AIGetEndGestureInvoker : AITaskScripted
{
	static const string RELATED_INVOKER = "RelatedInvokerOut";
	protected ref ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> m_RelatedInvoker;
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_ChimeraAIBystander bystander = SCR_ChimeraAIBystander.Cast(owner);
		if (!bystander)
			return ENodeResult.FAIL;
		
		m_RelatedInvoker = bystander.GetOnAnimationBehaviorAction();
		SetVariableOut(RELATED_INVOKER, m_RelatedInvoker);				
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		RELATED_INVOKER
	};

	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}

	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "SCR_AIGetEndGestureInvoker: Gets 'End gesture invoker' on this agent. Other may exist on agent's CrowdGroup that may or not exist.";
	}
}
