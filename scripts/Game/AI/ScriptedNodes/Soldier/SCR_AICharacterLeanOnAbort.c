class SCR_AICharacterLeanOnAbort: AITaskScripted
{
	protected bool m_bAbortDone;
	
	//-----------------------------------------------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------
	protected static override bool CanReturnRunning()
	{
		return true;	
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------
	protected override void OnEnter(AIAgent owner)
	{
		m_bAbortDone = false;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------
	protected override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		return ENodeResult.RUNNING;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------
	protected override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (m_bAbortDone)
			return;
		IEntity ent = owner.GetControlledEntity();
		if (!ent)
		{
			NodeError(this, owner, "Abort called on AIAgent without entity. Invalid usage.");
		};
		CharacterControllerComponent cc = CharacterControllerComponent.Cast(ent.FindComponent(CharacterControllerComponent));
		if (cc && cc.IsLeaning())
		{
			cc.SetWantedLeaning(0);
		};
		m_bAbortDone = true;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------------------------
	override static string GetOnHoverDescription()
	{
		return "Clears leaning state of owner entity OnAbort";
	}
};