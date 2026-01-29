/*
Server hosting dialog sub menu for setting up mods
*/

class SCR_ServerHostingModSubMenu : SCR_SubMenuBase
{
	protected const string NAV_ENABLE_ALL = "#AR-ServerHosting_EnableAll";
	protected const string NAV_DISABLE_ALL = "#AR-Workshop_DisableAll";
	
	protected const string SORT_BTN_SORT_LABEL = "#AR-Editor_TooltipDetail_WaypointIndex_Name";
	protected const string SORT_BTN_SORT_ACTION = "MenuFilter";
	protected const string SORT_BTN_PLACE_LABEL = "#AR-Button_Confirm-UC";
	
	// Attributes 
	[Attribute("{5CEB59FD637E20B9}UI/layouts/Menus/ContentBrowser/AddonManager/AddonListTab/AddonLineDSConfig.layout", UIWidgets.ResourceNamePicker, "Used layout for mod enabling", "layout")]
	protected ResourceName m_EntryLayout;
	
	// Fields 
	protected ref SCR_ServerHostingModsWidgets m_Widgets = new SCR_ServerHostingModsWidgets();
	
	protected ScrollLayoutWidget m_wDisableScroll;
	protected ScrollLayoutWidget m_wEnableScroll;
	
	protected ref array<ref Widget> m_aEnabled = {};
	protected ref array<ref Widget> m_aDisabled = {};
	
	// Navigation buttons 
	protected SCR_InputButtonComponent m_NavWorkshop;
	protected SCR_InputButtonComponent m_NavEnable;
	protected SCR_InputButtonComponent m_NavEnableAll;
	protected SCR_InputButtonComponent m_NavSelectSort;
	protected SCR_InputButtonComponent m_NavChangeSortOrder;
	
	protected SCR_AddonLineDSConfigComponent m_FocusedLine;
	protected SCR_AddonLineDSConfigComponent m_OrderedLine;
	
	// Keep track of currently active addons and which line they are associated to
	protected ref map<ref SCR_WorkshopItem, ref SCR_AddonLineDSConfigComponent> m_ActiveAddons = new map<ref SCR_WorkshopItem, ref SCR_AddonLineDSConfigComponent>();

	protected bool m_bIsSorting;
	
	protected MissionWorkshopItem m_SelectedScenario;
	
	protected ref ScriptInvokerVoid m_OnWorkshopButtonActivate;
	protected ref ScriptInvokerVoid m_OnRequestDefaultScenario;

	// --- Overrides ---
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_Widgets.Init(w);
		
		// Find scroll components
		m_wDisableScroll = ScrollLayoutWidget.Cast(m_Widgets.m_wDisabledAddonsList.GetParent());
		m_wEnableScroll = ScrollLayoutWidget.Cast(m_Widgets.m_wEnabledAddonsList.GetParent());
		
		// Setup action all buttons 
		m_Widgets.m_ButtonEnableAllComponent0.m_OnClicked.Insert(OnEnableAllClicked);
		m_Widgets.m_ButtonDisableAllComponent0.m_OnClicked.Insert(OnDisableAllClicked);
		
		// Disable Delete All buttons
		m_Widgets.m_wDeleteAllEnabled.SetVisible(false);
		m_Widgets.m_wDeleteAllDisabled.SetVisible(false);
		
		CreateAddonList();

		SetupDownloadingCallbacks();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		ClearDownloadingCallbacks();
		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		m_NavWorkshop = CreateNavigationButton("MenuDownloadManager", "#AR-Workshop_WorkshopPageName", false);
		if (m_NavWorkshop)
			m_NavWorkshop.m_OnActivated.Insert(OnOpenWorkshopButton);

		m_NavSelectSort = CreateNavigationButton(SORT_BTN_SORT_ACTION, SORT_BTN_SORT_LABEL, true);
		if (m_NavSelectSort)
			m_NavSelectSort.m_OnActivated.Insert(OnNavSelectSortActivate);

