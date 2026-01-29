class SCR_AIAnimateBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParamRef<IEntity> m_RootEntity = new SCR_BTParamRef<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParamRef<SCR_AIAnimationScript> m_AgentScript = new SCR_BTParamRef<SCR_AIAnimationScript>(SCR_AIActionTask.AGENT_SCRIPT_PORT);
	ref SCR_BTParamRef<ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction>> m_RelatedInvoker = new SCR_BTParamRef<ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction>>(SCR_AIActionTask.RELATED_INVOKER_PORT);
	
	protected int m_iAnimationIndex;
	protected SCR_AIAnimateActivity m_animateActivity;
		
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIAnimateBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, IEntity rootEntity, float priority = PRIORITY_BEHAVIOR_ANIMATE, float priorityLevel = PRIORITY_LEVEL_NORMAL, SCR_AIAnimationScript scriptForAgent = null, ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> relatedInvoker = null)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Behavior_Animate.bt";
		m_RootEntity.Init(this, rootEntity);
		m_AgentScript.Init(this, scriptForAgent);
		m_RelatedInvoker.Init(this, relatedInvoker);
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.SAFE;
	}
	
	//-----------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		m_animateActivity = SCR_AIAnimateActivity.Cast(GetRelatedGroupActivity());
		if (!m_animateActivity)
		{
			PrintFormat("Behavior %1 does not have proper related activity %2!", this, GetRelatedGroupActivity());
			Fail();
		}
		m_animateActivity.AddAgentToAnimatedAgents(m_Utility.GetOwner());
	}
	
	//-----------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		if (m_animateActivity)
			m_animateActivity.RemoveAgentFromAnimatedAgents(m_Utility.GetOwner(), false);
	}
	
	//-----------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		if (m_animateActivity)
			m_animateActivity.RemoveAgentFromAnimatedAgents(m_Utility.GetOwner(), true);
	}
};

class SCR_AIGetAnimateBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIAnimateBehavior(null, null, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};
