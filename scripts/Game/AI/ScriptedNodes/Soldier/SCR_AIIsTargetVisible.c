// Script File//------------------------------------------------------------------------------------------------
class SCR_AIIsTargetVisible : DecoratorScripted
{
	static const string BASE_TARGET_PORT = "BaseTargetIn";
	
	SCR_AICombatComponent m_CombatComp;
	
	//-----------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_CombatComp)
		{
			IEntity controlledEntity = owner.GetControlledEntity();
			if (controlledEntity)
				m_CombatComp = SCR_AICombatComponent.Cast(owner.FindComponent(SCR_AICombatComponent));
			
			if (!m_CombatComp)
					return false;
		}
			
		BaseTarget target;
		GetVariableIn(BASE_TARGET_PORT,target);
		
		if (!target)
			return false;
		
		bool visible = m_CombatComp.IsTargetVisible(target);
		
		return visible;
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}	
	
	//-----------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "IsTargetVisible: Checks if target is visible. The check is performed through Combat Component.";
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		BASE_TARGET_PORT
	};
	protected override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
};
