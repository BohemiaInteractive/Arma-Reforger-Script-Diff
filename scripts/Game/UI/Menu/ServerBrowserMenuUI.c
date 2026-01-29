/*
Main class for server browser menu handling
Handles cooperation of varios components for server searching, joning, mods handling, etc.
*/

void ScriptInvokerRoomMethod(Room room);
typedef func ScriptInvokerRoomMethod;
typedef ScriptInvokerBase<ScriptInvokerRoomMethod> ScriptInvokerRoom;

class ServerBrowserMenuUI : MenuRootBase
{
	// Server list feedback strings
	const string TAG_DETAILS_FALLBACK_SEACHING = "Searching";
	const string TAG_DETAILS_FALLBACK_EMPTY = "Empty";
	const string TAG_MESSAGE_SEARCHING = "SEARCHING";
	const string TAG_MESSAGE_CONNECTING = "CONNECTING";

	const int SERVER_LIST_VIEW_SIZE = 32;
	const int ROOM_CONTENT_LOAD_DELAY = 500;
	const int ROOM_REFRESH_RATE = 10 * 1000;

	// This should be the same value as in High latency filter in {4F6F41C387ADC14E}Configs/ServerBrowser/ServerBrowserFilterSet.conf
	// TODO: unify with the value set in filters
	static const int HIGH_PING_SERVER_THRESHOLD = 200;
	
	protected bool m_bHostedServers;

	// Widget class reference
	protected ref ServerBrowserMenuWidgets m_Widgets = new ServerBrowserMenuWidgets();

	// Server list entry for user interactions
	protected SCR_ServerBrowserEntryComponent m_SelectedServerEntry;

	// Widget handlers
	protected SCR_TabViewComponent m_TabView;
	protected SCR_FilterPanelComponent m_FilterPanel;
	protected SCR_PooledServerListComponent m_ScrollableList;
	protected SCR_ServerScenarioDetailsPanelComponent m_ServerScenarioDetails;

	// States
	protected SCR_EListMenuWidgetFocus m_eFocusedWidgetState = SCR_EListMenuWidgetFocus.LIST;

	// Skip the details dialog on double click
	protected bool m_bQuickJoin = true;

	// Lobby Rooms handling
	protected ref array<Room> m_aRooms = {};
	protected ref array<Room> m_aDirectFoundRooms = {};

	protected ClientLobbyApi m_Lobby;
	
	protected SCR_ConfigurableDialogUi m_ModListFailDialog;

	// Privileges
	protected ref SCR_ScriptPlatformRequestCallback m_CallbackGetPrivilege;

	// Search callbacks
	protected ref BackendCallback m_CallbackLastSearch;
	protected ref BackendCallback m_CallbackScroll = new BackendCallback();
	protected ref SCR_RoomCallback m_CallbackFavorite = new SCR_RoomCallback();

	// Supposedly used for direct join process
	protected ref BackendCallback m_CallbackSearchTarget = new BackendCallback();
	
	// Room data refresh
	protected ref BackendCallback m_CallbackAutoRefresh;
	protected bool m_bFirstRoomLoad = true;
	protected bool m_bForceUnfilteredRequest;
	protected int m_iTotalNumberOfRooms;

	// Managers
	protected ref SCR_RoomModsManager m_ModsManager = new SCR_RoomModsManager();
	protected ref SCR_ServerBrowserDialogManager m_Dialogs = new SCR_ServerBrowserDialogManager();
	protected ref SCR_RoomPasswordVerification m_PasswordVerification = new SCR_RoomPasswordVerification();
	
	// Joinign
	protected ref BackendCallback m_CallbackJoin;
	protected ref BackendCallback m_CallbackQueue;

	protected ref RoomJoinData m_JoinData = new RoomJoinData();
	
	// Filter parameters
	protected ref FilteredServerParams m_ParamsFilter = new FilteredServerParams();
	protected ref FilteredServerParams m_DirectJoinParams;
	
	// Server mod content
	protected ref array<ref SCR_WorkshopItem> m_aRequiredMods = {}; 
	protected ref array<ref SCR_WorkshopItemActionDownload> m_aUnrelatedDownloads = {};

	// Message components
	protected SCR_SimpleMessageComponent m_SimpleMessageWrap;
	protected SCR_SimpleMessageComponent m_SimpleMessageList;

	protected bool m_bIsWaitingForBackend = false;

	// Reconnecting to last played server
	protected ref BackendCallback m_CallbackSearchPreviousRoom = new BackendCallback();
	protected ref SCR_GetRoomsIds m_SearchIds = new SCR_GetRoomsIds();

	protected ref array<ref SCR_FilterEntry> m_aFiltersToSelect = {};

	// Script invokers
	protected ref ScriptInvokerVoid m_OnFavoritesResponse = new ScriptInvokerVoid();

	// Entry Actions
	protected EInputDeviceType m_eLastInputType;
	protected bool m_bWasEntrySelected;
	
	// Cache last clicked entry to trigger the correct dialog after the double click window
	protected SCR_ServerBrowserEntryComponent m_ClickedEntry;
	protected SCR_MenuActionsComponent m_ActionsComponent;
	
	protected static ref SCR_ScriptPlatformRequestCallback m_CallbackGetMPPrivilege;
	protected static MissionWorkshopItem m_MissionToFilter;
	
	// Joining process state
	protected Room m_JoinProcessTargetRoom;
	protected Room m_RejoinRoom;
	
	protected int m_iSelectedTab = 0;
	
