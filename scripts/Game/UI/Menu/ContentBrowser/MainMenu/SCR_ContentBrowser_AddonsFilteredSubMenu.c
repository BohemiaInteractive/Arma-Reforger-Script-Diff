class SCR_ContentBrowser_AddonsFilteredSubMenu : SCR_ContentBrowser_AddonsSubMenu
{
	//------------------------------------------------------------------------------------------------
	override protected void DisplayOfflineItems(int pageId)
	{
		m_WorkshopApi.SetPageSize(GRID_N_COLUMNS * GRID_N_ROWS);

		// Get offline items from API
		array<WorkshopItem> rawWorkshopItems = {};
		m_WorkshopApi.GetOfflineItems(rawWorkshopItems);

		// Bail if there is no downloaded content
		if (rawWorkshopItems.IsEmpty())
		{
			SetPanelsMode(showEmptyPanel: true, messagePresetTag: MESSAGE_TAG_NOTHING_DOWNLOADED);
			UpdateItemsFoundMessage(0, 0);
			return;
		}

		// Register items in Addon Manager
		array<ref SCR_WorkshopItem> itemsUnfiltered = {};
		SCR_WorkshopItem itemRegistered;
		foreach (WorkshopItem i : rawWorkshopItems)
		{
			// Skip local items 
			if (WorldSaveItem.Cast(i))
				continue;
			
			itemRegistered = SCR_AddonManager.GetInstance().Register(i);
			itemsUnfiltered.Insert(itemRegistered);
		}

		// Filter items
		array<ref SCR_WorkshopItem> itemsFiltered = Filter_OfflineFilterAndSearch(itemsUnfiltered);

		// Bail if no items after filtering
		if (itemsFiltered.IsEmpty())
		{
			SetPanelsMode(showEmptyPanel: false, forceFiltersList: true, messagePresetTag: MESSAGE_TAG_NOTHING_DOWNLOADED_2);
			UpdateItemsFoundMessage(0, rawWorkshopItems.Count());
			return;
		}

		// Sort items
		array<SCR_WorkshopItem> itemsSortedWeakPtrs = {};
		foreach (SCR_WorkshopItem i : itemsFiltered)
		{
			itemsSortedWeakPtrs.Insert(i);
		}

		bool sortOrder = !m_Widgets.m_SortingHeaderComponent.GetSortOrderAscending();
		string currentSortingItem = m_Widgets.m_SortingHeaderComponent.GetSortElementName();
		if (currentSortingItem.IsEmpty())
		{
			currentSortingItem = "name";
			sortOrder = false;
		}

		switch (currentSortingItem)
		{
			case "name":
				SCR_Sorting<SCR_WorkshopItem, SCR_CompareWorkshopItemName>.HeapSort(itemsSortedWeakPtrs, sortOrder);
				break;
			case "time_since_played":
				SCR_Sorting<SCR_WorkshopItem, SCR_CompareWorkshopItemTimeSinceLastPlay>.HeapSort(itemsSortedWeakPtrs, sortOrder);
				break;
			case "time_since_downloaded":
				SCR_Sorting<SCR_WorkshopItem, SCR_CompareWorkshopItemTimeSinceFirstDownload>.HeapSort(itemsSortedWeakPtrs, sortOrder);
				break;
		}

		array<ref SCR_WorkshopItem> itemsSorted = {};
		foreach (SCR_WorkshopItem i : itemsSortedWeakPtrs)
		{
			itemsSorted.Insert(i);
		}

		// Update page count
		m_iPageCount = Math.Ceil(itemsSorted.Count() / (GRID_N_ROWS * GRID_N_COLUMNS));

		// Display items
		SetPanelsMode(false, m_Widgets.m_FilterPanelComponent.AnyFilterButtonsVisible(), string.Empty, m_bFirstPage);
		array<ref SCR_WorkshopItem> itemsAtPage = SelectItemsAtPage(itemsSorted, pageId);
		GridScreen_DisplayItems(itemsAtPage);

		m_bFirstPage = false;

		UpdateItemsFoundMessage(itemsSorted.Count(), rawWorkshopItems.Count());
	}
}