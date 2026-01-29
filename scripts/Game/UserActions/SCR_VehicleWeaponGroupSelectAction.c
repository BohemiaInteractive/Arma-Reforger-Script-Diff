class SCR_VehicleWeaponGroupSelectAction : SCR_VehicleWeaponActionBase
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		array<ref SCR_WeaponGroup> weaponGroups = {};
		fireModeManager.GetAllWeaponGroups(weaponGroups);
		if (weaponGroups.Count() <= 1)
			return false;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! By default toggle the current state of the interaction
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		if (fireModeManager)
			fireModeManager.NextWeaponsGroup();
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if (!fireModeManager)
			return false;
		
		UIInfo info = GetUIInfo();
		if (!info)
			return false;
		
		string weaponsGroupName;
		fireModeManager.GetWeaponGroupID(weaponsGroupName);
		outName = info.GetName() + " " + weaponsGroupName;

		return true;
	}
}
