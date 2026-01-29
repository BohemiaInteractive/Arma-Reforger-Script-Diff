//------------------------------------------------------------------------------------------------
class SCR_VehicleWeaponFireModeSwitchAction : SCR_VehicleWeaponActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		array<int> availableFireModes = {};
		fireModeManager.GetAvailableFireModes(availableFireModes);
		
		if (availableFireModes.IsEmpty() || availableFireModes.Count() == 1)
			return false;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! By default toggle the current state of the interaction
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		if (fireModeManager)
			fireModeManager.NextFireMode();
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!fireModeManager)
			return false;

		UIInfo info = GetUIInfo();
		if (!info)
			return false;
		
		EWeaponGroupFireMode fireMode = fireModeManager.GetFireMode();
		
		LocalizedString fireModeName;
		if (fireMode == EWeaponGroupFireMode.SALVO)
			fireModeName = "#AR-UserAction_Helicopter_FireMode_Salvo";
		else if (fireMode == EWeaponGroupFireMode.RIPPLE)
			fireModeName = "#AR-UserAction_Helicopter_FireMode_Ripple";
		else if (fireMode == EWeaponGroupFireMode.SEQUENTIAL)
			fireModeName = "#AR-UserAction_Helicopter_FireMode_Sequential";
		
		outName = info.GetName() + " " + fireModeName;

		return true;
	}
}
