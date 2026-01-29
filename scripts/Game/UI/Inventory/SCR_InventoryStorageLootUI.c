//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Storage UI Layout
class SCR_InventoryStorageLootUI : SCR_InventoryStorageBaseUI
{
	private GenericEntity m_Character;
	private CharacterVicinityComponent m_Vicinity;
//	protected string											sGridPath 					= "centerFrame.gridFrame.size.grid";	
	
	//------------------------------------------------------------------------------------------------
	override SCR_EAnalyticalItemSlotType GetAnalyticalItemSlotType()
	{
		return SCR_EAnalyticalItemSlotType.VICINITY;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! creates the slot
	protected override SCR_InventorySlotUI CreateSlotUI( InventoryItemComponent pComponent, SCR_ItemAttributeCollection pAttributes = null )
	{
		const SCR_ItemAttributeCollection attributes = SCR_ItemAttributeCollection.Cast(pComponent.GetAttributes());
		if (!attributes || !attributes.IsVisible(pComponent))
			return null; // following slots dont add themselves as handlers if the item is not visible, thus skip creation of those as without a strong ref they will self destruct, and simply waste time

		BaseInventoryStorageComponent inventoryStorageComponent = GetCurrentNavigationStorage();
		
		if (inventoryStorageComponent)
		{
			IEntity storageOwner = inventoryStorageComponent.GetOwner();
			
			if (!storageOwner || !storageOwner.FindComponent(SCR_ArsenalInventoryStorageManagerComponent))
				return new SCR_InventorySlotUI(pComponent, this, true, pAttributes: attributes);
			
			SCR_ArsenalInventorySlotUI slotUI = new SCR_ArsenalInventorySlotUI(pComponent, this, false, -1, attributes);
			
			if (inventoryStorageComponent)
			{
				slotUI.SetArsenalResourceComponent(SCR_ResourceComponent.FindResourceComponent(inventoryStorageComponent.GetOwner()));
				SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Insert(slotUI, inventoryStorageComponent.GetOwner());
			}
			return slotUI;
		}
		else if (WeaponAttachmentsStorageComponent.Cast(pComponent))
		{
			return new SCR_InventorySlotWeaponUI(pComponent, this, true, pAttributes: attributes);
		}
		else if (SCR_ResourceComponent.FindResourceComponent(pComponent.GetOwner()))
		{
			return new SCR_SupplyInventorySlotUI(pComponent, this, true, pAttributes: attributes);
		}
		else if (BaseInventoryStorageComponent.Cast( pComponent))
		{
			return new SCR_InventorySlotStorageEmbeddedUI(pComponent, this, true, pAttributes: attributes);
		}
		else
		{
			return new SCR_InventorySlotUI(pComponent, this, true, pAttributes: attributes);
		}
	}

	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		if (!m_Character)
			return;
		//get the workspace
		if ( m_workspaceWidget == null ) 
			m_workspaceWidget = GetGame().GetWorkspace();
		
		//Get inventory manager and the actual character's topmost storage
		m_InventoryManager = SCR_InventoryStorageManagerComponent.Cast( m_Character.FindComponent( SCR_InventoryStorageManagerComponent ) );
		m_InventoryStorage = SCR_CharacterInventoryStorageComponent.Cast( m_Character.FindComponent( SCR_CharacterInventoryStorageComponent ) );
		m_Vicinity = CharacterVicinityComponent.Cast( m_Character.FindComponent(CharacterVicinityComponent) );
		m_wWidthOverride = SizeLayoutWidget.Cast(m_widget.FindWidget("TitleWidthOverride"));
		
		if (m_wWidthOverride)
			m_wWidthOverride.SetWidthOverride(m_iMaxColumns * SLOT_LAYOUT_WIDTH);
		
		if( m_InventoryStorage )
		{
			m_wGrid = GridLayoutWidget.Cast( m_widget.FindAnyWidget( m_sGridPath ) );	
			m_wStorageName = TextWidget.Cast( m_widget.FindAnyWidget( "StorageName" ) );
			//Set the proper ratio according to the number of columns and rows
			SizeLayoutWidget wSizeContainer = SizeLayoutWidget.Cast( m_widget.FindAnyWidget( "SizeLayout0" ) );
			if ( wSizeContainer )
				wSizeContainer.SetAspectRatio( m_iMaxColumns / m_iMaxRows );
		}
		if (m_wStorageName)
			m_wStorageName.SetText("#AR-Inventory_Vicinity");

		Widget storageSpace = m_widget.FindAnyWidget("StorageSpaceIndicator");
		if (storageSpace)
			storageSpace.SetVisible(false);
		
		Widget item = m_widget.FindAnyWidget("itemStorage");
		if (item)
			item.SetVisible(false);
	}
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached( Widget w )
	{
		super.HandlerAttached( w );
		InitPaging();
		SetRowsAndColumns();
		CreateEmptyLayout();
		CreateSlots();
		SortSlots();
		ShowPage(0);			
	}		
	
	//------------------------------------------------------------------------------------------------
	protected override void GetAllItems( out notnull array<IEntity> pItemsInStorage, BaseInventoryStorageComponent pStorage = null )
	{	
		if( !m_Vicinity )
			return;
		
		if (pStorage)
			super.GetAllItems(pItemsInStorage, pStorage);
		else
			m_Vicinity.GetAvailableItems(pItemsInStorage);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_InventoryStorageLootUI(
		BaseInventoryStorageComponent storage,
		LoadoutAreaType slotID = null,
		SCR_InventoryMenuUI menuManager = null,
		int iPage = 0,
		array<BaseInventoryStorageComponent> aTraverseStorage = null,
		GenericEntity character = null,
		int rows = 8,
		int cols = 6)
	{
		m_iMaxRows = rows;
		m_iMaxColumns = cols;
		m_iMatrix = new SCR_Matrix( m_iMaxColumns, m_iMaxRows );
		SetSlotAreaType( slotID );
		m_Character = character;
		m_bEnabled = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryStorageLootUI()
	{
	}
};