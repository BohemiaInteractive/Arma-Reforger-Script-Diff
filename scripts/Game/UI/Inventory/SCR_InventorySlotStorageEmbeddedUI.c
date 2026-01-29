
//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Slot UI Layout
class SCR_InventorySlotStorageEmbeddedUI : SCR_InventorySlotStorageUI
{		
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------							
	
	//------------------------------------------------------------------------------------------------
	override void SetSelected( bool select )
	{
		super.SetSelected( select );
		
		if ( !m_wSelectedEffect )
			return;

		m_wSelectedEffect.SetVisible(select);
	}
	
		
	//------------------------------------------------------------------------------------------------
	void SCR_InventorySlotStorageEmbeddedUI( InventoryItemComponent pComponent = null, SCR_InventoryStorageBaseUI pStorageUI = null, bool bVisible = true, int iSlotIndex = -1, SCR_ItemAttributeCollection pAttributes = null )
	{
		m_pStorageUI = pStorageUI;
	}
	

	//------------------------------------------------------------------------------------------------
	void ~SCR_InventorySlotStorageEmbeddedUI()
	{
	}
};