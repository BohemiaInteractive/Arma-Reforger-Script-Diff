[EntityEditorProps(category: "GameScripted/UI/Inventory", description: "Inventory 2.0 Weapon Slots UI class")]

//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Storage UI Layout - Shows the storages slots on player ( backpack, ALICE, vest, etc )
class SCR_InventoryWeaponSlotsUI : SCR_InventoryStorageBaseUI
{	
	protected ref array<WeaponSlotComponent> m_aWeaponSlots = new array<WeaponSlotComponent>(); ;
	protected EquipedWeaponStorageComponent m_weaponStorage;
		
	//------------------------------------------------------------------------------------------------
	override SCR_EAnalyticalItemSlotType GetAnalyticalItemSlotType()
	{
		return SCR_EAnalyticalItemSlotType.HORIZONTAL;
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_weaponStorage = EquipedWeaponStorageComponent.Cast(m_Storage);
		
		super.HandlerAttached(w);
		CreateEmptyLayout();
		CreateSlots();
		SortSlots();
		InitMatrix();
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitPaging()
	{}
	
	//------------------------------------------------------------------------------------------------
	override bool IsTraversalAllowed()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! 
	override protected int CreateSlots()
	{
		array<IEntity> pItemsInStorage = {};
		array<Managed> weaponSlots = {};
		
		m_Player.FindComponents(WeaponSlotComponent, weaponSlots);
		
		m_iMaxColumns = Math.Min(weaponSlots.Count(), SCR_InventoryMenuUI.WEAPON_SLOTS_COUNT);
		
		pItemsInStorage.Resize(m_iMaxColumns);
		m_aWeaponSlots.Resize(m_iMaxColumns);
		
		WeaponSlotComponent weaponSlot;
		int weaponSlotIndex;
		foreach (Managed managed : weaponSlots)
		{
			weaponSlot = WeaponSlotComponent.Cast(managed);
			if (!weaponSlot)
				continue;
			
			weaponSlotIndex = weaponSlot.GetWeaponSlotIndex();
			if (weaponSlotIndex < m_iMaxColumns)
			{
				pItemsInStorage.Set(weaponSlotIndex, weaponSlot.GetWeaponEntity());
				m_aWeaponSlots.Set(weaponSlotIndex, weaponSlot);
			}
		}
		
		UpdateOwnedSlots(pItemsInStorage);
		FillWithEmptySlots();
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! 
	override protected void UpdateOwnedSlots(notnull array<IEntity> pItemsInStorage)
	{
		int count = pItemsInStorage.Count();
		if (count < m_aSlots.Count())
		{
			for (int i = m_aSlots.Count() - count; i > 0; i--)
			{
				SCR_InventorySlotUI slotUI = m_aSlots[m_aSlots.Count() - 1];
				if (slotUI)
					slotUI.Destroy();
			}
		}
		m_aSlots.Resize(count);
		for (int i = 0; i < count; i++)
		{
			InventoryItemComponent pComponent = GetItemComponentFromEntity( pItemsInStorage[i] );
			
			if (m_aSlots[i])
				m_aSlots[i].UpdateReferencedComponent(pComponent);
			else
				m_aSlots[i] = CreateSlotUI(pComponent, i, null, i, m_aWeaponSlots[i].GetWeaponSlotType());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void FillWithEmptySlots()
	{
		array<int> aCoordinates;
		int iWidgetColumnSize, iWidgetRowSize;
		int iRelativeOffset = 0; 				//if there's an item taking more than one slot, offset the following items to the right

		foreach( int iIndex, SCR_InventorySlotUI pSlot: m_aSlots )
		{
			if( !pSlot )			//if pSlot doesn't exist it means no assigned item in this slot and we create the "empty" slot
			{
				ESlotSize eSlotSize;
				if ( iIndex > 1 )							
					eSlotSize = ESlotSize.SLOT_1x1;		
				else
					eSlotSize = ESlotSize.SLOT_2x1;	//first 2 slots are dedicated to weapons ( thus 2x1 layout)
				
				SCR_ItemAttributeCollection pAttrib = new SCR_ItemAttributeCollection();
				pAttrib.SetSlotType( ESlotID.SLOT_ANY );
				pAttrib.SetSlotSize( eSlotSize );
				pSlot = CreateSlotUI( null, iIndex, pAttrib, iIndex, m_aWeaponSlots[iIndex].GetWeaponSlotType() );
				m_aSlots.Set( iIndex, pSlot );
			}
			Widget w = pSlot.GetWidget();
			if( !w )
				continue;
			
			iWidgetColumnSize = pSlot.GetColumnSize();
			iWidgetRowSize = pSlot.GetRowSize();
		
			int iCol = pSlot.GetSlotIndex() + iRelativeOffset;
			m_iMatrix.ReservePlace( iWidgetColumnSize, iWidgetRowSize, iCol, 0 );	
			GridSlot.SetColumn( w, iCol );
			iRelativeOffset += ( iWidgetColumnSize - 1 );
			GridSlot.SetRow( w, 0 );
			
			GridSlot.SetColumnSpan( w, iWidgetColumnSize );
			GridSlot.SetRowSpan( w, iWidgetRowSize );
		}
		
		SetRowsAndColumns();
	}
		
	//------------------------------------------------------------------------------------------------
	// ! creates the slot
	protected SCR_InventorySlotWeaponSlotsUI CreateSlotUI(InventoryItemComponent pComponent, int iSlotIndex, SCR_ItemAttributeCollection pAttributes = null, int iWeaponSlotIndex = -1, string sWeaponSlotType = "")
	{
		return SCR_InventorySlotWeaponSlotsUI( pComponent, this, true, iSlotIndex, pAttributes, iWeaponSlotIndex, sWeaponSlotType );			//creates the slot 
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetRowsAndColumns()
	{
		// leave this empty because we need to initialize rows and columns in different place
	}
		
	//------------------------------------------------------------------------------------------------
	int GetActualSlotsUsedCount()
	{
		int result = 0;
		int slotCount = m_aSlots.Count();
		SCR_InventorySlotUI slot;
		for (int id = 0; id < slotCount; ++id)
		{
			slot = m_aSlots[id];
			if (slot)
				result += slot.GetColumnSize();
		}

		return result;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! creates the the grid from array of UI items
	override protected void SortSlots()
	{
		int iWidgetColumnSize, iWidgetRowSize;
		const int iPageCounter = 0;				// TODO: check for good const usage
		int iRelativeOffset = 0; 				// if there's an item taking more than one slot, offset the following items to the right

		//reset all elements to 0 - free it
		m_iMatrix.Reset(); 		
									
		foreach( SCR_InventorySlotUI pSlot: m_aSlots )
		{
			if( !pSlot )
				continue;
			
			Widget w = pSlot.GetWidget();
			if( !w )
				continue;
			
			iWidgetColumnSize = pSlot.GetColumnSize();
			iWidgetRowSize = pSlot.GetRowSize();
		
			int iCol = pSlot.GetSlotIndex() + iRelativeOffset;
			m_iMatrix.ReservePlace( iWidgetColumnSize, iWidgetRowSize, iCol, 0 );
			GridSlot.SetColumn( w, iCol );
			iRelativeOffset += ( iWidgetColumnSize - 1 );
			GridSlot.SetRow( w, 0 );
			pSlot.SetPage( iPageCounter );
			
			GridSlot.SetColumnSpan( w, iWidgetColumnSize );
			GridSlot.SetRowSpan( w, iWidgetRowSize );
		}
		
		m_iNrOfPages = iPageCounter + 1;
		FillWithEmptySlots();	
	}
	
	//------------------------------------------------------------------------------------------------
	void InitMatrix()
	{
		if (!m_wGrid)
			return;

		m_iMaxColumns = GetActualSlotsUsedCount();
		m_iMatrix = new SCR_Matrix(m_iMaxColumns, m_iMaxRows);
		m_iPageSize = m_iMaxRows * m_iMaxColumns;

		for (int y = 0; y < m_iMaxRows; ++y)
		{
			m_wGrid.SetRowFillWeight(y, 1);
			for (int x = 0; x < m_iMaxColumns; ++x)
			{
				m_wGrid.SetColumnFillWeight(x, 1);
			}
		}

		SizeLayoutWidget wSizeContainer = SizeLayoutWidget.Cast(m_widget.FindAnyWidget("SizeLayout0"));
		if (wSizeContainer)
			wSizeContainer.SetAspectRatio(m_iMaxColumns / m_iMaxRows);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_InventoryWeaponSlotsUI(BaseInventoryStorageComponent storage, LoadoutAreaType slotID = null, SCR_InventoryMenuUI menuManager = null, int iPage = 0, array<BaseInventoryStorageComponent> aTraverseStorage = null )
	{
		m_MenuHandler = menuManager;
		m_iMaxRows 		= 1;
		m_iMaxColumns 	= 0;
		m_iMatrix = new SCR_Matrix( m_iMaxColumns, m_iMaxRows );
		m_sGridPath = "WeaponsGrid";
		m_Storage = storage;
	}	
}
