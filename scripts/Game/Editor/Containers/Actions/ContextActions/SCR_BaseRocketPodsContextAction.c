[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_BaseRocketPodsContextAction : SCR_SelectedEntitiesContextAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return selectedEntity.GetEntityType() == EEditableEntityType.VEHICLE;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	IEntity GetVehicleWeaponsRack(notnull IEntity vehicle, bool addIfAbsent = true)
	{
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(vehicle.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return null;
		
		array<EntitySlotInfo> slots = {};
		SCR_WeaponRackSlotInfo weaponRackSlot;
		slotManager.GetSlotInfos(slots);
		foreach (EntitySlotInfo slot : slots)
		{
			weaponRackSlot = SCR_WeaponRackSlotInfo.Cast(slot);
			if (!weaponRackSlot)
				continue;

			if (weaponRackSlot.GetAttachedEntity())
				return weaponRackSlot.GetAttachedEntity();
			
			if (addIfAbsent)
				return AddWeaponsRackToSlot(weaponRackSlot);
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void GetVehicleWeapons(notnull IEntity weaponsRack, out array<IEntity> weapons)
	{
		array<Managed> weaponSlots = {};
		WeaponSlotComponent weaponSlot;
		weapons = {};
		IEntity weapon;
		weaponsRack.FindComponents(WeaponSlotComponent, weaponSlots);
		foreach (Managed comp : weaponSlots)
		{
			weaponSlot = WeaponSlotComponent.Cast(comp);
			if (!weaponSlot)
				continue;
			
			weapon = weaponSlot.GetWeaponEntity();
			if (weapon)
				weapons.Insert(weapon);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity AddWeaponsRackToSlot(notnull SCR_WeaponRackSlotInfo weaponRackSlot)
	{
		ResourceName rackName = weaponRackSlot.GetDefaultWeaponRack();
		if (rackName.IsEmpty())
			return null;
		
		Resource resource = Resource.Load(rackName);
		if (!resource.IsValid())
			return null;

		IEntity weaponsRack = GetGame().SpawnEntityPrefab(resource);
		if (!weaponsRack)
			return null;
		
		weaponRackSlot.AttachEntity(weaponsRack);
		return weaponsRack;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanReloadRocketPod(notnull IEntity rocketPod)
	{
		SCR_RocketEjectorMuzzleComponent rocketMuzzle = SCR_RocketEjectorMuzzleComponent.Cast(rocketPod.FindComponent(SCR_RocketEjectorMuzzleComponent));
		if (!rocketMuzzle)
			return false;

		IEntity rocket;
		for (int i, count = rocketMuzzle.GetBarrelsCount(); i < count; i++)
		{
			if (rocketMuzzle.CanReloadBarrel(i))
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ReloadRocketPod(notnull IEntity rocketPod)
	{
		SCR_RocketEjectorMuzzleComponent rocketMuzzle = SCR_RocketEjectorMuzzleComponent.Cast(rocketPod.FindComponent(SCR_RocketEjectorMuzzleComponent));
		if (!rocketMuzzle)
			return;
		
		ResourceName defaultRocketName = rocketMuzzle.GetDefaultRocketPrefab();
		if (defaultRocketName.IsEmpty())
			return;
		
		Resource resource = Resource.Load(defaultRocketName);
		if (!resource.IsValid())
			return;

		IEntity rocket;
		for (int i = 0, count = rocketMuzzle.GetBarrelsCount(); i < count; i++)
		{
			if (!rocketMuzzle.CanReloadBarrel(i))
				continue;
			
			rocket = GetGame().SpawnEntityPrefab(resource);
			rocketMuzzle.ReloadBarrel(i, rocket);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void DisarmRocketPod(notnull IEntity rocketPod)
	{
		SCR_RocketEjectorMuzzleComponent rocketMuzzle = SCR_RocketEjectorMuzzleComponent.Cast(rocketPod.FindComponent(SCR_RocketEjectorMuzzleComponent));
		if (!rocketMuzzle)
			return;
		
		ResourceName defaultRocketName = rocketMuzzle.GetDefaultRocketPrefab();
		if (defaultRocketName.IsEmpty())
			return;
		
		Resource resource = Resource.Load(defaultRocketName);
		if (!resource.IsValid())
			return;

		IEntity rocket;
		for (int i, count = rocketMuzzle.GetBarrelsCount(); i < count; i++)
		{
			if (rocketMuzzle.CanReloadBarrel(i))
				continue;
			
			rocketMuzzle.UnloadBarrel(i);
		}
	}
	
};
