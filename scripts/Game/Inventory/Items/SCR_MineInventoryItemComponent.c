[EntityEditorProps(category: "GameScripted/Components", description: "Mine inventory item component.")]
class SCR_MineInventoryItemComponentClass : SCR_PlaceableInventoryItemComponentClass
{
}

class SCR_MineInventoryItemComponent : SCR_PlaceableInventoryItemComponent
{
	//! called locally for character that placed it
	protected static ref ScriptInvokerInt s_onMinePlaced;
	
	//------------------------------------------------------------------------------------------------
	protected override void PlacementDone(notnull ChimeraCharacter user)
	{
		super.PlacementDone(user);

		RplComponent rplComp = user.GetRplComponent();
		if (!rplComp || rplComp.IsProxy())
			return;

		SCR_PressureTriggerComponent pressureTrigger = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (!pressureTrigger)
			return;

		pressureTrigger.SetUser(user);

		if (s_onMinePlaced)
			s_onMinePlaced.Invoke(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool ShouldHideInVicinity()
	{
		SCR_PressureTriggerComponent triggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (!triggerComponent)
			return false;
		
		return triggerComponent.IsActivated();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvokerInt GetOnMinePlaced()
	{
		if (!s_onMinePlaced)
			s_onMinePlaced = new ScriptInvokerInt();
		
		return s_onMinePlaced;
	}
}
