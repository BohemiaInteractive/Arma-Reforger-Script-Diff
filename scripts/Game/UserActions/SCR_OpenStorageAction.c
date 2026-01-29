//------------------------------------------------------------------------------------------------
//! modded version for to be used with the inventory 2.0 
class SCR_OpenStorageAction : SCR_InventoryAction
{
	#ifndef DISABLE_INVENTORY
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		auto vicinity = CharacterVicinityComponent.Cast( pUserEntity .FindComponent( CharacterVicinityComponent ));
		if ( !vicinity )
			return;
		
		manager.SetStorageToOpen(pOwnerEntity);
		manager.OpenInventory();
	}
	
	#endif
	
	
};