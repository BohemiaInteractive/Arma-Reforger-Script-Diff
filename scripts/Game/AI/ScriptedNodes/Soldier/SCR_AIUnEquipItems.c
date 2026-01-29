class SCR_AIUnEquipItems : AITaskScripted
{
	protected SCR_GadgetManagerComponent m_GadgetManager;

	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "UnEquip on abort or alternatively on simulate" )]
	private bool m_bKeepRunningUntilAborted;
	private bool m_bHasAborted;

	// -----------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity ownerEntity = owner.GetControlledEntity();
		if (!ownerEntity)
 			NodeError(this, owner, "Missing owner entity!");
		
		m_GadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(ownerEntity);
	}

	// -----------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		m_bHasAborted = false;
	}

	// -----------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_bKeepRunningUntilAborted && m_GadgetManager)
			{
				m_GadgetManager.RemoveHeldGadget();
				return ENodeResult.SUCCESS;
			}
		
 		return ENodeResult.RUNNING;
	}

	// -----------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (m_bHasAborted)
			return;
		
		m_GadgetManager.RemoveHeldGadget();
		m_bHasAborted = true;
	}

	// -----------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		if (m_bKeepRunningUntilAborted)
			return "Item unequiped when this node is aborted";
		else 
			return "Item unequiped when node starts running";
	}
	
	// -----------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}

	// -----------------------------------------------------------------------------------------------
	static override bool CanReturnRunning()
	{
		return true;
	}
	
	// -----------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Scripted Node: UnEquip any generic item or gadget that is currently being held by the character.";
	}
};
