//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Storage UI Layout
class SCR_InventoryStorageVestUI : SCR_InventoryStorageBaseUI
{
	protected ref array<InventoryItemComponent>					m_pInventoryItemsComponent 	= new array<InventoryItemComponent>();
	private array<IEntity>										m_pItems;	
//	protected string											sGridPath 					= "centerFrame.gridFrame.size.grid";	
	
	

	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
	
			
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
	void SCR_InventoryStorageVestUI( BaseInventoryStorageComponent storage, LoadoutAreaType slotID = null, SCR_InventoryMenuUI menuManager = null, int iPage = 0, array<BaseInventoryStorageComponent> aTraverseStorage = null )
	{
		//TODO: make consistent the visibility of variable ( SetSlotID vs m_iMaxRows )
		SetSlotAreaType( slotID );
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryStorageVestUI()
	{
	}
};