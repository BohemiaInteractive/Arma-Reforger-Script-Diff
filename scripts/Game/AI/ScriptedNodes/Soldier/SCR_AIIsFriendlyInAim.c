class SCR_AIIsFriendlyInAim : DecoratorScripted
{
	protected PerceptionComponent m_PerceptionComponent;
	#ifdef WORKBENCH
	protected ref Shape m_FriendlyAimShape;
	#endif
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_PerceptionComponent)
			return false;
	
		IEntity friendlyEntInAim = m_PerceptionComponent.GetFriendlyInLineOfFire();
#ifdef WORKBENCH
		if (friendlyEntInAim && DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_FRIENDLY_IN_AIM))
			m_FriendlyAimShape = Shape.CreateSphere(COLOR_RED, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, friendlyEntInAim.GetOrigin() + Vector(0, 2, 0), 0.1);	
		else 
			m_FriendlyAimShape = null;
#endif		
		
		return friendlyEntInAim != null;
	}
		
	override protected void OnInit(AIAgent owner)
	{
		IEntity ent = owner.GetControlledEntity();
		
		if (!ent)
			NodeError(this, owner, "SCR_AIIsFriendlyInAim must be used with soldiers");
		
		m_PerceptionComponent = PerceptionComponent.Cast(ent.FindComponent(PerceptionComponent));
		
		if (!m_PerceptionComponent)
			NodeError(this, owner, "Didn't find PerceptionComponent");
	}
	
	protected static override string GetOnHoverDescription()
	{
		return "SCR_AIIsFriendlyInAim: Returns true when we have a friendly unit in aim. Used for checking against friendly fire.";
	}
};
