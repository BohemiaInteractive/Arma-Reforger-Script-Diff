//------------------------------------------------------------------------------------------------
class SCR_MinePickUpItemAction : SCR_PickUpItemAction
{
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		SCR_PressureTriggerComponent mineTriggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (mineTriggerComponent && mineTriggerComponent.IsActivated())
			return false;
		
		return super.CanBeShownScript(user);
	}
	
};
