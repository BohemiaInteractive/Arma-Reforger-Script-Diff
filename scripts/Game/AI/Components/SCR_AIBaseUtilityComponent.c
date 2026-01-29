[ComponentEditorProps(category: "GameScripted/AI", description: "Component for utility AI system calculations")]
class SCR_AIBaseUtilityComponentClass : AIBaseUtilityComponentClass
{
}

// TODO:
// - For handling orders, use enums

class SCR_AIBaseUtilityComponent : AIBaseUtilityComponent
{
	// This can be set externally to request a breakpoint on next EvaluateBehavior
	#ifdef AI_DEBUG
	protected bool m_bEvaluationBreakpoint;
	protected int m_iDiagCounter = 0;
	#endif
		
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] msg
	//! \param[in] receiver
	void SendMessage(AIMessage msg, AIAgent receiver = null)
	{
		AICommunicationComponent commsComp = GetAIAgent().GetCommunicationComponent();
		if (!commsComp)
			return;
		
		commsComp.RequestBroadcast(msg, receiver);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	int DiagGetCounter()
	{
		#ifdef AI_DEBUG
		return m_iDiagCounter;
		#else
		return 0;
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void DiagIncreaseCounter()
	{
		#ifdef AI_DEBUG
		m_iDiagCounter++;
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void DiagSetBreakpoint()
	{
		#ifdef AI_DEBUG
		m_bEvaluationBreakpoint = true;
		#endif
	}
	
	#ifdef AI_DEBUG
	//------------------------------------------------------------------------------------------------
	protected void AddDebugMessage(string str, EAIDebugMsgType messageType = EAIDebugMsgType.UTILITY)
	{
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(GetOwner().FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(str, msgType: messageType);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DebugLogActionsPriority()
	{
		array<ref AIActionBase> actions = {};
		GetActions(actions);
		foreach (AIActionBase action : actions)
		{
			string actionStr = GetActionLogString(action);
			AddDebugMessage(actionStr);
			
			SCR_AICompositeActionParallel parallelAction = SCR_AICompositeActionParallel.Cast(action);
			if (parallelAction)
			{
				array<AIActionBase> subactions = {};
				parallelAction.GetSubactions(subactions);
				foreach (AIActionBase subaction : subactions)
				{
					string subactionStr = "  " + GetActionLogString(subaction);
					AddDebugMessage(subactionStr);
				}
			}
			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetActionLogString(AIActionBase action)
	{
		SCR_AIActionBase scrActionBase = SCR_AIActionBase.Cast(action);
			
		string _strSuspended = string.Empty;
		if (action.GetActionIsSuspended())
			_strSuspended = "(S) ";
		float score = action.Evaluate();
		string _strDebugInfo;
		if (scrActionBase)
			_strDebugInfo = scrActionBase.GetActionDebugInfo();
		
		return string.Format("    %1%2 %3 %4", _strSuspended, score.ToString(3, 1), action.Type().ToString(), _strDebugInfo);
	}
	
	#endif
}
