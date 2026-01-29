class SCR_AIDecoBehaviorAllowsLooking : DecoratorScripted
{
	protected SCR_AIUtilityComponent m_Utility;
	
	//------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_Utility)
		{
			m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
			if (!m_Utility)
				return false;
		}
		
		return !m_Utility.m_CurrentBehavior || m_Utility.m_CurrentBehavior.m_bAllowLook;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Returns true if current behavior allows looking";
	}
};