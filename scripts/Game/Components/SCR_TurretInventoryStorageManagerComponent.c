[EntityEditorProps(category: "GameScripted/UI/Inventory")]
class SCR_TurretInventoryStorageManagerComponentClass : SCR_InventoryStorageManagerComponentClass
{
}

class SCR_TurretInventoryStorageManagerComponent : SCR_InventoryStorageManagerComponent
{
	//------------------------------------------------------------------------------------------------
	protected override void FillInitialStorages(out array<BaseInventoryStorageComponent> storagesToAdd)
	{
		super.FillInitialStorages(storagesToAdd);
		
		if (!GetGame().InPlayMode())
			return;

		array<Managed> outComponents = {};
		GetOwner().FindComponents(WeaponSlotComponent, outComponents);
		
		foreach (Managed outComponent : outComponents)
		{
			WeaponSlotComponent weaponSlotComponent = WeaponSlotComponent.Cast(outComponent);
			IEntity ent = weaponSlotComponent.GetWeaponEntity();
			if (!ent)
				continue;

			BaseInventoryStorageComponent invComp = BaseInventoryStorageComponent.Cast(ent.FindComponent(BaseInventoryStorageComponent));
			if (invComp)
				storagesToAdd.Insert(invComp);
		}
	}
}
