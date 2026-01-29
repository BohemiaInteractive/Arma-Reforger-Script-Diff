class SCR_AIActivateAllBirds: AITaskScripted
{
	
	[Attribute("false", UIWidgets.CheckBox, "If true the birds will be activated, if false they will be deactivated")]
	private bool m_activateBirds;
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		
		AIFlock flock = AIFlock.Cast(owner);
		if(!flock)
		{
			NodeError(this,owner,"Owner is not a flock. Can't activate.");
			return ENodeResult.FAIL;
		}
		
		flock.ActivateAllBirds(m_activateBirds);
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
		return "Active All Birds: Broadcast to all clients if they should activete/deactivete their individual birds";
	}
	
	//------------------------------------------------------------------------------------------------
		
	static override protected bool CanReturnRunning()
	{
		return false;
	}
};