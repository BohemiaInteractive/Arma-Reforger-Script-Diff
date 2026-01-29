// base class for debug nodes. override PrintInternal in child class
class SCR_AIDebugTask : AITaskScripted
{
	[Attribute("0", UIWidgets.CheckBox, "Message prefixed with ref to self")]
	private bool m_bIncludeMe;
	
	[Attribute("", UIWidgets.EditBox, "Message to show")]
	private string m_sDebugMessage;
	
	[Attribute("0", UIWidgets.CheckBox, "Fail node after print?")]
	protected bool m_bFailAfter;
	
	static const string PORT_DEBUG_MESSAGE = "DebugMessage"; 
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_DEBUG_MESSAGE
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	// override this in child class
	//------------------------------------------------------------------------------------------------
	protected void PrintInternal(string in)
	{
	}

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
//switch defines when neede to debug on server
//#ifdef AI_DEBUG
#ifdef WORKBENCH
		string prefix;
		if ( m_bIncludeMe )
			prefix = owner.ToString() + ": ";
		PrintInternal(prefix + m_sDebugMessage + SCR_AINodePortsHelpers.GetStringFromPort(this, PORT_DEBUG_MESSAGE));
#endif
		if (m_bFailAfter)
			return ENodeResult.FAIL;
		else 
			return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return false;
	}	
	
	//------------------------------------------------------------------------------------------------
	override string GetNodeMiddleText()
	{
		if (m_bFailAfter)
			return "FAILS";
		else 
			return "";
	}
};
