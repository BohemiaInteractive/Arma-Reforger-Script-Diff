[EntityEditorProps(category: "GameScripted/UI/Inventory", description: "Inventory Gadget Slot UI class")]

//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Slot UI Layout
class SCR_InventorySlotGadgetUI : SCR_InventorySlotUI
{
					
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------							
					
	//------------------------------------------------------------------------------------------------	
	override bool RemoveItem()
	{
		//try to delete the UI slot
		if( super.RemoveItem() )
			return true;
					
		return true;
	}
		
	//------------------------------------------------------------------------------------------------
		
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		if(! m_Attributes )
				return;
		
		//normally the gadgets might be larger than 1x1 (shown in a storage), but the gadget list is 1x1
		m_iSizeX = 1;
		m_iSizeY = 1;
		//and create the visual slot
		m_widget = GetGame().GetWorkspace().CreateWidgets( SLOT_LAYOUT_1x1, m_pStorageUI.GetStorageGrid() );
		m_widget.AddHandler( this );															//calls the HandlerAttached()
	}
	
			
	//------------------------------------------------------------------------------------------------	
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
			
	//------------------------------------------------------------------------------------------------
	void SCR_InventorySlotGadgetUI( InventoryItemComponent pComponent = null, SCR_InventoryStorageBaseUI pStorageUI = null, bool bVisible = true, int iSlotIndex = -1, SCR_ItemAttributeCollection pAttributes = null )
	{
	}
	

	//------------------------------------------------------------------------------------------------
	void ~SCR_InventorySlotGadgetUI()
	{
	}
};