/*
Submenu for local or online addons.

!! If you want filters to work, provide filters through the layout prefab or by some other way - into m_FilterCategories variable.
*/

enum EContentBrowserAddonsSubMenuMode
{
	MODE_OFFLINE,				// Gets items from Workshop offline API
	MODE_ONLINE,				// Gets items from Workshop online API
	MODE_EXTERNAL_ITEM_ARRAY	// Operates on fixed array of items supplied externally
}

class SCR_ContentBrowser_AddonsSubMenu : SCR_SubMenuBase
{
	// This is quite universal and can work in many modes...
	[Attribute("0", UIWidgets.ComboBox, "This menu is quite universal and can work in many modes - what do you want it to be?", "", ParamEnumArray.FromEnum(EContentBrowserAddonsSubMenuMode))]
	protected EContentBrowserAddonsSubMenuMode m_eMode;

	[Attribute("")]
	protected string m_sOnlinePageType;
	
	[Attribute("SOUND_FE_TURN_PAGE")]
	protected string m_sScrollWheelPageTurningSound;
	
	// --- Constants ---
	protected const int GRID_N_COLUMNS = 4;
	protected const int GRID_N_ROWS = 3;
	protected const int LOAD_PAGE_DELAY = 500;

	protected const ResourceName LAYOUT_GRID_TILE = "{67B0EFBDF7EAFF27}UI/layouts/Menus/ContentBrowser/Tile/WorkshopTile.layout";

	protected const string TEXT_PAGE_INDICATOR = "#AR-Editor_ContentBrowser_PageIndex_Text";
	
	// Message tags
	protected const string MESSAGE_TAG_NOTHING_FOUND = "nothing_found";
	protected const string MESSAGE_TAG_NOTHING_DOWNLOADED = "nothing_downloaded";
	protected const string MESSAGE_TAG_NOTHING_DOWNLOADED_2 = "nothing_downloaded2";
	protected const string MESSAGE_TAG_ERROR_CONNECTION = "error_connection";
	protected const string MESSAGE_TAG_LOADING = "loading";
	protected const string MESSAGE_TAG_NO_SAVES_FOUND = "no_saves_found";

	// --- Protected / Private ---
	protected ref SCR_ContentBrowser_AddonsSubMenuBaseWidgets m_Widgets = new SCR_ContentBrowser_AddonsSubMenuBaseWidgets();

	protected SCR_MenuActionsComponent m_MenuActionsComponent;
	
	protected WorkshopApi m_WorkshopApi;

	protected ref SCR_WorkshopDownloadSequence m_DownloadRequest;

	protected ref array<SCR_ContentBrowserTileComponent> m_aTileComponents = {};

	protected bool m_bConnected; // True when we are connected to backend, used to determine when to start sending requests
	protected bool m_bFailToLoad;

	protected bool m_bClearCacheAtNextRequest; // Will force workshop to clear page cache. Should be set wherever sorting or filtering changes.

	protected ref array<ref SCR_WorkshopItem> m_aExternalItems = {};	// Array with externally provided items - used only in MODE_EXTERNAL_ITEM_ARRAY

	protected SCR_ContentBrowserTileComponent m_LastFocusedTile; // Mainly used to restore focus back to something

	protected ref SCR_ContentBrowser_GetAssetListParams m_GetAssetListParams;
	protected ref SCR_ContentBrowser_GetAssetListParams m_GetAssetListParamsDefault;

	protected bool m_bFirstPage = true; // True before first page is received.

	protected bool m_bPanelsModeEmpty; // True when we are showing empty panels

	protected bool m_bMousePageUsed;

	protected bool m_bFirstLoad = true;

	// State of paging
	protected int m_iCurrentPage;
	protected int m_iPageCount;

	// Navigation buttons
	protected SCR_InputButtonComponent m_NavDetails;
	protected SCR_InputButtonComponent m_NavPrimary;
	protected SCR_InputButtonComponent m_NavEnable;
	protected SCR_InputButtonComponent m_NavFavourite;
	protected SCR_InputButtonComponent m_NavRepair;
	protected SCR_InputButtonComponent m_NavFilter;

	// Focus
	protected SCR_EListMenuWidgetFocus m_eFocusedWidgetState;
	
	// Items found message
	protected int m_iItemsTotal;
	protected int m_iOnlineFilteredItems;
	protected ref BackendCallback m_PageCallbackItemsTotal = null;
	protected ref BackendCallback m_PageCallback = null;
	
	// Mouse wheel input
	protected bool m_bCanPlayMouseWheelSound = true;
	protected const int MOUSE_WHEEL_SOUND_COOLDOWN = 75;
	
	// --- Overrides ---
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		InitWidgets();

		// Init main parts of the layout
		GridScreen_Init();

		// Try to load filters
		m_Widgets.m_FilterPanelComponent.TryLoad();

		m_WorkshopApi = GetGame().GetBackendApi().GetWorkshop();
		InitWorkshopApi();

		// Init event handlers
		InitWidgetEventHandlers();

		switch (m_eMode)
		{
			case EContentBrowserAddonsSubMenuMode.MODE_ONLINE:
			{
				m_bConnected = SCR_ServicesStatusHelper.IsAuthenticated();
				if (m_bConnected)
					OnConnected();
				else
					SetPanelsMode(true, false, MESSAGE_TAG_ERROR_CONNECTION, false);
				
				break;
			}
			
			case EContentBrowserAddonsSubMenuMode.MODE_OFFLINE:
			{
				// Offline mode - request page instantly
				RequestPage(0);
				break;
			}
			
			// For EContentBrowserAddonsSubMenuMode.MODE_EXTERNAL_ITEM_ARRAY we need to wait for the items
		}

		// Subscribe to addon manager events
		SCR_AddonManager.GetInstance().m_OnAddonOfflineStateChanged.Insert(Callback_OnAddonOfflineStateChanged);

		// Subscribe to page refresh request events
		SCR_AddonManager.GetInstance().m_OnAddonReportedStateChanged.Insert(Callback_OnAddonReportedStateChanged);
		
