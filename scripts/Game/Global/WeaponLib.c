//------------------------------------------------------------------------------------------------
//! SCR_WeaponLib Class
//!
//! Contains weapon related script functions
//------------------------------------------------------------------------------------------------
class SCR_WeaponLib
{
	//-----------------------------------------------------------------------------------------------------------
	//! Returns BaseWeaponComponent of current weapon
	static BaseWeaponComponent GetCurrentWeaponComponent(notnull ChimeraCharacter owner)
	{
		// Detect weapon manager
		CharacterControllerComponent controller = owner.GetCharacterController();
		if (!controller)
			return null;

		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManagerComponent();
		if (!weaponManager)
			return null;

		BaseWeaponComponent weapon = SCR_WeaponLib.GetCurrentWeaponComponent(weaponManager);
	
		return weapon;
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Returns BaseWeaponComponent of current weapon
	static BaseWeaponComponent GetCurrentWeaponComponent(notnull BaseWeaponManagerComponent weaponManager)
	{
		// Get weapon OR weapon slot
		BaseWeaponComponent weapon = weaponManager.GetCurrent();		
		if (!weapon) 
			return null;	
		
		// Try casting to weapon slot to check if it's weapon slot OR weapon
		WeaponSlotComponent weaponSlot = WeaponSlotComponent.Cast(weapon);
		
		// Weapon slot detected, retrieve weapon from it
		if (weaponSlot)
		{
			IEntity weaponEntity = weaponSlot.GetWeaponEntity();
			if (!weaponEntity) 
				return null;
		
			BaseWeaponComponent wpnComponent = BaseWeaponComponent.Cast(weaponEntity.FindComponent(BaseWeaponComponent));
			if (!wpnComponent) 
				return null;
			
			weapon = wpnComponent;
		}
	
		return weapon;
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Returns true if current weapon has component we are testing
	static bool CurrentWeaponHasComponent(notnull BaseWeaponManagerComponent weaponManager, typename component)
	{
		BaseWeaponComponent weapon = weaponManager.GetCurrent();
		if (!weapon) 
			return false;
		
		// Try casting to weapon slot to check if it's weapon slot OR weapon
		WeaponSlotComponent weaponSlot = WeaponSlotComponent.Cast(weapon);
		
		// Weapon slot detected, retrieve weapon from it
		if (weaponSlot)
		{
			IEntity weaponEntity = weaponSlot.GetWeaponEntity();
			if (weaponEntity && weaponEntity.FindComponent(component))
				return true;
		}
	
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get ID of next muzzle
	static int GetNextMuzzleID(notnull BaseWeaponComponent weapon)
	{
		array<BaseMuzzleComponent> muzzles = {};
		weapon.GetMuzzlesList(muzzles);
		if (muzzles.IsEmpty())
			return -1;
		
		int currentMuzzleID = muzzles.Find(weapon.GetCurrentMuzzle());
		
		return (currentMuzzleID + 1) % muzzles.Count();
	}
};