		m_NavChangeSortOrder = CreateNavigationButton("MenuVertical", SORT_BTN_SORT_LABEL, true);
		if (m_NavChangeSortOrder)
			FlagNavigationButtonVisibility(m_NavChangeSortOrder, false);

		m_NavEnableAll = CreateNavigationButton(UIConstants.MENU_ACTION_ENABLE_ALL, NAV_ENABLE_ALL, true);
		if (m_NavEnableAll)
			m_NavEnableAll.m_OnActivated.Insert(OnNavEnableAllActivated);
		
		m_NavEnable = CreateNavigationButton(UIConstants.MENU_ACTION_ENABLE, SCR_WorkshopUiCommon.LABEL_ENABLE, true);
		if (m_NavEnable)
			m_NavEnable.m_OnActivated.Insert(OnNavEnableActivated);
		
		ShowNavigationButtons(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		// Focus fist entry next frame to ensure focus will happend
		GetGame().GetCallqueue().Call(FocusFirstEntry);
		
		UpdateWorkshopButton();
		
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		super.OnTabShow();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();
	
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
		
		ReleaseSort();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateWorkshopButton()
	{	
		if (!m_NavWorkshop)
			return;

		SCR_ConnectionUICommon.SetConnectionButtonEnabled(m_NavWorkshop, SCR_ServicesStatusHelper.SERVICE_WORKSHOP);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateWorkshopButton();
	}
	
	//------------------------------------------------------------------------------------------------
	override void ShowNavigationButtons(bool show)
	{
		super.ShowNavigationButtons(show && !m_ActiveAddons.IsEmpty());
	}

	//------------------------------------------------------------------------------------------------
	// Protected 
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void FocusFirstEntry()
	{
		if (!m_aDisabled.IsEmpty() && m_aDisabled[0])
			GetGame().GetWorkspace().SetFocusedWidget(m_aDisabled[0]);
		else if (!m_aEnabled.IsEmpty() && m_aEnabled[0])
			GetGame().GetWorkspace().SetFocusedWidget(m_aEnabled[0]);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add all offline mods to selection
	protected void CreateAddonList()
	{
		if (!SCR_AddonManager.GetInstance())
			return;
		
		array<ref SCR_WorkshopItem> addons = SCR_AddonManager.GetInstance().GetOfflineAddons();
		
		foreach (int i, SCR_WorkshopItem addon : addons)
		{
			SCR_ERevisionAvailability availability = SCR_AddonManager.ItemAvailability(addon.Internal_GetWorkshopItem());
			
			if (availability == SCR_ERevisionAvailability.ERA_AVAILABLE)
				AddLineEntry(addon, i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddLineEntry(SCR_WorkshopItem item, int id)
	{
		WorkshopItem workshopItem = item.GetWorkshopItem();
		if (workshopItem)
		{
			if (WorldSaveItem.Cast(workshopItem))
				return;
		}
		
		// Line setup
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_EntryLayout, m_Widgets.m_wDisabledAddonsList);
		SCR_AddonLineDSConfigComponent line = SCR_AddonLineDSConfigComponent.Cast(
			w.FindHandler(SCR_AddonLineDSConfigComponent));
		
		if (!line)
			return; 
		
		// Callbacks 
		line.GetOnEnableButton().Insert(OnAddonEnabled);
		line.GetOnDisableButton().Insert(OnAddonDisabled);
		line.GetOnSortConfirm().Insert(OnSortConfirm);
		line.GetEventOnButtonUp().Insert(OnLineButtonUp);
		line.GetEventOnButtonDown().Insert(OnLineButtonDown);
		line.GetOnFocus().Insert(OnLineFocus);
		line.GetOnFocusLost().Insert(OnLineFocusLost);
		line.GetOnRequiredDisabled().Insert(OnRequiredAddonDisabled);
		
		line.Init(item);
		m_aDisabled.Insert(w);
		w.SetZOrder(id);
		
		// Enable map 
		m_ActiveAddons.Insert(item, line);
	}
	
	//------------------------------------------------------------------------------------------------
	protected int LastFocusedId(Widget line, bool enabled)
	{
		array<ref Widget> list = {};
		
		if (enabled)
			list = m_aDisabled;
		else
			list = m_aEnabled;
		
		// Single entry?
		if (list.Count() == 1)
			return -1;
		
		return list.Find(line);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return closest workshop item, first check down, then up if at the end of the list
	protected Widget ClosestAddonLine(int lineId, bool enabled)
	{
		array<ref Widget> list = m_aDisabled;
		
		if (enabled)
			list = m_aEnabled;
		
		// Not found || The only element in the list || The list has no elements 
		if (lineId < 0 || lineId > list.Count() || list.IsEmpty())
			return null;
		
		if (lineId == 0)
			return list[0];
		
		return list[lineId - 1];
	}
	
	//-------------------------------------------------------------------------------------------
	protected SCR_AddonLineDSConfigComponent FindLineByModId(string id)
	{
		for (int i = 0, count = m_ActiveAddons.Count(); i < count; i++)
		{
			SCR_AddonLineDSConfigComponent line = m_ActiveAddons.GetElement(i);
			if (!line)
				continue;
			
			SCR_WorkshopItem item = line.GetWorkshopItem();
			if (!item)
				return null;
			
			if (id == item.GetId())
				return line;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup callbacks for all running downloads 
	protected void SetupDownloadingCallbacks()
	{
		SCR_DownloadManager downloadManager = SCR_DownloadManager.GetInstance();
		if (!downloadManager)
			return;
		
		array<ref SCR_WorkshopItemActionDownload> actions = downloadManager.GetDownloadQueue();
		foreach (SCR_WorkshopItemActionDownload action : actions)
		{
			action.m_OnCompleted.Insert(OnDownloadComplete);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearDownloadingCallbacks()
	{
		SCR_DownloadManager downloadManager = SCR_DownloadManager.GetInstance();
		if (!downloadManager)
			return;
		
		array<ref SCR_WorkshopItemActionDownload> actions = downloadManager.GetDownloadQueue();
		foreach (SCR_WorkshopItemActionDownload action : actions)
		{
			action.m_OnCompleted.Remove(OnDownloadComplete);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateNavigationButtons(SCR_AddonLineDSConfigComponent line, bool forceDisabled = false)
	{
		if (!line)
			forceDisabled = true;
		
		bool enabled = line && line.IsEnabled();
		
		if (m_NavEnable)
		{
			bool issues = line && (!line.CanBeEnabled() && !enabled);
			
			m_NavEnable.SetEnabled(!m_OrderedLine && !issues && !forceDisabled, false);
			
			if (enabled)
				m_NavEnable.SetLabel(SCR_WorkshopUiCommon.LABEL_DISABLE);
			else
				m_NavEnable.SetLabel(SCR_WorkshopUiCommon.LABEL_ENABLE);
			
			if (issues)
				m_NavEnable.SetTexture(UIConstants.ICONS_IMAGE_SET, SCR_WorkshopUiCommon.ICON_DEPENDENCIES, Color.FromInt(UIColors.WARNING_DISABLED.PackToInt()));
			else
				m_NavEnable.ResetTexture();
		}
		
		if (m_NavEnableAll)
		{
			m_NavEnableAll.SetEnabled(!m_OrderedLine && !forceDisabled, false);
			
			if (enabled)
				m_NavEnableAll.SetLabel(NAV_DISABLE_ALL);
			else
				m_NavEnableAll.SetLabel(NAV_ENABLE_ALL);
		}
		
		if (m_NavSelectSort)
			m_NavSelectSort.SetEnabled(!forceDisabled && enabled, false);
	}
	
	//------------------------------------------------------------------------------------------------
	// Callbacks
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void OnAddonEnabled(SCR_ScriptedWidgetComponent component)
	{	
		SCR_AddonLineDSConfigComponent line = SCR_AddonLineDSConfigComponent.Cast(component);
		if (!line)
			return;
		
		SCR_WorkshopItem item = line.GetWorkshopItem();
		if (!item)
			return;

		// Enable dependencies
		array<ref SCR_WorkshopItem> dependencies = item.GetLatestDependencies();
		dependencies.Insert(item);
		
		int indexToFocus = m_aDisabled.Find(component.GetRootWidget());

		EnableItems(true, dependencies, indexToFocus);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAddonDisabled(SCR_ScriptedWidgetComponent component)
	{
		SCR_AddonLineBaseComponent line = SCR_AddonLineBaseComponent.Cast(component);
		if (!line)
			return;
		
		SCR_WorkshopItem item = line.GetWorkshopItem();
		if (!item)
			return;
		
		// Unselect dependent mods
		array<ref SCR_WorkshopItem> dependent = item.GetDependentAddons();
		
		// Add item 
		dependent.Insert(item);
		
		int indexToFocus = m_aEnabled.Find(component.GetRootWidget());
		
		EnableItems(false, dependent, indexToFocus);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineFocus(SCR_ScriptedWidgetComponent component)
	{
		SCR_AddonLineDSConfigComponent line = SCR_AddonLineDSConfigComponent.Cast(component);
		if (!line)
			return;
		
		m_FocusedLine = line;
		
		UpdateNavigationButtons(line);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineFocusLost(SCR_ScriptedWidgetComponent component)
	{
		m_FocusedLine = null;
		
		SCR_AddonLineDSConfigComponent line = SCR_AddonLineDSConfigComponent.Cast(component);
		UpdateNavigationButtons(line, true);
		
		// Prevent move between list when ordering 
		if (m_OrderedLine && m_OrderedLine != m_FocusedLine)
			GetGame().GetCallqueue().Call(FocusLine, m_OrderedLine);
	}
	
	//----------------------------------------------------------------------------------------------
	protected void OnDownloadComplete(SCR_WorkshopItemAction action)
	{
		// Add mod to mod list 
		AddLineEntry(action.GetWorkshopItem(), m_aDisabled.Count());
		action.m_OnCompleted.Remove(OnDownloadComplete);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRequiredAddonDisabled(SCR_ScriptedWidgetComponent component)
	{
		RequestTabChange(SCR_EServerHostingDialogTabs.SETTINGS);
		
		if (m_OnRequestDefaultScenario)
			m_OnRequestDefaultScenario.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable/disable multiple addons recursively. This does NOT actually enable/disable the addon, just changes it's state in relation to creating the server config
	protected void EnableItems(bool enable, array<ref SCR_WorkshopItem> items, int indexToFocus = -1)
	{
		if (items.IsEmpty())
			return;

		SCR_WorkshopItem item = items[0];
		SCR_AddonLineDSConfigComponent line = m_ActiveAddons.Get(item);
		
		if (line)
		{
			Widget lineRoot = line.GetRootWidget();
			
			// Enable 
			if (enable && !line.IsEnabled())
			{
				// Move to enabled
				m_Widgets.m_wEnabledAddonsList.AddChild(lineRoot);
				m_aEnabled.Insert(lineRoot);
				m_aDisabled.RemoveItem(lineRoot);
				
				lineRoot.SetZOrder(m_aEnabled.Count() - 1);
			}
			else if (!enable && line.IsEnabled())
			{
				// Move to disabled
				m_Widgets.m_wDisabledAddonsList.AddChild(lineRoot);
				m_aDisabled.Insert(lineRoot);
				m_aEnabled.RemoveItem(lineRoot);
				
				lineRoot.SetZOrder(m_aDisabled.Count() - 1);
			}
			
			line.SetWidgetEnabled(enable);
		}
		
		// Order disabled mods 
		foreach (int i, Widget disabled : m_aDisabled)
		{
			disabled.SetZOrder(i);
		}
		
		// Order enabled mods
		foreach (int i, Widget enabled : m_aEnabled)
		{
			enabled.SetZOrder(i);
			
			SCR_AddonLineDSConfigComponent lineCmp = SCR_AddonLineDSConfigComponent.Cast(m_aEnabled[i].FindHandler(SCR_AddonLineDSConfigComponent));
			if (lineCmp)
				lineCmp.SetOnBottom(i == m_aEnabled.Count() - 1);
		}
		
		// Call next, keep going 
		items.Remove(0);
		if (!items.IsEmpty())
		{
			EnableItems(enable, items, indexToFocus);
			return;
		}
	
		// On finished fix list scrolls 
		float x,y; 
		m_wDisableScroll.GetSliderPos(x, y);
		
		if (y > 1)
			m_wDisableScroll.SetSliderPos(0, 1);
		
		m_wEnableScroll.GetSliderPos(x, y);
		
		if (y > 1)
			m_wEnableScroll.SetSliderPos(0, 1);
		
		// Reset the focus effects if is to stay focused in the same line
		if (m_FocusedLine && !indexToFocus)
			OnLineFocus(m_FocusedLine);
		
		if (indexToFocus >= 0) 
		{
			Widget w = ClosestAddonLine(indexToFocus, !enable);
			if (w)
				GetGame().GetWorkspace().SetFocusedWidget(w)
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateRequiredItems()
	{
		SCR_AddonLineDSConfigComponent line;
		
		// Clear required flags
		for (int i = 0, count = m_ActiveAddons.Count(); i < count; i++)
		{
			line = m_ActiveAddons.GetElement(i);
			if (!line)
				continue;
		
			line.SetRequired(false);
		}
		
		// Find required owner addon
		if (!m_SelectedScenario)
			return;

		line = FindLineByModId(m_SelectedScenario.GetOwnerId());
		if (!line)
			return;

		SCR_WorkshopItem item = line.GetWorkshopItem();
		if (!item)
			return;
		
		// Find required dependencies
		array<ref SCR_WorkshopItem> dependencies = item.GetLatestDependencies();
		dependencies.Insert(item);
		
		foreach (SCR_WorkshopItem dependency : dependencies)
		{
			line = m_ActiveAddons.Get(dependency);
			if (line)
				line.SetRequired(true);
		}
		
		// Automatically enable required mods and update required flag
		EnableItems(true, dependencies);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineButtonUp(SCR_ScriptedWidgetComponent component)
	{
		SCR_AddonLineDSConfigComponent line = SCR_AddonLineDSConfigComponent.Cast(component);
		if (!line)
			return;
		
		// Check if not on top 
		int id = line.GetRootWidget().GetZOrder();
		
		if (id == -1 || id == 0)
			return;
	
		// Enter sorting mode
		if (!m_bIsSorting)
			StartSort(line);
			
		// Move up 
		m_aEnabled[id] = m_aEnabled[id-1];
		m_aEnabled[id].SetZOrder(id);
		
		m_aEnabled[id-1] = line.GetRootWidget(); 
		m_aEnabled[id-1].SetZOrder(id-1);
		
		// Set line is on bottom 
		if (id == m_aEnabled.Count() - 1)
		{
			SCR_AddonLineDSConfigComponent otherLine = SCR_AddonLineDSConfigComponent.Cast(m_aEnabled[id].FindHandler(SCR_AddonLineDSConfigComponent));
			if (otherLine)
				otherLine.SetOnBottom(true);
		}
	
		line.SetOnBottom(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineButtonDown(SCR_ScriptedWidgetComponent component)
	{
		SCR_AddonLineDSConfigComponent line = SCR_AddonLineDSConfigComponent.Cast(component);
		if (!line)
			return;
		
		// Check if not on bottom 
		int id = line.GetRootWidget().GetZOrder();
		
		if (id == -1 || id >= m_aEnabled.Count() - 1)
			return;
		
		// Enter sorting mode
		if (!m_bIsSorting)
			StartSort(line);
		
		// Move down 
		m_aEnabled[id] = m_aEnabled[id+1];
		m_aEnabled[id].SetZOrder(id);
		
		m_aEnabled[id+1] = line.GetRootWidget(); 
		m_aEnabled[id+1].SetZOrder(id+1);
		
		// Set line is on bottom 
		SCR_AddonLineDSConfigComponent otherLine = SCR_AddonLineDSConfigComponent.Cast(m_aEnabled[id].FindHandler(SCR_AddonLineDSConfigComponent));
		if (otherLine)
			otherLine.SetOnBottom(false);

		if (id + 1 == m_aEnabled.Count() - 1)
			line.SetOnBottom(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSortConfirm(SCR_ScriptedWidgetComponent component)
	{
		SCR_AddonLineDSConfigComponent line = SCR_AddonLineDSConfigComponent.Cast(component);
		if (line)
			Sort(line);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEnableAllClicked()
	{
		array<ref SCR_WorkshopItem> toEnable = {};
		SCR_WorkshopItem item;
		SCR_AddonLineDSConfigComponent line;
		
		for (int i = 0, count = m_ActiveAddons.Count(); i < count; i++)
		{
			item = m_ActiveAddons.GetKey(i);
			line = m_ActiveAddons.GetElement(i);
			if (!item || !line || !line.CanBeEnabled())
				continue;
			
			toEnable.Insert(item);
		}
		
		EnableItems(true, toEnable);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDisableAllClicked()
	{
		array<ref SCR_WorkshopItem> toDisable = {};

		foreach (SCR_WorkshopItem item, SCR_AddonLineDSConfigComponent line : m_ActiveAddons)
		{
			if (!line || line.IsItemRequired())
				continue;
			
			toDisable.Insert(item);
		}
		
		EnableItems(false, toDisable);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnOpenWorkshopButton()
	{		
		if (m_OnWorkshopButtonActivate)
			m_OnWorkshopButtonActivate.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnNavEnableActivated(SCR_InputButtonComponent button, string action)
	{
		if (!m_FocusedLine)
			return;
				
		bool enabled = m_FocusedLine.IsEnabled();
		
		// TODO: temporary hacky fix for this method being called multiple times in a row. It's the only button posing this problem, and the invoker is bound only once...
		GetGame().GetCallqueue().Call(SetLineEnabled, enabled);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetLineEnabled(bool enabled)
	{
		GetGame().GetCallqueue().Remove(SetLineEnabled);
		
		if (enabled)
			m_FocusedLine.OnDisableButton();
		else
			m_FocusedLine.OnEnableButton();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnNavEnableAllActivated()
	{
		if (!m_FocusedLine)
			return;
		
		if (m_FocusedLine.IsEnabled())
			OnDisableAllClicked();
		else
			OnEnableAllClicked();
	}
		
	//------------------------------------------------------------------------------------------------
	protected void OnNavSelectSortActivate(SCR_InputButtonComponent button, string action)
	{
		Sort(m_FocusedLine);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Sort(SCR_AddonLineDSConfigComponent line)
	{
		if (m_OrderedLine)
		{
			ReleaseSort();
			return;
		}
		
		StartSort(line);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void StartSort(SCR_AddonLineDSConfigComponent line)
	{
		if (!line || !line.IsEnabled())
			return;
		
		m_OrderedLine = line;

		m_OrderedLine.NotifySorting(true);
		
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_DOWN, EActionTrigger.DOWN, OnDownSort);
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_UP, EActionTrigger.DOWN, OnUpSort);
		
		if (m_NavChangeSortOrder)
			SetNavigationButtonVisibile(m_NavChangeSortOrder, true);
		
		if (m_NavSelectSort)
		{
			// Place and stop ordering
			m_NavSelectSort.SetLabel(SORT_BTN_PLACE_LABEL);
			m_NavSelectSort.SetAction(UIConstants.MENU_ACTION_SELECT);
		}
		
		if (m_NavEnable)
			SetNavigationButtonVisibile(m_NavEnable, false);
	
		if (m_NavEnableAll)
			SetNavigationButtonVisibile(m_NavEnableAll, false);
		
		// Panel buttons
		m_Widgets.m_ButtonEnableAllComponent0.SetEnabled(false);
		m_Widgets.m_ButtonDisableAllComponent0.SetEnabled(false);
		
		m_bIsSorting = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ReleaseSort()
	{
		if (!m_OrderedLine)
			return;
			
		m_OrderedLine.NotifySorting(false);
		m_OrderedLine = null;
		
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_DOWN, EActionTrigger.DOWN, OnDownSort);
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_UP, EActionTrigger.DOWN, OnUpSort);
		
		if (m_NavChangeSortOrder)
			SetNavigationButtonVisibile(m_NavChangeSortOrder, false);
		
		if (m_NavSelectSort)
		{
			// Start ordering
			m_NavSelectSort.SetLabel(SORT_BTN_SORT_LABEL);
			m_NavSelectSort.SetAction(SORT_BTN_SORT_ACTION);
		}
		
		if (m_NavEnable)
			SetNavigationButtonVisibile(m_NavEnable, true);
		
		if (m_NavEnableAll)
			SetNavigationButtonVisibile(m_NavEnableAll, true);
		
		// Panel buttons
		m_Widgets.m_ButtonEnableAllComponent0.SetEnabled(true);
		m_Widgets.m_ButtonDisableAllComponent0.SetEnabled(true);
		
		// Refocus 
		OnLineFocus(m_FocusedLine);
		
		m_bIsSorting = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDownSort()
	{
		if (!m_OrderedLine)
			return;
		
		OnLineButtonDown(m_OrderedLine);
		GetGame().GetCallqueue().Call(FocusLine, m_OrderedLine);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnUpSort()
	{
		if (!m_OrderedLine)
			return;
		
		OnLineButtonUp(m_OrderedLine);
		GetGame().GetCallqueue().Call(FocusLine, m_OrderedLine);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FocusLine(SCR_AddonLineDSConfigComponent line)
	{
		GetGame().GetWorkspace().SetFocusedWidget(line.GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	// API
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Fill all entries with values from given DS config 
	void EnableModsFromDSConfig(notnull SCR_DSConfig config)
	{
		array<ref DSMod> mods = config.game.mods;
		array<ref SCR_WorkshopItem> items = {};
		SCR_AddonLineDSConfigComponent line;

		foreach (DSMod mod : mods)
		{
			line = FindLineByModId(mod.modId);
			if (line)
			{
				// Do not restore corrupted mod
				if (!line.CanBeEnabled())
					continue;
				
				items.Insert(line.GetWorkshopItem());
			}
		}
		
		EnableItems(true, items);
	}
	
	//------------------------------------------------------------------------------------------------
	// Enables necessary mods for the given scenario
	void SetScenario(MissionWorkshopItem scenario)
	{
		m_SelectedScenario = scenario;
		UpdateRequiredItems();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get all selected mods from addon entries
	array<ref DSMod> SelectedModsList()
	{
		array<ref DSMod> mods = {};
		
		foreach (Widget widget : m_aEnabled)
		{
			SCR_AddonLineDSConfigComponent line = SCR_AddonLineDSConfigComponent.Cast(widget.FindHandler(SCR_AddonLineDSConfigComponent));
			if (!line)
				continue;
			
			// Save all enabled
			if (line.IsEnabled())
			{
				SCR_WorkshopItem item = line.GetWorkshopItem();
				if (!item)
					continue;
				
				DSMod mod = new DSMod();
				
				mod.modId = item.GetId();
				mod.name = item.GetName();
				mod.version = item.GetCurrentLocalRevision().GetVersion();
				
				mods.Insert(mod);
			}
		}
		
		return mods;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetEventOnWorkshopButtonActivate()
	{
		if (!m_OnWorkshopButtonActivate)
			m_OnWorkshopButtonActivate = new ScriptInvokerVoid();

		return m_OnWorkshopButtonActivate;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnRequestDefaultScenario()
	{
		if (!m_OnRequestDefaultScenario)
			m_OnRequestDefaultScenario = new ScriptInvokerVoid();

		return m_OnRequestDefaultScenario;
	}
}
