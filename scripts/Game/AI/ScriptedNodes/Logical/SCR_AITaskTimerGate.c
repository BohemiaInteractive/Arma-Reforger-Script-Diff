class SCR_AITaskTimerGate: SCR_AITaskTimer
{
	//------------------------------------------------------------------------------------------------
		
	override void OnEnter(AIAgent owner)
	{
		
	}	
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (GetVariableType(true, "Time") == float)
		{
			GetVariableIn("Time", m_Time);
			if ( m_Time < 0)
				isInfinite = true; 
		}; 
		
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
			bool res = elapsedTime > m_Time && !isInfinite;
			SetVariableOut("Trigger", res);
		}
		
		if (GetVariableType(false, "ElapsedTime") == float)
		{
			SetVariableOut("ElapsedTime", elapsedTime);
		}
		
		if (elapsedTime < m_Time || isInfinite)
		{
			return ENodeResult.RUNNING;
		}
		else
		{
			elapsedTime = 0;
			return ENodeResult.SUCCESS;
		}
		
		
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Task Timer 2: similar to Task Timer, only is running while time has not elapsed";
	}
	
	//------------------------------------------------------------------------------------------------
		
	static override protected bool CanReturnRunning()
	{
		return true;
	}
};