class SCR_AISetBehaviorAllowLook : AITaskScripted
{
	[Attribute()]
	protected bool m_bAllowLook;
	
	private SCR_AIBehaviorBase m_CurrentBehavior;
	
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
		
		m_CurrentBehavior.m_bAllowLook = m_bAllowLook;
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------	
	protected static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription() {return "Sets value of m_bAllowLook in current behavior.";}
	
	//------------------------------------------------------------------------------------------------
	override string GetNodeMiddleText()
	{
		return string.Format("Allow Look: %1", m_bAllowLook);
	}
};