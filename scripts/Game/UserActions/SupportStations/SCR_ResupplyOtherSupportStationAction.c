class SCR_ResupplyOtherSupportStationAction : SCR_BaseResupplySupportStationAction
{	
	[Attribute("0", desc: "If false both player and AI have this action. If true then the resupply action is only availible for player or possessed characters")]
	protected bool m_bPlayerOnly;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{	
		//~ Hide if not player or not possessed
		if (m_bPlayerOnly && SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(GetOwner()) <= 0)
			return false;
		
		if (!SCR_ArsenalManagerComponent.IsArsenalTypeEnabled_Static(SCR_EArsenalTypes.GADGETS))
			return false;
		
		if (!super.CanBeShownScript(user))
			return false; 
		
		bool canPerform = super.CanBePerformedScript(user);
		if (!canPerform && GetShowButDisabled())
			return true;
		
		return canPerform;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{	
		return !GetShowButDisabled();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool RequiresGadget()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool PrioritizeHeldGadget()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		super.DelayedInit(owner);
		
		//~ Set target as owner inventory no need to ever change it
		m_InventoryManagerTarget = SCR_InventoryStorageManagerComponent.Cast(owner.FindComponent(SCR_InventoryStorageManagerComponent));
	}
}