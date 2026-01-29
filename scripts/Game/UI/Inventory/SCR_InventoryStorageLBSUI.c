
//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Storage UI Layout
class SCR_InventoryStorageLBSUI : SCR_InventoryStorageBaseUI
{
	protected ref array<InventoryItemComponent>					m_pInventoryItemsComponent 	= new array<InventoryItemComponent>();
	private array<IEntity>										m_pItems;	
//	protected string											sGridPath 					= "centerFrame.gridFrame.size.grid";	
	
	

	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override SCR_EAnalyticalItemSlotType GetAnalyticalItemSlotType()
	{
		return SCR_EAnalyticalItemSlotType.CHARACTER_STORAGE;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! 
	protected override int CreateSlots( )
	{
		ref array<IEntity> pItemsInStorage = new array<IEntity>();
		//GetAllItems( pItemsInStorage, 2 );
		GetAllItems( pItemsInStorage );
		UpdateOwnedSlots(pItemsInStorage);
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! Getting all items from the storages slotted in the parent storage ( i.e. ALICE has multiple pouches and cartridge boxes )
	protected override void GetAllItems( out notnull array<IEntity> pItemsInStorage, BaseInventoryStorageComponent pStorage = null )
	{
		array<BaseInventoryStorageComponent> pStorages = new array<BaseInventoryStorageComponent>();
		array<IEntity> pItems = new array<IEntity>();
		
		if (pStorage)
		{
			if (ClothNodeStorageComponent.Cast(pStorage))
			{
				pStorage.GetOwnedStorages(pStorages, 1, false);
				
				foreach (BaseInventoryStorageComponent pStor : pStorages)
				{
					if (!pStor)
						continue;
					pStor.GetAll(pItems);
					pItemsInStorage.Copy(pItems);
				}
				return;
			}
			
			pStorage.GetAll(pItemsInStorage);
			return;
		}
		

		m_Storage.GetOwnedStorages( pStorages, 1, false );
		foreach ( BaseInventoryStorageComponent pStor : pStorages )
		{
			if (! pStor )
				continue;
			pStor.GetAll( pItems );
			pItemsInStorage.Copy( pItems );
		}
		
	}
			
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
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
	void SCR_InventoryStorageLBSUI( BaseInventoryStorageComponent storage, LoadoutAreaType slotID = null, SCR_InventoryMenuUI menuManager = null, int iPage = 0, array<BaseInventoryStorageComponent> aTraverseStorage = null )
	{
		SetSlotAreaType( slotID );
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryStorageLBSUI()
	{
	}
};