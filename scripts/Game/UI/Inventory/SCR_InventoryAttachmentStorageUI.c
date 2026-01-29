class SCR_InventoryAttachmentStorageUI : SCR_InventoryStorageBaseUI
{
	protected ref InventorySearchPredicate m_SearchPredicate;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		InitPaging();
		SetRowsAndColumns();
		CreateEmptyLayout();
		if (CreateSlots() == 0)
		{
			CloseStorage();
			return;
		}
		SortSlots();
		ShowPage(0);

		m_widget.FindAnyWidget("HeaderOverlay").SetVisible(false);
		ActivateCloseStorageButton();
	}
	
	//------------------------------------------------------------------------------------------------
	override event void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		DestroyEmptySlots();
	}

	protected override void SetPreviewItem()
	{
	}

	protected override int CreateSlots()
	{
		array<IEntity> pItemsInStorage = {};
		int count = GetRelevantItems(pItemsInStorage);
		UpdateOwnedSlots(pItemsInStorage);

		return count;
	}

	protected int GetRelevantItems(out notnull array<IEntity> items)
	{
		if (m_SearchPredicate)
			return m_InventoryManager.FindItems(items, m_SearchPredicate, EStoragePurpose.PURPOSE_ANY);

		return 0;
	}
	
	override bool IsTraversalAllowed()
	{
		return false;
	}

	override void Refresh()
	{
		if (!m_wGrid)
			return;
		
		InitPaging();
		SetRowsAndColumns();
		CreateEmptyLayout();
		if (CreateSlots() == 0)
		{
			CloseStorage();
			return;
		}
		SortSlots();
		ShowPage(0);
	}

	protected void ActivateCloseStorageButton()
	{
		m_wCloseStorageButton = ButtonWidget.Cast(m_widget.FindAnyWidget("CloseStorageBtn"));
		if (m_wCloseStorageButton)
		{
			m_wCloseStorageButton.SetVisible(true);
			SCR_InventoryNavigationButtonBack nav = SCR_InventoryNavigationButtonBack.Cast(m_wCloseStorageButton.FindHandler(SCR_InventoryNavigationButtonBack));
			if (nav)
			{
				if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.GAMEPAD)
					nav.EnableCloseWithGamepad(false);
				nav.m_OnActivated.Insert(CloseStorage);
			}
		}
	}

	protected void CloseStorage()
	{
		m_MenuHandler.RemoveAttachmentStorage(this);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_InventoryAttachmentStorageUI(
		BaseInventoryStorageComponent storage,
		LoadoutAreaType slotID = null,
		SCR_InventoryMenuUI menuManager = null,
		int iPage = 0,
		array<BaseInventoryStorageComponent> aTraverseStorage = null,
		InventorySearchPredicate searchPredicate = null)
	{
		m_Storage = storage;
		m_MenuHandler 	= menuManager;
		m_eSlotAreaType = slotID;
		m_iMaxRows 		= 1;
		m_iMaxColumns 	= 4;
		m_iMatrix = new SCR_Matrix(m_iMaxColumns, m_iMaxRows);
		m_iPageSize 	= m_iMaxRows * m_iMaxColumns;
		m_SearchPredicate = searchPredicate;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_InventoryAttachmentPointUI : SCR_InventoryStorageBaseUI
{
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		w.FindAnyWidget("HeaderOverlay").SetVisible(false);

		m_aSlots.Insert(CreateSlotUI(null));
	}

	protected override SCR_InventorySlotUI CreateSlotUI(InventoryItemComponent pComponent, SCR_ItemAttributeCollection pAttributes = null)
	{
		return new SCR_InventoryAttachmentSlotUI(null, this);
	}

	protected override void SetPreviewItem();
	protected string GetBoneName();
	TNodeId GetBoneIndex();

	//------------------------------------------------------------------------------------------------
	void SCR_InventoryAttachmentPointUI(
		BaseInventoryStorageComponent storage,
		LoadoutAreaType slotID = null,
		SCR_InventoryMenuUI menuManager = null,
		int iPage = 0,
		array<BaseInventoryStorageComponent> aTraverseStorage = null)
	{
		m_Storage = storage;
		m_MenuHandler 	= menuManager;
		m_eSlotAreaType = slotID;
		m_iMaxRows 		= 1;
		m_iMaxColumns 	= 1;
		m_iMatrix = new SCR_Matrix(m_iMaxColumns, m_iMaxRows);
	}
}

//------------------------------------------------------------------------------------------------
class SCR_InventoryAttachmentSlotUI : SCR_InventorySlotUI
{
	override void UpdateReferencedComponent(InventoryItemComponent pComponent, SCR_ItemAttributeCollection attributes = null)
	{
		Widget wGrid = m_pStorageUI.GetStorageGrid();
		m_widget = GetGame().GetWorkspace().CreateWidgets(SLOT_LAYOUT_1x1, wGrid);
		m_widget.AddHandler(this);
	}

	override bool OnDrop(SCR_InventorySlotUI slot)
	{
		return m_pStorageUI.OnDrop(slot);
	}
}