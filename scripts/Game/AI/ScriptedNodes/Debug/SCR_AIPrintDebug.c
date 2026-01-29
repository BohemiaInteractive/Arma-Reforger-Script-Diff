// BT node for printing debug messsages
class SCR_AIPrintDebug : SCR_AIDebugTask
{
	//------------------------------------------------------------------------------------------------
	protected override void PrintInternal(string in)
	{
		Print(in);
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
//switch defines when neede to debug on server
//#ifdef AI_DEBUG
#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_PRINT_DEBUG))
		{		
			return super.EOnTaskSimulate(owner, dt);
		}
#endif
		if (m_bFailAfter)
			return ENodeResult.FAIL;
		else
			return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "BT node for printing debug messsages. String can be in In port or as parameter";
	}
};
