 class SCR_AIDecoHasActionOfType : DecoratorScripted
{
	[Attribute("", UIWidgets.EditBox, "Write type name of the action class")]
	protected string m_sActionType;
	
	protected SCR_AIBaseUtilityComponent m_Utility;
	
	//------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_Utility)
		{
			m_Utility = SCR_AIBaseUtilityComponent.Cast(owner.FindComponent(SCR_AIBaseUtilityComponent));
			if (!m_Utility)
				return false;
		}	
		
		return m_Utility.HasActionOfType(m_sActionType.ToType());
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Returns true when we have an action of specified type";
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetNodeMiddleText()
	{
		return string.Format("Action type: %1", m_sActionType);
	}
};