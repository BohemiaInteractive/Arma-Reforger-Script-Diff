class SCR_AIDecoCombatEnemy : DecoratorScripted
{
	SCR_AICombatComponent m_CombatComponent;
	
	//-------------------------------------------------------------------------------------------
	//! Override in child class
	protected BaseTarget GetEnemy(SCR_AICombatComponent combatComp);
	
	//-------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_CombatComponent)
			return false;
		
		bool retVal = GetEnemy(m_CombatComponent) != null;
		return retVal;
	}
	
	//-------------------------------------------------------------------------------------------
	protected override void OnInit(AIAgent owner)
	{
		m_CombatComponent = SCR_AICombatComponent.Cast(owner.GetControlledEntity().FindComponent(SCR_AICombatComponent));
	}
	
	//-------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return false; }
};

class SCR_AIDecoCurrentEnemy: SCR_AIDecoCombatEnemy
{
	//-------------------------------------------------------------------------------------------
	override protected BaseTarget GetEnemy(SCR_AICombatComponent combatComp)
	{
		return combatComp.GetCurrentTarget();
	}
	
	//-------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Returns true when SCR_AICombatComponent.GetCurrentTarget() != null.";
	}
	
	//-------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true; }
};

class SCR_AIDecoLastSeenEnemy: SCR_AIDecoCombatEnemy
{
	//-------------------------------------------------------------------------------------------
	override protected BaseTarget GetEnemy(SCR_AICombatComponent combatComp)
	{
		return combatComp.GetLastSeenEnemy();
	}
	
	//-------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Returns true when SCR_AICombatComponent.GetCurrentTarget() != null.";
	}
	
	//-------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true; }
};