		// Listen for input device change to update navigation buttons
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceUserChanged);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		// Check and show backend down
		super.OnTabShow();

		// Init workshop API
		// We do it on tab show becasue this tab and others persists when all other tabs are closed,
		// But we can switch back to it later, and we must setup the workshop api again
		InitWorkshopApi();

		if (m_MenuActionsComponent)
			m_MenuActionsComponent.ActivateActions();
		
		if (m_bFailToLoad || m_eMode == EContentBrowserAddonsSubMenuMode.MODE_EXTERNAL_ITEM_ARRAY)
			RequestPage(m_iCurrentPage);
		
		if (m_bShown)
		{
			if (!m_bConnected && m_eMode == EContentBrowserAddonsSubMenuMode.MODE_ONLINE)
				GetGame().GetCallqueue().Call(SwitchFocus, SCR_EListMenuWidgetFocus.NULL);
			else
				GetGame().GetCallqueue().Call(SwitchFocus, m_eFocusedWidgetState);
		}
		
		SCR_AnalyticsApplication.GetInstance().WorkshopSetTab(m_iIndex);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();

		// Save configuration of filters
		m_Widgets.m_FilterPanelComponent.Save();
		
		if (m_MenuActionsComponent)
			m_MenuActionsComponent.DeactivateActions();
		
		// clear callbacks to kill pending requests
		m_PageCallback = null;
		m_PageCallbackItemsTotal = null;

		// put browser into state so that it again properly loads when shown again
		m_bFailToLoad = true;
		m_bFirstLoad = true;
		m_bFirstPage = true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabRemove()
	{
		super.OnTabRemove();

		// Unsubscribe from addon manager events
		SCR_AddonManager.GetInstance().m_OnAddonOfflineStateChanged.Remove(Callback_OnAddonOfflineStateChanged);

		// Unsubscribe from page refresh request events
		SCR_AddonManager.GetInstance().m_OnAddonReportedStateChanged.Remove(Callback_OnAddonReportedStateChanged);

		if (m_LastFocusedTile)
			m_LastFocusedTile.GetOnChange().Remove(OnTileStateChange);
		
		// clear callbacks to kill pending requests
		m_PageCallback = null;
		m_PageCallbackItemsTotal = null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();

		InitWorkshopApi();
	}

	//------------------------------------------------------------------------------------------------
	//! Called on each frame
	override void OnMenuUpdate(float tDelta)
	{
		// Update connection state, check if we have reconnected
		bool connectedOld = m_bConnected;
		bool connectedNew = SCR_ServicesStatusHelper.IsAuthenticated();
		m_bConnected = connectedNew;
		if (connectedNew != connectedOld)
		{
			// Only call OnConnected if it's an online tab
			if (connectedNew && m_eMode == EContentBrowserAddonsSubMenuMode.MODE_ONLINE)
				OnConnected();
		}
		
		super.OnMenuUpdate(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		if (m_bShown)
			SwitchFocus(m_eFocusedWidgetState);

		if (m_bShown && m_MenuActionsComponent)
			m_MenuActionsComponent.ActivateActions();
		
		super.OnMenuFocusGained();
	}
	
	// --- Protected ---
	//------------------------------------------------------------------------------------------------
	protected void InitWidgets()
	{
		m_Widgets.Init(GetRootWidget());
		
		// Create nav buttons
		if (m_eMode != EContentBrowserAddonsSubMenuMode.MODE_EXTERNAL_ITEM_ARRAY)
		{
			m_NavFilter = CreateNavigationButton("MenuFilter", "#AR-FieldManual_Page_EditorAssetBrowser_Text_3", false);
			m_NavDetails = CreateNavigationButton(UIConstants.MENU_ACTION_SELECT, "#AR-Workshop_Details_MenuTitle", true);
		}
		
		m_NavRepair = CreateNavigationButton(UIConstants.MENU_ACTION_SELECT_HOLD, "#AR-FactionCommander_TaskRepair", true);
		m_NavEnable = CreateNavigationButton(UIConstants.MENU_ACTION_ENABLE, SCR_WorkshopUiCommon.LABEL_ENABLE, true);
		m_NavPrimary = CreateNavigationButton(UIConstants.MENU_ACTION_SELECT_HOLD, SCR_WorkshopUiCommon.LABEL_DOWNLOAD, true);
		m_NavFavourite = CreateNavigationButton(UIConstants.MENU_ACTION_FAVORITE, UIConstants.FAVORITE_LABEL_ADD, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void InitWidgetEventHandlers()
	{
		m_Widgets.m_PrevPageButton_NoScrollComponent.m_OnActivated.Insert(OnPrevPageButton);
		m_Widgets.m_NextPageButton_NoScrollComponent.m_OnActivated.Insert(OnNextPageButton);

		m_Widgets.m_FilterPanelComponent.GetWidgets().m_FilterSearchComponent.m_OnConfirm.Insert(OnFilterSearchConfirm);
		m_Widgets.m_FilterPanelComponent.GetOnFilterPanelToggled().Insert(OnFilterPanelToggled);
		m_Widgets.m_SortingHeaderComponent.m_OnChanged.Insert(OnSortingHeaderChange);
		m_Widgets.m_FilterPanelComponent.GetOnFilterChanged().Insert(OnFilterChange);

		m_MenuActionsComponent = SCR_MenuActionsComponent.FindComponent(m_wRoot);
		if (m_MenuActionsComponent)
			m_MenuActionsComponent.GetOnAction().Insert(OnInputAction);
		
		// Event handlers of nav buttons
		if (m_NavFilter)
			m_NavFilter.m_OnActivated.Insert(OnFilterButton);
		
		if (m_NavDetails)
			m_NavDetails.m_OnClicked.Insert(OnDetailsButton);

		m_NavEnable.m_OnActivated.Insert(OnEnableButton);
		m_NavPrimary.m_OnActivated.Insert(OnPrimaryButton);
		m_NavFavourite.m_OnActivated.Insert(OnFavouriteButton);
		m_NavRepair.m_OnActivated.Insert(OnRepairButton);
	}
	
	//------------------------------------------------------------------------------------------------
	// Inits workshop API according to current mode
	protected void InitWorkshopApi()
	{
		WorkshopItem.SetThumbnailGridScale(GetGame().GetWorkspace().DPIScale(SCR_ContentBrowserTileComponent.GetTileWidth()));

		switch (m_eMode)
		{
			case EContentBrowserAddonsSubMenuMode.MODE_ONLINE:
				m_bClearCacheAtNextRequest = true;
				break;

			case EContentBrowserAddonsSubMenuMode.MODE_OFFLINE:
				m_WorkshopApi.ScanOfflineItems();
				break;
		}

		// Register tags for filters
		SCR_FilterSet filterSet = m_Widgets.m_FilterPanelComponent.m_FilterSet;
		if (!filterSet)
			return;
		
		SCR_ContentBrowserFilterTag filterTag;
		foreach (SCR_FilterCategory category : filterSet.GetFilterCategories())
		{
			foreach (SCR_FilterEntry filterEntry : category.GetFilters())
			{
				filterTag = SCR_ContentBrowserFilterTag.Cast(filterEntry);
				if (filterTag)
					filterTag.RegisterInWorkshopApi(m_WorkshopApi);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// Requests page from backend or displays items according to current mode
	protected void RequestPage(int pageId)
	{
		// When we request page, we also must clean the grid and side panel
		GridScreen_Clear();
		m_Widgets.m_AddonDetailsPanelComponent.SetWorkshopItem(null);
		SetPanelsMode(showEmptyPanel: m_bFirstPage, messagePresetTag: MESSAGE_TAG_LOADING);

		m_iCurrentPage = pageId;
		m_bFailToLoad = false;

		switch (m_eMode)
		{
			case EContentBrowserAddonsSubMenuMode.MODE_EXTERNAL_ITEM_ARRAY:
				DisplayExternalItems(pageId);
				break;
			
			case EContentBrowserAddonsSubMenuMode.MODE_ONLINE:
				if (m_bFirstLoad)
					RequestOnlinePageUnfiltered(pageId);
				else
					RequestOnlinePage(pageId);
				break;
			
			case EContentBrowserAddonsSubMenuMode.MODE_OFFLINE:
				DisplayOfflineItems(pageId);
				break;
		}
	}

	// --- Backend page requests ---
	//------------------------------------------------------------------------------------------------
	//! Requests an unfiltered page from backend to get the total number of items
	protected void RequestOnlinePageUnfiltered(int pageId)
	{
		// Hotfix: 0 is not allowed page size and there is no other way to get unfiltered total count
		m_WorkshopApi.SetPageSize(1);

		m_PageCallbackItemsTotal = new BackendCallback();
		m_PageCallbackItemsTotal.SetOnSuccess(Callback_OnRequestPageGetAllAssets);
		
		if (!m_GetAssetListParamsDefault)
			m_GetAssetListParamsDefault = new SCR_ContentBrowser_GetAssetListParams(this, true);

		m_GetAssetListParamsDefault.limit = GRID_N_ROWS * GRID_N_COLUMNS;
		m_GetAssetListParamsDefault.offset = pageId * GRID_N_ROWS * GRID_N_COLUMNS;

		m_WorkshopApi.RequestPage(m_PageCallbackItemsTotal, m_GetAssetListParamsDefault, m_bClearCacheAtNextRequest);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Requests a filtered page from backend to get the actual items to display
	protected void RequestOnlinePage(int pageId)
	{
		m_WorkshopApi.SetPageSize(GRID_N_COLUMNS * GRID_N_ROWS);

		if (!m_PageCallback)
		{
			m_PageCallback = new BackendCallback();
			m_PageCallback.SetOnSuccess(Callback_OnRequestPageGetAssets);
			m_PageCallback.SetOnError(Callback_OnRequestPageTimeout);
		}
		
		if (!m_GetAssetListParams)
			m_GetAssetListParams = new SCR_ContentBrowser_GetAssetListParams(this);

		m_GetAssetListParams.limit = GRID_N_ROWS * GRID_N_COLUMNS;
		m_GetAssetListParams.offset = pageId * GRID_N_ROWS * GRID_N_COLUMNS;
		m_GetAssetListParams.type = m_sOnlinePageType;

		//m_GetAssetListParams.PackToFile("$profile:GetAssetList.json");
		#ifdef WORKSHOP_DEBUG
		ContentBrowserUI._print(string.Format("WorkshopApi.RequestPage() limit: %1, offset: %2", m_GetAssetListParams.limit, m_GetAssetListParams.offset));
		#endif

		m_WorkshopApi.RequestPage(m_PageCallback, m_GetAssetListParams, m_bClearCacheAtNextRequest);

		m_bClearCacheAtNextRequest = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Selects items from the externally provided array and shows them on grid, only makes sense in external array mode
	protected void DisplayExternalItems(int pageId)
	{
		m_WorkshopApi.SetPageSize(GRID_N_COLUMNS * GRID_N_ROWS);

		array<ref SCR_WorkshopItem> itemsAtPage = SelectItemsAtPage(m_aExternalItems, pageId);

		// Filtering is disabled in this mode
		// Update page count
		float nItemsPerPage = GRID_N_ROWS * GRID_N_COLUMNS;
		m_iPageCount = Math.Ceil(m_aExternalItems.Count() / nItemsPerPage);

		SetPanelsMode(false);
		
		foreach(SCR_WorkshopItem item: itemsAtPage)
			item.LoadDetails();

		GridScreen_DisplayItems(itemsAtPage);

		m_bFirstPage = false;

		UpdateItemsFoundMessage(m_aExternalItems.Count(), m_aExternalItems.Count());
	}

	//------------------------------------------------------------------------------------------------
	protected void DisplayOfflineItems(int pageId)
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
			if (WorldSaveItem.Cast(i) && !i.GetActiveRevision())
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

	// --- Widgets ---
	//------------------------------------------------------------------------------------------------
	// When pages are scrolled, tiles are not deleted, we show different data in the tiles instead to avoid unneeded widget allocations.
	// Here we create tile widgets in advance.
	protected void GridScreen_Init()
	{
		for (int i = 0; i < GRID_N_COLUMNS * GRID_N_ROWS; i++)
		{
			int colId = i % GRID_N_COLUMNS;
			int rowId = i / GRID_N_COLUMNS;

			// Create tile widgets
			Widget w = GetGame().GetWorkspace().CreateWidgets(LAYOUT_GRID_TILE, m_Widgets.m_Grid);

			// Set grid positioning
			GridSlot.SetRow(w, rowId);
			GridSlot.SetColumn(w, colId);

			// Init the tile component
			SCR_ContentBrowserTileComponent comp = SCR_ContentBrowserTileComponent.Cast(w.FindHandler(SCR_ContentBrowserTileComponent));
			comp.SetWorkshopItem(null); // Widget will be hidden because of this

			// Tile event handlers
			comp.m_OnClick.Insert(OnTileClick);
			comp.GetOnFocus().Insert(OnTileFocus);
			comp.GetOnFocusLost().Insert(OnTileFocusLost);

			m_aTileComponents.Insert(comp);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Tiles are cleared up here. They are not deleted, but hidden.
	protected void GridScreen_Clear()
	{
		foreach (SCR_ContentBrowserTileComponent comp : m_aTileComponents)
		{
			comp.SetWorkshopItem(null);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void GridScreen_DisplayItems(array<ref SCR_WorkshopItem> items)
	{
		// Clear old items
		GridScreen_Clear();

		int nItems = Math.ClampInt(items.Count(), 0, GRID_N_COLUMNS * GRID_N_ROWS);

		for (int i = 0; i < nItems; i++)
		{
			SCR_ContentBrowserTileComponent comp = m_aTileComponents[i];
			comp.SetWorkshopItem(items[i]);
		}
		
		if (nItems > 0 && m_eFocusedWidgetState != SCR_EListMenuWidgetFocus.FILTERING)
			SwitchFocus(SCR_EListMenuWidgetFocus.LIST);
		
		UpdatePagingWidgets();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetPanelsMode(bool showEmptyPanel, bool forceFiltersList = false, string messagePresetTag = string.Empty, bool animateFiltersList = false)
	{
		m_bPanelsModeEmpty = showEmptyPanel;

		// Show either main+filter panel or empty panel
		m_Widgets.m_EmptyPanel.SetVisible(showEmptyPanel);
		m_Widgets.m_FilterPanel.SetVisible(!showEmptyPanel);
		m_Widgets.m_LeftPanelArea.SetVisible(!showEmptyPanel);

		// Show message or hide it
		m_Widgets.m_EmptyPanelMessage.SetVisible(!messagePresetTag.IsEmpty());
		m_Widgets.m_MainPanelMessage.SetVisible(!messagePresetTag.IsEmpty());

		// Set message based on tag
		if (!messagePresetTag.IsEmpty())
		{
			if (showEmptyPanel)
				m_Widgets.m_EmptyPanelMessageComponent.SetContentFromPreset(messagePresetTag);
			else
				m_Widgets.m_MainPanelMessageComponent.SetContentFromPreset(messagePresetTag);
		}

		// Force-show filter list
		if (forceFiltersList)
		{
			m_Widgets.m_FilterPanelComponent.ShowFilterListBox(true, animateFiltersList);
			if (!m_Widgets.m_FilterPanelComponent.GetFocused())
				SwitchFocus(SCR_EListMenuWidgetFocus.FILTERING);
		}
		
		UpdateNavigationButtons();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateItemsFoundMessage(int current, int total)
	{
		if (!m_Widgets || !m_Widgets.m_FilterPanelComponent)
			return;

		m_Widgets.m_FilterPanelComponent.SetItemsFoundMessage(current, total, current != total);

	}

	//------------------------------------------------------------------------------------------------
	//! Returns currently selected workshop item
	protected SCR_WorkshopItem GetSelectedItem()
	{
		Widget focusedWidget = GetGame().GetWorkspace().GetFocusedWidget();

		if (!focusedWidget)
			return null;

		SCR_ContentBrowserTileComponent comp = SCR_ContentBrowserTileComponent.Cast(focusedWidget.FindHandler(SCR_ContentBrowserTileComponent));

		if (!comp)
			return null;

		return comp.GetWorkshopItem();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates the widgets related to paging: buttons, text
	protected void UpdatePagingWidgets()
	{
		m_Widgets.m_PagingButtons.SetVisible(m_iPageCount != 0);

		if (m_iPageCount == 0)
			return;

		m_Widgets.m_PageIndicatorText.SetTextFormat(TEXT_PAGE_INDICATOR, m_iCurrentPage + 1, m_iPageCount);

		m_Widgets.m_PrevPageButton_NoScrollComponent.SetEnabled(m_iCurrentPage > 0, false);
		m_Widgets.m_NextPageButton_NoScrollComponent.SetEnabled(m_iCurrentPage < m_iPageCount - 1, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateNavigationButtons()
	{
		// Filters button
		SetNavigationButtonVisibile(m_NavFilter, !m_bPanelsModeEmpty);

		// Item buttons
		SCR_WorkshopItem item = GetSelectedItem();
		if (!item || item.GetRestricted())
		{
			SetNavigationButtonVisibile(m_NavDetails, false);
			SetNavigationButtonVisibile(m_NavFavourite, false);
			SetNavigationButtonVisibile(m_NavEnable, false);
			SetNavigationButtonVisibile(m_NavPrimary, false);
			SetNavigationButtonVisibile(m_NavRepair, false);			
			return;
		}
		
		bool show = GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE && !m_bPanelsModeEmpty;
		string primaryLabel = SCR_WorkshopUiCommon.GetPrimaryActionName(item);

		SCR_EAddonPrimaryActionState state = SCR_WorkshopUiCommon.GetPrimaryActionState(item);
		
		SetNavigationButtonVisibile(m_NavDetails, show && m_eMode != EContentBrowserAddonsSubMenuMode.MODE_EXTERNAL_ITEM_ARRAY);
		SetNavigationButtonVisibile(m_NavFavourite, show);
		SetNavigationButtonVisibile(m_NavEnable, show && item.GetOffline() && !SCR_WorkshopUiCommon.IsDownloadingAddonOrDependencies(item) && !(!item.GetEnabled() && state == SCR_EAddonPrimaryActionState.DEPENDENCIES_DOWNLOAD));
		SetNavigationButtonVisibile(m_NavPrimary, show && !primaryLabel.IsEmpty());
		SetNavigationButtonVisibile(m_NavRepair, show && primaryLabel.IsEmpty() && item.GetCorrupted());
		
		// Favorite
		if (m_NavFavourite)
			m_NavFavourite.SetLabel(UIConstants.GetFavoriteLabel(item.GetFavourite()));

		// Enable
		if (m_NavEnable && item.GetOffline())
		{
			string enableLabel;
			if (item.GetEnabled())
				enableLabel = SCR_WorkshopUiCommon.LABEL_DISABLE;
			else
				enableLabel = SCR_WorkshopUiCommon.LABEL_ENABLE;
			
			m_NavEnable.SetLabel(enableLabel);
		}

		// Primary
		if (!primaryLabel.IsEmpty() && m_NavPrimary)
			m_NavPrimary.SetLabel(primaryLabel);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if cursor over a given widget
	protected bool IsWidgetUnderCursor(Widget w)
	{
		Widget wCurrent = WidgetManager.GetWidgetUnderCursor();
		while (wCurrent)
		{
			if (wCurrent == w)
				return true;
			wCurrent = wCurrent.GetParent();
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns array of items within specified page, page ID starting from 0.
	protected array<ref SCR_WorkshopItem> SelectItemsAtPage(array<ref SCR_WorkshopItem> items, int pageId)
	{
		array<ref SCR_WorkshopItem> itemsOut = {};

		float nItemsPerPage = GRID_N_ROWS * GRID_N_COLUMNS;
		int nPages = Math.Ceil(items.Count() / nItemsPerPage);

		if (pageId < 0 || pageId >= nPages)
			return itemsOut;

		int idStart = nItemsPerPage * pageId;			// Select from this
		int idEnd = nItemsPerPage * (pageId + 1) - 1;	// Up to this (including this)

		if (idEnd >= items.Count())
			idEnd = items.Count() - 1;

		for (int i = idStart; i <= idEnd; i++)
		{
			itemsOut.Insert(items[i]);
		}

		return itemsOut;
	}

	//------------------------------------------------------------------------------------------------
	//! Switch focus
	protected void SwitchFocus(SCR_EListMenuWidgetFocus focus)
	{
		m_eFocusedWidgetState = focus;
		Widget focusTarget;

		switch (focus)
		{
			case SCR_EListMenuWidgetFocus.LIST:
			{
				if (m_LastFocusedTile && m_LastFocusedTile.IsVisible())
					focusTarget = m_LastFocusedTile.GetRootWidget();
				else if (m_Widgets.m_Grid)
					focusTarget = m_Widgets.m_Grid.GetChildren();

				break;
			}

			case SCR_EListMenuWidgetFocus.FILTERING:
			{
				focusTarget = m_Widgets.m_FilterPanelComponent.GetWidgets().m_FilterButton;
				break;
			}

			case SCR_EListMenuWidgetFocus.SORTING:
			{
				focusTarget = m_Widgets.m_SortingHeader.GetChildren();
				break;
			}

			case SCR_EListMenuWidgetFocus.NULL:
			{
				GetGame().GetWorkspace().SetFocusedWidget(null);
				return;
			}
		}

		// Fallback
		if (!focusTarget || !focusTarget.IsVisible())
			focusTarget = m_Widgets.m_SortingHeader.GetChildren();

		// Execute focusing
		GetGame().GetWorkspace().SetFocusedWidget(focusTarget);

		// If we've focused a tile, we need to manually update the details panel (by switching page we focus the same widget, but the tile data is different)
		if (!focusTarget)
			return;

		SCR_ContentBrowserTileComponent comp = SCR_ContentBrowserTileComponent.FindComponent(focusTarget);
		if (comp)
			OnTileFocus(comp)
	}

	// --- Events ---
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceUserChanged(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		UpdateNavigationButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	// Called when we have lost connection and connected again, or when the tab is opened first time and connection is already active
	// Not called for offline tab.
	protected void OnConnected()
	{
		// Reinit the workshop API
		InitWorkshopApi();

		// Ask the same page again
		RequestPage(m_iCurrentPage);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileClick(SCR_ScriptedWidgetComponent baseComp)
	{
		SCR_ContentBrowserTileComponent comp = SCR_ContentBrowserTileComponent.Cast(baseComp);

		if (!comp || m_eMode == EContentBrowserAddonsSubMenuMode.MODE_EXTERNAL_ITEM_ARRAY)
			return;

		SCR_WorkshopItem selectedItem = comp.GetWorkshopItem();
		if (!selectedItem)
			return;
		
		ContentBrowserDetailsMenu.OpenForWorkshopItem(selectedItem);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileFocus(ScriptedWidgetComponent comp)
	{	
		SCR_ContentBrowserTileComponent tile = SCR_ContentBrowserTileComponent.Cast(comp);
		if (!tile)
			return;
		
		SCR_WorkshopItem item = tile.GetWorkshopItem();

		UpdateNavigationButtons();
		tile.GetOnChange().Insert(OnTileStateChange);
		
		if (!item || m_Widgets.m_AddonDetailsPanelComponent.GetItem() == item)
			return;

		m_Widgets.m_AddonDetailsPanelComponent.SetWorkshopItem(item);

		m_LastFocusedTile = tile;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTileFocusLost(ScriptedWidgetComponent comp)
	{
		SCR_ContentBrowserTileComponent tile = SCR_ContentBrowserTileComponent.Cast(comp);
		if (!tile)
			return;
		
		UpdateNavigationButtons();
		tile.GetOnChange().Remove(OnTileStateChange);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTileStateChange(ScriptedWidgetComponent comp)
	{
		UpdateNavigationButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFilterPanelToggled(bool shown)
	{
		if (shown)
			SwitchFocus(SCR_EListMenuWidgetFocus.FILTERING);
		else
			SwitchFocus(SCR_EListMenuWidgetFocus.LIST);

		SCR_AnalyticsApplication.GetInstance().WorkshopUseFilterOn();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputAction(string name, float multiplier)
	{
		if (!m_bShown || GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE)
			return;
		
		switch(name)
		{
			case UIConstants.MENU_ACTION_SELECT: 		OpenItemDetails(); 			break;
			case UIConstants.MENU_ACTION_ENABLE:		SetItemEnabled(); 			break;
			case UIConstants.MENU_ACTION_SELECT_HOLD:	ExecuteItemPrimaryAction(); break;
			case UIConstants.MENU_ACTION_FAVORITE:		SetItemFavorite();			break;
			case UIConstants.MENU_ACTION_MOUSE_WHEEL:	OnScrollPage(name);			break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFilterButton(SCR_ButtonBaseComponent comp)
	{
		// Toggle the state of the filter panel
		bool show = !m_Widgets.m_FilterPanelComponent.GetFilterListBoxShown();
		m_Widgets.m_FilterPanelComponent.ShowFilterListBox(show);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPrimaryButton(SCR_ButtonBaseComponent comp)
	{
		ExecuteItemPrimaryAction();
	}

	//------------------------------------------------------------------------------------------------
	protected void ExecuteItemPrimaryAction()
	{
		SCR_WorkshopItem item = GetSelectedItem();
		if (!item || item.GetRestricted())
			return;

		SCR_WorkshopUiCommon.ExecutePrimaryAction(item, m_DownloadRequest);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEnableButton(SCR_ButtonBaseComponent comp)
	{
		SetItemEnabled();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetItemEnabled()
	{
		SCR_WorkshopItem item = GetSelectedItem();
		if (!item)
			return;
		
		SCR_EAddonPrimaryActionState state = SCR_WorkshopUiCommon.GetPrimaryActionState(item);
		if (item.GetRestricted() || !item.GetOffline() || SCR_WorkshopUiCommon.IsDownloadingAddonOrDependencies(item) || (!item.GetEnabled() && state == SCR_EAddonPrimaryActionState.DEPENDENCIES_DOWNLOAD))
			return;
		
		SCR_WorkshopUiCommon.OnEnableAddonButton(item);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRepairButton(SCR_ButtonBaseComponent comp)
	{
		SetItemRepair();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetItemRepair()
	{
		SCR_WorkshopItem item = GetSelectedItem();
		if (!item || item.GetRestricted())
			return;

		SCR_ValidateRepair_Dialog dialogValidator = SCR_CommonDialogs.CreateValidateRepairDialog();
		dialogValidator.LoadAddon(item);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFavouriteButton(SCR_ButtonBaseComponent comp)
	{
		SetItemFavorite();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetItemFavorite()
	{
		SCR_WorkshopItem item = GetSelectedItem();
		if (!item || item.GetRestricted())
			return;

		item.SetFavourite(!item.GetFavourite());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDetailsButton(SCR_ButtonBaseComponent comp)
	{
		OpenItemDetails();
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenItemDetails()
	{
		SCR_WorkshopItem item = GetSelectedItem();
		if (m_eMode == EContentBrowserAddonsSubMenuMode.MODE_EXTERNAL_ITEM_ARRAY || !item || item.GetRestricted())
			return;
		
		OnTileClick(m_LastFocusedTile);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnRequestPageGetAllAssets(BackendCallback callback)
	{
		m_iItemsTotal = m_WorkshopApi.GetTotalItemCount();
		UpdateItemsFoundMessage(m_iOnlineFilteredItems, m_iItemsTotal);
		m_bFirstLoad = false;
		RequestOnlinePage(m_iCurrentPage);	
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnRequestPageGetAssets()
	{
		// Search Box Message
		m_iOnlineFilteredItems = m_WorkshopApi.GetTotalItemCount();

		array<WorkshopItem> items = {};
		m_WorkshopApi.GetPageItems(items);

		if (m_iOnlineFilteredItems < items.Count())
			m_iOnlineFilteredItems = items.Count();

		if (m_iItemsTotal < m_iOnlineFilteredItems)
			m_iItemsTotal = m_iOnlineFilteredItems;

		if (!m_bFirstLoad)
			UpdateItemsFoundMessage(m_iOnlineFilteredItems, m_iItemsTotal);

		array<ref SCR_WorkshopItem> itemsRegistered = {};
		SCR_WorkshopItem iRegistered
		foreach (WorkshopItem i : items)
		{
			iRegistered = SCR_AddonManager.GetInstance().Register(i);
			if (iRegistered)
				itemsRegistered.Insert(iRegistered);
		}

		if (!itemsRegistered.IsEmpty())
		{
			// There are some items received
			SetPanelsMode(false, m_bFirstPage && m_Widgets.m_FilterPanelComponent.AnyFilterButtonsVisible(), string.Empty, m_bFirstPage);
		}
		else
		{
			// No items received
			SetPanelsMode(false, messagePresetTag: MESSAGE_TAG_NOTHING_FOUND, forceFiltersList: true);
		}

		m_iPageCount = m_WorkshopApi.GetPageCount();
		GridScreen_DisplayItems(itemsRegistered);

		m_bFirstPage = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnRequestPageTimeout()
	{
		if (GetShown())
		{
			// This submenu is shown, show the timeout dialog
			SCR_ConfigurableDialogUi dlg = SCR_CommonDialogs.CreateTimeoutTryAgainCancelDialog();
			dlg.m_OnCancel.Insert(Callback_OnRequestPageTimeout_OnCloseDialog);
			dlg.m_OnConfirm.Insert(Callback_OnRequestPageTimeout_OnDialogTryAgain);
		}
		else
		{
			// This submenu is not shown (probably we switched to another tab)
			// Try to perform the request again
			m_bClearCacheAtNextRequest = true;
			RequestPage(m_iCurrentPage);
		}

		m_bFailToLoad = true;

		m_iPageCount = 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnRequestPageTimeout_OnCloseDialog()
	{
		// Set message
		SetPanelsMode(m_bPanelsModeEmpty, messagePresetTag: MESSAGE_TAG_ERROR_CONNECTION);
		RequestTabChange(EWorkshopTabId.OFFLINE);
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnRequestPageTimeout_OnDialogTryAgain()
	{
		m_bClearCacheAtNextRequest = true;
		RequestPage(m_iCurrentPage);
	}

	//------------------------------------------------------------------------------------------------
	//! Called by SCR_AddonManager when some addon is downloaded or uninstalled
	protected void Callback_OnAddonOfflineStateChanged(SCR_WorkshopItem item, bool newState)
	{
		if (m_eMode == EContentBrowserAddonsSubMenuMode.MODE_OFFLINE)
		{
			// Some addon was installed or uninstalled, we must refresh the page
			RequestPage(m_iCurrentPage);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called by SCR_AddonManager when some addon is reported or unreported
	protected void Callback_OnAddonReportedStateChanged(SCR_WorkshopItem item, bool newReported)
	{
		// Refresh current page if it has this addon
		bool found = false;
		foreach (SCR_ContentBrowserTileComponent comp : m_aTileComponents)
		{
			if (comp.GetWorkshopItem() == item)
			{
				m_bClearCacheAtNextRequest = true;
				RequestPage(m_iCurrentPage);
				break;
			}
		}
	}

	// --- Paging ---
	//------------------------------------------------------------------------------------------------
	protected void OnPrevPageButton()
	{
		EInputDeviceType inputType = -1;
		if (m_bMousePageUsed)
			inputType = EInputDeviceType.MOUSE;

		m_bMousePageUsed = false;

		if (inputType == EInputDeviceType.MOUSE && !IsWidgetUnderCursor(m_Widgets.m_LeftPanelArea))
			return;

		m_iCurrentPage--;
		m_iCurrentPage = Math.ClampInt(m_iCurrentPage, 0, m_iPageCount - 1);

		RequestPage(m_iCurrentPage);
		UpdatePagingWidgets();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNextPageButton()
	{
		EInputDeviceType inputType = -1;
		if (m_bMousePageUsed)
			inputType = EInputDeviceType.MOUSE;

		m_bMousePageUsed = false;

		if (inputType == EInputDeviceType.MOUSE && !IsWidgetUnderCursor(m_Widgets.m_LeftPanelArea))
			return;

		m_iCurrentPage++;
		m_iCurrentPage = Math.ClampInt(m_iCurrentPage, 0, m_iPageCount - 1);

		RequestPage(m_iCurrentPage);
		UpdatePagingWidgets();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnScrollPage(string name)
	{
		float value = GetGame().GetInputManager().GetActionValue(name);
		if (value != 0)
			m_bMousePageUsed = true;
				
		bool playSound;
		
		if (value < 0 && m_iCurrentPage < m_iPageCount - 1)
		{
			OnNextPageButton();
			playSound = true;
		}
		else if (value > 0 && m_iCurrentPage > 0)
		{
			OnPrevPageButton();
			playSound = true;
		}
		
		if (playSound && m_bCanPlayMouseWheelSound && IsWidgetUnderCursor(m_Widgets.m_LeftPanelArea))
		{
			SCR_UISoundEntity.SoundEvent(m_sScrollWheelPageTurningSound);
			GetGame().GetCallqueue().CallLater(OnMouseWheelSoundCooldownEnd, MOUSE_WHEEL_SOUND_COOLDOWN);
			m_bCanPlayMouseWheelSound = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMouseWheelSoundCooldownEnd()
	{
		m_bCanPlayMouseWheelSound = true;
	}
	
	// --- Filters and Sorting ---
	//------------------------------------------------------------------------------------------------
	//! Called when filter listbox selection changes
	protected void OnFilterChange(SCR_FilterEntry filter)
	{
		m_bClearCacheAtNextRequest = true;
		RequestPage(0);
		SCR_AnalyticsApplication.GetInstance().WorkshopSetFilter(filter.GetCategory().m_sInternalName, filter.m_sInternalName);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when user confirms the value in the seacrh string
	protected void OnFilterSearchConfirm(SCR_EditBoxComponent comp, string newValue)
	{
		m_bClearCacheAtNextRequest = true;
		RequestPage(0);
		SCR_AnalyticsApplication.GetInstance().WorkshopUseSearch();
	}

	//------------------------------------------------------------------------------------------------
	//! Called when sorting header changes
	protected void OnSortingHeaderChange(SCR_SortHeaderComponent sortHeader)
	{
		m_bClearCacheAtNextRequest = true;
		RequestPage(0);
		SCR_AnalyticsApplication.GetInstance().WorkshopSetSorting(sortHeader.GetSortElementName());
	}

	//------------------------------------------------------------------------------------------------
	protected array<ref SCR_WorkshopItem> SearchItems(array<ref SCR_WorkshopItem> items, string searchStr)
	{
		array<ref SCR_WorkshopItem> itemsOut = {};

		if (searchStr.IsEmpty())
		{
			foreach (SCR_WorkshopItem i : items)
			{
				itemsOut.Insert(i);
			}

			return itemsOut;
		}

		string searchStrLower = searchStr;
		searchStrLower.ToLower();
		string element;
		array<WorkshopTag> tags = {};
		bool foundTag;
		foreach (SCR_WorkshopItem i : items)
		{
			// Name
			element = i.GetName();
			element.ToLower();
			if (element.Contains(searchStrLower))
			{
				itemsOut.Insert(i);
				continue;
			}

			// Author
			element = i.GetAuthorName();
			element.ToLower();
			if (element.Contains(searchStrLower))
			{
				itemsOut.Insert(i);
				continue;
			}
			
			// Description
			element = i.GetDescription();
			element.ToLower();
			if (element.Contains(searchStrLower))
			{
				itemsOut.Insert(i);
				continue;
			}

			// Tags
			tags.Clear();
			i.GetWorkshopItem().GetTags(tags);
			foundTag = false;
			foreach (WorkshopTag tag : tags)
			{
				element = tag.Name();
				element.ToLower();
				if (element == searchStrLower)
				{
					foundTag = true;
					break;
				}
			}

			if (foundTag)
			{
				itemsOut.Insert(i);
				continue;
			}
		}

		return itemsOut;
	}

	//------------------------------------------------------------------------------------------------
	//! Filters the array according to currently selected filters.
	//! !! Works only for offline configuration of filters!
	protected array<ref SCR_WorkshopItem> Filter_OfflineFilterAndSearch(array<ref SCR_WorkshopItem> itemsIn)
	{
		SCR_FilterSet filterSet = m_Widgets.m_FilterPanelComponent.GetFilter();
		string searchStr = m_Widgets.m_FilterPanelComponent.GetEditBoxSearch().GetValue();
		array<ref SCR_WorkshopItem> items = SearchItems(itemsIn, searchStr);

		// !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !
		// !  !  Filter categories and filter names are bound by strings, be careful while changing them   !  !
		// !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !
		// Arrays produced by each category. In the end we find intersection of those arrays.
		array<ref array<ref SCR_WorkshopItem>> arraysFromCategories = {};

		// Quick filters - only one is selected at a time
		SCR_FilterCategory catQuick = filterSet.FindFilterCategory("quick_filters");
		SCR_FilterEntry filterAll = catQuick.FindFilter("all");
		SCR_FilterEntry filterFav = catQuick.FindFilter("favourite");
		SCR_FilterEntry filterAddonsOnly = catQuick.FindFilter("addons_only");
		SCR_FilterEntry filterSavensOnly = catQuick.FindFilter("saves_only");
		
		if (filterAll.GetSelected())
			arraysFromCategories.Insert(items);
		
		else if (filterFav.GetSelected())
			arraysFromCategories.Insert(SCR_AddonManager.SelectItemsBasic(items, EWorkshopItemQuery.FAVOURITE));
		
		else if (filterAddonsOnly.GetSelected())
			arraysFromCategories.Insert(SCR_AddonManager.SelectItemsBasic(items, EWorkshopItemQuery.ONLY_WORKSHOP_ITEM));
		
		else if (filterSavensOnly.GetSelected())
			arraysFromCategories.Insert(SCR_AddonManager.SelectItemsBasic(items, EWorkshopItemQuery.ONLY_WORLD_SAVES));

		// Type tags
		array<WorkshopTag> selectedWorkshopTags = {};
		SCR_ContentBrowserFilterTag filterEntryTag;
		SCR_FilterCategory catMANWType = filterSet.FindFilterCategory("manw_winners");
		if (catMANWType)
		{
			foreach (SCR_FilterEntry f : catMANWType.GetFilters())
			{
				filterEntryTag = SCR_ContentBrowserFilterTag.Cast(f);
				if (filterEntryTag && filterEntryTag.GetSelected())
					selectedWorkshopTags.Insert(filterEntryTag.GetWorkshopTag());
			}
		}
		SCR_FilterCategory catType = filterSet.FindFilterCategory("type");
		foreach (SCR_FilterEntry f : catType.GetFilters())
		{
			filterEntryTag = SCR_ContentBrowserFilterTag.Cast(f);
			if (filterEntryTag && filterEntryTag.GetSelected())
				selectedWorkshopTags.Insert(filterEntryTag.GetWorkshopTag());
		}
		// If any tag filter is selected, but not all of them (because it would be same as selecting none)
		if (!selectedWorkshopTags.IsEmpty() && selectedWorkshopTags.Count() != catType.GetFilters().Count())
		{
			array<ref SCR_WorkshopItem> itemsFilteredByTags = {};
			foreach (SCR_WorkshopItem item : items)
			{
				if (item.GetWorkshopItem().HasAnyTag(selectedWorkshopTags))
					itemsFilteredByTags.Insert(item);
			}
			arraysFromCategories.Insert(itemsFilteredByTags);
		}

		// State
		SCR_FilterCategory catState = filterSet.FindFilterCategory("state");
		if (!catState)
			return {};

		if (catState.GetAllSelected() || catState.GetAllDeselected())
		{
			arraysFromCategories.Insert(items);
		}
		else
		{
			EWorkshopItemQuery query = 0;
			if (catState.FindFilter("enabled").GetSelected())
				query = query | EWorkshopItemQuery.ENABLED;
			
			if (catState.FindFilter("disabled").GetSelected())
				query = query | EWorkshopItemQuery.NOT_ENABLED;

			array<ref SCR_WorkshopItem> queryResult = SCR_AddonManager.SelectItemsOr(items, query);
			arraysFromCategories.Insert(queryResult);
		}

		// Condition
		SCR_FilterCategory catCondition = filterSet.FindFilterCategory("condition");
		if (catCondition.GetAllSelected() || catCondition.GetAllDeselected())
		{
			arraysFromCategories.Insert(items);
		}
		else
		{
			EWorkshopItemQuery query = 0;
			if (catCondition.FindFilter("ready").GetSelected())
				query = query | EWorkshopItemQuery.NO_PROBLEMS;
			
			if (catCondition.FindFilter("update_available").GetSelected())
				query = query | EWorkshopItemQuery.UPDATE_AVAILABLE | EWorkshopItemQuery.DEPENDENCY_UPDATE_AVAILABLE;
			
			if (catCondition.FindFilter("repair_required").GetSelected())
				query = query | EWorkshopItemQuery.DEPENDENCY_MISSING | EWorkshopItemQuery.ENABLED_AND_DEPENDENCY_DISABLED;

			array<ref SCR_WorkshopItem> queryResult = SCR_AddonManager.SelectItemsOr(items, query);
			arraysFromCategories.Insert(queryResult);
		}

		// Find intersection of all arrays
		return Filter_IntersectArrays(arraysFromCategories);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns intersection of all arrays (array of elements which are contained in all arrays)
	protected array<ref SCR_WorkshopItem> Filter_IntersectArrays(array<ref array<ref SCR_WorkshopItem>> arrays)
	{
		array<ref SCR_WorkshopItem> result = {};

		// Trivial cace: no arrays
		if (arrays.IsEmpty())
			return result;

		array<ref SCR_WorkshopItem> array0 = arrays[0];
		
		// Trivial case: one array
		if (arrays.Count() == 1)
		{
			result.Resize(array0.Count());
			
			foreach (int i, SCR_WorkshopItem item : array0)
			{
				result[i] = item;
			}

			return result;
		}

		// Non-trivial case: more than one array
		bool foundInAll;
		foreach (SCR_WorkshopItem item : array0)
		{
			// This item must be found in all other arrays too
			foundInAll = true;
			for (int arrayId = 1; arrayId < arrays.Count(); arrayId++)
			{

				if (!arrays[arrayId].Contains(item))
				{
					foundInAll = false;
					break;
				}
			}

			if (foundInAll)
				result.Insert(item);
		}

		return result;
	}
	
	//--- API ---
	//------------------------------------------------------------------------------------------------
	//! Called from OnPack of a Json API Struct.
	//! Here we pack the request data according to current filtering and sorging parameters.
	void PackGetAssetListParams(SCR_ContentBrowser_GetAssetListParams p, bool defaultValues = false)
	{
		SCR_FilterSet filterSet = m_Widgets.m_FilterPanelComponent.GetFilter();
		SCR_FilterCategory otherCategory = filterSet.FindFilterCategory("other");
		
		// Pack search string
		string searchStr = m_Widgets.m_FilterPanelComponent.GetWidgets().m_FilterSearchComponent.GetValue();
		if (!searchStr.IsEmpty() && !defaultValues)
			p.StoreString("search", searchStr);				// <- "search": "..."
		
		// Pack sorting and filters
		PackSortingData(p);
		PackQuickFilterData(p, defaultValues);
		PackTagFilterData(p, defaultValues);

		// --- Reported ---
		// Pack reported
		SCR_FilterEntry reportedFilter = otherCategory.FindFilter("reported");
		p.StoreBoolean("reported", reportedFilter.GetSelected(defaultValues));		// <- "reported": true
		p.StoreBoolean("hidden", reportedFilter.GetSelected(defaultValues));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Pack filter sorting settings - order type, dirrection
	protected void PackSortingData(SCR_ContentBrowser_GetAssetListParams params)
	{
		// Pack sorting
		string orderBy = "name";
		string currentSortingItem = m_Widgets.m_SortingHeaderComponent.GetSortElementName();
		bool sortAscending = m_Widgets.m_SortingHeaderComponent.GetSortOrderAscending();

		const string orderAsc = "asc";
		const string orderDesc = "desc";

		switch (currentSortingItem)
		{
			case "popularity":
				orderBy = "popularity";
				break;
			case "rating":
				orderBy = "averageRating";
				break;
			case "recently_added":
				orderBy = "createdAt";
				sortAscending = !sortAscending; // Invert sorting direction, when ascen
				break;
			case "name":
				orderBy = "name";
				break;
			case "subscribers":
				orderBy = "subscriberCount";
				break;
		}

		string orderDirection = orderAsc;
		if (!sortAscending)
			orderDirection = orderDesc;

		params.StoreString("orderBy", orderBy);					// <- "orderBy": "..."
		params.StoreString("orderDirection", orderDirection);	// <- "orderDirection": "..."
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PackQuickFilterData(SCR_ContentBrowser_GetAssetListParams params, bool defaultValues)
	{
		SCR_FilterSet filterSet = m_Widgets.m_FilterPanelComponent.GetFilter();
		
		// Pack 'quick filters'
		SCR_FilterCategory quickCategory = filterSet.FindFilterCategory("quick_filters");
		SCR_FilterEntry filterAll = quickCategory.FindFilter("all");
		SCR_FilterEntry filterFav = quickCategory.FindFilter("favourite");
		//SCR_FilterEntry filterSub = quickCategory.FindFilter("subscribed");
		SCR_FilterEntry filterRated = quickCategory.FindFilter("rated");

		if (filterFav.GetSelected(defaultValues))
			params.StoreBoolean("favorited", true);				// <- "favorited": true
		//else if (filterSub.GetSelected())
		//	p.StoreBoolean("subscribed", true);				// <- "subscribed": true
		else if (filterRated.GetSelected(defaultValues))
		{
			params.StartObject("averageRating");					// <- "AverageRating" object
			params.StoreFloat("gt", 0.75);						// <- "gt": 1.23
			params.StoreFloat("lt", 1);							// <- "lt": 1.23
			params.EndObject();									// <- End AverageRating
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PackTagFilterData(SCR_ContentBrowser_GetAssetListParams params, bool defaultValues)
	{
		SCR_FilterSet filterSet = m_Widgets.m_FilterPanelComponent.GetFilter();
		SCR_FilterCategory otherCategory = filterSet.FindFilterCategory("other");
		
		// Make arrays with tags to include or exclude
		array<ref array<string>> tagArraysInclude = {};
		array<string> tagsExclude = {};

		// Tags from type filters
		SCR_FilterCategory manwCategory = filterSet.FindFilterCategory("manw_winners");
		if (manwCategory.GetAnySelected() && !defaultValues)
		{
			array<string> typeTags = {};
			foreach (SCR_FilterEntry filter : manwCategory.GetFilters())
			{
				if (filter.GetSelected(defaultValues))
					typeTags.Insert(filter.m_sInternalName);
			}
			tagArraysInclude.Insert(typeTags);
		}

		// Tags from type filters
		SCR_FilterCategory typeCategory = filterSet.FindFilterCategory("type");
		if (typeCategory.GetAnySelected() && !defaultValues)
		{
			array<string> typeTags = {};
			foreach (SCR_FilterEntry filter : typeCategory.GetFilters())
			{
				if (filter.GetSelected(defaultValues))
					typeTags.Insert(filter.m_sInternalName);
			}
			tagArraysInclude.Insert(typeTags);
		}

		// Tags from test mods
		SCR_FilterEntry testFilter = otherCategory.FindFilter("TESTDEV");
		if (!testFilter.GetSelected(defaultValues))
			tagsExclude.Insert(testFilter.m_sInternalName);

		// Pack the tags
		if (!tagArraysInclude.IsEmpty() || !tagsExclude.IsEmpty())
		{
			params.StartObject("tags");							// <- Start "tags" object

			if (!tagArraysInclude.IsEmpty())
			{
				params.StartArray("include");					// <- Start "include"  array

				foreach (array<string> tagArray : tagArraysInclude)
				{
					params.ItemArray();							// <- Start tags array
					foreach (string tag : tagArray)
					{
						params.ItemString(tag);					// <- Array element
					}
					params.EndArray();							// <- End tags array
				}

				params.EndArray();								// <- End "include" array
			}

			if (!tagsExclude.IsEmpty())
			{
				params.StartArray("exclude");					// <- Start "exclude" array
				foreach (string tag : tagsExclude)
				{
					params.ItemString(tag);						// <- Array element
				}
				params.EndArray();								// <- End "exclude" array
			}

			params.EndObject();									// <- End "tags" object
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the item array on which this tab operates. Only possible in external item array mode.
	void SetWorkshopItems(array<ref SCR_WorkshopItem> items)
	{
		if (m_eMode != EContentBrowserAddonsSubMenuMode.MODE_EXTERNAL_ITEM_ARRAY)
			return;

		m_aExternalItems = items;
		RequestPage(0);
	}
}

class SCR_ContentBrowser_GetAssetListParams : PageParams
{
	// We will call back to submenu to perform packing of the JSON struct
	SCR_ContentBrowser_AddonsSubMenu m_AddonsSubMenu;
	protected bool m_bUseDefaultValues;

	//------------------------------------------------------------------------------------------------
	void SCR_ContentBrowser_GetAssetListParams(SCR_ContentBrowser_AddonsSubMenu subMenu, bool defaultValues = false)
	{
		m_bUseDefaultValues = defaultValues;
		m_AddonsSubMenu = subMenu;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPack()
	{
		m_AddonsSubMenu.PackGetAssetListParams(this, m_bUseDefaultValues);
	}
}

class SCR_CompareWorkshopItemName : SCR_SortCompare<SCR_WorkshopItem>
{
	override static int Compare(SCR_WorkshopItem left, SCR_WorkshopItem right)
	{
		string name1 = left.GetName();
		string name2 = right.GetName();

		if (name1.Compare(name2) == -1)
			return 1;
		else
			return 0;
	}
}

class SCR_CompareWorkshopItemTimeSinceLastPlay : SCR_SortCompare<SCR_WorkshopItem>
{
	override static int Compare(SCR_WorkshopItem left, SCR_WorkshopItem right)
	{
		int timeLeft = left.GetTimeSinceLastPlay();
		if (timeLeft < 0)			// Negative value means we have never played this.
			timeLeft =	0x7FFFFFFF;	// We set it to max positive int for sorting purpuse.
		int timeRight = right.GetTimeSinceLastPlay();
		if (timeRight < 0)
			timeRight =	0x7FFFFFFF;

		return timeLeft < timeRight;
	}
}

class SCR_CompareWorkshopItemTimeSinceFirstDownload : SCR_SortCompare<SCR_WorkshopItem>
{
	override static int Compare(SCR_WorkshopItem left, SCR_WorkshopItem right)
	{
		int timeLeft = left.GetTimeSinceFirstDownload();
		if (timeLeft < 0)			// Negative value means we never downloaded this
			timeLeft =	0x7FFFFFFF;	// We set it to max positive int for sorting purpuse.
		int timeRight = right.GetTimeSinceFirstDownload();
		if (timeRight < 0)
			timeRight =	0x7FFFFFFF;

		return timeLeft < timeRight;
	}
}

// Sort by time since last played
class SCR_CompareWorkshopItemTargetSize : SCR_SortCompare<SCR_WorkshopItem>
{
	override static int Compare(SCR_WorkshopItem left, SCR_WorkshopItem right)
	{
		float leftSize = left.GetTargetRevisionPatchSize();
		float rightSize = right.GetTargetRevisionPatchSize();

		return leftSize < rightSize;
	}
}
