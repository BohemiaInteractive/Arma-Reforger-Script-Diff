[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_RefillMagazineContextAction : SCR_SelectedEntitiesContextAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		if (selectedEntity.GetEntityType() != EEditableEntityType.VEHICLE)
			return false;
		
		return FindTurrets(selectedEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		array<BaseMagazineComponent> mags = {};
		
		GetMagazines(selectedEntity, mags);
		
		foreach (BaseMagazineComponent mag : mags)
		{
			if (mag.GetAmmoCount() < mag.GetMaxAmmoCount())
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool FindTurrets(notnull SCR_EditableEntityComponent selectedEntity, out array<Turret> turrets = null)
	{
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(selectedEntity.GetOwner().FindComponent(SlotManagerComponent));
		if (!slotManager)
			return false;
		
		array<EntitySlotInfo> slots = {};
		SCR_WeaponRackSlotInfo weaponRackSlot;
		slotManager.GetSlotInfos(slots);
		Turret localTurret;
		turrets = {};
		
		foreach (EntitySlotInfo slot : slots)
		{
			localTurret = Turret.Cast(slot.GetAttachedEntity());
			if (localTurret)
				turrets.Insert(localTurret);
		}
		
		if (turrets.IsEmpty())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void GetMagazines(notnull SCR_EditableEntityComponent selectedEntity, out array<BaseMagazineComponent> magazines)
	{
		array<Turret> turrets;
		if (!FindTurrets(selectedEntity, turrets))
			return;
		
		array<Managed> weaponComps = {};
		IEntity weaponEntity;
		MuzzleComponent muzzle;
		WeaponSlotComponent weaponSlot;
		BaseMagazineComponent mag;
		
		foreach (Turret turret : turrets)
		{
			turret.FindComponents(WeaponSlotComponent, weaponComps);
			
			foreach (Managed slot : weaponComps)
			{
				weaponSlot = WeaponSlotComponent.Cast(slot);
				if (!weaponSlot)
					continue;
				weaponEntity = weaponSlot.GetWeaponEntity();
				if (!weaponEntity)
					continue;
				muzzle = MuzzleComponent.Cast(weaponEntity.FindComponent(MuzzleComponent));
				if (!muzzle)
					continue;
				
				mag = muzzle.GetMagazine();
				if (mag)
					magazines.Insert(mag);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		array<BaseMagazineComponent> mags = {};
		
		GetMagazines(selectedEntity, mags);
		
		foreach (BaseMagazineComponent mag : mags)
		{
			mag.SetAmmoCount(mag.GetMaxAmmoCount());
		}
	}
};
