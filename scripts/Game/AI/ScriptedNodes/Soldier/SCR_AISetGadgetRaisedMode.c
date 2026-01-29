class SCR_AISetGadgetRaisedMode : AITaskScripted
{
	[Attribute("0", UIWidgets.CheckBox)]
	protected bool m_bRaised;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "When true, always returns running and executes on abort, otherwise executes on simulate.")]
	private bool m_bKeepRunningUntilAborted;
	
	protected CharacterControllerComponent m_CharacterController;
	
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	override void OnInit(AIAgent owner)
	{
		IEntity controlledEntity = owner.GetControlledEntity();
		if (controlledEntity)
		{
			m_CharacterController = CharacterControllerComponent.Cast(controlledEntity.FindComponent(CharacterControllerComponent));
		}
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_CharacterController)
			return ENodeResult.FAIL;
		
		if (m_bKeepRunningUntilAborted)
			return ENodeResult.RUNNING;
		
		m_CharacterController.SetGadgetRaisedModeWanted(m_bRaised);
		return ENodeResult.SUCCESS;
	}
	
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (m_CharacterController && m_bKeepRunningUntilAborted)
		{
			m_CharacterController.SetGadgetRaisedModeWanted(m_bRaised);
		}
	}
	
	static override string GetOnHoverDescription()
	{
		return "Sets raised mode of gadget, either on simulate or on abort.";
	}
};
