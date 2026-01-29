[EntityEditorProps(category: "GameScripted/UI/Inventory", description: "Inventory 2.0 Storages List UI class")]

//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Storage UI Layout - Shows the storages slots on player ( backpack, ALICE, vest, etc )
class SCR_InventoryStoragesListUI : SCR_InventoryStorageBaseUI
{
	protected ref array<SCR_UniversalInventoryStorageComponent>		m_UniStorages			= new array<SCR_UniversalInventoryStorageComponent>();
	protected ref array<IEntity>									m_Items					= new array<IEntity>();
	
	//TODO: move to inventory manager
	protected const ResourceName INVENTORY_CONFIG = "{024B56A4DE577001}Configs/Inventory/InventoryUI.conf";
	protected ref SCR_InventoryConfig m_pInventoryUIConfig;
	
	 //~ How many Entries in a row before max row is increased
	protected const int MAX_ENTRIES_EACH_COLUMN = 12;
		
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
				
	//------------------------------------------------------------------------------------------------
	override SCR_EAnalyticalItemSlotType GetAnalyticalItemSlotType()
	{
		return SCR_EAnalyticalItemSlotType.VERTICAL;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	SCR_InventorySlotUI GetUISlotBySlotID( ESlotID slotID )
	{
		return m_aSlots.Get( slotID );
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_InventorySlotUI FindItemSlot(InventoryItemComponent item)
	{
		for (int i = 0; i < m_aSlots.Count(); i++)
		{
			SCR_InventorySlotUI pSlot = m_aSlots.Get(i);
			if ( pSlot )
			{
				if ( pSlot.GetInventoryItemComponent() == item )
				return pSlot;
			}
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! 
	override void InitPaging() { return; }
	
	
	//------------------------------------------------------------------------------------------------
	// ! 
	// TODO: maybe not needed as method?
	void GetAllStorages()
	{
		if( m_InventoryStorage )
			m_InventoryStorage.GetStorages( m_UniStorages );		
	}
	
	
	//------------------------------------------------------------------------------------------------
	// ! creates the slot
	protected SCR_InventorySlotUI CreateStorageSlotUI(notnull InventoryItemComponent pItemComponent )
	{
		//Salinebags and tourniquets must not be visible in inventory when applied
		if (pItemComponent.GetParentSlot().Type() == SCR_TourniquetStorageSlot || pItemComponent.GetParentSlot().Type() == SCR_SalineBagStorageSlot)
			return null;

		if ( pItemComponent.Type() == ClothNodeStorageComponent )
		{
			return new SCR_InventorySlotLBSUI( pItemComponent, this );		
		}
		else if ( BaseInventoryStorageComponent.Cast( pItemComponent ) )
		{
			return new SCR_InventorySlotStorageUI( pItemComponent, this );		
		}
		else
		{
			return new SCR_InventorySlotUI( pItemComponent, this );		
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// ! 
	override int CreateSlots()
	{
		if(!m_InventoryStorage)
			return -1;

		m_Items.Clear();
		m_InventoryStorage.GetAll(m_Items);		
		/*
		foreach ( SCR_InventorySlotUI pSlot: m_aSlots )
			pSlot.Destroy();
		*/
		m_aSlots.Clear();

		SCR_InventorySlotUI uiSlot;
		foreach (IEntity item: m_Items)
		{
			if (!item)
				continue;

			uiSlot = CreateStorageSlotUI(InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent)));
			if (!uiSlot)
				continue;

			m_aSlots.Insert(uiSlot);
		}

		return m_aSlots.Count() - 1;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! sorting slots by their index, Changing Loadoaut layout in BaseLoadoutManager will affect it.
	override void SortSlots()
	{		
		const int iWidgetColumnSize = 1;
		const int iWidgetRowSize = 1;
		int column, row;
			
		//reset all elements to 0 - free it
		m_iMatrix.Reset();
		
		foreach ( SCR_InventorySlotUI pSlot: m_aSlots )
		{
			column = 0;
			if(!pSlot)
				continue;

			InventoryItemComponent pItem = pSlot.GetInventoryItemComponent();
			if (!pItem)
				continue;

			#ifdef DEBUG_INVENTORY20
				ItemAttributeCollection pAttrs = pItem.GetAttributes();
				if ( pAttrs )
				{
					string sName = pAttrs.GetUIInfo().GetName();
					Print( sName );
				}
			#endif

			Widget w = pSlot.GetWidget();
			if( w )
			{
				row = GetSlotRow(pSlot, pItem);

				if (!CorrectSlotPositioning(column, row))
					continue;

				GridSlot.SetColumn(w, column);
				GridSlot.SetRow(w, row);
				
				SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(m_Storage);
				
				if (characterStorage && pSlot.GetLoadoutArea())
				{
					pSlot.SetSlotBlocked(characterStorage.IsAreaBlocked(pSlot.GetLoadoutArea().Type()));
				}
				
				GridSlot.SetColumnSpan( w, iWidgetColumnSize );
				GridSlot.SetRowSpan( w, iWidgetRowSize );
			}
		}

		FillWithEmptySlots();
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to verify and correct if provided position isnt within limits
	//! \param[in,out] column
	//! \param[in,out] row
	//! \return true when given position is within limits
	protected bool CorrectSlotPositioning(inout int column, inout int row)
	{
		if (row >= MAX_ENTRIES_EACH_COLUMN)
		{//>= because we count from 0
			int overflow = row;
			while (overflow >= MAX_ENTRIES_EACH_COLUMN)
			{
				column++;
				overflow -= MAX_ENTRIES_EACH_COLUMN;
			}
			row = overflow;
		}

		if (column >= m_iMaxColumns)
		{
			Print("SCR_InventoryStoragesListUI.CorrectSlotPositioning (" + __FILE__ + " L" + __LINE__ + ") - There is not enough space in the layout to fit more slots => wanted/max row=" + row + "/" + MAX_ENTRIES_EACH_COLUMN + " wanted/max column=" + column + "/" + m_iMaxColumns, LogLevel.WARNING);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to extract information about the position of the slot in the ui slot list
	//! \param[in] inventoryItemComp of slotted entity
	//! \param[in] uiSlot
	//! \return row id that coresponds to the configuration of this slot
	protected int GetSlotRow(notnull SCR_InventorySlotUI uiSlot, notnull InventoryItemComponent inventoryItemComp)
	{
		int loadoutAreaId = m_pInventoryUIConfig.GetRowByArea( uiSlot.GetLoadoutArea() );

		if (loadoutAreaId == - 1)
			loadoutAreaId = m_pInventoryUIConfig.GetRowByCommonItemType( uiSlot.GetCommonItemType() );

		if (SCR_HandSlotStorageSlot.Cast(inventoryItemComp.GetParentSlot()))
			loadoutAreaId = m_pInventoryUIConfig.GetRowByCommonItemType(ECommonItemType.HAND_SLOT_ITEM);

		return loadoutAreaId;
	}

	//------------------------------------------------------------------------------------------------
	override protected void FillWithEmptySlots()
	{
		array<SCR_LoadoutArea> validLoadoutAreas = {};
		m_pInventoryUIConfig.GetValidLoaddoutAreas(validLoadoutAreas);

		SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(m_Storage);
		bool notEmpty;
		int row, column;
		Widget slotWidget;
		SCR_ItemAttributeCollection itemAttributeCollection;
		SCR_InventorySlotUI newUiSlot;
		ResourceName iconName;
		foreach (int i, SCR_LoadoutArea area : validLoadoutAreas)
		{
			column = 0;
			row = i;

			if (!CorrectSlotPositioning(column, row))
				break;

			notEmpty = false;
			foreach (SCR_InventorySlotUI uiSlot: m_aSlots)
			{
				slotWidget = uiSlot.GetWidget();
				if (!slotWidget)
					continue;

				if (column != GridSlot.GetColumn(slotWidget))
					continue;

				if (row != GridSlot.GetRow(slotWidget))
					continue;

				notEmpty = true;
				break;
			}

			if (notEmpty)
				continue;

			itemAttributeCollection = new SCR_ItemAttributeCollection();
			itemAttributeCollection.SetSlotType(ESlotID.SLOT_ANY);
			itemAttributeCollection.SetSlotSize(ESlotSize.SLOT_1x1);

			newUiSlot = new SCR_InventorySlotUI(pStorageUI: this, pAttributes: itemAttributeCollection);
			GridSlot.SetColumn(newUiSlot.GetWidget(), column);
			GridSlot.SetRow(newUiSlot.GetWidget(), row);
			m_aSlots.Insert(newUiSlot);

			iconName = m_pInventoryUIConfig.GetIconByRow(i);
			if (iconName != ResourceName.Empty )
				newUiSlot.SetIcon(iconName);

			if (!characterStorage)
				continue;

			if (area.m_LoadoutArea)
				newUiSlot.SetSlotBlocked(characterStorage.IsAreaBlocked(area.m_LoadoutArea.Type()));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// ! do we want to delete all slots in the actual grid and show the content of the selected child storage? ( no )
	override bool IsTraversalAllowed()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! selects the actual slot
	/*
	override void SelectSlot( SCR_InventorySlotUI pSlot )
	{
		m_MenuHandler.StoreSelectedStorageSlot( SCR_InventorySlotStorageUI.Cast( pSlot ) );
	}
	*/
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached( Widget w )
	{
		super.HandlerAttached( w );
		LayoutSlot.SetSizeMode( m_widget, LayoutSizeMode.Fill );
		LayoutSlot.SetFillWeight( m_widget, 0.8889 );
		
		
		RefreshList();
	}
	
	//------------------------------------------------------------------------------------------------
	void RefreshList()
	{
		CreateSlots();	
		SortSlots();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_InventoryStoragesListUI(BaseInventoryStorageComponent storage, LoadoutAreaType slotID = null, SCR_InventoryMenuUI menuManager = null, int iPage = 0, array<BaseInventoryStorageComponent> aTraverseStorage = null )
	{
		m_MenuHandler = menuManager;
		
		Resource resource = BaseContainerTools.LoadContainer(INVENTORY_CONFIG);
		if (!resource || !resource.IsValid())
		{
			Print("Cannot load " + INVENTORY_CONFIG + " | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
			return;
		}
		
		m_pInventoryUIConfig = SCR_InventoryConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(resource.GetResource().ToBaseContainer()));
		
		//~ Get items in row
		if (m_pInventoryUIConfig)
			m_iMaxRows = m_pInventoryUIConfig.GetValidLoadoutAreaCount();
		
		int tempMaxRowCount = m_iMaxRows;
		m_iMaxColumns = 0;
		
		//~ Set ammount of columns
		if (MAX_ENTRIES_EACH_COLUMN > 1)
		{
			while (tempMaxRowCount > 0)
			{
				tempMaxRowCount -= MAX_ENTRIES_EACH_COLUMN;
				m_iMaxColumns++;
			}
		}
		else 
		{
			m_iMaxColumns = 1;
		}
		
		m_iMatrix = new SCR_Matrix(m_iMaxColumns, MAX_ENTRIES_EACH_COLUMN);
		m_sGridPath = "CharacterGrid";
		m_Storage = storage;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryStoragesListUI()
	{
		if( m_UniStorages )
			delete m_UniStorages;
	}
}
