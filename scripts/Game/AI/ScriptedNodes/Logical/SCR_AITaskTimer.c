class SCR_AITaskTimer: AITaskScripted
{
	[Attribute("10", UIWidgets.EditBox, "Time")]
	float m_Time;
	
	//------------------------------------------------------------------------------------------------
	protected float elapsedTime;
	protected bool isInfinite;
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		if (GetVariableType(true, "Time") == float)
		{
			GetVariableIn("Time", m_Time);
			if ( m_Time < 0 ) 
				isInfinite = true;
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		bool reset;
		GetVariableIn("ResetTime", reset);
		if ( reset )
		{
			elapsedTime = 0;
			SetVariableOut("ResetTimeOut", false);	
		};		
		elapsedTime += dt;
		
		if (GetVariableType(false, "Trigger") == bool)
		{
			bool res = (elapsedTime > m_Time && !isInfinite);
			SetVariableOut("Trigger", res);
		}
		
		if (GetVariableType(false, "ElapsedTime") == float)
		{
			SetVariableOut("ElapsedTime", elapsedTime);
		}
		
		if (elapsedTime > m_Time && !isInfinite)
		{
			elapsedTime -= m_Time;
		}
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Task Timer: For triggering into output variable after specified time.";
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		"Time",
		"ResetTime"
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;		
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		"Trigger",
		"ElapsedTime",
		"ResetTimeOut"
	};
	override TStringArray GetVariablesOut()
	{	
		return s_aVarsOut;
	}
};