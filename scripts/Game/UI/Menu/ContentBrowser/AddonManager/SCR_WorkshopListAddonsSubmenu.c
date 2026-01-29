class SCR_WorkshopListAddonsSubmenu : SCR_SubMenuBase
{
	[Attribute("5", UIWidgets.EditBox, "Max number of addon that will be enabled in single frame")]
	protected int m_iAddonsInFrame;

	[Attribute("{10A415ADD724538B}UI/layouts/Menus/ContentBrowser/AddonManager/AddonListTab/AddonLineWorkshop.layout")]
	protected ResourceName m_sLineLayout;
	
	protected SCR_InputButtonComponent m_NavEnable;
	protected SCR_InputButtonComponent m_NavEnableAll;
	protected SCR_InputButtonComponent m_NavDelete;
	protected SCR_InputButtonComponent m_NavDeleteAll;
	protected SCR_InputButtonComponent m_NavOpenDetails;
	protected SCR_InputButtonComponent m_NavUpdate;
	protected SCR_InputButtonComponent m_NavValidate;
	
	protected SCR_InputButtonComponent m_NavOpenTools;
	protected SCR_InputButtonComponent m_NavOpenPresets;

	protected ref SCR_ListAddonsSubMenuWidgets m_Widgets = new SCR_ListAddonsSubMenuWidgets();

	protected SCR_WorkshopAddonLineComponent m_FocusedAddonLine;
	protected SCR_WorkshopItem m_LastEnabledItem;
	protected bool m_bIsFocusedEnabled;

	protected ref array<Widget> m_aEntriesEnabled = {};
	protected ref array<Widget> m_aEntriesDisabled = {};
	protected ref map<SCR_WorkshopItem, SCR_WorkshopAddonLineComponent> m_aEntriesComponents = new map<SCR_WorkshopItem, SCR_WorkshopAddonLineComponent>();
	
	protected bool m_bIsEnablingAddons = false;
	
	protected SCR_MenuActionsComponent m_MenuActionsComponent;
	
	protected SCR_AddonDetailsPanelComponent m_AddonInfoPanel;
	
	protected ScrollLayoutWidget m_EnabledAddonsScroll; 
	protected ScrollLayoutWidget m_DisabledAddonsScroll;
	
	protected const string ACTION_DELETE = 		"MenuDelete";
	protected const string ACTION_DELETE_ALL =	"MenuUnsubscribeAll";

	// --- Overrides ---
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		m_Widgets.Init(GetRootWidget());

		// Tools
		m_Widgets.m_ToolsButtonComponent0.m_OnClicked.Insert(OnToolsButton);

		// Move all mods
		m_Widgets.m_ButtonEnableAllComponent0.m_OnClicked.Insert(OnButtonEnableAll);
		m_Widgets.m_ButtonDisableAllComponent0.m_OnClicked.Insert(OnButtonDisableAll);

		// Delete all mods
		m_Widgets.m_DeleteAllEnabledComponent0.m_OnClicked.Insert(OnDeleteAllEnabled);
		m_Widgets.m_DeleteAllDisabledComponent0.m_OnClicked.Insert(OnDeleteAllDisabled);

		// Validate and repair all mods
		m_Widgets.m_ValidateRepairAllEnabledComponent0.m_OnClicked.Insert(OnValidateAllEnabled);
		m_Widgets.m_ValidateRepairAllDisabledComponent0.m_OnClicked.Insert(OnValidateAllDisabled);
	
		
		// Right Panel
		Widget panel = menuRoot.FindAnyWidget("m_AddonInfoPanel");
		if (panel)
			m_AddonInfoPanel = SCR_AddonDetailsPanelComponent.Cast(panel.FindHandler(SCR_AddonDetailsPanelComponent));
		
		// Scroll widgets
		m_EnabledAddonsScroll = ScrollLayoutWidget.Cast(m_Widgets.m_wEnabledAddonsList.GetParent()); 
		m_DisabledAddonsScroll = ScrollLayoutWidget.Cast(m_Widgets.m_wDisabledAddonsList.GetParent());
		
		CreateNavigationButtons();
		
		RefreshAll();
		
		// Subscribe to addon manager events
		SCR_AddonManager.GetInstance().m_OnAddonOfflineStateChanged.Insert(Callback_OnAddonOfflineStateChanged);
		SCR_AddonManager.GetInstance().m_OnAddonsEnabledChanged.Insert(Callback_OnAddonEnabledStateChanged);

		// Change input schceme check
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		// Listen for inputs while using mouse
		m_MenuActionsComponent = SCR_MenuActionsComponent.FindComponent(GetRootWidget());
		if (m_MenuActionsComponent)
			m_MenuActionsComponent.GetOnAction().Insert(OnActionTriggered);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabRemove()
	{
		super.OnTabRemove();

		// Unsubscribe from addon manager events
		SCR_AddonManager.GetInstance().m_OnAddonOfflineStateChanged.Remove(Callback_OnAddonOfflineStateChanged);
		SCR_AddonManager.GetInstance().m_OnAddonsEnabledChanged.Remove(Callback_OnAddonEnabledStateChanged);
		
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		if (m_MenuActionsComponent)
			m_MenuActionsComponent.ActivateActions();
		
		RefreshAll();
		
		SCR_AnalyticsApplication.GetInstance().WorkshopSetTab(SCR_EAnalyticsWorkshopTab.MOD_MANAGER);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();

		if (m_MenuActionsComponent)
			m_MenuActionsComponent.DeactivateActions();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		if (m_bShown)
			FocusLastWidget();

		super.OnMenuFocusGained();
		
		if (m_bShown && m_MenuActionsComponent)
			m_MenuActionsComponent.ActivateActions();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		
		if (m_bShown && m_MenuActionsComponent)
			m_MenuActionsComponent.ActivateActions();
	}
	
	// --- Protected ---
	//------------------------------------------------------------------------------------------------
	//! Refreshes all lists
	protected void RefreshAll()
	{
		m_aEntriesComponents.Clear();

		// Get offline items from API
		array<WorkshopItem> rawWorkshopItems = {};
		GetGame().GetBackendApi().GetWorkshop().GetOfflineItems(rawWorkshopItems);

		// Register items in Addon Manager
		array<SCR_WorkshopItem> itemsRegistered = {};
		array<SCR_WorkshopItem> saves = {};
		
		foreach (WorkshopItem i : rawWorkshopItems)
		{
			SCR_WorkshopItem itemRegistered = SCR_AddonManager.GetInstance().Register(i);
			
			// Separeta save
			if (itemRegistered.IsWorldSave())
			{
				if (!itemRegistered.GetWorkshopItem().GetActiveRevision())
					continue;
				
				saves.Insert(itemRegistered);
				continue;
			}
			
			// Addons
			itemsRegistered.Insert(itemRegistered);
		}
		
		// Sort by name...
		SCR_Sorting<SCR_WorkshopItem, SCR_CompareWorkshopItemName>.HeapSort(itemsRegistered);
		SCR_Sorting<SCR_WorkshopItem, SCR_CompareWorkshopItemName>.HeapSort(saves);
		
		// Convert back to array<ref ...>
		array<ref SCR_WorkshopItem> itemsSorted = {};
		foreach (SCR_WorkshopItem i : itemsRegistered)
		{
			itemsSorted.Insert(i);
		}
		
		foreach (SCR_WorkshopItem i : saves)
		{
			itemsSorted.Insert(i);
		}

		array<ref SCR_WorkshopItem> enabledItems = SCR_AddonManager.SelectItemsBasic(itemsSorted, EWorkshopItemQuery.ENABLED);
		array<ref SCR_WorkshopItem> disabledItems = SCR_AddonManager.SelectItemsBasic(itemsSorted, EWorkshopItemQuery.NOT_ENABLED);

		CreateListLines(m_Widgets.m_wEnabledAddonsList, m_EnabledAddonsScroll, enabledItems, m_aEntriesEnabled);
		CreateListLines(m_Widgets.m_wDisabledAddonsList, m_DisabledAddonsScroll, disabledItems, m_aEntriesDisabled);

		GetGame().GetCallqueue().Call(FocusLastWidget);
	}

	//------------------------------------------------------------------------------------------------
	protected void FocusFirstLine()
	{
		Widget focus;

		if (m_Widgets.m_wEnabledAddonsPanel && m_Widgets.m_wEnabledAddonsPanel.GetChildren())
			focus = m_Widgets.m_wEnabledAddonsList.GetChildren();
		else if (m_Widgets.m_wDisabledAddonsPanel && m_Widgets.m_wDisabledAddonsPanel.GetChildren())
			focus = m_Widgets.m_wDisabledAddonsPanel.GetChildren();

		if (!focus || !focus.IsFocusable())
		{
			if (!m_aEntriesDisabled.IsEmpty())
				focus = m_aEntriesDisabled[0];
			else if (!m_aEntriesEnabled.IsEmpty())
				focus = m_aEntriesEnabled[0];
			// Fallback to presets button, which is always present
			else
				focus = m_Widgets.m_wButtonEnableAll;
		}

		GetGame().GetWorkspace().SetFocusedWidget(focus);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateListLines(VerticalLayoutWidget vLayout, ScrollLayoutWidget scroll, array<ref SCR_WorkshopItem> items, out array<Widget> entries)
	{
		// Store scroll pos
		float scrollx, scrolly;
		if (scroll)
			scroll.GetSliderPos(scrollx, scrolly);

		// Clear entries
		foreach (Widget entry : entries)
		{
			vLayout.RemoveChild(entry);
		}
				
		entries.Clear();
	
		foreach (SCR_WorkshopItem item : items)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sLineLayout, vLayout);
			SCR_WorkshopAddonLineComponent comp = SCR_WorkshopAddonLineComponent.Cast(w.FindHandler(SCR_WorkshopAddonLineComponent));
			comp.Init(item);
			comp.GetOnEnableButton().Insert(OnEnableButton);
			comp.GetOnDisableButton().Insert(OnDisableButton);
			comp.GetOnFocus().Insert(OnLineFocus);
			comp.GetOnFocusLost().Insert(OnLineFocusLost);

			entries.Insert(w);

			// Store widgets and component
			m_aEntriesComponents.Insert(item, comp);
		}

		// Restore scroll pos
		if (scroll)
			scroll.SetSliderPos(scrollx, scrolly);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create navigation button for actions
	protected void CreateNavigationButtons()
	{
		m_NavOpenDetails = CreateNavigationButton(UIConstants.MENU_ACTION_SELECT, "#AR-Workshop_Details_MenuTitle", true, false);
		if (m_NavOpenDetails)
			m_NavOpenDetails.m_OnActivated.Insert(OnNavOpenDetailsActivated);
		
		m_NavEnableAll = CreateNavigationButton(UIConstants.MENU_ACTION_ENABLE_ALL, "#AR-Workshop_TabName_All", true, false);
		if (m_NavEnableAll)
			m_NavEnableAll.m_OnActivated.Insert(OnNavEnableAllActivated);
		
		m_NavEnable = CreateNavigationButton(UIConstants.MENU_ACTION_ENABLE, SCR_WorkshopUiCommon.LABEL_ENABLE, true, false);
		if (m_NavEnable)
			m_NavEnable.m_OnActivated.Insert(OnNavEnableActivated);
		
		m_NavDeleteAll = CreateNavigationButton(ACTION_DELETE_ALL, "#AR-ModManager_DeleteAll", true, false);
		if (m_NavDeleteAll)
			m_NavDeleteAll.m_OnActivated.Insert(OnNavDeleteAll);
		
		m_NavDelete = CreateNavigationButton(ACTION_DELETE, SCR_WorkshopUiCommon.LABEL_DELETE, true, false);
		if (m_NavDelete)
			m_NavDelete.m_OnActivated.Insert(OnNavDeleteActivated);
			
		m_NavUpdate = CreateNavigationButton(UIConstants.MENU_ACTION_SELECT_HOLD, SCR_WorkshopUiCommon.LABEL_UPDATE, true, false);
		if (m_NavUpdate)
			m_NavUpdate.m_OnActivated.Insert(OnNavUpdateActivate);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateNavigationButtons(bool show = true)
	{
		show = show && GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE;
		
		// Enable
		if (m_NavEnable)
		{
			SetNavigationButtonVisibile(m_NavEnable, show, false);

			if (show)
			{
				if (m_bIsFocusedEnabled)
					m_NavEnable.SetLabel(SCR_WorkshopUiCommon.LABEL_DISABLE);
				else
					m_NavEnable.SetLabel(SCR_WorkshopUiCommon.LABEL_ENABLE);
				
				bool issues = m_FocusedAddonLine && !m_FocusedAddonLine.CanBeEnabled();
				m_NavEnable.SetEnabled(!issues || m_bIsFocusedEnabled, false);
				
				if (issues && !m_bIsFocusedEnabled)
					m_NavEnable.SetTexture(UIConstants.ICONS_IMAGE_SET, GetEnableErrorTexture(m_FocusedAddonLine), Color.FromInt(UIColors.WARNING_DISABLED.PackToInt()));
				else
					m_NavEnable.ResetTexture();
			}
		}

		// Details
		if (m_NavOpenDetails)
			SetNavigationButtonVisibile(m_NavOpenDetails, show, false);

		// Enable All
		if (m_NavEnableAll)
		{
			SetNavigationButtonVisibile(m_NavEnableAll, show, false);

			if (show)
			{
				if (m_bIsFocusedEnabled)
					m_NavEnableAll.SetLabel("#AR-Workshop_DisableAll");
				else
					m_NavEnableAll.SetLabel("#AR-ServerHosting_EnableAll");
			}
		}

		SCR_WorkshopItem item;
		if (m_FocusedAddonLine)
			item = m_FocusedAddonLine.GetWorkshopItem();
		
		bool downloading = SCR_WorkshopUiCommon.IsDownloadingAddon(item);
		
		// Update
		if (m_NavUpdate)
		{			
			bool showNavUpdate = true;
			
			if (item)
			{			
				if (downloading)
					m_NavUpdate.SetLabel(SCR_WorkshopUiCommon.LABEL_CANCEL);
				
				else if (item.GetUpdateAvailable())
					m_NavUpdate.SetLabel(SCR_WorkshopUiCommon.LABEL_UPDATE);

				else if (item.GetAnyDependencyMissing() || item.GetAnyDependencyUpdateAvailable())
					showNavUpdate = false;
	
				else
				{
					showNavUpdate = false;
					m_NavUpdate.SetLabel(SCR_WorkshopUiCommon.LABEL_UPDATE);
				}
			}
			
			SetNavigationButtonVisibile(m_NavUpdate, showNavUpdate && show, false);
		}
		
		// Delete
		if (m_NavDelete)
		{
			bool visible = !downloading && show;
			
			SetNavigationButtonVisibile(m_NavDelete, visible, false);
			
			if (visible)
			{
				bool disable = m_FocusedAddonLine && m_FocusedAddonLine.GetErrorFlags() & SCR_EAddonLineErrorFlags.DOWNLOAD_ISSUES;
				
				// TODO: hack! Download incomplete preventing addon from being deleted must be fixed on backend side
				m_NavDelete.SetEnabled(!disable);
				
				if (disable)
					m_NavDelete.SetTexture(UIConstants.ICONS_IMAGE_SET, GetErrorTexture(m_FocusedAddonLine), UIColors.WARNING_DISABLED);
				else
					m_NavDelete.ResetTexture();
			}
		}
		
		// Delete All
		if (m_NavDeleteAll)
			SetNavigationButtonVisibile(m_NavDeleteAll, show, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetEnableErrorTexture(SCR_WorkshopAddonLineComponent line)
	{
		if (line && (line.GetErrorFlags() & SCR_EAddonLineErrorFlags.MISSING_DEPENDENCIES))
			return SCR_WorkshopUiCommon.ICON_DEPENDENCIES;

		return GetErrorTexture(line);
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetErrorTexture(SCR_WorkshopAddonLineComponent line)
	{
		if (line && line.GetErrorFlags() & SCR_EAddonLineErrorFlags.DOWNLOAD_ISSUES)
			return SCR_WorkshopUiCommon.DOWNLOAD_STATE_ICON_DOWNLOADING;
		
		if (line && line.GetErrorFlags() & SCR_EAddonLineErrorFlags.REVISION_AVAILABILITY_ISSUE)
		{
			SCR_WorkshopItem item = line.GetWorkshopItem();
			if (item)
				return SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorTexture(item.GetWorkshopItem());
		}

		if (line && line.GetErrorFlags() & SCR_EAddonLineErrorFlags.ITEM_ISSUES)
			return SCR_WorkshopUiCommon.ICON_DOWNLOAD;
		
		return UIConstants.ICON_NOT_AVAILABLE;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnNavEnableActivated()
	{
		if (!m_FocusedAddonLine)
			return;

		SCR_WorkshopAddonLineComponent line = ClosestAddonLine(m_FocusedAddonLine.GetRootWidget(), !m_bIsFocusedEnabled);

		if (line)
			m_LastEnabledItem = line.GetWorkshopItem();
		else
			m_LastEnabledItem = m_FocusedAddonLine.GetWorkshopItem();
		
		// Actions
		if (m_FocusedAddonLine.IsEnabled())
			m_FocusedAddonLine.OnDisableButton();
		else
			m_FocusedAddonLine.OnEnableButton();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNavEnableAllActivated()
	{
		if (m_FocusedAddonLine)
			m_LastEnabledItem = m_FocusedAddonLine.GetWorkshopItem();

		if (m_bIsFocusedEnabled)
			EnableAll(false);
		else
			EnableAll(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Find last enabled/disabled item by workshop item in other list
	protected void FocusLastWidget()
	{
		if (!m_LastEnabledItem)
			FocusFirstLine();

		array<Widget> list = {};

		// Select list
		if (!m_bIsFocusedEnabled)
		{
			if (!m_aEntriesDisabled.IsEmpty())
				list = m_aEntriesDisabled;
			else
				list = m_aEntriesEnabled;
		}
		else
		{
			if (!m_aEntriesEnabled.IsEmpty())
				list = m_aEntriesEnabled;
			else
				list = m_aEntriesDisabled;
		}

		// Search list
		for (int i = 0, count = list.Count(); i < count; i++)
		{
			SCR_WorkshopAddonLineComponent comp = SCR_WorkshopAddonLineComponent.Cast(list[i].FindHandler(SCR_WorkshopAddonLineComponent));
			if (!comp)
				continue;

			if (comp.GetWorkshopItem() == m_LastEnabledItem)
			{
				ApplyLineFocus(list[i], comp);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyLineFocus(Widget w, SCR_WorkshopAddonLineComponent comp)
	{
		GetGame().GetWorkspace().SetFocusedWidget(w);
		m_FocusedAddonLine = comp;
	}

	//------------------------------------------------------------------------------------------------
	//! Return closest workshop item, first check down, then up if at the end of the list
	protected SCR_WorkshopAddonLineComponent ClosestAddonLine(Widget line, bool enabled)
	{
		array<Widget> list = {};

		if (enabled)
			list = m_aEntriesDisabled;
		else
			list = m_aEntriesEnabled;

		// Single entry?
		if (list.Count() == 1)
			return SCR_WorkshopAddonLineComponent.Cast(line.FindHandler(SCR_WorkshopAddonLineComponent));

		int lineId = list.Find(line);

		// Not found?
		if (lineId == -1)
			return null;

		Widget closestLine;

		// Last?
		if (lineId == list.Count() -1)
		{
			// Get upper
			closestLine = list[lineId - 1];
		}
		else
		{
			// Get lower
			closestLine = list[lineId + 1];
		}

		return SCR_WorkshopAddonLineComponent.Cast(closestLine.FindHandler(SCR_WorkshopAddonLineComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNavDeleteActivated()
	{
		if (!m_FocusedAddonLine)
			return;
		
		SCR_WorkshopAddonLineComponent line = ClosestAddonLine(m_FocusedAddonLine.GetRootWidget(), !m_bIsFocusedEnabled);

		if (line)
			m_LastEnabledItem = line.GetWorkshopItem();
		else
			m_LastEnabledItem = m_FocusedAddonLine.GetWorkshopItem();

		if (m_FocusedAddonLine)
		{
			SCR_WorkshopItem item;
			if (m_FocusedAddonLine)
				item = m_FocusedAddonLine.GetWorkshopItem();
		
			if (SCR_WorkshopUiCommon.IsDownloadingAddon(item))
				return;

			SCR_DeleteAddonDialog dialog = SCR_DeleteAddonDialog.CreateDeleteAddon(item);
			dialog.m_OnClose.Insert(OnDeleteDialogClose);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDeleteDialogClose(SCR_ConfigurableDialogUi dialog)
	{
		GetGame().GetCallqueue().Call(FocusLastWidget);
		dialog.m_OnClose.Remove(OnDeleteDialogClose);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNavOpenDetailsActivated()
	{
		if (m_FocusedAddonLine)
			m_FocusedAddonLine.OnOpenDetailsButton();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNavUpdateActivate()
	{
		if (!m_FocusedAddonLine)
			return;

		SCR_WorkshopItem item = m_FocusedAddonLine.GetWorkshopItem();

		// Update
		if (item && item.GetUpdateAvailable())
			m_FocusedAddonLine.OnUpdateButton();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNavDeleteAll()
	{
		if (!m_FocusedAddonLine)
			return;
		
		if (m_FocusedAddonLine.IsEnabled())
			OnDeleteAllEnabled();
		else
			OnDeleteAllDisabled();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnValidateAllEnabled()
	{		
		SCR_ValidateRepair_Dialog dialogValidator = CreateValidateDialog();
		dialogValidator.LoadAddons(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnValidateAllDisabled()
	{		
		SCR_ValidateRepair_Dialog dialogValidator = CreateValidateDialog();
		dialogValidator.LoadAddons(false);
	}
	
	//-----------------------------------------------------------------------------------------------
	static SCR_ValidateRepair_Dialog CreateValidateDialog()
	{
		return SCR_CommonDialogs.CreateValidateRepairDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDeleteAllEnabled()
	{
		if (m_aEntriesEnabled.IsEmpty())
			return;
		
		SCR_DeleteAddonsListDialog dialog = SCR_DeleteAddonsListDialog.CreateDialog(GetItemsFromLines(m_aEntriesEnabled));
		dialog.m_OnClose.Insert(OnDeleteDialogClose);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDeleteAllDisabled()
	{
		if (m_aEntriesDisabled.IsEmpty())
			return;
		
		SCR_DeleteAddonsListDialog dialog = SCR_DeleteAddonsListDialog.CreateDialog(GetItemsFromLines(m_aEntriesDisabled));
		dialog.m_OnClose.Insert(OnDeleteDialogClose);
	}
	
	//------------------------------------------------------------------------------------------------
	protected array<SCR_WorkshopItem> GetItemsFromLines(array<Widget> lines)
	{
		array<SCR_WorkshopItem> items = {};
		
		SCR_AddonLineBaseComponent comp;
		SCR_WorkshopItem item;
		
		foreach (Widget line : lines)
		{
			comp = SCR_AddonLineBaseComponent.Cast(line.FindHandler(SCR_AddonLineBaseComponent));
			if (!comp)
				continue;
			
			item =	comp.GetWorkshopItem();
			if (!item)
				continue;
			
			items.Insert(item);
		}
		
		return items;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DeleteAll(array<Widget> lines)
	{
		SCR_AddonLineBaseComponent comp;
		SCR_WorkshopItem item;
		
		foreach (Widget line : lines)
		{
			comp = SCR_AddonLineBaseComponent.Cast(line.FindHandler(SCR_AddonLineBaseComponent));
			if (!comp)
				continue;
			
			item =	comp.GetWorkshopItem();
			if (!item)
				continue;
			
			if (item.GetEnabled())
				SCR_AddonManager.GetInstance().GetPresetStorage().ClearUsedPreset();
		
			if (item.GetSubscribed())
				item.SetSubscribed(false);
			
			item.DeleteLocally();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineFocus(SCR_ScriptedWidgetComponent component)
	{
		SCR_WorkshopAddonLineComponent comp = SCR_WorkshopAddonLineComponent.Cast(component);
		
		if (!comp)
			return;

		m_FocusedAddonLine = comp;
		
		SCR_WorkshopItem item = comp.GetWorkshopItem();
		if (item)
			item.m_OnCanceled.Insert(OnDownloadCanceled);
		
		m_bIsFocusedEnabled = m_FocusedAddonLine.IsEnabled();
		
		UpdateNavigationButtons();
		
		if (m_AddonInfoPanel)
			m_AddonInfoPanel.SetWorkshopItem(item);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineFocusLost(SCR_ScriptedWidgetComponent component)
	{
		SCR_WorkshopAddonLineComponent comp = SCR_WorkshopAddonLineComponent.Cast(component);
		if (comp)
		{
			SCR_WorkshopItem item = comp.GetWorkshopItem();
			if (item)
				item.m_OnCanceled.Remove(OnDownloadCanceled);
		}
		
		m_FocusedAddonLine = null;

		UpdateNavigationButtons(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnActionTriggered(string name, float multiplier)
	{
		if (GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE)
			return;
		
		switch(name)
		{
			case UIConstants.MENU_ACTION_ENABLE_ALL: 	OnNavEnableAllActivated(); 	break;
			case UIConstants.MENU_ACTION_ENABLE:		OnNavEnableActivated(); 	break;
			case ACTION_DELETE:							OnNavDeleteActivated(); 	break;
			case UIConstants.MENU_ACTION_SELECT_HOLD:	OnNavUpdateActivate(); 		break;
			case ACTION_DELETE_ALL:						OnNavDeleteAll();			break;
		}
	}
	
	// --- Line Buttons ---
	//------------------------------------------------------------------------------------------------
	protected void OnEnableButton(SCR_ScriptedWidgetComponent component)
	{
		SCR_AddonLineBaseComponent comp = SCR_AddonLineBaseComponent.Cast(component);
		if (!comp)
			return;
		
		SCR_WorkshopItem item = comp.GetWorkshopItem();
		SCR_WorkshopUiCommon.OnEnableAddonButton(item);

		item.SetEnabled(true);

		// Enable dependencies
		array<ref SCR_WorkshopItem> dependencies = item.GetLatestDependencies();
		foreach (SCR_WorkshopItem dep : dependencies)
		{
			dep.SetEnabled(true);
		}

		// Clear preset name
		SCR_AddonManager.GetInstance().GetPresetStorage().ClearUsedPreset();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDisableButton(SCR_ScriptedWidgetComponent component)
	{
		SCR_AddonLineBaseComponent comp = SCR_AddonLineBaseComponent.Cast(component);
		if (!comp)
			return;
		
		SCR_WorkshopItem item = comp.GetWorkshopItem();
		SCR_WorkshopUiCommon.OnEnableAddonButton(item);

		item.SetEnabled(false);

		// Clear preset name
		SCR_AddonManager.GetInstance().GetPresetStorage().ClearUsedPreset();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnToolsButton()
	{
		SCR_CommonDialogs.CreateModPresetsDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnButtonEnableAll()
	{
		EnableAll(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonDisableAll()
	{
		EnableAll(false);
	}

	//------------------------------------------------------------------------------------------------
	// By enable arg move all enabled/disabled mods to opposite state
	protected void EnableAll(bool enable)
	{
		array<ref SCR_WorkshopItem> addons = SCR_AddonManager.GetInstance().GetOfflineAddons();
	
		if (enable)
		{
			array<ref SCR_WorkshopItem> addonsToEnable = {};
			
			foreach (SCR_WorkshopItem addon : addons)
			{
				SCR_WorkshopAddonLineComponent line = m_aEntriesComponents.Get(addon);
				if (!line || !line.CanBeEnabled())
					continue;
				
				addonsToEnable.Insert(addon);
			}
			
			addons = addonsToEnable;
		}

		SCR_AddonManager addonsManager = SCR_AddonManager.GetInstance();

		addonsManager.EnableMultipleAddons(addons, enable);
		addonsManager.GetEventOnAllAddonsEnabled().Insert(OnAllAddonsEnabled);
		m_bIsEnablingAddons = true;

		if (m_DisabledAddonsScroll)
			m_DisabledAddonsScroll.SetSliderPos(0, 0);
		
		if (m_EnabledAddonsScroll)
			m_EnabledAddonsScroll.SetSliderPos(0, 0);

		// Clear preset name
		SCR_AddonManager.GetInstance().GetPresetStorage().ClearUsedPreset();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAllAddonsEnabled()
	{
		m_bIsEnablingAddons = false;
		RefreshAll();

		GetGame().GetCallqueue().CallLater(FocusFirstLine);
	}

	//------------------------------------------------------------------------------------------------
	//! Called by SCR_AddonManager when some addon is downloaded or uninstalled
	protected void Callback_OnAddonOfflineStateChanged(SCR_WorkshopItem item, bool newState)
	{
		// Some addon was installed or uninstalled, we must refresh the page
		if (!m_bIsEnablingAddons)
			RefreshAll();
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnAddonEnabledStateChanged()
	{
		// Something got enabled or disabled, refresh the page
		if (!m_bIsEnablingAddons)
			RefreshAll();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDownloadCanceled(SCR_WorkshopItem item)
	{
		// Canceling a download means removing local data as well, so we need to get rid of the line
		if (!m_bIsEnablingAddons)
			RefreshAll();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		if (!isGamepad)
			return;

		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (!focused)
			return;

		if (!focused.FindHandler(SCR_WorkshopAddonLineComponent))
			FocusFirstLine();
	}
}