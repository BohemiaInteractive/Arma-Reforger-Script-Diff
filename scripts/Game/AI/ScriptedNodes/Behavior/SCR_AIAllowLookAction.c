class SCR_AIAllowLookAction : AITaskScripted
{
	private SCR_AIBehaviorBase m_CurrentBehavior;
	private bool m_bLookAllowed;

	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		m_CurrentBehavior = utility.GetCurrentBehavior();
	}			
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_CurrentBehavior)
			return ENodeResult.FAIL;
		
		if(!m_CurrentBehavior.m_bAllowLook)
		{
			m_CurrentBehavior.m_bAllowLook = true;
			m_bLookAllowed = true;
		}	
		
		if (m_bLookAllowed)
			return ENodeResult.RUNNING;
		else 
			return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (m_bLookAllowed)
		{
			m_bLookAllowed = false;
			if (m_CurrentBehavior)
				m_CurrentBehavior.m_bAllowLook = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	protected static override bool VisibleInPalette() {return true;}	
	
	//------------------------------------------------------------------------------------------------
    static override bool CanReturnRunning() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription() {return "Node that allows free look of AI unit and disallows it on abort.";}
};