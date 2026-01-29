class SCR_OpenTurretStorageAction : SCR_OpenVehicleStorageAction
{
#ifndef DISABLE_INVENTORY
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		manager.SetStorageToOpen(pOwnerEntity);
		manager.OpenInventory();
	}
#endif
}