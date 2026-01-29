// Node for debug and test purpouses
class SCR_AIDebugCounter : AITaskScripted
{	
	[Attribute("0", UIWidgets.CheckBox, "Fail node after simulate?")]
	protected bool m_bFailAfter;
	
	[Attribute("0", UIWidgets.CheckBox, "Running until OnAbort")]
	protected bool m_bWaitOnAbort;
	
	[Attribute("0", UIWidgets.CheckBox, "Increment OnAbort")]
	protected bool m_bIncrementOnAbort;
	
	[Attribute("", UIWidgets.Auto, "Echo when increased")]
	protected string m_sDebugMessage;
	
	static const string PORT_VAR_IN = "Counter";
	static const string PORT_VAR_OUT= "Out"; 
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_VAR_IN
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_VAR_OUT
	};
    override array<string> GetVariablesOut()
    {
        return s_aVarsOut;
    }

	//------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (m_bIncrementOnAbort)
		{
			int count;
			GetVariableIn(PORT_VAR_IN, count);
			count++;
			if (m_sDebugMessage != string.Empty)
				Print("DebugCounter: " + m_sDebugMessage);
			SetVariableOut(PORT_VAR_OUT, count);
		}
	}

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{		
		if (m_bWaitOnAbort)
			return ENodeResult.RUNNING;
		
		int count;
		GetVariableIn(PORT_VAR_IN, count);
		count++;
		if (m_sDebugMessage != string.Empty)
			Print("DebugCounter: " + m_sDebugMessage);
		SetVariableOut(PORT_VAR_OUT, count);
		
		if (m_bFailAfter)
			return ENodeResult.FAIL;

		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------	
	static override protected bool CanReturnRunning()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Increments variable for debug with configurable node behavior.";
	}		
	
	//------------------------------------------------------------------------------------------------
	override string GetNodeMiddleText()
	{
		string text = "";
		if (m_bIncrementOnAbort)
			text = "Increment OnAbort\n";
		if (m_bWaitOnAbort)
			return text + "RUNNING";
		if (m_bFailAfter)
			return text + "FAILS";
		else 
			return text + "SUCESS";
	}
};
