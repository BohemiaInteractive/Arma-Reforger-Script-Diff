class SCR_HandSlotPickUpItemAction : SCR_PickUpItemAction
{
	#ifndef DISABLE_INVENTORY
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		BaseInventoryStorageComponent storage = manager.FindStorageForItem(pOwnerEntity, EStoragePurpose.PURPOSE_DEPOSIT);
		if (!storage)
			return;
		
		manager.InsertItem(pOwnerEntity, storage);
	}
	#endif
}
