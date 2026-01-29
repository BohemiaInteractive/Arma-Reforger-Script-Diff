class SCR_EquipDeployableItemAction : SCR_DeployableInventoryItemPickUpAction
{
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SCR_InvEquipAnyItemCB inventoryCallBack = new SCR_InvEquipAnyItemCB();
		inventoryCallBack.m_pItem = pOwnerEntity;
		inventoryCallBack.m_pStorageToPickUp = manager.GetCharacterStorage();
		inventoryCallBack.m_iSlotToFocus = -1;
		inventoryCallBack.m_bShouldEquip = true;

		manager.InsertItem(pOwnerEntity, cb: inventoryCallBack);
	}
}