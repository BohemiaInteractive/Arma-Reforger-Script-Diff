class SCR_AIDecoHasRole : DecoratorScripted
{
	[Attribute("0", UIWidgets.ComboBox, "Role to find:", "", ParamEnumArray.FromEnum(EUnitRole) )]
	private EUnitRole m_eUnitRole;
		
	//-------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(owner);
		if (!chimeraAgent)
			SCR_AgentMustChimera(this, owner);
		if (!chimeraAgent.m_InfoComponent)
			NodeError(this, owner, "Agent does not have info component!");
		return chimeraAgent.m_InfoComponent.HasRole(m_eUnitRole);
	}	
	
	//-------------------------------------------------------------------	
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//-------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Decorator that tests if character has requested role";
	}	
	
	//-------------------------------------------------------------------
	override string GetNodeMiddleText() 
	{ 
		return "Test " + typename.EnumToString(EUnitRole,m_eUnitRole);	
	};
};