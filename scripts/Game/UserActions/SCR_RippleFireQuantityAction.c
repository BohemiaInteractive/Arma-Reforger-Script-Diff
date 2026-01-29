class SCR_RippleFireQuantityAction : SCR_VehicleWeaponActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (fireModeManager && fireModeManager.GetFireMode() == EWeaponGroupFireMode.RIPPLE)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! By default toggle the current state of the interaction
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		if (fireModeManager)
			fireModeManager.NextRippleQuantity();
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!fireModeManager)
			return false;
		
		UIInfo info = GetUIInfo();
		if (!info)
			return false;
		
		outName = info.GetName() + " " + fireModeManager.GetRippleQuantity();

		return true;
	}
}
