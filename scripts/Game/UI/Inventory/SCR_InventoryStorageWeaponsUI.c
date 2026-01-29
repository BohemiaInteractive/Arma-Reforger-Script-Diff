//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Storage UI Layout
class SCR_InventoryStorageWeaponsUI : SCR_InventoryStorageBaseUI
{
	protected ref array<InventoryItemComponent>					m_pInventoryItemsComponent 	= new array<InventoryItemComponent>();
	private array<IEntity>										m_pItems;	
//	protected string											sGridPath 					= "centerFrame.gridFrame.size.grid";	
	
	

	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
		if ( m_InventoryStorage )		
			m_Storage = m_InventoryStorage.GetWeaponStorage();
		
		m_wStorageName.SetText( "#AR-Editor_TooltipDetail_Weapons_Name" );
		m_wProgressBar.SetVisible(false);
	}	
	
	//------------------------------------------------------------------------------------------------
	// ! creates the slot
	protected SCR_InventorySlotUI CreateWeaponSlotUI( InventoryItemComponent pStorageComponent )
	{
		return new SCR_InventorySlotWeaponUI( pStorageComponent, this );		
	}
		
	//------------------------------------------------------------------------------------------------
	// ! 
	override int CreateSlots( )
	{
		if ( !m_aSlots.IsEmpty() )
			DeleteSlots();
		array<IEntity> pItemsInStorage = new array<IEntity>();
		m_Storage.GetAll(pItemsInStorage);
		
		if( !pItemsInStorage )
			return -1;
		
		foreach ( IEntity pItem: pItemsInStorage )
		{
			InventoryItemComponent pComponent = GetItemComponentFromEntity( pItem );
			if( pComponent )
				m_aSlots.Insert( CreateWeaponSlotUI( pComponent ) );
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
	void SCR_InventoryStorageWeaponsUI( BaseInventoryStorageComponent storage, LoadoutAreaType slotID = null, SCR_InventoryMenuUI menuManager = null, int iPage = 0,array<BaseInventoryStorageComponent> aTraverseStorage = null )
	{
		//TODO: make consistent the visibility of variable ( SetSlotID vs m_iMaxRows )
		SetSlotAreaType( slotID );
		m_iMaxRows 		= 3;
		m_iMaxColumns 	= 4;
		m_iMatrix = new SCR_Matrix( m_iMaxColumns, m_iMaxRows );
		m_sGridPath = "GridLayout0";
		m_Storage = storage;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryStorageWeaponsUI()
	{
	}
};