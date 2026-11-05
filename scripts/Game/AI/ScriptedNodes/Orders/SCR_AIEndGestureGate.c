class SCR_AIEndGestureGate : AITaskScripted
{
	static const string ABORT_ORDER = "AbortFastOut";
	static const string RELATED_INVOKER = "RelatedInvokerIn";
	static const string ANIMATION_INDEX = "AnimationIndexIn";
	ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> m_RelatedInvoker;
	protected bool m_bShouldSucceed;
	protected bool m_bAbortFast;
	protected AIAgent m_ParentGroup, m_OwnerAgent;
	protected int m_iAnimationIndex;

	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		if (!GetVariableIn(RELATED_INVOKER, m_RelatedInvoker) || !m_RelatedInvoker)
			Print("SCR_AIEndGestureOrder: missing invoker!", LogLevel.ERROR);
		else
			m_RelatedInvoker.Insert(OnAnimationCancel);

		GetVariableIn(ANIMATION_INDEX, m_iAnimationIndex);
		m_bShouldSucceed = false;
		m_OwnerAgent = owner;
		m_ParentGroup = owner.GetParentGroup();
	}

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (m_bShouldSucceed)
		{
			SetVariableOut(ABORT_ORDER, m_bAbortFast);
			return ENodeResult.SUCCESS;
		}

		return ENodeResult.RUNNING;
	}

	//------------------------------------------------------------------------------------------------
	void OnAnimationCancel(AIAgent agent, bool startingAnimation, bool abortFast, int animationIndex)
	{
		if (animationIndex == m_iAnimationIndex && !startingAnimation && (m_ParentGroup == agent || m_OwnerAgent == agent))
		{
			m_bAbortFast = abortFast;
			m_bShouldSucceed = true;
			m_RelatedInvoker.Remove(OnAnimationCancel);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		ABORT_ORDER
	};

	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		ANIMATION_INDEX,
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
		return "SCR_AIEndProcessGestureOrder: waits for gesture order abort signal provided by invoker and returns true when triggered";
	}

#ifdef AI_DEBUG
	//------------------------------------------------------------------------------------------------
	protected void AddDebugMessage(AIAgent agent, string str)
	{
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(agent.FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(str, msgType: EAIDebugMsgType.MAILBOX);
	}
#endif // AI_DEBUG
}
