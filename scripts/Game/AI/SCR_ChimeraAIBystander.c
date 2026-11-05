class SCR_ChimeraAIBystanderClass : SCR_ChimeraAIAgentClass
{
}

class SCR_ChimeraAIBystander : SCR_ChimeraAIAgent
{
	protected ref ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> m_OnAnimationBehaviorActionInvoker;
	protected bool m_bIsInvokerDisabled;		// if invoker is disabled, potential listener can see no invoke will
												// relevant for situation when adding a listener after invoker was invoked

	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> GetOnAnimationBehaviorAction()
	{
		if (!m_bIsInvokerDisabled && !m_OnAnimationBehaviorActionInvoker)
			m_OnAnimationBehaviorActionInvoker = new ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction>();
		
		return m_OnAnimationBehaviorActionInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	void ToggleAnimationInvoker(bool disableInvoker)
	{
		m_bIsInvokerDisabled = disableInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsInvokerDisabled()
	{
		return m_bIsInvokerDisabled;
	}

	//------------------------------------------------------------------------------------------------
	//! invokes invokers on bystander and on its CrowdGroup if it has one
	void InvokeRelevantInvokerToEndGesture(bool abortFast, bool disableCharacterInvoker)
	{
		ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> invoker;
		SCR_CrowdManagerGroup myGroup = SCR_CrowdManagerGroup.Cast(GetParentGroup());

		if (myGroup)
		{
			invoker = myGroup.GetOnAnimationBehaviorAction_Group();
		}	
		else
		{
			ToggleAnimationInvoker(disableCharacterInvoker);
			invoker = GetOnAnimationBehaviorAction();
		}
		
		if (invoker)
			invoker.Invoke(this, false, abortFast, 0);
	}
}
