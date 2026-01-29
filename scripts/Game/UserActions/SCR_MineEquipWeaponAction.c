//------------------------------------------------------------------------------------------------
class SCR_MineEquipWeaponAction : SCR_EquipWeaponAction
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

//------------------------------------------------------------------------------------------------
class SCR_MineEquipHolsterAction : SCR_EquipWeaponHolsterAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		SCR_PressureTriggerComponent mineTriggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (mineTriggerComponent && mineTriggerComponent.IsActivated())
			return false;
		
		return super.CanBeShownScript(user);
	}
}