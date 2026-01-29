[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_DisarmRocketPodsContextAction : SCR_BaseRocketPodsContextAction
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

			//~ Cannot reload so show action
			if (!CanReloadRocketPod(weapon))
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		array<IEntity> vehicleWeapons = {};
		GetVehicleWeapons(GetVehicleWeaponsRack(selectedEntity.GetOwner()), vehicleWeapons);
		
		SCR_RocketEjectorMuzzleComponent rocketMuzzle;
		foreach (IEntity weapon : vehicleWeapons)
		{
			rocketMuzzle = SCR_RocketEjectorMuzzleComponent.Cast(weapon.FindComponent(SCR_RocketEjectorMuzzleComponent));
			if (!rocketMuzzle)
				continue; 
			
			//~ Disarm Rocket
			DisarmRocketPod(weapon);
		}
	}	
};
