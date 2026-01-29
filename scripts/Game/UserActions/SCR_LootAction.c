class SCR_LootAction : SCR_InventoryAction
{
	#ifndef DISABLE_INVENTORY
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		auto vicinity = CharacterVicinityComponent.Cast( pUserEntity.FindComponent( CharacterVicinityComponent ));
		if ( !vicinity )
			return;
		
		vicinity.SetItemOfInterest(pOwnerEntity);
		manager.SetLootStorage( pOwnerEntity );
 	 	manager.OpenInventory();
	}
	#endif
};