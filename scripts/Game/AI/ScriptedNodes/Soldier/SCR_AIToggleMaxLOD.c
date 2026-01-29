class SCR_AIToggleMaxLOD : AITaskScripted
{
	static const string PORT_AGENT	= "AgentIn";
	
	[Attribute("1", UIWidgets.CheckBox, desc: "Enable MaxLOD prevention?")]
	protected bool m_bPreventMaxLOD;
	
	[Attribute("0", UIWidgets.CheckBox, desc: "Execute in OnAbort?")]
	protected bool m_bPerformOnAbort;
	
	protected bool m_bAbortFinished;
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_bAbortFinished = false;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_bPerformOnAbort)
			return ToggleMaxLOD(owner);
		return ENodeResult.RUNNING;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	protected ENodeResult ToggleMaxLOD(AIAgent owner)
	{
		AIAgent agent;
		if (!GetVariableIn(PORT_AGENT, agent))
			return NodeError(this, owner, "No agent provided!");
		if (!agent)
			return ENodeResult.SUCCESS;
		
		if (m_bPreventMaxLOD)
			agent.PreventMaxLOD();
		else
			agent.AllowMaxLOD();
		return ENodeResult.SUCCESS;	
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (m_bPerformOnAbort && !m_bAbortFinished)
		{
			ToggleMaxLOD(owner);
			m_bAbortFinished = true;
		}
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_AGENT
	};
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	};
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	static override bool CanReturnRunning()
	{
		return true;
	};
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	};
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	override string GetNodeMiddleText()
	{
		string value;
		if (m_bPreventMaxLOD)
			value = "PREVENTED";
		else
			value = "ALLOWED";
		return "MaxLOD is "+ value;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "PreventMaxLOD: prevents AIAgent to change to MAX lod - that disactivates AI. Use attribute to enable/disable the prevention.";
	};
};