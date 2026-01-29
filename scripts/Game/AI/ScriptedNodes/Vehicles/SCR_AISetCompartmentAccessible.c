class SCR_AISetCompartmentAccessible : AITaskScripted
{
	static const string COMPARTMENT_PORT = "CompartmentIn";
	
	protected bool m_bAbortDone;
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		m_bAbortDone = false;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		return ENodeResult.RUNNING;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------	
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (m_bAbortDone)
			return;
		
		m_bAbortDone = true;
		BaseCompartmentSlot compartment;
		if (!GetVariableIn(COMPARTMENT_PORT, compartment))
			return;
		compartment.SetCompartmentAccessible(true);
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		COMPARTMENT_PORT
	};
	
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	};
	
	static override bool VisibleInPalette()
	{
		return true;
	};
	
	static override string GetOnHoverDescription()
	{
		return "SetCompartmnetAccessible: makes the compartment accessible by other AIs OnAbort";
	};
	
	static override bool CanReturnRunning()
	{
		return true;
	};
};