	//------------------------------------------------------------------------------------------------
	// --- OVERRIDES ---
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Opening menu - do server browser setup - TODO@wernerjak - cleanup open
	override void OnMenuOpen()
	{
		m_Lobby = GetGame().GetBackendApi().GetClientLobby();

		// Items preloading
		WorkshopApi workshopApi = GetGame().GetBackendApi().GetWorkshop();
		if (workshopApi && workshopApi.NeedAddonsScan())
			workshopApi.ScanOfflineItems();

		// Find widgets
		m_Widgets.FindAllWidgets(GetRootWidget());

		// Accessing handlers
		SetupHandlers();
		SetupCallbacks();
		SetupRefreshCallback();

		SetupParams(m_Lobby);

		// Setup list and message
		if (SCR_ServicesStatusHelper.IsBackendReady())
			Messages_ShowMessage(TAG_MESSAGE_SEARCHING);
		else
			Messages_ShowMessage(TAG_MESSAGE_CONNECTING, true);

		if (m_ScrollableList)
			m_ScrollableList.ShowEmptyRooms();

		// Setup Actions
		m_ActionsComponent = SCR_MenuActionsComponent.FindComponent(GetRootWidget());
		if (m_ActionsComponent)
			m_ActionsComponent.GetOnAction().Insert(OnActionTriggered);

		UpdateNavigationButtons();

		SwitchFocus(SCR_EListMenuWidgetFocus.SORTING);
		
		GetGame().OnUserSettingsChangedInvoker().Insert(FilterCrossplayCheck);
		
		GetGame().GetGameBlocklist().UpdateBlockList();
		
		// Hide news menu button (top right corner) on PS
		if (GetGame().GetPlatformService().GetLocalPlatformKind() == PlatformKind.PSN)
		{
			Widget newsButton = GetRootWidget().FindAnyWidget("NewsButton");
			if (newsButton)
			{
				newsButton.SetVisible(false);
				newsButton.SetEnabled(false);
			}
		}
		
		super.OnMenuOpen();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();
		
		SCR_AnalyticsApplication.GetInstance().CloseMultiplayerMenu();		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpened()
	{
		// Kick errors
		SCR_KickDialogs.SetReconnectEnabled(true);
		SCR_KickDialogs.GetOnReconnect().Insert(OnLastRoomReconnectConfirm);
		SCR_KickDialogs.GetOnCancel().Insert(OnRejoinCancel);
		
		super.OnMenuOpened();
		
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		// Focus back on the host button
		if (m_bHostedServers)
			GetGame().GetCallqueue().Call(FocusWidget, m_Widgets.m_wHostNewServerButton);
		else
			SwitchFocus(SCR_EListMenuWidgetFocus.LIST, true);
		
		super.OnMenuFocusGained();
		
		if (m_ActionsComponent)
			m_ActionsComponent.ActivateActions();
	}

	//------------------------------------------------------------------------------------------------
	//! Closing menu - clearing server browser data
	override void OnMenuClose()
	{
		// Kick errors
		SCR_KickDialogs.SetReconnectEnabled(false);
		SCR_KickDialogs.GetOnReconnect().Remove(OnLastRoomReconnectConfirm);
		SCR_KickDialogs.GetOnCancel().Remove(OnRejoinCancel);
		
		ClearScenarioFilters();
		
		// Clearing handlers
		if (m_TabView)
			m_TabView.GetOnChanged().Remove(OnTabViewSwitch);

		// Store filter parameters
		m_Lobby.StoreParams();

		// Remove callbacks
		ClearRefreshCallback();

		m_ScrollableList.GetOnSetPage().Remove(CallOnServerListSetPage);

		ClearConnectionTimeoutWaiting();

		// Save filter
		m_FilterPanel.Save();
		
		GetGame().OnUserSettingsChangedInvoker().Remove(FilterCrossplayCheck);		
		
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	//! Updating menu - continuous menu hanling
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		// Run actions after backend initialization
		if (m_bIsWaitingForBackend)
			WaitingForRunningBackend();

		// Scroll list update
		if (m_ScrollableList)
			m_ScrollableList.UpdateScroll();

		//! Update Entry buttons
		EInputDeviceType inputDeviceType = GetGame().GetInputManager().GetLastUsedInputDevice();
		bool isEntrySelected = GetSelectedEntry();
		bool shouldUpdateButtons = inputDeviceType != m_eLastInputType || isEntrySelected != m_bWasEntrySelected;
		if (shouldUpdateButtons)
			UpdateNavigationButtons();

		m_eLastInputType = inputDeviceType;
		m_bWasEntrySelected = isEntrySelected;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		
		if (m_ActionsComponent)
			m_ActionsComponent.ActivateActions();

		SCR_AnalyticsApplication.GetInstance().OpenMultiplayerMenu();
	}
	
	//------------------------------------------------------------------------------------------------
	// --- INPUTS ---
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void UpdateNavigationButtons()
	{
		if (!m_Widgets)
			return;

		SCR_ServerBrowserEntryComponent entry = GetSelectedEntry();
		bool versionMismatch, unjoinable;
		bool enabled = entry && entry.GetIsEnabled(versionMismatch, unjoinable);
		bool visible = entry && GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE;

		if (m_Widgets.m_JoinButton)
			m_Widgets.m_JoinButton.SetVisible(visible && !unjoinable, false);
		
		if (m_Widgets.m_DetailsButton)
			m_Widgets.m_DetailsButton.SetVisible(visible && !unjoinable, false);
		
		if (m_Widgets.m_FavoritesButton)
			m_Widgets.m_FavoritesButton.SetVisible(visible && enabled, false);

		if (!enabled || !visible)
			return;

		Room room = entry.GetRoomInfo();
		if (!room)
			return;

		if (m_Widgets.m_FavoritesButton)
			m_Widgets.m_FavoritesButton.SetLabel(UIConstants.GetFavoriteLabel(room.IsFavorite()));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnActionTriggered(string action, float multiplier)
	{
		//!TODO: Why are these being triggered twice?

		//! Proceed only if using mouse, the navigation buttons will take care of the interactions with keyboard or gamepad
		if (GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE)
			return;

		switch (action)
		{
			case UIConstants.MENU_ACTION_DOUBLE_CLICK: 			OnServerEntryClickInteraction(multiplier); break;
			case SCR_ScenarioUICommon.ACTION_SERVER_DETAILS:	OnActionDetails(); break;
			case UIConstants.MENU_ACTION_FAVORITE:				OnActionFavorite(); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Bind action for refreshing server list
	[MenuBindAttribute()]
	void OnActionRefresh()
	{
		Refresh();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Refresh()
	{
		PrintDebug("m_RejoinRoom: " + m_RejoinRoom, "OnActionRefresh");

		// Try negotiate missing MP privilege
		// kuceramar: I dont think this needs to be there anymore as the check is happening before this is opened but lets check later-
		if (!SocialComponent.IsMultiplayerAllowed())
		{
			Messages_ShowMessage("MISSING_PRIVILEGE_MP");
			return;
		}

		// Clear list
		m_CallbackScroll.SetOnSuccess(OnScrollSuccess);
		m_CallbackScroll.SetOnError(OnScrollError);

		//  Setup list loading
		if (m_ScrollableList)
		{
			m_ScrollableList.MoveToTop();
			m_ScrollableList.ShowEmptyRooms();
			m_ScrollableList.ShowScrollbar(false);
		}
		
		CheckBackendState();

		// Start loading
		if (m_bFirstRoomLoad)
			SearchRooms();
		else
			GetGame().GetCallqueue().CallLater(SearchRooms, ROOM_CONTENT_LOAD_DELAY, false);

		if (SCR_ServicesStatusHelper.IsBackendReady())
			Messages_ShowMessage(TAG_MESSAGE_SEARCHING);
		else
			Messages_ShowMessage(TAG_MESSAGE_CONNECTING, true);

		if (m_ServerScenarioDetails)
			m_ServerScenarioDetails.SetDefaultScenario(TAG_DETAILS_FALLBACK_SEACHING);
	}

	//------------------------------------------------------------------------------------------------
	//! Bind action for opening dialog with manual connect to ip
	[MenuBindAttribute()]
	void OnActionManualConnect()
	{
		MultiplayerDialogUI multiplayerDialog = m_Dialogs.CreateManualJoinDialog();

		if (!multiplayerDialog)
			return;

		// Ip handling
		multiplayerDialog.m_OnConfirm.Clear();
		multiplayerDialog.m_OnConfirm.Insert(JoinActions_DirectJoin);

		// Cancel
		multiplayerDialog.m_OnCancel.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Bind action for switching to filter component
	[MenuBindAttribute()]
	void OnActionFilter()
	{
		if (!m_FilterPanel)
			return;

		SCR_EListMenuWidgetFocus focus =  SCR_EListMenuWidgetFocus.FILTERING; 
		if (m_FilterPanel.GetFilterListBoxShown())
			focus =  SCR_EListMenuWidgetFocus.LIST; 

		// Set focus
		m_FilterPanel.ShowFilterListBox(focus == SCR_EListMenuWidgetFocus.FILTERING);
		SwitchFocus(focus);
	}

	//------------------------------------------------------------------------------------------------
	//! Bind action for leaving menu
	[MenuBindAttribute()]
	void OnActionBack()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	//! Action for favoriting server
	protected void OnActionFavorite()
	{
		if (!m_SelectedServerEntry)
			return;

		m_SelectedServerEntry.SetFavorite(!m_SelectedServerEntry.IsFavorite());
	}

	//------------------------------------------------------------------------------------------------
	//! Action for server details
	protected void OnActionDetails()
	{
		SCR_ServerBrowserEntryComponent entry = GetSelectedEntry();
		if (!entry)
			return;

		m_bQuickJoin = false;

		// Join
		Room room = entry.GetRoomInfo();
		if (room && room.Joinable())
			JoinActions_Join();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnServerEntryClickInteraction(float multiplier)
	{
		//! multiplier value in the action is used to differentiate between single and double click

		if (!m_ClickedEntry || !GetEntryUnderCursor())
			return;

		switch (Math.Floor(multiplier))
		{
			case 1: OnServerEntryClick(m_ClickedEntry); break;
			case 2: OnServerEntryDoubleClick(m_ClickedEntry); break;
		}

		m_ClickedEntry = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Join to the room on double click on server entry
	protected void OnServerEntryDoubleClick(SCR_ServerBrowserEntryComponent entry)
	{
		if (entry != m_SelectedServerEntry)
			return;
		
		m_bQuickJoin = true;
		
		// Join
		if (entry && entry.GetRoomInfo() && entry.GetRoomInfo().Joinable())
			JoinActions_Join();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntryMouseButton(string tag)
	{
		OnServerEntryDoubleClick(GetSelectedEntry());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnJoinButton()
	{
		OnServerEntryClick(GetSelectedEntry());
	}

	//------------------------------------------------------------------------------------------------
	//! Join to the room on double click on server entry
	protected void OnServerEntryClick(notnull SCR_ServerBrowserEntryComponent entry)
	{
		//! This means the player moved the mouse to another entry during the small window it takes the component to check for double click...
		//! ...so another room got focused, but we want the clicked one to be the focused one
		if (entry != m_SelectedServerEntry)
			OnServerEntryFocusEnter(entry);
		
		//! On MOUSE clicking opens the details dialog. On KEYBOARD and GAMEPAD single click is quick join, there's a separate button for details
		m_bQuickJoin = GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE;

		// Join
		if (entry.GetRoomInfo() && entry.GetRoomInfo().Joinable())
			JoinActions_Join();
		
		//TODO: handle edge case in which we ask for the details while downloading another version of a required mod, make sure the shown download size is correct
	}

	//------------------------------------------------------------------------------------------------
	// --- SERVICE CHECK ---
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Start looking for servers once backend is runnig
	protected void WaitingForRunningBackend()
	{
		if (!SCR_ServicesStatusHelper.IsAuthenticated())
			return;

		// Call room search or find last server
		string lastId = m_Lobby.GetPreviousRoomId();

		if (!m_Lobby.IsPingAvailable())
			return;

		ClearConnectionTimeoutWaiting();
		Refresh();

		// Clear error msg
		SCR_KickDialogs.Clear();

		// Stop waiting for backend
		m_bIsWaitingForBackend = false;
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void CheckBackendState()
	{	
		if (!SCR_ServicesStatusHelper.IsAuthenticated())
		{
			m_bIsWaitingForBackend = true;
			GetGame().GetCallqueue().Remove(ConnectionTimeout);
			GetGame().GetCallqueue().CallLater(ConnectionTimeout, SCR_ServicesStatusHelper.CONNECTION_CHECK_EXPIRE_TIME);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowTimeoutDialog()
	{
		Messages_ShowMessage("NO_CONNECTION");
		if (m_Dialogs)
		{
			m_Dialogs.DisplayDialog(EJoinDialogState.BACKEND_TIMEOUT);
			m_Dialogs.GetOnConfirm().Clear();
			m_Dialogs.GetOnConfirm().Insert(OnConnectionTimeoutDialogConfirm);
		}
	}
	//------------------------------------------------------------------------------------------------
	//! Fail wating for backend if takes too long
	protected void ConnectionTimeout()
	{
		if (!SCR_ServicesStatusHelper.IsAuthenticated())
		{
			ShowTimeoutDialog();
		}
		ClearConnectionTimeoutWaiting();

		m_bIsWaitingForBackend = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnConnectionTimeoutDialogConfirm()
	{
		m_Dialogs.GetOnConfirm().Remove(OnConnectionTimeoutDialogConfirm);
		
		if (SCR_ServicesStatusHelper.IsBackendReady())
			Messages_ShowMessage(TAG_MESSAGE_SEARCHING);
		else
			Messages_ShowMessage(TAG_MESSAGE_CONNECTING, true);		
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearConnectionTimeoutWaiting()
	{
		GetGame().GetCallqueue().Remove(ConnectionTimeout);
	}

	//------------------------------------------------------------------------------------------------
	// --- ROOMS ---
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Call this on receiving filtered rooms from search
	//! Setting up server list
	protected void OnRoomsFound(array<Room> rooms = null)
	{		
		// Load rooms
		ClientLobbyApi lobby = GetGame().GetBackendApi().GetClientLobby();

		if (rooms)
		{
			// Direct join rooms
			lobby.Target(rooms);
		}
		else
		{
			// Room from search
			lobby.Rooms(m_aRooms);
			//TODO: [BUG] On trunk, switching to Community Tab returns an empty rooms array even if there are rooms?!

			if (m_aRooms.IsEmpty())
				PrintDebug("No room found", "OnRoomsFound");
		}
		

		DisplayRooms(m_aRooms);

		// Move to top in list
		if (m_ScrollableList && (m_ScrollableList.IsListFocused() || m_bFirstRoomLoad))
			m_ScrollableList.FocusFirstAvailableEntry();

		if (m_bFirstRoomLoad)
		{
			m_ScrollableList.SetIsListFocused(true);
			m_bFirstRoomLoad = false;
		}

		m_ScrollableList.ShowScrollbar(true);
		
		int currentRoomsCount = m_Lobby.TotalRoomCount();
		if (currentRoomsCount > m_iTotalNumberOfRooms)
			m_iTotalNumberOfRooms = currentRoomsCount;
		
		// Items Found Message
		if (m_FilterPanel)
			m_FilterPanel.SetItemsFoundMessage(m_Lobby.TotalRoomCount(), m_iTotalNumberOfRooms, m_Lobby.TotalRoomCount() != m_iTotalNumberOfRooms);
	}

	//------------------------------------------------------------------------------------------------
	protected void DisplayRooms(array<Room> rooms = null)
	{
		// Don't display if missing MP privilege
		if (!SocialComponent.IsMultiplayerAllowed())
			return;

		// Check list
		if (!m_ScrollableList || !m_ServerScenarioDetails)
		{
			PrintDebug("Missing important component references!", "DisplayRooms");
			return;
		}

		// Mods filter when UGC not enabled
		SCR_AddonManager addonMgr = SCR_AddonManager.GetInstance();
		if (!addonMgr)
		{
			PrintDebug("Could not find addon mngr to verify ugc privilege", "DisplayRooms");
			return;
		}
		
		if (m_ParamsFilter.IsModdedFilterSelected() && !addonMgr.GetUgcPrivilege())
		{
			Messages_ShowMessage("MISSING_PRIVILEGE_UGC");

			// Negotatiate UGC privilege
			addonMgr.NegotiateUgcPrivilegeAsync();
		
			SwitchFocus(SCR_EListMenuWidgetFocus.LIST);
			if (m_FilterPanel)
				m_FilterPanel.ShowFilterListBox(false);
			
			return;
		}

		int roomCount = m_Lobby.TotalRoomCount();

		// Display no rooms found
		if (roomCount < 1)
		{
			// Setup list
			m_ScrollableList.SetRooms(rooms, 0, true);
			m_ScrollableList.ShowEmptyRooms();

			if (m_ServerScenarioDetails)
				m_ServerScenarioDetails.SetDefaultScenario(TAG_DETAILS_FALLBACK_EMPTY);

			// Show filters
			if (!m_FilterPanel.GetFilterListBoxShown())
				SwitchFocus(SCR_EListMenuWidgetFocus.FILTERING);
			if (m_FilterPanel)
				m_FilterPanel.ShowFilterListBox(true);
			
			// Show message
			Messages_ShowMessage("NO_FILTERED_SERVERS");

			return;
		}

		// Display rooms
		// Set rooms to fill server list
		m_ScrollableList.SetRooms(m_aRooms, m_Lobby.TotalRoomCount(), true);

		// Hide message
		Messages_Hide();
		
		if (m_FilterPanel && !m_FilterPanel.GetFilterListBoxShown())
			SwitchFocus(SCR_EListMenuWidgetFocus.LIST);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this once new room data are fetched
	protected void OnRoomAutoRefresh(BackendCallback callback)
	{
		DisplayRooms(m_aRooms);
		
		m_Dialogs.UpdateServerFullDialog();
	}

	// --- Search Rooms ---
	// First search to get the total number of servers
	//------------------------------------------------------------------------------------------------
	protected void SearchRooms()
	{
		//TODO: query for filtered first, then independently query for total number, in order to reduce load time at start
		
		// Start loading and show loading feedback
		m_SelectedServerEntry = null;
		m_ModsManager.Clear();

		if (!m_bForceUnfilteredRequest)
		{
			SearchRoomsFiltered();
			return;
		}

		// Callback
		ref BackendCallback searchCallback = new BackendCallback;
		
		// Invoker actions
		searchCallback.SetOnSuccess(OnSearchAllRooms);
		searchCallback.SetOnError(OnSearchAllRooms);
		
		m_CallbackLastSearch = searchCallback;
		
		FilteredServerParams params = new FilteredServerParams();
		m_Lobby.SearchRooms(params, searchCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSearchAllRooms()
	{
		m_CallbackLastSearch = null;
		
		// Total number of active servers
		m_iTotalNumberOfRooms = m_Lobby.TotalRoomCount();
		
		SearchRoomsFiltered();
	}
	
	// Searches to get the filtered servers to display	
	//------------------------------------------------------------------------------------------------
	protected void SearchRoomsFiltered()
	{
		if (m_ScrollableList)
			m_Lobby.SetViewSize(m_ScrollableList.GetPageEntriesCount() * 2);
		else
			m_Lobby.SetViewSize(SERVER_LIST_VIEW_SIZE);
		
		// Callback
		ref BackendCallback searchCallback = new BackendCallback;

		// Invoker actions
		searchCallback.SetOnSuccess(OnSearchRoomsSuccess);
		searchCallback.SetOnError(OnSearchRoomsFail);
		
		m_CallbackLastSearch = searchCallback;
		
		m_Lobby.SearchRooms(m_ParamsFilter, searchCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSearchRoomsSuccess(BackendCallback callback)
	{
		PrintDebug(string.Format("Success - servers found: %1", m_Lobby.TotalRoomCount()), "OnSearchRoomsSuccess");

		// Display rooms
		OnRoomsFound();
		m_CallbackLastSearch = null;

		PrintDebug("AFTER - m_RejoinRoom: " + m_RejoinRoom, "OnSearchRoomsSuccess");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSearchRoomsFail(BackendCallback callback)
	{
		// Handle timeout
		if (callback.GetRestResult() == ERestResult.EREST_ERROR_TIMEOUT)
		{
			ShowTimeoutDialog();
			PrintDebug("time out!", "OnSearchRoomsTimeOut");
			return;
		}
		
		PrintDebug(string.Format("Room search fail - code: %1 | restCode: %2 | apiCode: %3", callback.GetHttpCode(), callback.GetRestResult(), callback.GetApiCode()), "OnSearchRoomsFail");
		m_CallbackLastSearch = null;

		// Run again if failed ping
		if (m_bFirstRoomLoad && m_ParamsFilter.GetSortOrder() == m_ParamsFilter.SORT_PING)
		{
			Refresh();
			return;
		}

		Messages_ShowMessage("BACKEND_SERVICE_FAIL");
	}

	//------------------------------------------------------------------------------------------------
	//! Restore filtering parameters in UI
	protected void SetupParams(ClientLobbyApi lobby)
	{
		FilteredServerParams params = FilteredServerParams.Cast(lobby.GetParameters());
		if (!params)
		{
			// Default filter setup
			string strParams = lobby.GetStrParams();
			if (!strParams.IsEmpty())
			{
				m_ParamsFilter.ExpandFromRAW(strParams);
				lobby.ClearParams();
			}
		}
		else
		{
			// Restore previous filter setup
			m_ParamsFilter = params;
		}
		
		// Reset search
		m_ParamsFilter.SetSearch(string.Empty);
		
		SetupFilteringUI(m_ParamsFilter);
	}

	//------------------------------------------------------------------------------------------------
	//! Apply filter setup on each ui filtering widget - restoring filter UI states
	protected void SetupFilteringUI(FilteredServerParams filterParams)
	{
		// Tabview
		if (m_TabView && m_ParamsFilter)
		{
			m_TabView.ShowTab(m_iSelectedTab, true, false);
		}
		
		if (m_FilterPanel)
		{
			bool show = m_FilterPanel.AnyFilterButtonsVisible();
			m_FilterPanel.ShowFilterListBox(show, show);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! General callbacks setup
	protected void SetupCallbacks()
	{
		// Loading
		SCR_MenuLoadingComponent.m_OnMenuOpening.Insert(OnOpeningByLoadComponent);

		// Server list
		m_ScrollableList.GetOnSetPage().Insert(CallOnServerListSetPage);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupRefreshCallback()
	{
		m_CallbackAutoRefresh = new BackendCallback();
		m_CallbackAutoRefresh.SetOnSuccess(OnRoomAutoRefresh);
		m_Lobby.SetRefreshRate(ROOM_REFRESH_RATE);
		m_Lobby.SetRefreshCallback(m_CallbackAutoRefresh);
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearRefreshCallback()
	{
		m_CallbackAutoRefresh = null;
	}

	//------------------------------------------------------------------------------------------------
	// --- ENTRIES ---
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void OnServerEntryFocusEnter(SCR_ScriptedWidgetComponent entry)
	{	
		SCR_ServerBrowserEntryComponent serverEntry = SCR_ServerBrowserEntryComponent.Cast(entry);
		if (!serverEntry || !m_ModsManager)
			return;

		Room room = serverEntry.GetRoomInfo();
		if (!room)
		{
			m_SelectedServerEntry = null;
			return;
		}
		
		serverEntry.m_OnClick.Insert(OnEntryMouseClick);
		
		// Update visuals
		if (m_SelectedServerEntry == serverEntry)
			return;
		
		DisplayFavoriteAction(room.IsFavorite());
		UpdateNavigationButtons();
		
		m_SelectedServerEntry = serverEntry;

		m_ModsManager.Clear();
		m_SelectedServerEntry.SetModsManager(m_ModsManager);

		ReceiveRoomContent(room, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnServerEntryFocusLeave(SCR_ScriptedWidgetComponent entry)
	{
		SCR_ServerBrowserEntryComponent serverEntry = SCR_ServerBrowserEntryComponent.Cast(entry);

		serverEntry.m_OnClick.Remove(OnEntryMouseClick);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntryMouseClick(SCR_ScriptedWidgetComponent button)
	{
		m_ClickedEntry = SCR_ServerBrowserEntryComponent.Cast(button);
	}

	//------------------------------------------------------------------------------------------------
	protected void ReceiveRoomContent(notnull Room room, bool receiveMods)
	{
		GetGame().GetCallqueue().Remove(ReceiveRoomContent_Mods);
		
		if (m_ServerScenarioDetails)
			m_ServerScenarioDetails.DisplayRoomData(room, receiveMods);

		// Allow check only if client is authorized to join server
		if (!room.IsAuthorized())
			return;

		// Check room mods count
		array<Dependency> roomMod = {};
		room.GetItems(roomMod);

		// Show mod count immidiatelly
		if (!roomMod.IsEmpty() && m_ServerScenarioDetails)
			m_ServerScenarioDetails.DisplayModsCount(roomMod.Count());

		// Receive mod data only if currently view is focused
		if (room.IsDownloadListLoaded())
		{
			ReceiveRoomContent_Mods(room);
			return;
		}
		else
		{
			// Load mods after short delay - to prevent spamming mods receive request with fast server selecting
			GetGame().GetCallqueue().CallLater(ReceiveRoomContent_Mods, ROOM_CONTENT_LOAD_DELAY, false, room);
		}		
	}

	//------------------------------------------------------------------------------------------------
	//! Separated receive scenario data for server
	//! Call this later if scenario is from mod to prevent too many request
	protected void ReceiveRoomContent_Scenario(Room room)
	{
		// Load scenario
		m_ModsManager.GetOnGetScenario().Insert(OnLoadingScenario);
		m_ModsManager.ReceiveRoomScenario(room);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Separeted receive mods data for server
	//! Call this later to prevent too many request
	protected void ReceiveRoomContent_Mods(Room room)
	{
		if (!m_ModsManager)
			return;

		m_ModsManager.GetOnGetAllDependencies().Insert(OnLoadingDependencyList);
		m_ModsManager.ReceiveRoomMods(room);
	}
	
	//------------------------------------------------------------------------------------------------
	// --- SETUP ---
	//------------------------------------------------------------------------------------------------
	//! Getting reference for all server widget elements
	protected void SetupHandlers()
	{
		// Tab view
		m_TabView = SCR_TabViewComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_TAB_VIEW, SCR_TabViewComponent));
		if (m_TabView)
		{
			m_TabView.GetOnChanged().Insert(OnTabViewSwitch);
			OnTabViewSwitch(null, null, m_TabView.GetShownTab());

			if (!GetGame().IsPlatformGameConsole())
				m_TabView.AddTab("", "#AR-Workshop_ButtonHost");
		}

		// Filter panel
		m_FilterPanel = SCR_FilterPanelComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_FILTER, SCR_FilterPanelComponent));
		if (m_FilterPanel)
		{
			// Invoker
			m_FilterPanel.GetOnFilterPanelToggled().Insert(OnFilterPanelToggle);
			m_FilterPanel.GetOnFilterChanged().Insert(OnChangeFilter);

			// Attempt load previous filter setup
			m_FilterPanel.TryLoad();
			m_ParamsFilter.SetFilters(m_FilterPanel.GetFilter());
			
			FilterCrossplayCheck();
		}

		// Sorting header
		SCR_SortHeaderComponent sortBar = SCR_SortHeaderComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_SERVER_HEADER, SCR_SortHeaderComponent));
		if (sortBar)
		{
			sortBar.m_OnChanged.Insert(OnChangeSort);

			// Initial sort
			// todo move default sorting values out of here, it can be set in layout file now
			sortBar.SetCurrentSortElement(3, ESortOrder.DESCENDING);
		}

		// Search edit box
		SCR_EditBoxComponent searchEditBox = SCR_EditBoxSearchComponent.Cast(m_Widgets.FindHandlerReference(m_Widgets.m_wSearchEditBox, m_Widgets.WIDGET_SEARCH, SCR_EditBoxSearchComponent));
		if (searchEditBox)
			searchEditBox.m_OnConfirm.Insert(OnSearchEditBoxConfirm);

		// Server list and scroll --------------------
		m_ScrollableList = SCR_PooledServerListComponent.Cast(m_Widgets.FindHandlerReference(
			null, m_Widgets.WIDGET_SCROLLABLE_LIST, SCR_PooledServerListComponent
		));

		// Add callbacks
		array<SCR_ServerBrowserEntryComponent> entries = m_ScrollableList.GetRoomEntries();

		foreach (SCR_ServerBrowserEntryComponent entry : entries)
		{
			// Set invoker actions
			entry.GetOnFocus().Insert(OnServerEntryFocusEnter);
			entry.GetOnFocusLost().Insert(OnServerEntryFocusLeave);
			entry.GetOnFavorite().Insert(OnRoomEntrySetFavorite);
			entry.GetOnMouseInteractionButtonClicked().Insert(OnEntryMouseButton);
		}

		// Detail screen
		m_ServerScenarioDetails = SCR_ServerScenarioDetailsPanelComponent.Cast(m_Widgets.FindHandlerReference(
			null, m_Widgets.WIDGET_SERVER_SCENARIO_DETAILS_PANEL, SCR_ServerScenarioDetailsPanelComponent
		));

		// Bacis setup and hide
		if (m_ServerScenarioDetails)
			m_ServerScenarioDetails.SetModsManager(m_ModsManager);

		// Messages
		m_SimpleMessageWrap = SCR_SimpleMessageComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_MESSAGE_WRAP, SCR_SimpleMessageComponent));
		m_SimpleMessageList = SCR_SimpleMessageComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_MESSAGE_LIST, SCR_SimpleMessageComponent));
		if (m_SimpleMessageList)
			m_SimpleMessageList.SetVisible(false);
		
		
		// Navigation buttons
		m_Widgets.m_JoinButton.m_OnActivated.Insert(OnJoinButton);
		m_Widgets.m_DetailsButton.m_OnActivated.Insert(OnActionDetails);
		m_Widgets.m_FavoritesButton.m_OnActivated.Insert(OnActionFavorite);		
	}
	
	//------------------------------------------------------------------------------------------------
	//!	Init behavior when filter panel script is attached
	//! If crossplay is enabled we show all servers so both filters are by default disabled
	//! If crossplay is disabled we show only servers with crossplay disabled by force
	protected void FilterCrossplayCheck()
	{
		if (!m_FilterPanel)
			return;
		
		SCR_FilterSet filterSet = m_FilterPanel.GetFilter();
		if (!filterSet)
			return;
		
		SCR_FilterCategory crossPlay = filterSet.FindFilterCategory("Crossplay");
		if (!crossPlay)
			return;
		
		// Cross play privilege missing
		bool isCrossEnabled = GetGame().IsCrossPlayEnabled();
		
		if (crossPlay.m_wCategoryTitleWidget)
			crossPlay.m_wCategoryTitleWidget.SetVisible(isCrossEnabled);
				
		SCR_FilterEntry enabled = crossPlay.FindFilter("CrossplayEnabled");
		if (enabled)
		{
			//This sets an actual filter value for filtering servers
			enabled.SetSelected(false);
			
			if (enabled.m_FilterComponent)
			{
				enabled.m_FilterComponent.SetVisible(isCrossEnabled);
				enabled.m_FilterComponent.SetEnabled(isCrossEnabled);
			}
			
			//This sets only visualisation of widget
			m_FilterPanel.SelectFilter(enabled, false, false);
		}

		SCR_FilterEntry disabled = crossPlay.FindFilter("CrossPlayDisabled");
		if (disabled)
		{
			//This sets an actual filter value for filtering servers
			disabled.SetSelected(!isCrossEnabled);
			
			if (disabled.m_FilterComponent)
			{
				disabled.m_FilterComponent.SetVisible(isCrossEnabled);
				disabled.m_FilterComponent.SetEnabled(isCrossEnabled);
			} 
			
			//This sets only visualisation of widget
			m_FilterPanel.SelectFilter(disabled, !isCrossEnabled, false);
		}
		
		Refresh();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Switch focus
	protected void SwitchFocus(SCR_EListMenuWidgetFocus focus, bool force = false)
	{
		if (m_eFocusedWidgetState == focus && !force)
			return; 

		Widget focusTarget;

		switch(focus)
		{
			case SCR_EListMenuWidgetFocus.LIST:
			{
				if (m_SelectedServerEntry && m_SelectedServerEntry.IsVisible())
					focusTarget = m_SelectedServerEntry.GetRootWidget();
				else if (m_ScrollableList)
					focusTarget = m_ScrollableList.FirstAvailableEntry();

				break;
			}
		
			case SCR_EListMenuWidgetFocus.FILTERING:
			{
				focusTarget = m_FilterPanel.GetWidgets().m_FilterButton;
				break;
			}
			
			case SCR_EListMenuWidgetFocus.SORTING:
			{
				focusTarget = m_Widgets.m_wSortSessionFavorite;
				break;
			}
		}

		m_eFocusedWidgetState = focus;
		
		if (!focusTarget || !focusTarget.IsVisible())
		{
			// Fallback
			focusTarget = m_Widgets.m_wSortSessionFavorite;
			m_eFocusedWidgetState = SCR_EListMenuWidgetFocus.SORTING;
		}

		GetGame().GetWorkspace().SetFocusedWidget(focusTarget);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find filter by it's internal name define in ServerBrowserFilterSet.conf
	//! Set filter enabled
	void ActivateFilter(string filterName, bool enabled, bool processFilters = true)
	{
		// Setup filter
		SCR_FilterEntry filter = new SCR_FilterEntry;
		filter.m_sInternalName = filterName;
		filter.SetSelected(true);

		// Register filter
		m_aFiltersToSelect.Insert(filter);

		// Activate filter if possible
		if (processFilters)
			ActivateFiltersFromList();
	}

	//------------------------------------------------------------------------------------------------
	protected void ActivateFiltersFromList()
	{
		// Filters array check
		if (!m_aFiltersToSelect || m_aFiltersToSelect.IsEmpty())
			return;

		// Filter panel check
		if (!m_FilterPanel || !m_ParamsFilter)
			return;

		// Get filter set
		ref SCR_FilterSet filterSet = m_FilterPanel.GetFilter();
		if (!filterSet)
			return;

		// Check and enable each filter
		foreach (SCR_FilterEntry filter : m_aFiltersToSelect)
		{
			if (!filter)
				continue;

			// Find filter to select
			SCR_FilterEntry targetFilter = filterSet.FindFilter(filter.m_sInternalName);

			// Set select if is found
			if (targetFilter && targetFilter.GetSelected() != filter.GetSelected())
			{
				targetFilter.SetSelected(filter.GetSelected());
				m_FilterPanel.SelectFilter(targetFilter, filter.GetSelected());
			}
		}

		// Clear filters
		m_aFiltersToSelect.Clear();

		// Filter
		m_ParamsFilter.SetFilters(filterSet);
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	//! Separated focus function for later call
	protected void FocusWidget(Widget w)
	{
		GetGame().GetWorkspace().SetFocusedWidget(w);
	}

	//------------------------------------------------------------------------------------------------
	protected void CallOnServerListSetPage(int page)
	{
		GetGame().GetCallqueue().Remove(OnServerListSetPage);
		GetGame().GetCallqueue().CallLater(OnServerListSetPage, ROOM_CONTENT_LOAD_DELAY, false, page);
	}
	
	//------------------------------------------------------------------------------------------------
	// --- MESSAGES ---
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Show message feedback by selected message component
	//! showWrap = true will hide server list and display message on full layout
	protected void Messages_ShowMessage(string messageTag, bool showWrap = false)
	{
		// Check message component references
		if (!m_Widgets.m_wPanelEmpty || !m_SimpleMessageWrap || !m_SimpleMessageList)
		{
			string msg = string.Format("Missing ref - WrapLayout: %1 Wrap: %2, List: %3", m_Widgets.m_wPanelEmpty, m_SimpleMessageWrap, m_SimpleMessageList);
			PrintDebug(msg, "Messages_ShowMessage");
			return;
		}

		// Show menu content
		if (m_Widgets.m_wContent)
			m_Widgets.m_wContent.SetVisible(!showWrap);
		
		m_Widgets.m_wPanelEmpty.SetVisible(showWrap);
		
		// Fill the messages with content
		m_SimpleMessageWrap.SetContentFromPreset(messageTag);
		m_SimpleMessageList.SetContentFromPreset(messageTag);
		
		// Display messages
		m_SimpleMessageWrap.SetVisible(showWrap);
		m_SimpleMessageList.SetVisible(!showWrap);
		
		// Hide footer buttons during full layout mode
		if (m_Widgets.m_RefreshButton)
			m_Widgets.m_RefreshButton.SetVisible(!showWrap, false);
		
		if (m_Widgets.m_DirectJoinButton)
			m_Widgets.m_DirectJoinButton.SetVisible(!showWrap, false);
		
		if (m_Widgets.m_FilterButton)
			m_Widgets.m_FilterButton.SetVisible(!showWrap, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Hide both message widgets
	protected void Messages_Hide()
	{
		Widget wrapLayout = m_Widgets.m_wPanelEmpty;

		// Check message component references
		if (!wrapLayout || !m_SimpleMessageWrap || !m_SimpleMessageList)
		{
			string msg = string.Format("Missing ref - WrapLayout: %1 Wrap: %2, List: %3", wrapLayout, m_SimpleMessageWrap, m_SimpleMessageList);
			PrintDebug(msg, "Messages_Hide");
			return;
		}

		// Hide
		m_SimpleMessageWrap.SetVisible(false);
		m_SimpleMessageList.SetVisible(false);

		wrapLayout.SetVisible(false);

		if (m_Widgets.m_wContent)
			m_Widgets.m_wContent.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	// --- CALLBACKS ---
	//------------------------------------------------------------------------------------------------
	//! Call this when server browser is open by loading component
	//! Start waiting for backend
	protected void OnOpeningByLoadComponent(int menuPreset)
	{
		// Check menu opening
		if (menuPreset == ChimeraMenuPreset.ServerBrowserMenu)
			return;
	}

	//------------------------------------------------------------------------------------------------
	//! Set focus to last filtering element
	protected void OnChangeSort(SCR_SortHeaderComponent sortHeader)
	{
		if (!m_ParamsFilter)
			return;

		// Reload data
		bool sortAscending = sortHeader.GetSortOrderAscending();
		string sortElementName = sortHeader.GetSortElementName();
		m_ParamsFilter.SetSorting(sortElementName, sortAscending);

		Refresh();

		SCR_AnalyticsApplication.GetInstance().MultiplayerMenuSetSorting(sortElementName);
	}

	//------------------------------------------------------------------------------------------------
	//! Set tab filters on switching tab view
	protected void OnTabViewSwitch(SCR_TabViewComponent tabView, Widget w, int id)
	{
		// Default setup
		m_ParamsFilter.SetRecentlyPlayedFilter(false);
		m_Widgets.m_wHostNewServerButton.SetVisible(false);

		// with id == SCR_EServerBrowserTabs.ALL, nothing is forced
		bool onlyFavorite = id == SCR_EServerBrowserTabs.FAVORITES;
		bool onlyOfficial = id == SCR_EServerBrowserTabs.OFFICIAL;
		bool onlyCommunity = id == SCR_EServerBrowserTabs.COMMUNITY;
		bool onlyRecent = id == SCR_EServerBrowserTabs.RECENT;
		bool onlyOwned = id == SCR_EServerBrowserTabs.HOST;

		// limit to Official or Community
		m_ParamsFilter.SetOfficialFilter(onlyOfficial);
		m_ParamsFilter.SetCommunityFilter(onlyCommunity);

		// limit to Favorite
		m_ParamsFilter.SetFavoriteFilter(onlyFavorite);

		// limit to recent
		m_ParamsFilter.SetRecentlyPlayedFilter(onlyRecent);

		// limit to owned (hosting widgets)
		m_ParamsFilter.SetOwnedOnly(onlyOwned);
		m_Widgets.m_wHostNewServerButton.SetVisible(onlyOwned);
		m_Widgets.m_wHostNewServerButton.SetEnabled(onlyOwned);
		if( onlyOwned )
		{
			// Call focus later to prevent override from auto widget focus
			GetGame().GetCallqueue().CallLater(FocusWidget, 0, false, m_Widgets.m_wHostNewServerButton);
		}		
		m_bHostedServers = onlyOwned;

		// Set tab filter json
		m_iSelectedTab = id;

		Refresh();
		
		Widget focus = GetGame().GetWorkspace().GetFocusedWidget();
		if (!m_FilterPanel || !m_FilterPanel.GetFilterListBoxShown() || !focus || !focus.IsVisible())
			SwitchFocus(SCR_EListMenuWidgetFocus.LIST, true);
		
		SCR_AnalyticsApplication.GetInstance().MultiplayerMenuSetTab(id);		
	}
	
	// --- Filters ---
	//------------------------------------------------------------------------------------------------
	protected void OnFilterPanelToggle(bool show)
	{
		if (!m_FilterPanel)
			return;
		
		if (show || m_Lobby.TotalRoomCount() == 0)
			SwitchFocus(SCR_EListMenuWidgetFocus.FILTERING); // Focus back to server list
		else
			SwitchFocus(SCR_EListMenuWidgetFocus.LIST); // Focus back to last entry

		SCR_AnalyticsApplication.GetInstance().MultiplayerMenuUseFilterOn();
	}

	//------------------------------------------------------------------------------------------------
	//! Call this when any of filter in filter panel is changed
	//! Get last updated filter and set it up for next search
	protected void OnChangeFilter(SCR_FilterEntry filter)
	{
		// Set filter and refresh
		m_ParamsFilter.SetFilters(m_FilterPanel.GetFilter());
		Refresh();

		// Is crossplay filter
		if (!GetGame().IsCrossPlayEnabled() && filter.GetCategory().m_sInternalName == "Crossplay")
		{
			FilterCrossplayCheck();
			Refresh();
		}

		SCR_AnalyticsApplication.GetInstance().MultiplayerMenuSetFilter(filter.GetCategory().m_sInternalName, filter.m_sInternalName);
	}
	
	// --- Scrolling ---
	//------------------------------------------------------------------------------------------------
	//! Call this when scroll returns error server response
	protected void OnScrollError(BackendCallback callback)
	{
		if (callback.GetRestResult() == ERestResult.EREST_ERROR_TIMEOUT)
		{
			PrintDebug("Scroll timeout!", "OnScrollResponse");
			return;
		}
		PrintDebug("Scroll error!", "OnScrollResponse");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnScrollSuccess(BackendCallback callback)
	{
		// Check if loaded room should display details
		bool loadedRoomFocused = false;
		if (m_SelectedServerEntry && !m_ScrollableList.IsRoomLoaded(m_SelectedServerEntry))
			loadedRoomFocused = true;

		// Update rooms
		m_Lobby.Rooms(m_aRooms);

		if (m_aRooms.IsEmpty())
		{
			PrintDebug("No room found", "OnScrollSuccess");

			if (m_Lobby.TotalRoomCount() > 0)
			{
				if (m_ScrollableList)
					CallOnServerListSetPage(m_ScrollableList.GetCurrentPage());
				else
					Refresh();
			}
		}

		m_ScrollableList.UpdateLoadedPage();

		DisplayRooms(m_aRooms);

		// Focus on new room
		if (loadedRoomFocused)
		{
			Room room = m_SelectedServerEntry.GetRoomInfo();
			if (room)
				ReceiveRoomContent(room, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this actions when server list page is changed
	protected void OnServerListSetPage(int page)
	{
		// Check list
		if (!m_ScrollableList)
			return;

		// Get entries count
		int entriesC = m_ScrollableList.GetPageEntriesCount();
		int pos = entriesC * page;

		// Setup scroll callback
		m_CallbackScroll.SetOnSuccess(OnScrollSuccess);
		m_CallbackScroll.SetOnError(OnScrollError);

		array<Room> rooms = {};
		m_Lobby.Rooms(rooms);
		m_Lobby.Scroll(pos, m_CallbackScroll);
	}
	
	// --- Search ---
	//------------------------------------------------------------------------------------------------
	//! Fetch new servers on confirming search string
	protected void OnSearchEditBoxConfirm(SCR_EditBoxComponent editBox, string sInput)
	{
		m_ParamsFilter.SetSearch(sInput);
		Refresh();
		
		SCR_AnalyticsApplication.GetInstance().MultiplayerMenuUseSearch();
	}

	// --- Favoriting ---
	//------------------------------------------------------------------------------------------------
	//! Favoriting server
	protected void OnRoomEntrySetFavorite(SCR_ListMenuEntryComponent entry, bool favorite)
	{
		SCR_ServerBrowserEntryComponent serverEntry = SCR_ServerBrowserEntryComponent.Cast(entry);
		if(!serverEntry)
			return;
			
		Room roomClicked = serverEntry.GetRoomInfo();
		if (!roomClicked)
			return;

		int roomId = m_aRooms.Find(roomClicked);
		if (roomId < 0)
			return;
			
		roomClicked.SetFavorite(favorite, m_CallbackFavorite);

		// Setup callback
		m_CallbackFavorite.SetRoom(roomClicked);
		m_CallbackFavorite.SetOnSuccess(OnRoomSetFavoriteResponse);
		m_CallbackFavorite.SetOnError(OnRoomSetFavoriteError);

		SCR_AnalyticsApplication.GetInstance().MultiplayerMenuUseFavorite();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRoomSetFavoriteResponse(SCR_RoomCallback callback)
	{
		if (callback.GetRoom())
		{
			DisplayFavoriteAction(callback.GetRoom().IsFavorite());
			m_OnFavoritesResponse.Invoke();
		}

		// Clear
		m_CallbackFavorite.SetRoom(null);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRoomSetFavoriteError(SCR_RoomCallback callback)
	{
		if (callback.GetRestResult() == ERestResult.EREST_ERROR_TIMEOUT)
		{
			PrintDebug("Timeout!", "OnRoomSetFavoriteResponse");
		}
		else
		{
			PrintDebug("Error!", "OnRoomSetFavoriteResponse");
		}
		
		// Clear
		m_CallbackFavorite.SetRoom(null);
	}

	//------------------------------------------------------------------------------------------------
	//! Based on given boolean favorite nav button is displaying eather add or remove favorite
	protected void DisplayFavoriteAction(bool isFavorite)
	{
		if (m_Widgets && m_Widgets.m_FavoritesButton && m_SelectedServerEntry)
			m_Widgets.m_FavoritesButton.SetLabel(UIConstants.GetFavoriteLabel(isFavorite));
	}

	// --- Reconnect ---
	//------------------------------------------------------------------------------------------------
	//! Confirm action to find new server
	protected void OnLastRoomReconnectConfirm()
	{
		SCR_KickDialogs.GetOnReconnect().Remove(OnLastRoomReconnectConfirm);
		SCR_KickDialogs.GetOnCancel().Remove(OnRejoinCancel);

		// Find
		string lastId = m_Lobby.GetPreviousRoomId();
		JoinProcess_FindRoomById(lastId, m_CallbackSearchPreviousRoom);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setting server details
	//! Call this when room receive scenario data
	protected void OnLoadingScenario(Room room)
	{
		// Remove action
		m_ModsManager.GetOnGetScenario().Remove(OnLoadingScenario);

		if (!room)
			return;
		
		// Get scenario item
		MissionWorkshopItem scenarioItem = room.HostScenario();

		// Hide scneario img if scenario if modded client can't see UGC
		Dependency scenarioMod = room.HostScenarioMod();
		bool hideScenario = scenarioMod && !SCR_AddonManager.GetInstance().GetUgcPrivilege();

		// Display room and scenario data
		if (m_ServerScenarioDetails)
		{
			// Hide scenario image
			m_ServerScenarioDetails.SetHideScenarioImg(hideScenario);

			if (scenarioItem)
				m_ServerScenarioDetails.SetScenario(scenarioItem);
			else
				m_ServerScenarioDetails.DisplayDefaultScenarioImage();
		}

		//! Update Details Dialog
		if (m_Dialogs)
		{
			m_Dialogs.UpdateRoomDetailsScenarioImage(scenarioItem);
			m_Dialogs.UpdateServerFullScenarioImage(scenarioItem);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!  Call this on rejoin dialog cancel
	protected void OnRejoinCancel()
	{
		SCR_KickDialogs.GetOnReconnect().Remove(OnLastRoomReconnectConfirm);
		SCR_KickDialogs.GetOnCancel().Remove(OnRejoinCancel);
	}
	
	//------------------------------------------------------------------------------------------------
	// --- HELPERS ---
	//------------------------------------------------------------------------------------------------
	//! Check if is possible to attempt to join room
	//! Returns true if room version and platform match to client
	protected bool CanJoinRoom(Room room)
	{
		if (!room)
		{
			PrintDebug("No room found for can join room check", "CanJoinRoom");
			return false;
		}

		// Room to client version check
		bool versionMatch = ClientRoomVersionMatch(room);

		//TODO: check platform restriction

		return versionMatch && !m_ModsManager.HasBlockedMods() && room.Joinable();
	}

	//------------------------------------------------------------------------------------------------
	protected bool ClientRoomVersionMatch(Room room)
	{
		if (!room)
			return false;

		string clientV = GetGame().GetBuildVersion();
		string roomV = room.GameVersion();


		return (clientV == roomV);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ServerBrowserEntryComponent GetEntryUnderCursor()
	{
		Widget w = WidgetManager.GetWidgetUnderCursor();

		if (!w)
			return null;

		return SCR_ServerBrowserEntryComponent.Cast(w.FindHandler(SCR_ServerBrowserEntryComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ServerBrowserEntryComponent GetSelectedEntry()
	{
		// We are not over a line, use currently focused line
		Widget wfocused = GetGame().GetWorkspace().GetFocusedWidget();
		SCR_ServerBrowserEntryComponent comp;
		if (wfocused)
			comp = SCR_ServerBrowserEntryComponent.Cast(wfocused.FindHandler(SCR_ServerBrowserEntryComponent));

		EInputDeviceType inputDevice = GetGame().GetInputManager().GetLastUsedInputDevice();
		bool isCursorOnInnerButton = m_SelectedServerEntry && m_SelectedServerEntry.IsInnerButtonInteraction();

		if (inputDevice == EInputDeviceType.MOUSE && (GetEntryUnderCursor() || isCursorOnInnerButton))
			return m_SelectedServerEntry;

		return comp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearScenarioFilters()
	{
		if (!m_ParamsFilter)
			return;
		
		m_ParamsFilter.SetScenarioId("");
		m_ParamsFilter.SetHostedScenarioModId("");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetMenuHeader(string header)
	{
		SCR_CoreMenuHeaderComponent menuHeader = SCR_CoreMenuHeaderComponent.FindComponentInHierarchy(GetRootWidget());
		if (menuHeader)
			menuHeader.SetTitle(header);
	}
	
	//------------------------------------------------------------------------------------------------
	// --- API ---
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	void FilterScenarioId(string scenarioId)
	{
		if (m_ParamsFilter)
			m_ParamsFilter.SetScenarioId(scenarioId);
	}

	//------------------------------------------------------------------------------------------------
	void FilterHostedScenarioModId(string scenarioModId)
	{
		if (m_ParamsFilter)
			m_ParamsFilter.SetHostedScenarioModId(scenarioModId);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFilteredScenario(MissionWorkshopItem scenario)
	{
		if (m_Widgets && m_Widgets.m_HostButton)
			m_Widgets.m_HostButton.SetScenario(scenario);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set to find server by mission workshop item
	static void OpenWithScenarioFilter(MissionWorkshopItem mission)
	{
		ServerBrowserMenuUI sb = ServerBrowserMenuUI.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu));
		if (!sb)
			return;

		// Scenario id
		string scenarioId = mission.Id();
		sb.FilterScenarioId(scenarioId);

		// Modded
		string modId = "";
		WorkshopItem owner = mission.GetOwner();
		if (owner)
			modId = owner.Id();

		if (modId != string.Empty)
			sb.FilterHostedScenarioModId(modId);

		sb.SetMenuHeader(mission.Name());
		sb.SetFilteredScenario(mission);
		//SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ServerBrowserMenu);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsServerPingAboveThreshold(Room room)
	{
		//TODO: unify this check with the value in the filters
		return room && room.GetPing() >= HIGH_PING_SERVER_THRESHOLD;
	}
	
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	// <>-----<> ROOM JOINING PROCESS <>-----<>
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	
	// --- TRIGGERS ---
	// --- Entry interaction ---
	//------------------------------------------------------------------------------------------------
	//! Action for joining to selected server
	protected void JoinActions_Join()
	{
		// Prevent join if no entry selected
		if (!m_SelectedServerEntry)
		{
			PrintDebug("Quick join is not possible because there is no selected server", "JoinActions_Join");
			return;
		}

		// Join
		JoinProcess_Init(m_SelectedServerEntry.GetRoomInfo());

		GameSessionStorage.s_Data["m_iRejoinAttempt"] = "0";
	}

	// --- Manual Connect ---
	//------------------------------------------------------------------------------------------------
	// Action for finding server on direct join
	protected void JoinActions_DirectJoin(string params, EDirectJoinFormats format, bool publicNetwork)
	{
		JoinProcess_FindRoom(params, format, publicNetwork);
		GameSessionStorage.s_Data["m_iRejoinAttempt"] = "0";
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize joining process to specific room
	protected void JoinProcess_FindRoom(string params, EDirectJoinFormats format, bool publicNetwork)
	{
		m_DirectJoinParams = new FilteredServerParams();

		// Format
		switch (format)
		{
			// IP:PORT
			case EDirectJoinFormats.IP_PORT:
			{
				m_DirectJoinParams.SetHostAddress(params);
				break;
			}

			// Direct join code
			case EDirectJoinFormats.JOIN_CODE:
			{
				m_DirectJoinParams.SetJoinCode(params);
				break;
			}

			// Invalid format
			case EDirectJoinFormats.INVALID:
			{
				PrintDebug("Invalid format of direct join", "JoinProcess_FindRoom");
				return;
			}
		}

		// Set room search
		m_DirectJoinParams.SetUsePlayerLimit(false);
		m_Lobby.SearchTarget(m_DirectJoinParams, m_CallbackSearchTarget);

		// Setup join dialog
		SetupJoinDialogs();
		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.SEARCHING_SERVER);

		// Set invokers
		m_CallbackSearchTarget.SetOnSuccess(JoinProcess_OnFindRoomSuccess);
		m_CallbackSearchTarget.SetOnError(JoinProcess_OnFindRoomFail);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this when room is found
	protected void JoinProcess_OnFindRoomSuccess()
	{
		GetGame().GetBackendApi().GetClientLobby().Target(m_aDirectFoundRooms);
		if (m_aDirectFoundRooms.IsEmpty())
		{
			PrintDebug("No room found through direct join!", "JoinProcess_OnFindRoomSuccess");
			JoinProcess_OnFindRoomFail();
			return;
		}

		if (m_aDirectFoundRooms.Count() == 1)
		{
			// Check content of specific server
			m_ModsManager.Clear();

			JoinProcess_Init(m_aDirectFoundRooms[0]);
		}
		else
		{
			// Show multiple rooms in list - fallback logic, shouldn't happend in relased game
			OnRoomsFound(m_aDirectFoundRooms);
		}

		// Clear direct join callback
		JoinProcess_CleanFindCallback();
	}

	//------------------------------------------------------------------------------------------------
	//! Call this if room wasn't found because of error or time out
	protected void JoinProcess_OnFindRoomFail()
	{	
		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.SERVER_NOT_FOUND);

		JoinProcess_CleanFindCallback();
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_CleanFindCallback()
	{
		m_CallbackSearchTarget.SetOnSuccess(null);
		m_CallbackSearchTarget.SetOnError(null);
	}

	// --- Reconnect ---
	//------------------------------------------------------------------------------------------------
	//! Specific call for id search
	protected void JoinProcess_FindRoomById(string id, BackendCallback callback)
	{
		// Setup ID
		m_SearchIds.ClearIds();
		m_SearchIds.RegisterId(id);

		// Search
		m_Lobby.GetRoomsByIds(m_SearchIds, callback);
		callback.SetOnSuccess(JoinProcess_OnFindRoomByIdResponse);
		callback.SetOnError(JoinProcess_OnFindRoomFail);

		// Setup join dialog
		SetupJoinDialogs();
		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.SEARCHING_SERVER);
	}

	//------------------------------------------------------------------------------------------------
	//! Reaction for rooms found by id response success
	protected void JoinProcess_OnFindRoomByIdResponse(BackendCallback callback)
	{
		m_Lobby.Rooms(m_aDirectFoundRooms);

		// No rooms received
		if (m_aDirectFoundRooms.IsEmpty())
		{
			JoinProcess_OnFindRoomFail();
			return;
		}

		// Get first room
		m_RejoinRoom = m_aDirectFoundRooms[0];
		JoinProcess_Init(m_RejoinRoom);
	}

	// --- Process ---
	//------------------------------------------------------------------------------------------------
	//! Initialize joining process to specific room
	void JoinProcess_Init(Room roomToJoin)
	{
		if (m_JoinProcessTargetRoom)
			return;
		
		// Cleanup just to be sure
		JoinProcess_Clear();
		
		// Setup room
		m_JoinProcessTargetRoom = roomToJoin;
		
		// Setup join dialog
		SetupJoinDialogs();

		//this case shouldnt happen as filter cannot be changed when crossplay is disabled
		if (m_JoinProcessTargetRoom.IsCrossPlatform() && !GetGame().IsCrossPlayEnabled())
		{
			//i dont think we have different dialog?
			//m_Dialogs.DisplayDialog(EJoinDialogState.MOD_UGC_PRIVILEGE_MISSING);
			return;
		}

		//Quick Join: Next step check version
		if (m_bQuickJoin)
			JoinProcess_CheckVersion();
		//Single click Join: Next step check password
		else
			JoinProcess_CheckRoomPasswordProtected();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if client and room versions match
	//! Progress only if versions are matching, otherwise show error and stop progress
	protected void JoinProcess_CheckVersion()
	{
		// Chekc match
		bool versionsMatch = ClientRoomVersionMatch(m_JoinProcessTargetRoom);

#ifdef SB_DEBUG
#else
		// Stop join process with error dialog with wrong version
		if (!versionsMatch)
		{
			m_Dialogs.SetJoinRoom(m_JoinProcessTargetRoom);
			m_Dialogs.DisplayDialog(EJoinDialogState.VERSION_MISMATCH);
			return;
		}
#endif

		// Check room password protection
		JoinProcess_CheckHighPing();
	}

	//------------------------------------------------------------------------------------------------
	//! Display a warning dialog if the player chose to join a server with high ping
	protected void JoinProcess_CheckHighPing()
	{
		if (IsServerPingAboveThreshold(m_JoinProcessTargetRoom))
		{
			m_Dialogs.DisplayDialog(EJoinDialogState.HIGH_PING_SERVER);
			m_Dialogs.GetOnConfirm().Insert(OnHighPingServerWarningDialogConfirm);
			
			return;
		}
		
		JoinProcess_CheckRoomPasswordProtected();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnHighPingServerWarningDialogConfirm()
	{
		m_Dialogs.GetOnConfirm().Remove(OnHighPingServerWarningDialogConfirm);
		JoinProcess_CheckRoomPasswordProtected();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if room requires password to join
	protected void JoinProcess_CheckRoomPasswordProtected()
	{
		// Skip password if using direct join code or is invited
		bool skipPassword;

		if (m_Lobby.GetInviteRoom() == m_JoinProcessTargetRoom || m_RejoinRoom)
			skipPassword = m_JoinProcessTargetRoom.IsAuthorized();

		// Next step if no password protection
		if (!m_JoinProcessTargetRoom.PasswordProtected() || skipPassword)
		{
			JoinProcess_CheckBlockedPlayers();
			return;
		}
		
		m_PasswordVerification.GetOnVerified().Insert(OnPasswordVerified);
		m_PasswordVerification.GetOnFailVerification().Clear();
		m_PasswordVerification.GetOnFailVerification().Insert(OnRejoinAuthorizationFailed);
		m_PasswordVerification.CheckRejoinAuthorization(m_JoinProcessTargetRoom);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRejoinAuthorizationFailed(string message)
	{
		m_PasswordVerification.GetOnFailVerification().Remove(OnRejoinAuthorizationFailed);
		
		m_Dialogs.CloseCurrentDialog();
		JoinProcess_PasswordClearInvokers();
		GetGame().GetCallqueue().CallLater(JoinProcess_PasswordDialogOpen);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_PasswordDialogOpen()
	{
		m_Dialogs.DisplayDialog(EJoinDialogState.PASSWORD_REQUIRED);
		m_Dialogs.GetOnCancel().Insert(JoinProcess_PasswordClearInvokers);

		m_PasswordVerification.SetupDialog(m_Dialogs.GetCurrentDialog(), m_JoinProcessTargetRoom);
		m_PasswordVerification.GetOnVerified().Insert(OnPasswordVerified);
		m_PasswordVerification.GetOnFailVerification().Insert(OnPasswordFailVerification);
	}

	//------------------------------------------------------------------------------------------------
	//! On successfull room password verification continue to cotent handling
	protected void OnPasswordVerified()
	{
		JoinProcess_PasswordClearInvokers();

		if (!m_JoinProcessTargetRoom)
		{
			#ifdef WORKBENCH
				Print("ServerBrowserMenuUI - OnPasswordVerified() - NULL ROOM");
			#endif
			
			return;
		}
		
		JoinProcess_CheckBlockedPlayers();
		
		ReceiveRoomContent(m_JoinProcessTargetRoom, true);
	}

	//------------------------------------------------------------------------------------------------
	//! On room password verication fail restart attemp
	protected void OnPasswordFailVerification(string message)
	{
		m_Dialogs.DisplayDialog(EJoinDialogState.PASSWORD_REQUIRED);
		m_PasswordVerification.SetupDialog(m_Dialogs.GetCurrentDialog(), m_JoinProcessTargetRoom, message);
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_PasswordClearInvokers()
	{
		m_PasswordVerification.GetOnVerified().Remove(OnPasswordVerified);
		m_PasswordVerification.GetOnFailVerification().Remove(OnPasswordFailVerification);
		m_Dialogs.GetOnCancel().Remove(JoinProcess_PasswordClearInvokers);
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_CheckBlockedPlayers()
	{
		GameBlocklist bl = GetGame().GetGameBlocklist();
		bl.OnGetBlockedPlayersInRoomInvoker.Insert(JoinProcess_OnCheckedBlockedPlayersInRoom);
		bl.CheckBlockedPlayersInRoom(m_JoinProcessTargetRoom);
	}
	
	//------------------------------------------------------------------------------------------------
	// The GameBlocklist.s_OnCheckedBlockedPlayersInRoomInvoker might return rooms different than the join target, 
	// as the GameBlocklist can process multiple request at a time, and thus return multiple of these invokers.
	protected void JoinProcess_OnCheckedBlockedPlayersInRoom(Room checkedRoom, array<BlockedRoomPlayer> blockedPlayers)
	{
		if (checkedRoom != m_JoinProcessTargetRoom)
			return;
		
		GetGame().GetGameBlocklist().OnGetBlockedPlayersInRoomInvoker.Remove(JoinProcess_OnCheckedBlockedPlayersInRoom);
		
		if (!blockedPlayers || blockedPlayers.IsEmpty())
		{
			JoinProcess_ShowJoinDetailsDialog();
			return;
		}
		
		m_Dialogs.CreateBlockedPlayersWarningDialog(blockedPlayers);
		m_Dialogs.GetOnConfirm().Insert(JoinProcess_ShowJoinDetailsDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_ShowJoinDetailsDialog()
	{
		m_Dialogs.GetOnConfirm().Remove(JoinProcess_ShowJoinDetailsDialog);
		
		// Skip the dialog on double click
		if (m_bQuickJoin)
		{
			JoinProcess_LoadModContent();
			return;
		}

		CreateServerDetailsDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateServerDetailsDialog()
	{
		// Create details dialog
		array<ref SCR_WorkshopItem> items = {};
		array<Dependency> dependencies = {};
		m_JoinProcessTargetRoom.GetItems(dependencies);

		if (m_ModsManager.GetRoomItemsScripted().Count() == dependencies.Count())
			items = m_ModsManager.GetRoomItemsScripted();

		SCR_ServerDetailsDialog serverDetails = m_Dialogs.CreateServerDetailsDialog(m_JoinProcessTargetRoom, items, m_OnFavoritesResponse);
		serverDetails.SetCanJoin(CanJoinRoom(m_JoinProcessTargetRoom));
		serverDetails.m_OnFavorites.Insert(OnActionFavorite);

		bool loaded = m_JoinProcessTargetRoom.IsDownloadListLoaded();

		if (!dependencies.IsEmpty())
		{
			//Fill with last loaded mod list
			if (!m_JoinProcessTargetRoom.IsDownloadListLoaded())
				m_ModsManager.GetOnGetAllDependencies().Insert(OnServerDetailModsLoaded);
			else
				OnServerDetailModsLoaded(m_JoinProcessTargetRoom);
		}
		else
		{
			// Fill with emtpy data
			m_Dialogs.FillRoomDetailsMods({});
		}

		m_Dialogs.GetCurrentDialog().m_OnConfirm.Insert(JoinProcess_LoadModContent);
		m_Dialogs.GetCurrentDialog().m_OnClose.Insert(OnServerDetailsClosed);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this to fill details mods list
	protected void OnServerDetailModsLoaded(Room room)
	{
		m_Dialogs.FillRoomDetailsMods(m_ModsManager.GetRoomItemsScripted(), m_ModsManager);
		m_ModsManager.GetOnGetAllDependencies().Remove(OnServerDetailModsLoaded);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnServerDetailsClosed(SCR_ConfigurableDialogUi dialog)
	{
		m_ModsManager.GetOnGetAllDependencies().Remove(OnServerDetailModsLoaded);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Load data about room mods to check which data is client missing
	//! Show state if mods are already loaded or wait for receiving mods data
	protected void JoinProcess_LoadModContent()
	{
		// Check mods use privilege - UGC privilege
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		array<Dependency> deps = {};

		m_JoinProcessTargetRoom.GetItems(deps);

		if (!mgr.GetUgcPrivilege() && !deps.IsEmpty())
		{
			m_Dialogs.DisplayDialog(EJoinDialogState.MOD_UGC_PRIVILEGE_MISSING);

			// Negotatiate UGC privilege
			mgr.m_OnUgcPrivilegeResult.Insert(Platform_OnUgcPrivilegeResult);
			mgr.NegotiateUgcPrivilegeAsync();
			return;
		}

		// Show state of mods if all loaded
		JoinProcess_LoadModContentVisualize();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when mods list is received
	protected void OnLoadingDependencyList(Room room)
	{
		// Remove data receiving actions
		m_ModsManager.GetOnGetAllDependencies().Remove(OnLoadingDependencyList);

		array<ref SCR_WorkshopItem> updated = m_ModsManager.GetRoomItemsUpdated();
		array<ref SCR_WorkshopItem> outdated = m_ModsManager.GetRoomItemsToUpdate();

		// setup of server detail
		bool modsUpdated = outdated.IsEmpty();

		if (m_ServerScenarioDetails)
			m_ServerScenarioDetails.DisplayMods();
		
		ReceiveRoomContent_Scenario(room);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnModListFailDialogClose(SCR_ConfigurableDialogUi dialog)
	{
		m_ModListFailDialog = null;
		
		JoinProcess_Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Platform_OnUgcPrivilegeResult(bool result)
	{
		SCR_AddonManager.GetInstance().m_OnUgcPrivilegeResult.Remove(Platform_OnUgcPrivilegeResult);
		if (result)
			JoinProcess_LoadModContentVisualize();
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_LoadModContentVisualize()
	{		
		// Show state of mods if all loaded
		if (m_JoinProcessTargetRoom.IsDownloadListLoaded())
		{
			JoinProcess_CheckModContent(m_JoinProcessTargetRoom);
			return;
		}

		// Show mods loading dialog
		m_Dialogs.DisplayDialog(EJoinDialogState.CHECKING_CONTENT);

		// Set wait for loading
		m_ModsManager.GetOnGetAllDependencies().Insert(JoinProcess_CheckModContent);
		m_ModsManager.GetOnModsFail().Insert(JoinProcess_OnModCheckFailed);
		m_ModsManager.GetOnDependenciesLoadingPrevented().Insert(OnDependenciesLoadingPrevented);

		m_ModsManager.ReceiveRoomMods(m_JoinProcessTargetRoom);
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_OnModCheckFailed()
	{
		m_Dialogs.CloseCurrentDialog();
		OnModListFail();
		
		m_ModsManager.GetOnGetAllDependencies().Remove(JoinProcess_CheckModContent);
		m_ModsManager.GetOnModsFail().Remove(JoinProcess_OnModCheckFailed);
		m_ModsManager.GetOnDependenciesLoadingPrevented().Remove(OnDependenciesLoadingPrevented);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when mods loading got error
	//! Show mods loading fail message
	protected void OnModListFail()
	{
		if (m_ModListFailDialog)
			return;
		
		m_ModListFailDialog = SCR_CommonDialogs.CreateRequestErrorDialog();
		m_ModListFailDialog.m_OnClose.Insert(OnModListFailDialogClose);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDependenciesLoadingPrevented(array<ref SCR_WorkshopItem> dependencies)
	{
		m_Dialogs.CloseCurrentDialog();
		JoinProcess_CheckModContent(m_JoinProcessTargetRoom);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check state of room required mods on client
	//! Stop progess if room mods are restricted
	protected void JoinProcess_CheckModContent(Room room)
	{
		m_Dialogs.CloseCurrentDialog();

		// Remove mods check actions
		m_ModsManager.GetOnGetAllDependencies().Remove(JoinProcess_CheckModContent);
		m_ModsManager.GetOnModsFail().Remove(JoinProcess_OnModCheckFailed);
		m_ModsManager.GetOnDependenciesLoadingPrevented().Remove(OnDependenciesLoadingPrevented);
		
		// Restricted content check
		array<ref SCR_WorkshopItem> items = m_ModsManager.GetRoomItemsScripted();

		array<ref SCR_WorkshopItem> restricedMods = SCR_AddonManager.SelectItemsOr(items, EWorkshopItemQuery.RESTRICTED);
		bool restricted = restricedMods.Count() > 0;

		// Stop join if there are restricted mods
		if (restricted)
		{
			SCR_ReportedAddonsDialog dialog = m_ModsManager.DisplayRestrictedAddonsList();
			dialog.GetOnAllReportsCanceled().Insert(JoinProcess_LoadModContentVisualize);
			return;
		}
		
		JoinProcess_CheckUnrelatedDownloads();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Display a warning dialog if there are unrelated downloads running, as these will be stopped
	protected void JoinProcess_CheckUnrelatedDownloads()
	{	
		array<ref SCR_WorkshopItem> items = m_ModsManager.GetRoomItemsScripted();
		m_aUnrelatedDownloads = SCR_DownloadManager.GetInstance().GetUnrelatedDownloads(items);
			
		if (!m_aUnrelatedDownloads.IsEmpty())
			JoinProcess_DisplayUnrelatedDownloadsWarning();
		else
			JoinProcess_CheckRequiredDownloads();
	}
	
	//------------------------------------------------------------------------------------------------
	// Display a dialog asking for unrelated download stop confirmation (this includes wrong versions of required mods)
	protected void JoinProcess_DisplayUnrelatedDownloadsWarning()
	{	
		m_Dialogs.DisplayJoinDownloadsWarning(m_aUnrelatedDownloads, SCR_EJoinDownloadsConfirmationDialogType.UNRELATED);
		m_Dialogs.GetOnConfirm().Insert(JoinProcess_StopDownloadingUnrelatedMods);
	}
	
	//------------------------------------------------------------------------------------------------
	// Pause existing downloads that are unrelated to the specific server we want to join
	protected void JoinProcess_StopDownloadingUnrelatedMods()
	{	
		m_Dialogs.GetOnConfirm().Remove(JoinProcess_StopDownloadingUnrelatedMods);

		// Display filler dialog
		m_Dialogs.DisplayDialog(EJoinDialogState.UNRELATED_DOWNLOADS_CANCELING);
		
		// Stop downloads
		foreach (SCR_WorkshopItemActionDownload download : m_aUnrelatedDownloads)
		{
			download.Cancel();
		}
		
		//TODO: pause downloads instead of clearing them, and allow the player to resume them once out of multiplayer games
		//TODO: give the option to keep downloading while playing multiplayer?
		
		GetGame().GetCallqueue().Call(JoinProcess_CheckUnrelatedDownloadsCanceling);
	}
	
	//------------------------------------------------------------------------------------------------
	//Check if all unrelated downloads have been stopped. Recursive
	protected void JoinProcess_CheckUnrelatedDownloadsCanceling()
	{
		array<ref SCR_WorkshopItem> items = m_ModsManager.GetRoomItemsScripted();
		array<ref SCR_WorkshopItemActionDownload> downloads = SCR_DownloadManager.GetInstance().GetUnrelatedDownloads(items);
		if (!downloads.IsEmpty())
		{
			GetGame().GetCallqueue().Call(JoinProcess_CheckUnrelatedDownloadsCanceling);
			return;
		}
		
		GetGame().GetCallqueue().Call(JoinProcess_CheckRequiredDownloads);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_CheckRequiredDownloads()
	{
		m_Dialogs.CloseCurrentDialog();
		
		// Get necessary downloads
		array<ref SCR_WorkshopItem> items = m_ModsManager.GetRoomItemsScripted();
		m_aRequiredMods = SCR_AddonManager.SelectItemsBasic(items, EWorkshopItemQuery.NOT_LOCAL_VERSION_MATCH_DEPENDENCY);
		if (!m_aRequiredMods.IsEmpty())
		{
			JoinProcess_DownloadRequiredMods();
			return;
		}
		
		JoinProcess_WaitToCheckRunningDownloads();
	}
	
	//------------------------------------------------------------------------------------------------
	// Start downloading necessary mods for the server
	protected void JoinProcess_DownloadRequiredMods()
	{
		SCR_DownloadManager.GetInstance().DownloadItems(m_aRequiredMods);
		
		m_Dialogs.DisplayJoinDownloadsWarning(SCR_DownloadManager.GetInstance().GetDownloadQueue(), SCR_EJoinDownloadsConfirmationDialogType.REQUIRED);
		
		m_Dialogs.GetOnDownloadComplete().Insert(JoinProcess_PrepareFinalJoinRequest);
		m_Dialogs.GetOnCancel().Insert(OnDownloadRequiredModsCancel);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDownloadRequiredModsCancel()
	{
		m_Dialogs.GetOnCancel().Remove(OnDownloadRequiredModsCancel);
		
		SCR_DownloadManager.GetInstance().EndAllDownloads();
		SCR_DownloadManager.GetInstance().ClearFailedDownloads();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_PrepareFinalJoinRequest(Room roomToJoin)
	{
		m_Dialogs.GetOnDownloadComplete().Remove(JoinProcess_PrepareFinalJoinRequest);
		
		JoinProcess_WaitToCheckRunningDownloads();
	}
	
	//------------------------------------------------------------------------------------------------
	// We need to wait a frame before performing the next check due to how the DownloadManager handles it's queue
	protected void JoinProcess_WaitToCheckRunningDownloads()
	{
		GetGame().GetCallqueue().Call(JoinProcess_CheckRunningDownloads);
	}
	
	//! Final step: display a warning dialog if there are still downloads running, as these will be stopped (at this point, this should not happen)
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_CheckRunningDownloads()
	{
		m_Dialogs.CloseCurrentDialog();
		
		int nCompleted, nTotal;
		SCR_DownloadManager.GetInstance().GetDownloadQueueState(nCompleted, nTotal);

		if (nTotal <= 0)
		{
			JoinProcess_Join();
			return;
		}
	
		m_Dialogs.DisplayJoinDownloadsWarning(SCR_DownloadManager.GetInstance().GetDownloadQueue(), SCR_EJoinDownloadsConfirmationDialogType.ALL);
		m_Dialogs.GetOnConfirm().Insert(OnInterruptDownloadConfirm);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInterruptDownloadConfirm()
	{
		m_Dialogs.GetOnConfirm().Remove(OnInterruptDownloadConfirm);
		
		if (!m_JoinProcessTargetRoom)
		{
			PrintDebug("Missing m_JoinProcessTargetRoom!", "OnInteruptDownloadConfirm");
			return;
		}

		// TODO: pause and cache instead of canceling
		// Cancel downloading
		SCR_DownloadManager.GetInstance().EndAllDownloads();

		JoinProcess_Join();
	}
	
	//------------------------------------------------------------------------------------------------
	// Join ...Finally!
	protected void JoinProcess_Join()
	{
		m_Dialogs.CloseCurrentDialog();
		m_Dialogs.DisplayDialog(EJoinDialogState.JOIN);
		
		// Add join callbacks
		m_CallbackJoin = new BackendCallback;
		m_CallbackJoin.SetOnSuccess(JoinProcess_OnJoinSuccess);
		m_CallbackJoin.SetOnError(JoinProcess_OnJoinFail);
		
		// Join server
		m_JoinProcessTargetRoom.Join(m_CallbackJoin, m_JoinData);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this if joining to room was successful
	//! Connect to the server
	protected void JoinProcess_OnJoinSuccess(BackendCallback callback)
	{
		// Connect - gathers mods needed and calls for a reload
		if (!GameStateTransitions.RequestConnectViaRoom(m_Lobby.GetJoinRoom()))
			return; // Transition is not guaranteed

		// Save menu to reopen
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ServerBrowserMenu);

		GetGame().GetMenuManager().CloseAllMenus();
		GetGame().GetBackendApi().GetWorkshop().Cleanup();

		JoinProcess_Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	// This also includes being placed in joining queue
	protected void JoinProcess_OnJoinFail(BackendCallback callback)
	{
		ERestResult apiCode = callback.GetApiCode();
		
		// TODO: Backend - callback on Queue is called twice and apiCode is 0 - ignore such callback
		if (!apiCode)
			return;
		
		// Timeout handle
		if (callback.GetRestResult() == ERestResult.EREST_ERROR_TIMEOUT)
		{
			JoinProcess_Clear();
			m_Dialogs.CloseCurrentDialog();
			SCR_CommonDialogs.CreateTimeoutOkDialog();
			return;
		}
		
		m_Dialogs.CloseCurrentDialog();
		
		SCR_EJoinFailUI result = SCR_EJoinFailUI.GENERIC;
		
		if (apiCode == EApiCode.EACODE_ERROR_MP_ROOM_QUEUE_JOIN)
			result = SCR_EJoinFailUI.ENQUEUED;
		else if (apiCode == EApiCode.EACODE_ERROR_MP_ROOM_QUEUE_FULL)
			result = SCR_EJoinFailUI.SERVER_FULL_QUEUE_FULL;
		else if (apiCode == EApiCode.EACODE_ERROR_MP_ROOM_IS_FULL)
			result = SCR_EJoinFailUI.SERVER_FULL_QUEUE_DISABLED;
		else if (!m_JoinData.scope.IsEmpty())
			result = SCR_EJoinFailUI.BANNED;
		
		switch (result)
		{
			// QUEUE: provide a dialog with queue info and setup callbacks for queue results
			case SCR_EJoinFailUI.ENQUEUED:
			{
				Room joinRoom = m_Lobby.GetJoinRoom();
				if (!joinRoom)
					break;
				
				SCR_ServerFullDialog dialog = m_Dialogs.CreateServerFullDialog();
				if (!dialog)
					break;
				
				JoinProcess_Clear();
				
				dialog.Init(joinRoom, result, joinRoom.HostScenario());
				dialog.GetOnLeaveQueueRequest().Insert(JoinProcess_LeaveJoinQueue);
				
				// Callback
				m_CallbackQueue = new BackendCallback();
				m_CallbackQueue.SetOnSuccess(JoinProcess_OnJoinSuccess);
				m_CallbackQueue.SetOnError(JoinProcess_OnQueueJoinFail);
				
				joinRoom.SetQueueBackendCallback(m_CallbackQueue);
				
				// Remove auto refresh callback, as it is not needed while in queue
				ClearRefreshCallback();
				break;
			}
			
			// Provide a warning dialog that allows to reattempt joining
			case SCR_EJoinFailUI.SERVER_FULL_QUEUE_FULL:
			case SCR_EJoinFailUI.SERVER_FULL_QUEUE_DISABLED:
			{
				SCR_ServerFullDialog dialog = m_Dialogs.CreateServerFullDialog();
				if (dialog)
				{
					dialog.Init(m_JoinProcessTargetRoom, result, m_JoinProcessTargetRoom.HostScenario(), m_OnFavoritesResponse);
					
					dialog.GetOnRetryFullServerJoin().Insert(JoinProcess_Init);
					dialog.GetOnFavorite().Insert(OnActionFavorite);
				}
			
				SetupRefreshCallback();
				JoinProcess_Clear();
				break;
			}
			
			// Provide an error dialog with ban info
			case SCR_EJoinFailUI.BANNED:
			{
				m_Dialogs.DisplayJoinBan(m_JoinData);
				SetupRefreshCallback();
				JoinProcess_Clear();
				break;
			}
			
			// Provide a generic error dialog
			default:
			{
				m_Dialogs.DisplayJoinFail(apiCode);
				SetupRefreshCallback();
				JoinProcess_Clear();
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_LeaveJoinQueue()
	{
		m_Lobby.GetJoinRoom().LeaveJoinQueue();
		m_Dialogs.CloseCurrentDialog();
		
		JoinProcess_Clear();
		
		// Restore auto refresh callback
		SetupRefreshCallback();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_OnQueueJoinFail(BackendCallback callback)
	{
		EApiCode apiCode = callback.GetApiCode();
		
		// TODO: Backend - callback on Queue is called twice and apiCode is 0 - ignore such callback
		if (!apiCode)
			return;
		
		// Still waiting in queue, update the dialog
		if (apiCode == EApiCode.EACODE_ERROR_MP_ROOM_QUEUE_WAIT)
		{
			m_Dialogs.UpdateServerFullDialog();
			return;
		}
		
		// Out of queue, joining failed
		m_Dialogs.CloseCurrentDialog();
		m_Dialogs.DisplayJoinFail(apiCode);
		JoinProcess_Clear();
		
		// Restore auto refresh callback
		SetupRefreshCallback();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this to kill joining process at any stage
	protected void JoinProcess_Clear()
	{
		m_JoinProcessTargetRoom = null;
		
		//m_CallbackJoin = null;
		//m_CallbackQueue = null;
		
		m_Lobby.ClearInviteRoom();

		// Clear mods manager callbacks
		m_ModsManager.GetOnGetAllDependencies().Clear();
		m_ModsManager.GetOnModsFail().Clear();
		m_ModsManager.GetOnDependenciesLoadingPrevented().Clear();
		m_ModsManager.GetOnGetScenario().Clear();

		// Clear dialog
		m_Dialogs.GetOnConfirm().Clear();
		m_Dialogs.GetOnCancel().Clear();
		m_Dialogs.GetOnJoinProcessCancel().Clear();
		m_Dialogs.GetOnDownloadComplete().Clear();
		
		// TODO: remove specific subscriptions instead of clearing!
		
		GetGame().GetGameBlocklist().OnGetBlockedPlayersInRoomInvoker.Remove(JoinProcess_OnCheckedBlockedPlayersInRoom);
	}

	//------------------------------------------------------------------------------------------------
	//! Open joining dialog in default state
	protected void SetupJoinDialogs()
	{
		// Check dialog
		if (!m_Dialogs)
			return;

		// Setup basics
		m_Dialogs.SetModManager(m_ModsManager);
		m_Dialogs.SetJoinRoom(m_JoinProcessTargetRoom);

		// Invokers
		m_Dialogs.GetOnDownloadComplete().Clear();
		m_Dialogs.GetOnJoinProcessCancel().Insert(JoinProcess_Clear);

		m_CallbackLastSearch = null;
	}
	
	//------------------------------------------------------------------------------------------------
	// --- PRIVILEGES HANDLING ---
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void OnMPPrivilegeResult(UserPrivilege privilege, UserPrivilegeResult result)
	{
		// Sucessful
		if (privilege == UserPrivilege.MULTIPLAYER_GAMEPLAY && result == UserPrivilegeResult.ALLOWED)
			Refresh();

		m_CallbackGetPrivilege.m_OnResult.Remove(OnMPPrivilegeResult);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCrossPlayPrivilegeResult(UserPrivilege privilege, UserPrivilegeResult result)
	{
		// Sucessful
		if (privilege == UserPrivilege.CROSS_PLAY && result == UserPrivilegeResult.ALLOWED)
			Refresh();

		m_CallbackGetPrivilege.m_OnResult.Remove(OnCrossPlayPrivilegeResult);
	}
	
	//------------------------------------------------------------------------------------------------
	// --- STATIC ---
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	static void TryOpenServerBrowser()
	{
		if (!m_CallbackGetMPPrivilege)
			m_CallbackGetMPPrivilege = new SCR_ScriptPlatformRequestCallback();
		
		m_CallbackGetMPPrivilege.m_OnResult.Insert(ServerBrowserOnPrivilegeResult);
		SocialComponent.RequestMultiplayerPrivilege(m_CallbackGetMPPrivilege);
	}
	
	//------------------------------------------------------------------------------------------------
	protected static void ServerBrowserOnPrivilegeResult(UserPrivilege privilege, UserPrivilegeResult result)
	{
		if (privilege == UserPrivilege.MULTIPLAYER_GAMEPLAY && result != UserPrivilegeResult.ALLOWED)
			SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "mp_you_dont_have_the_right");
		else if (m_MissionToFilter)
			OpenWithScenarioFilter(m_MissionToFilter);
		else
			GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu);
		
		m_MissionToFilter = null;
		m_CallbackGetMPPrivilege.m_OnResult.Remove(ServerBrowserOnPrivilegeResult);
	}
	
	//------------------------------------------------------------------------------------------------
	static void TryOpenServerBrowserWithScenarioFilter(MissionWorkshopItem mission)
	{
		if (!m_CallbackGetMPPrivilege)
			m_CallbackGetMPPrivilege = new SCR_ScriptPlatformRequestCallback();
		
		m_MissionToFilter = mission;
		m_CallbackGetMPPrivilege.m_OnResult.Insert(ServerBrowserOnPrivilegeResult);
		SocialComponent.RequestMultiplayerPrivilege(m_CallbackGetMPPrivilege);
	}

	//------------------------------------------------------------------------------------------------
	// --- DEBUG ---
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Custom debug print displayed with -scrDefine=SB_DEBUG argument
	protected void PrintDebug(string msg, string functionName = string.Empty)
	{
		#ifdef SB_DEBUG
		//Setup function format
		string fncStr = string.Empty;
		if (functionName != string.Empty)
			fncStr = string.Format(" Fnc: %1()", functionName);

		// Display message
		PrintFormat("[ServerBrowserMenuUI]%1 -- Msg: %2", fncStr, msg);
		#endif
	}
}

//------------------------------------------------------------------------------------------------
//! Overrided GetRoomsIds class to manipulation in script
class SCR_GetRoomsIds extends GetRoomsIds
{
	protected ref array<string> roomIds = {};

	//------------------------------------------------------------------------------------------------
	void RegisterId(string id)
	{
		roomIds.Insert(id);
		RegV("roomIds");
	}

	//------------------------------------------------------------------------------------------------
	void ClearIds()
	{
		roomIds.Clear();
	}
}

enum SCR_EJoinFailUI
{
	GENERIC,
	ENQUEUED,
	SERVER_FULL_QUEUE_FULL,
	SERVER_FULL_QUEUE_DISABLED,
	BANNED
}

enum SCR_EServerBrowserTabs
{
	ALL = 0,
	COMMUNITY,
	OFFICIAL,
	FAVORITES,
	RECENT,
	HOST
}
