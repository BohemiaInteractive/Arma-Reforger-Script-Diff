class SCR_AIPlayAnimation : AITaskScripted
{
	static const string PORT_ENTITY_IN				= "RootEntityIn";
	static const string PORT_AGENT_SCRIPT_IN		= "AgentScriptIn";
	static const string PORT_ANIMATION_INDEX_IN		= "AnimationIndexIn";
	static const string PORT_ABORT_SLOW				= "AbortSlowIn";
	static const string PORT_RELATED_INVOKER		= "RelatedInvoker";
	
	protected SCR_AIAnimation_Base m_AIAnimation;
	protected bool m_bAbortDone;
	protected bool m_bInPlayed;	
	protected ref ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> m_OnAnimationBehaviorAction; // related invoker for signaling the starting or stopping of some animation
	protected int m_iAnimationIndex;
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		m_bAbortDone = false;
		m_bInPlayed = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		vector mat[4];
		if (!m_bInPlayed)
		{
			IEntity rootEntity;
			if (!GetVariableIn(PORT_ENTITY_IN, rootEntity))
				return ENodeResult.FAIL;
			
			SCR_AIAnimationScript agentScript;
			GetVariableIn(PORT_AGENT_SCRIPT_IN, agentScript);
			GetVariableIn(PORT_ANIMATION_INDEX_IN, m_iAnimationIndex);
			if (!rootEntity || !agentScript || !agentScript.IsAnimationIndexValid(m_iAnimationIndex))
				return ENodeResult.FAIL;
			agentScript.GetAnimationWorldTransform(rootEntity, m_iAnimationIndex, mat);
			m_AIAnimation = agentScript.GetAnimationClass(m_iAnimationIndex);
			if (!m_AIAnimation)
				return ENodeResult.FAIL;
			GetVariableIn(PORT_RELATED_INVOKER, m_OnAnimationBehaviorAction);
			if (m_OnAnimationBehaviorAction)
				m_OnAnimationBehaviorAction.Invoke(owner, true, m_iAnimationIndex);
			
			m_AIAnimation.StartAnimation(owner.GetControlledEntity(), mat);
			m_bInPlayed = true;
		}
		
		return ENodeResult.RUNNING;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (!m_bAbortDone && m_AIAnimation && m_bInPlayed)
		{
			bool abortSlow;
			GetVariableIn(PORT_ABORT_SLOW, abortSlow);
			if (m_OnAnimationBehaviorAction)
				m_OnAnimationBehaviorAction.Invoke(owner, false, m_iAnimationIndex);
			m_AIAnimation.StopAnimation(owner.GetControlledEntity(), !abortSlow);
		}	
		m_bAbortDone = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette() { return true; }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {	
		PORT_ENTITY_IN,
		PORT_AGENT_SCRIPT_IN, 
		PORT_ANIMATION_INDEX_IN,
		PORT_ABORT_SLOW,
		PORT_RELATED_INVOKER,
	};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool CanReturnRunning() {return true;}
	
	//------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "PlayAnimation: plays animation stored in SCR_AIAnimationScript given by index of that animation and root entity (usually the waypoint). OnAbort plays OUT animation.\nThe node does not check when animation ends! Must be aborted from above!";
	}
};