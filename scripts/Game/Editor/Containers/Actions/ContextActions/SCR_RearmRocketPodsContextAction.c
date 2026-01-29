[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_RearmRocketPodsContextAction : SCR_BaseRocketPodsContextAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		if (!super.CanBeShown(selectedEntity, cursorWorldPosition, flags))
			return false;
		
		IEntity weaponRack = GetVehicleWeaponsRack(selectedEntity.GetOwner(), false);
		if (!weaponRack)
			return false;
		
		array<IEntity> vehicleWeapons = {};
		GetVehicleWeapons(weaponRack, vehicleWeapons);
		
		SCR_RocketEjectorMuzzleComponent rocketMuzzle;
		foreach (IEntity weapon : vehicleWeapons)
		{
			rocketMuzzle = SCR_RocketEjectorMuzzleComponent.Cast(weapon.FindComponent(SCR_RocketEjectorMuzzleComponent));
			if (!rocketMuzzle)
				continue; 
			
			ResourceName defaultmag = rocketMuzzle.GetDefaultRocketPrefab();
			if (defaultmag.IsEmpty())
				continue;
			
			//~ Can reload so show action
			if (CanReloadRocketPod(weapon))
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		if (!selectedEntity)
			return;

		IEntity weaponRack = GetVehicleWeaponsRack(selectedEntity.GetOwner());
		if (!weaponRack)
			return;

		array<IEntity> vehicleWeapons = {};
		GetVehicleWeapons(weaponRack, vehicleWeapons);
		
		SCR_RocketEjectorMuzzleComponent rocketMuzzle;
		foreach (IEntity weapon : vehicleWeapons)
		{
			rocketMuzzle = SCR_RocketEjectorMuzzleComponent.Cast(weapon.FindComponent(SCR_RocketEjectorMuzzleComponent));
			if (!rocketMuzzle)
				continue; 
			
			//~ Rearm rocket
			ReloadRocketPod(weapon);
		}
	}	
};
