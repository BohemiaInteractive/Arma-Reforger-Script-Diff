
//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Storage UI Layout
class SCR_InventoryStorageBackpackUI : SCR_InventoryStorageBaseUI
{
	protected ref array<InventoryItemComponent>					m_pInventoryItemsComponent 	= new array<InventoryItemComponent>();
	private array<IEntity>										m_pItems;	
//	protected string											sGridPath 					= "centerFrame.gridFrame.size.grid";	
	
	

	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
			
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override SCR_EAnalyticalItemSlotType GetAnalyticalItemSlotType()
	{
		return SCR_EAnalyticalItemSlotType.CHARACTER_STORAGE;
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached( Widget w )
	{
		super.HandlerAttached( w );
				
		CreateEmptyLayout();
		Refresh();
		/*
		CreateSlots();
		SortSlots();	
		ShowPage( m_iLastShownPage );		
		*/
	}	
	
	
	//------------------------------------------------------------------------------------------------
	void SCR_InventoryStorageBackpackUI( BaseInventoryStorageComponent storage, LoadoutAreaType slotID = null, SCR_InventoryMenuUI menuManager = null, int iPage = 0, array<BaseInventoryStorageComponent> aTraverseStorage = null )
	{
		SetSlotAreaType( slotID );
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryStorageBackpackUI()
	{
	}
};