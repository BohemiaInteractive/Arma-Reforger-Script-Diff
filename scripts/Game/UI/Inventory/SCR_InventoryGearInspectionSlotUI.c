class SCR_InventoryGearInspectionPointUI : SCR_InventoryAttachmentPointUI
{	
	protected ref array<InventoryStorageSlot> m_aStorageSlotsInStorage = {};
	
	protected Widget m_wFrameSlot;
	protected SCR_WeaponAttachmentsStorageComponent m_WeaponStorage;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		CreateSlots();
		
		SCR_WeaponAttachmentsStorageComponent weaponStorage = SCR_WeaponAttachmentsStorageComponent.Cast(m_Storage);
		if (!weaponStorage)
			return;
		
		m_WeaponStorage = weaponStorage;
		m_WeaponStorage.m_OnItemAddedToSlotInvoker.Insert(RefreshInspectionSlots);
		m_WeaponStorage.m_OnItemRemovedFromSlotInvoker.Insert(RefreshInspectionSlots);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_WeaponStorage)
		{
			m_WeaponStorage.m_OnItemAddedToSlotInvoker.Remove(RefreshInspectionSlots);
			m_WeaponStorage.m_OnItemRemovedFromSlotInvoker.Remove(RefreshInspectionSlots);
			m_WeaponStorage = null;
		}
		
		super.HandlerDeattached(w);
		ClearSlots();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RefreshInspectionSlots()
	{
		ClearSlots();
		CreateSlots();
		m_MenuHandler.UpdateGearInspectionPreview();
	}
	
	//------------------------------------------------------------------------------------------------
	override Widget GetStorageGrid()
	{
	 	return m_wFrameSlot;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsTraversalAllowed()
	{
		return false;
	}

	protected typename GetSlotType(InventoryStorageSlot slot, out string name)
	{
		typename type = slot.GetParentContainer().Type();
		if (type.IsInherited(BaseMuzzleComponent))
		{
			name = "#AR-Magazine_Name";
			BaseMagazineWell magwell = BaseMuzzleComponent.Cast(slot.GetParentContainer()).GetMagazineWell();
			
			if (magwell)
				return magwell.Type();
		}
		else if (type == AttachmentSlotComponent)
		{
			AttachmentSlotComponent attachment = AttachmentSlotComponent.Cast(slot.GetParentContainer());
			if (!attachment || !attachment.GetAttachmentSlotType())
			{
				name = string.Empty;
			 	return typename.Empty;
			}
			
			//TODO: FIX THIS, VERY BAD!
			
			type = attachment.GetAttachmentSlotType().Type();
			if (type.IsInherited(AttachmentOptics))
				name = "#AR-KeybindSeparator_WeaponOptics";
			if (type.IsInherited(AttachmentUnderBarrel))
				name = "#AR-AttachmentType_Underbarrel";
			if (type.IsInherited(AttachmentBayonet))
				name = "#AR-AttachmentType_Bayonet";
			if (type.IsInherited(AttachmentMuzzle))
				name = "#AR-AttachmentType_Suppressor";
			if (type.IsInherited(AttachmentHandGuard))
			{
				name = string.Empty;
				return typename.Empty;
			}
		}

		return type;
	}

	//------------------------------------------------------------------------------------------------
	override protected int CreateSlots()
	{
		array<IEntity> pItemsInStorage = {};
		int count = m_Storage.GetSlotsCount();
		
		for (int i = 0; i < count; ++i)
		{
			IEntity item = m_Storage.Get(i);
			InventoryStorageSlot storageSlot = m_Storage.GetSlot(i);
			
			m_aStorageSlotsInStorage.Insert(storageSlot);
			pItemsInStorage.Insert(item);
		}
		
		UpdateOwnedSlots(pItemsInStorage);
		
		for (int i = 0; i < count; ++i)
		{
			InventoryStorageSlot storageSlot = m_Storage.GetSlot(i);
			AttachmentSlotComponent attachment = AttachmentSlotComponent.Cast(storageSlot.GetParentContainer());
			if (attachment && !attachment.ShouldShowInInspection())
				m_aSlots[i].SetSlotVisible(false);
		}
		
		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void UpdateOwnedSlots(notnull array<IEntity> pItemsInStorage)
	{
		const int numberOfItems = pItemsInStorage.Count();
		const int initialNumberOfSlots = m_aSlots.Count();
		if (numberOfItems < initialNumberOfSlots)
		{
			SCR_InventorySlotUI slotUI;
			for (int i = initialNumberOfSlots - numberOfItems - 1; i > 0; i--)
			{
				slotUI = m_aSlots[i + numberOfItems];
				if (slotUI)
					slotUI.Destroy();
			}
		}

		m_aSlots.Resize(numberOfItems);
		IEntity item;
		InventoryItemComponent iic;
		SCR_InventorySlotGearInspectionUI inspectionSlot;
		foreach (int i, SCR_InventorySlotUI slot : m_aSlots)
		{
			item = pItemsInStorage[i];
			if (item)
				iic = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
			else
				iic = null;

			if (slot)
			{
				slot.UpdateReferencedComponent(iic);
			}
			else
			{
				slot = CreateSlotUI(iic);
				m_aSlots[i] = slot;
			}

			string slotName;
			const typename slotType = GetSlotType(m_Storage.GetSlot(i), slotName);
			if (!slotType || slotName.IsEmpty())
				continue;

			inspectionSlot = SCR_InventorySlotGearInspectionUI.Cast(slot);
			if (inspectionSlot)
				inspectionSlot.m_tAttachmentType = slotType;

			m_MenuHandler.AddItemToAttachmentSelection(slotName, inspectionSlot);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	// ! creates the slot
	protected override SCR_InventorySlotUI CreateSlotUI(InventoryItemComponent pComponent, SCR_ItemAttributeCollection pAttributes = null)
	{
		return new SCR_InventorySlotGearInspectionUI(pComponent, this);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePreviewSlotWidgets(notnull ItemPreviewWidget charPreview)
	{	
		foreach (int i, SCR_InventorySlotUI slotUI: m_aSlots)
		{
			slotUI.UpdateReferencedComponent(slotUI.GetInventoryItemComponent());
			
			Widget slotWidget = slotUI.GetWidget();
			InventoryStorageSlot storageSlot = m_Storage.GetSlot(i);
			m_Storage.GetOwner();

			vector screenPos;
			vector transform[4];
			
			//InventoryStorageSlot.GetInspectionWidgetOffset();
			
			transform[3] = m_aStorageSlotsInStorage.Get(i).GetInspectionWidgetOffset();
			
			charPreview.TryGetItemNodePositionInWidgetSpace(-1, transform, screenPos);
			FrameSlot.SetPos(slotWidget, screenPos[0], screenPos[1]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnDrop(SCR_InventorySlotUI slot)
	{
		if (!slot)
			return true;
		
		// SLOT is the original slot where of the item to replace. Add InventoryOperationCallback to tryReplace
		
		SCR_InvCallBack callBack = new SCR_InvCallBack();
		InventoryItemComponent itemComp = slot.GetInventoryItemComponent();
		IEntity item;
		
		if (itemComp)
			item = itemComp.GetOwner();
		
		callBack.m_pItem = item;
		callBack.m_pMenu = m_MenuHandler;
		
		callBack.m_pStorageFrom = slot.GetStorageUI();
		callBack.m_pStorageTo = this;
		callBack.m_pStorageMan = m_InventoryManager;
		callBack.m_eAttachAction = EAttachAction.ATTACH;
		callBack.m_pStorageToFocus = callBack.m_pStorageFrom;
		
		m_InventoryManager.EquipAny(m_Storage, item, -1, callBack);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearSlots()
	{
		foreach (SCR_InventorySlotUI slot: m_aSlots)
		{
			slot.Destroy();
		}
		m_aSlots.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	// ! leave this empty because we don't want to initiate the matrix
	override void SetRowsAndColumns() {}
	
	//------------------------------------------------------------------------------------------------
	// ! creates the the grid from array of UI items
	override protected void SortSlots() {}
	
	//------------------------------------------------------------------------------------------------
	// ! We want to avoid creating the grid
	void InitMatrix() {}
	
	//------------------------------------------------------------------------------------------------
	override void FillWithEmptySlots() {}
	
	//------------------------------------------------------------------------------------------------
	override void Refresh() {}
	
	//------------------------------------------------------------------------------------------------
	override void InitPaging() {}
	
	//------------------------------------------------------------------------------------------------
	void SCR_InventoryGearInspectionPointUI(
		BaseInventoryStorageComponent storage,
		LoadoutAreaType slotID = null,
		SCR_InventoryMenuUI menuManager = null,
		int iPage = 0,
		array<BaseInventoryStorageComponent> aTraverseStorage = null,
		Widget frameSlot = null)
	{
		m_Storage = storage;
		m_MenuHandler 	= menuManager;
		m_eSlotAreaType = slotID;
		m_iMaxRows 		= 1;
		m_iMaxColumns 	= 1;
		m_iMatrix = new SCR_Matrix(m_iMaxColumns, m_iMaxRows);
		m_wFrameSlot = frameSlot;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_InventorySlotGearInspectionUI : SCR_InventorySlotUI
{
	protected static const float SLOT_SIZE_DEFAULT = 32;
	protected static const float SLOT_SIZE_HIGHLIGHTED = 64;
	
	protected static const ResourceName ICON_VALID_SLOT = "{A1A2E6F6DB36CF61}UI/Textures/InventoryIcons/Inspection-gizmo-small_UI.edds";
	protected static const ResourceName ICON_INVALID_SLOT = "{2EB962B6F404C01E}UI/Textures/InventoryIcons/Inspection-gizmo-invalid-small_UI.edds";
	protected static const string SLOT_LAYOUT = "{EB835F21257F0C51}UI/layouts/Menus/Inventory/AttachmentItemSlot.layout";
	
	protected Widget m_wItemWidget = null;
	protected ImageWidget m_wIconWidget = null;
	protected SizeLayoutWidget m_wItemSizeLayout = null;

	protected bool m_bCompatibleAttachmentsShown;
	typename m_tAttachmentType;

	protected static SCR_InventorySlotGearInspectionUI s_SelectedPoint;
	bool m_bIsSelected;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateReferencedComponent(InventoryItemComponent pComponent, SCR_ItemAttributeCollection attributes = null)
	{
		if (m_widget)
			Destroy();
		
		m_pItem = pComponent;
		Widget m_wFrameSlot = m_pStorageUI.GetStorageGrid();
		m_widget = GetGame().GetWorkspace().CreateWidgets(SLOT_LAYOUT, m_wFrameSlot);
		m_widget.AddHandler(this);	//calls the HandlerAttached()
		
		m_wItemWidget = m_widget.FindAnyWidget("ButtonOverlay");	
		m_wIconWidget = ImageWidget.Cast(m_widget.FindAnyWidget("WidgetGizmo"));
				
		if (m_wIconWidget)
		{
			m_wIconWidget.LoadImageTexture(0, ICON_VALID_SLOT);
			m_wIconWidget.SetVisible(true);
		}
		
		if (m_wItemWidget)
			m_wItemWidget.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		Highlight(true);

		return super.OnMouseEnter(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (!m_bIsSelected)
			Highlight(false);

		return super.OnMouseLeave(w, enterW, x, y);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button == 0)
		{
			if (!m_bCompatibleAttachmentsShown)
				ShowCompatibleAttachments();
			else
				HideCompatibleAttachments();
		}

		return false;
	}

	void ShowCompatibleAttachments()
	{
		m_bCompatibleAttachmentsShown = true;
		Select(true);
		if (m_tAttachmentType.IsInherited(BaseMagazineWell))
		{
			SCR_MagazinePredicate predicate = new SCR_MagazinePredicate();
			predicate.magWellType = m_tAttachmentType;
			if (m_pStorageUI)
				m_pStorageUI.GetInventoryMenuHandler().ShowAttachmentStorage(predicate);
		}
		else
		{
			SCR_CompatibleAttachmentPredicate predicate = new SCR_CompatibleAttachmentPredicate();
			predicate.attachmentType = m_tAttachmentType;
			if (m_pStorageUI)
				m_pStorageUI.GetInventoryMenuHandler().ShowAttachmentStorage(predicate);
		}
	}

	void HideCompatibleAttachments()
	{
		m_bCompatibleAttachmentsShown = false;
		Select(false);
		m_pStorageUI.GetInventoryMenuHandler().CloseAttachmentStorage();
	}

	//------------------------------------------------------------------------------------------------
	void Highlight(bool highlight)
	{			
		if (m_widget)
		{
			if (highlight)
				FrameSlot.SetSize(m_widget, SLOT_SIZE_HIGHLIGHTED, SLOT_SIZE_HIGHLIGHTED);
			else
				FrameSlot.SetSize(m_widget, SLOT_SIZE_DEFAULT, SLOT_SIZE_DEFAULT);
		}
		
		if (m_wItemWidget)
			m_wItemWidget.SetVisible(highlight);
		
		if (m_wIconWidget)
			m_wIconWidget.SetVisible(!highlight);
	}

	void Select(bool select = true)
	{
		if (s_SelectedPoint)
		{
			s_SelectedPoint.m_bIsSelected = false;
			s_SelectedPoint.Highlight(false);
		}

		m_bIsSelected = select;
		Highlight(select);
		if (select)
		{
			s_SelectedPoint = this;
		}
		else
		{
			s_SelectedPoint = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	override BaseInventoryStorageComponent GetAsStorage() 
	{
		return m_pStorageUI.GetStorage();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnDrop(SCR_InventorySlotUI slot)
	{
		return m_pStorageUI.OnDrop(slot);
	}
};