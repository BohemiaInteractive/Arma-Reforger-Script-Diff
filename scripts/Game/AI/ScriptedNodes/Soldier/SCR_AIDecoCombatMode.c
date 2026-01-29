class SCR_AIDecoCombatMode : DecoratorScripted
{
	[Attribute("0", UIWidgets.ComboBox, "", enums:ParamEnumArray.FromEnum(EAIGroupCombatMode) )]
	protected EAIGroupCombatMode m_eCombatMode;
	
	protected SCR_AICombatComponent m_CombatComponent;
		
	//-------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		
		// Group
		if (group)
		{
			return group.GetGroupUtilityComponent().GetCombatModeActual() == m_eCombatMode;
		}
		
		// Soldier
		if (!m_CombatComponent)
		{
			IEntity controlledEntity = owner.GetControlledEntity();
			if (controlledEntity)
				m_CombatComponent = SCR_AICombatComponent.Cast(controlledEntity.FindComponent(SCR_AICombatComponent));
		}
		
		if (!m_CombatComponent)
			return false;
		
		return m_CombatComponent.GetCombatMode() == m_eCombatMode;
	}	
	
	//-------------------------------------------------------------------	
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//-------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Checks if agent is in this combat mode. Works both for soldier and group.";
	}	
	
	//-------------------------------------------------------------------
	override string GetNodeMiddleText() 
	{ 
		return "Combat mode: " + typename.EnumToString(EAIGroupCombatMode, m_eCombatMode);	
	};
};