
//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Slot UI Layout
class SCR_InventorySlotStorageUI : SCR_InventorySlotUI
{		
	protected Widget												m_wShownIcon;

	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
		
	//------------------------------------------------------------------------------------------------
	//! should be the slot visible?
	override void SetSlotVisible( bool bVisible )
	{
		super.SetSlotVisible( bVisible );
		if ( bVisible )
			m_wShownIcon = m_widget.FindAnyWidget( "SelectedFrame" );
		else
			m_wShownIcon = null;
	}
		
	//------------------------------------------------------------------------------------------------	
	void ToggleShownIconVisible()
	{
		if ( !m_wShownIcon )
			return;
		m_wShownIcon.SetVisible( !m_wShownIcon.IsVisible() );
	}
	
	//------------------------------------------------------------------------------------------------	
	override bool RemoveItem()
	{
		//try to delete the UI slot
		if( super.RemoveItem() )
			return true;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*
	override void Init()
	{
		if(! m_Attributes )
				return;		
		//normally the storages might be larger than 1x1 (shown in a storage), but the storages list is 1x1
		m_iSizeX = 1;
		m_iSizeY = 1;
		//and create the visual slot
		m_widget = GetGame().GetWorkspace().CreateWidgets( SLOT_LAYOUT_1x1, m_pStorageUI.GetStorageGrid() );
				
		m_widget.AddHandler( this );	//calls the HandlerAttached()
	}
	*/
	
	//------------------------------------------------------------------------------------------------	
	//! stores the type of the functionality of the item in the slot
	override void SetItemFunctionality()
	{
		m_eSlotFunction = ESlotFunction.TYPE_STORAGE;
	}
			
	//------------------------------------------------------------------------------------------------
	override void SetSelected( bool select )
	{
		//TODO: show the selected effect should be done as a component
		m_bSelected = select;
		if ( !m_wSelectedEffect )
			return;
		if ( select )
			m_wSelectedEffect.SetVisible( true );
		else
			m_wSelectedEffect.SetVisible( false );
	}
	
	//------------------------------------------------------------------------------------------------
		
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------

	
	//------------------------------------------------------------------------------------------------
	void SCR_InventorySlotStorageUI( InventoryItemComponent pComponent = null, SCR_InventoryStorageBaseUI pStorageUI = null, bool bVisible = true, int iSlotIndex = -1, SCR_ItemAttributeCollection pAttributes = null )
	{
		m_pStorageUI = SCR_InventoryStoragesListUI.Cast( pStorageUI );
	}
	

	//------------------------------------------------------------------------------------------------
	void ~SCR_InventorySlotStorageUI()
	{
	}
};