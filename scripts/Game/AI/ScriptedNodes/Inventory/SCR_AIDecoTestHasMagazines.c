class SCR_AIDecoTestHasMagazines : DecoratorTestScripted
{
	SCR_InventoryStorageManagerComponent inventoryManager;
	BaseWeaponComponent weapon;
	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		if (!inventoryManager && !weapon)
		{
			GenericEntity entity = GenericEntity.Cast(controlled);
			if ( !entity )
				return false;	
			inventoryManager = SCR_InventoryStorageManagerComponent.Cast(entity.FindComponent(SCR_InventoryStorageManagerComponent));
			if ( !inventoryManager )
				return false;
			BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(entity.FindComponent(BaseWeaponManagerComponent));
			if (!weaponManager)
				return false;
			weapon = weaponManager.GetCurrent();
			if (!weapon) 
				return false;						
		}
		int ammo = inventoryManager.GetMagazineCountByWeapon(weapon);
		return ammo > 0;
	}
};