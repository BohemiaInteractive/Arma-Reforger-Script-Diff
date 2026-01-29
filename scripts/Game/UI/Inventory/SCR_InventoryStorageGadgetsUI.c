//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Storage UI Layout
class SCR_InventoryStorageGadgetsUI : SCR_InventoryStorageBaseUI
{

	

	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	// ! creates the slot
	override SCR_InventorySlotUI CreateSlotUI( InventoryItemComponent pComponent, SCR_ItemAttributeCollection pAttributes = null )
	{
		return new SCR_InventorySlotGadgetUI( pComponent, this );			//creates the slot 
	}
	
	
	//------------------------------------------------------------------------------------------------
	// ! 
	override int CreateSlots( )
	{
		array<IEntity> pItemsInStorage = new array<IEntity>();
		GetAllItems( pItemsInStorage );
		
		if( !pItemsInStorage )
			return -1;
		
		foreach ( IEntity pItem: pItemsInStorage )
		{
			InventoryItemComponent pComponent = GetItemComponentFromEntity( pItem );
			if( pComponent )
				m_aSlots.Insert( CreateSlotUI( pComponent ) );
		}
		return m_aSlots.Count()-1;
	}			
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached( Widget w )
	{
		super.HandlerAttached( w );
				
		CreateEmptyLayout();
		CreateSlots();
		SortSlots();			
	}	
	
	
	//------------------------------------------------------------------------------------------------
	void SCR_InventoryStorageGadgetsUI( BaseInventoryStorageComponent storage, LoadoutAreaType slotID = null, SCR_InventoryMenuUI menuManager = null, int iPage = 0, array<BaseInventoryStorageComponent> aTraverseStorage = null )
	{
		//TODO: make consistent the visibility of variable ( SetSlotID vs m_iMaxRows )
		SetSlotAreaType( slotID );
		m_MenuHandler = menuManager;
		m_iMaxRows 		= 1;
		m_iMaxColumns 	= 6;
		m_iMatrix = new SCR_Matrix( m_iMaxColumns, m_iMaxRows );
		m_sGridPath = "GadgetGrid";
		m_Storage = storage;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryStorageGadgetsUI()
	{
	}
};