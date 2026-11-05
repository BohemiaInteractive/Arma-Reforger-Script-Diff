class SCR_AIEndGesture_InvokeOnAbort : AITaskScripted
{
	static const string RELATED_INVOKER = "RelatedInvokerIn";
	protected bool m_bAbortDone;

	//------------------------------------------------------------------------------------------------
	override event void OnEnter(AIAgent owner)
	{
		m_bAbortDone = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		return ENodeResult.RUNNING;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (m_bAbortDone)
			return;
		
		ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> relatedInvoker;
		GetVariableIn(RELATED_INVOKER, relatedInvoker);
		if (!relatedInvoker)
			return;
		
		relatedInvoker.Invoke(owner, false, false, 0);
		m_bAbortDone = true;
	}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		RELATED_INVOKER
	};

	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}

	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static override bool CanReturnRunning()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "SCR_AIEndGestureInvoke: invokes a provided invoker upon abort of the node.";
	}
}
