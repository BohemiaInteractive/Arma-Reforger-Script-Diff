class SCR_ItemSelectionMenuEntry : SCR_SelectionMenuEntry
{
	protected const ResourceName LAYOUT_ITEM = "{93472DECDA62C46F}UI/layouts/Common/RadialMenu/SelectionMenuEntryPreview.layout";
	protected SCR_CharacterInventoryStorageComponent m_Storage;
	protected int m_iQuickSlotID = -1;

	//------------------------------------------------------------------------------------------------
	override void Update()
	{
		SCR_SelectionMenuEntryComponent entry = GetEntryComponent();
		if (!entry)
			return;

		bool enabled;
		string name;
		string description;

		IEntity item = GetItem();
		if (item)
		{
			enabled = true;

			UIInfo uiInfo = GetUIInfo(item);
			if (uiInfo)
			{
				name = uiInfo.GetName();
				description = uiInfo.GetDescription();
			}
		}

		// Widget setup
		SetName(name);

		if (entry)
			entry.SetEnabled(enabled);

		SCR_SelectionMenuEntryPreviewComponent entryPreview = SCR_SelectionMenuEntryPreviewComponent.Cast(entry);
		if (entryPreview)
			entryPreview.SetPreviewItem(item);

		return;
	}

	//------------------------------------------------------------------------------------------------
	int GetQuickSlotID()
	{
		return m_iQuickSlotID;
	}

	//------------------------------------------------------------------------------------------------
	void SetQuickSlotID(int id)
	{
		m_iQuickSlotID = id;
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity GetItem()
	{
		if (m_iQuickSlotID < 0)
			return null;

		if (!m_Storage)
			return null;

		array<ref SCR_QuickslotBaseContainer> quickSlotItems = m_Storage.GetQuickSlotItems();
		if (quickSlotItems && m_iQuickSlotID < quickSlotItems.Count())
		{	
			SCR_QuickslotEntityContainer entityContainer = SCR_QuickslotEntityContainer.Cast(quickSlotItems[m_iQuickSlotID]);
			if (entityContainer)
				return entityContainer.GetEntity();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Callback for when this entry is supposed to be performed
	override void OnPerform()
	{
		super.OnPerform();

		IEntity item = GetItem();
		if (item && m_Storage && m_Storage.CanUseItem(item))
			m_Storage.UseItem(item);
	}

	//------------------------------------------------------------------------------------------------
	protected UIInfo GetUIInfo(IEntity item)
	{
		if (!item)
			return null;

		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return null;

		return itemComponent.GetUIInfo();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_ItemSelectionMenuEntry(notnull SCR_CharacterInventoryStorageComponent storage, int quickSlotID)
	{
		m_Storage = storage;
		m_iQuickSlotID = quickSlotID;

		SetCustomLayout(LAYOUT_ITEM);
		
		Enable(GetItem() != null);
	}
};
