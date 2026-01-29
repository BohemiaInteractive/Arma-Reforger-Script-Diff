

//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Slot UI Layout
class SCR_InventorySlotWeaponUI : SCR_InventorySlotUI
{
		
	protected SCR_UniversalInventoryStorageComponent 			m_pStorageComponent;
		
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------							
		
	//------------------------------------------------------------------------------------------------	
	//! returns the storage component associated with this UI component
	override UniversalInventoryStorageComponent GetStorageComponent() { return m_pStorageComponent; }
	
		
	//------------------------------------------------------------------------------------------------	
	override bool RemoveItem()
	{
		//try to delete the UI slot
		//if( !super.RemoveItem() )
		//	return false;
		SCR_InventoryStorageManagerComponent invMan = GetInventoryManager();
		EquipedWeaponStorageComponent weaponStorage = m_pStorageUI.GetWeaponStorage();
		IEntity item = m_pItem.GetOwner();
		if(!(invMan && invMan.CanMoveItem(item) && weaponStorage))
			return false;
		bool success = invMan.CanRemoveItemFromStorage(m_pItem.GetOwner(), weaponStorage);
		if( !success )
			return false;
		
		//TODO: return the check back once the true/false issue is solved
		invMan.TryRemoveItemFromInventory(item, weaponStorage);
		
		//TODO: and refresh the container on the right side if its storage was removed
		if( !m_pStorageUI )
			return false;
		m_pStorageUI.GetInventoryMenuHandler().ShowStorage(weaponStorage);
		
		// Sound
		GenericEntity itemEntity = GenericEntity.Cast(item);
		SoundComponent soundComponent = SoundComponent.Cast(itemEntity.FindComponent(SoundComponent));
		if (soundComponent)
		{
			soundComponent.SoundEvent(SCR_SoundEvent.SOUND_DROP);
		}
				
		return true;
	}
	
	//------------------------------------------------------------------------------------------------	
		
	//------------------------------------------------------------------------------------------------	
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------	
		
	//------------------------------------------------------------------------------------------------	
			
	//------------------------------------------------------------------------------------------------
	void SCR_InventorySlotWeaponUI( InventoryItemComponent pComponent = null, SCR_InventoryStorageBaseUI pStorageUI = null, bool bVisible = true, int iSlotIndex = -1, SCR_ItemAttributeCollection pAttributes = null )
	{
	}
	

	//------------------------------------------------------------------------------------------------
	void ~SCR_InventorySlotWeaponUI()
	{
	}
};