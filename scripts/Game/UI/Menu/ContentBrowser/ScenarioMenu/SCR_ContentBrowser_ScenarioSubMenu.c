enum EScenarioSubMenuMode
{
	MODE_ALL,		// Show all scenarios
	MODE_SAVES,		// Downloaded saves item
	MODE_FAVOURITE,	// Show only favourite scenarios
	MODE_RECENT,		// Show only recently played scenarios
}

class SCR_ContentBrowser_ScenarioSubMenu : SCR_ContentBrowser_ScenarioSubMenuBase
{
	// This is quite universal and can work in many modes...
	[Attribute("0", UIWidgets.ComboBox, "Mode in which this submenu must work.", "", ParamEnumArray.FromEnum(EScenarioSubMenuMode))]
	EScenarioSubMenuMode m_eMode;

	[Attribute("{723ED8FDA27DD0BB}UI/layouts/Menus/ContentBrowser/ScenariosMenu/ContentBrowser_GMSaveLine.layout", UIWidgets.ResourceNamePicker, ".layout for the scenario lines", params: "layout")]
	protected ResourceName m_sSaveLineLayout;
	
	// Constants
	protected const int RECENTLY_PLAYED_MAX_ENTRIES = 10; // How many recently played missions to show in the recently played tab
	
	// Message tags
	// Those which end with '2' should be used when no content is found due to filters.
	protected const string MESSAGE_TAG_NOTHING_FOUND =		"nothing_found";
	protected const string MESSAGE_TAG_NOTHING_FOUND_2 =	"nothing_found2";
	protected const string MESSAGE_TAG_NOTHING_FAVOURITE =	"nothing_favourite";
	protected const string MESSAGE_TAG_NOTHING_FAVOURITE_2 ="nothing_favourite2";
	protected const string MESSAGE_TAG_NOTHING_RECENT =		"nothing_recent";
	protected const string MESSAGE_TAG_NOTHING_RECENT_2 =	"nothing_recent2";
	protected const string MESSAGE_TAG_NO_SAVE = 			"no_save";
	protected const string MESSAGE_TAG_NO_SAVE_FOUND = 		"no_save_found";
	
	// Other
	protected ref SCR_ContentBrowser_ScenarioSubMenuWidgets m_Widgets = new SCR_ContentBrowser_ScenarioSubMenuWidgets;

	protected int m_iEntriesTotal;
	protected int m_iEntriesCurrent;
	protected static bool m_bIsOpened = false;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();

		m_bIsOpened = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		m_ScenarioDetailsPanel = m_Widgets.m_ScenarioDetailsPanelComponent;
		m_AddonDetailsPanel = m_Widgets.m_AddonDetailsPanelComponent;

		// Try to restore filters
		m_Widgets.m_FilterPanelComponent.TryLoad();

		UpdateScenarioList(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		// Init workshop API
		// We do it on tab show becasue this tab and others persists when all other tabs are closed,
		// But we can switch back to it later, and we must setup the workshop api again
		InitWorkshopApi();
		
		SCR_AnalyticsApplication.GetInstance().ScenariosSetTab(m_eMode);

		m_bIsOpened = true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();

		// Save configuration of filters
		m_Widgets.m_FilterPanelComponent.Save();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		//Bring back focus to the last selected line after closing a pop-up dialog, in order to always have an element focused
		Widget target;

		if (m_LastSelectedLine)
			target = m_LastSelectedLine.GetRootWidget();
		else if (m_Widgets && m_Widgets.m_ScenarioList)
			target = m_Widgets.m_ScenarioList.GetChildren();

		if (target)
			GetGame().GetWorkspace().SetFocusedWidget(target);

		super.OnMenuFocusGained();
	}

	//------------------------------------------------------------------------------------------------
	override void OnScenariosLoadFailed()
	{
		ScenarioEmptyMessage({}, MESSAGE_TAG_NOTHING_FOUND);
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitWidgets()
	{
		super.InitWidgets();

		// We provide scenarioSubMenuRoot as root because the widgets of the layours were exported starting from scenarioSubMenuRoot
		m_Widgets.Init(m_wRoot.FindWidget("scenarioSubMenuRoot"));

		m_Widgets.m_FilterPanelComponent.GetEditBoxSearch().m_OnConfirm.Insert(OnSearchConfirm);
		m_Widgets.m_SortingHeaderComponent.m_OnChanged.Insert(OnSortingHeaderChange);
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateNavigationButtons(bool visible = true)
	{
		visible = GetSelectedLine() && GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE;
		super.UpdateNavigationButtons(visible);
	}

	//------------------------------------------------------------------------------------------------
	override bool Play(MissionWorkshopItem scenario)
	{
		if (!super.Play(scenario))
			return false;

		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ScenarioMenu);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Continue(MissionWorkshopItem scenario)
	{
		if (!super.Continue(scenario))
			return false;

		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ScenarioMenu);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnRestartConfirmed()
	{
		super.OnRestartConfirmed();
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ScenarioMenu);
	}

	//------------------------------------------------------------------------------------------------
	//! Called from scenario line component when scenario state changes
	override void OnScenarioStateChanged(SCR_ContentBrowser_ScenarioLineComponent comp)
	{
		UpdateNavigationButtons();
		super.OnScenarioStateChanged(comp);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected Widget CreateLine(ResourceName layout, Widget parent, MissionWorkshopItem scenario)
	{
		SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(SCR_MissionHeader.ReadMissionHeader(scenario.Id()));
		if (missionHeader && !missionHeader.m_bShowInScenarioMenu)
			return null;
		
		if (WorldSaveItem.Cast(scenario.GetOwner()))
			layout = m_sSaveLineLayout;
		
		Widget w = super.CreateLine(layout, parent, scenario);
		if (!w)
			return null;
		
		WorldSaveItem save = WorldSaveItem.Cast(scenario.GetOwner());
		if (!save)
			return w;
		
		SCR_ContentBrowser_GMSaveLineComponent comp = SCR_ContentBrowser_GMSaveLineComponent.Cast(w.FindHandler(SCR_ContentBrowser_GMSaveLineComponent));
		if (!comp)
			return null;
		
		comp.SetSaveItem(save);
		
		return w;
	}
	
	// ---- PROTECTED ----
	//------------------------------------------------------------------------------------------------
	//! Requests missions from API and shows them in the list
	protected void UpdateScenarioList(bool setNewFocus)
	{
		// Get missions from Workshop API
		array<MissionWorkshopItem> missionItemsAll = {};
		m_WorkshopApi.GetPageScenarios(missionItemsAll, 0, SCR_WorkshopUiCommon.PAGE_SCENARIOS); // Get all missions at once

		// Remove scenarios from disabled addons
		for (int i = missionItemsAll.Count() - 1; i >= 0; i--)
		{
			MissionWorkshopItem m = missionItemsAll[i];
			WorkshopItem addon = m.GetOwner();

			if (!addon)
				continue;

			SCR_WorkshopItem scriptedItem = SCR_AddonManager.GetInstance().GetItem(addon.Id());
			
			if (scriptedItem && scriptedItem.GetAnyDependencyMissing())
				missionItemsAll.Remove(i);
		}

		for (int i = missionItemsAll.Count() - 1; i >= 0; i--)
		{
			if (SCR_ScenarioSequenceProgress.IsScenarioLocked(missionItemsAll[i]))
				missionItemsAll.Remove(i);
		}

		// Store total number of entries
		m_iEntriesTotal = missionItemsAll.Count();

		// Filter items according to current tab mode
		array<MissionWorkshopItem> missionItemsTabFiltered = {};
		switch (m_eMode)
		{
			// Select only fav. missions
			case EScenarioSubMenuMode.MODE_FAVOURITE:
			{
				foreach (MissionWorkshopItem m : missionItemsAll)
				{
					if (m.IsFavorite())
						missionItemsTabFiltered.Insert(m);
				}

				// Bail if there are no favourite missions
				if (ScenarioEmptyMessage(missionItemsTabFiltered, MESSAGE_TAG_NOTHING_FAVOURITE))
					return;

				break;
			}

			// Select only recently played missions
			case EScenarioSubMenuMode.MODE_RECENT:
			{
				// Sort missions by time since last play, select top latest N entries
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionTimeSinceLastPlay>.HeapSort(missionItemsAll, false);

				int i;
				while (missionItemsTabFiltered.Count() < RECENTLY_PLAYED_MAX_ENTRIES && i < missionItemsAll.Count())
				{
					int dt = missionItemsAll[i].GetTimeSinceLastPlay();
					if (dt != -1)
						missionItemsTabFiltered.Insert(missionItemsAll[i]);
					i++;
				}

				// Bail if no recent missions
				if (ScenarioEmptyMessage(missionItemsTabFiltered, MESSAGE_TAG_NOTHING_RECENT))
					return;

				break;
			}
			
			// Select only recently played missions
			case EScenarioSubMenuMode.MODE_SAVES:
			{
				foreach (MissionWorkshopItem m : missionItemsAll)
				{
					WorkshopItem addon = m.GetOwner();
					if (!addon)
						continue;
		
					SCR_WorkshopItem scriptedItem = SCR_AddonManager.GetInstance().GetItem(addon.Id());
					
					WorldSaveItem save;
					if (scriptedItem && scriptedItem.GetWorkshopItem())
						save = WorldSaveItem.Cast(scriptedItem.GetWorkshopItem());
					
					if (save)
						missionItemsTabFiltered.Insert(m);
				}
				
				// Bail if no save
				if (ScenarioEmptyMessage(missionItemsTabFiltered, MESSAGE_TAG_NO_SAVE))
					return;
				
				break;
			}

			// Select all missions
			default:
			{
				if (missionItemsAll.IsEmpty())
				{
					ScenarioList_ClearMissionEntries();

					m_iEntriesCurrent = 0;
					SetPanelsMode(true, MESSAGE_TAG_NOTHING_FOUND);
					return;
				}

				foreach (MissionWorkshopItem m : missionItemsAll)
				{
					WorkshopItem addon = m.GetOwner();
					if (!addon)
					{
						missionItemsTabFiltered.Insert(m);
						continue;
					}
		
					SCR_WorkshopItem scriptedItem = SCR_AddonManager.GetInstance().GetItem(addon.Id());
					
					WorldSaveItem save;
					if (scriptedItem && scriptedItem.GetWorkshopItem())
						save = WorldSaveItem.Cast(scriptedItem.GetWorkshopItem());
					
					if (!scriptedItem || !save)
						missionItemsTabFiltered.Insert(m);
				}
				
				break;
			}
		}
		
		// Filter scenarios based on search string
		string searchStr = m_Widgets.m_FilterPanelComponent.GetEditBoxSearch().GetValue();
		array<MissionWorkshopItem> missionItemsSearched = SearchScenarios(missionItemsTabFiltered, searchStr);

		// Bail if based on search there is no content
		if (missionItemsSearched.IsEmpty())
		{
			ScenarioList_ClearMissionEntries();

			// The message depends on current tab
			string messageTag;
			switch (m_eMode)
			{
				case EScenarioSubMenuMode.MODE_FAVOURITE:
					messageTag = MESSAGE_TAG_NOTHING_FAVOURITE_2;
					break;
				
				case EScenarioSubMenuMode.MODE_SAVES:
					messageTag = MESSAGE_TAG_NO_SAVE_FOUND;
					break;

				case EScenarioSubMenuMode.MODE_RECENT:
					messageTag = MESSAGE_TAG_NOTHING_RECENT_2;
					break;

				default:
					messageTag = MESSAGE_TAG_NOTHING_FOUND_2;
					break;
			}

			m_iEntriesCurrent = 0;
			SetPanelsMode(false, messageTag);
			return;
		}

		// At this point we are sure that there are some missions found
		// Choose sorting based on selected filter
		array<MissionWorkshopItem> missionItems = missionItemsSearched;
		ESortOrder eSortOrder = m_Widgets.m_SortingHeaderComponent.GetSortOrder();
		string currentSortingItem = m_Widgets.m_SortingHeaderComponent.GetSortElementName();

		bool sortOrder = false;
		if (eSortOrder == ESortOrder.DESCENDING)
			sortOrder = true;

		// Apply sorting of headers if required
		if (currentSortingItem.IsEmpty())
		{
			currentSortingItem = "name";
			sortOrder = false;
		}
		switch (currentSortingItem)
		{
			case "name":
			{
				// Sort save by addon name as it is actial name of save
				if (m_eMode == EScenarioSubMenuMode.MODE_SAVES)
				{
					SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionAddonName>.HeapSort(missionItems, sortOrder);
					break;
				}
				
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionName>.HeapSort(missionItems, sortOrder);
				break;
			}
			case "player_count": // sort by player count
			{
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionPlayerCount>.HeapSort(missionItems, sortOrder);
				break;
			}
			case "favourite":
			{
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionFavourite>.HeapSort(missionItems, sortOrder);
				break;
			}
			case "last_played":
			{
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionTimeSinceLastPlay>.HeapSort(missionItems, sortOrder);
				break;
			}
			case "source":
			{
				// Sort save by scenario name as it is used as source indicator
				if (m_eMode == EScenarioSubMenuMode.MODE_SAVES)
				{
					SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionName>.HeapSort(missionItems, sortOrder);
					break;
				}
				
				SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionAddonName>.HeapSort(missionItems, sortOrder);
				break;
			}
		}

		m_iEntriesCurrent = missionItemsSearched.Count();
		SetPanelsMode(false);
		ScenarioList_CreateMissionEntries(missionItems, setNewFocus);
	}

	//------------------------------------------------------------------------------------------------
	protected bool ScenarioEmptyMessage(array<MissionWorkshopItem> mission, string messageTag)
	{
		if (!mission.IsEmpty())
			return false;
		
		ScenarioList_ClearMissionEntries();

		m_iEntriesCurrent = 0;
		SetPanelsMode(true, messageTag);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static bool SearchStringLocalized(string str, string searchStrLower)
	{
		if (str.StartsWith("#"))
		{
			// Check translated lowercased string
			string strTranslated = WidgetManager.Translate(str);
			if (!strTranslated.IsEmpty())
			{
				string strTranslatedLower = strTranslated;
				strTranslatedLower.ToLower();

				if (strTranslatedLower.Contains(searchStrLower))
					return true;
			}
		}
		else
		{
			// Check lowercased string
			string strLower = str;
			strLower.ToLower();

			if (strLower.Contains(searchStrLower))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void ScenarioList_ClearMissionEntries()
	{
		// Delete previous entries in the list
		Widget child = m_Widgets.m_ScenarioList.GetChildren();
		while (child)
		{
			Widget nextChild = child.GetSibling();
			m_Widgets.m_ScenarioList.RemoveChild(child);
			child = nextChild;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Creates lines for missions in the scroll view
	protected void ScenarioList_CreateMissionEntries(array<MissionWorkshopItem> scenarios, bool setNewFocus)
	{
		ScenarioList_ClearMissionEntries();

		if (!CreateLines(scenarios, m_Widgets.m_ScenarioList))
			return;

		// Set focus on first line
		Widget firstLine = m_Widgets.m_ScenarioList.GetChildren();
		if (firstLine && setNewFocus)
		{
			// If this is called instantly, it doesn't set new focus when menu is first opened.
			// Probably because menu opening is handled in event handler of main menu's tile.
			GetGame().GetCallqueue().CallLater(GetGame().GetWorkspace().SetFocusedWidget, 0, false, firstLine, false);
		}

		// Reset scroll position
		m_Widgets.m_ScenarioScroll.SetSliderPos(0, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when user confirms the value in the seacrh string
	protected void OnSearchConfirm(SCR_EditBoxComponent comp, string newValue)
	{
		UpdateScenarioList(false);
		
		SCR_AnalyticsApplication.GetInstance().ScenariosUseSearch();		
	}

	//------------------------------------------------------------------------------------------------
	//! Called when sorting header changes
	protected void OnSortingHeaderChange(SCR_SortHeaderComponent sortHeader)
	{
		UpdateScenarioList(false);
		
		SCR_AnalyticsApplication.GetInstance().ScenariosSetSorting(sortHeader.GetSortElementName());
	}

	// ---- PUBLIC ----
	//------------------------------------------------------------------------------------------------
	void SetPanelsMode(bool showEmptyPanel, string messagePresetTag = string.Empty)
	{
		// Show either main+filter panel or empty panel
		m_Widgets.m_EmptyPanel.SetVisible(showEmptyPanel);
		m_Widgets.m_FilterPanel.SetVisible(!showEmptyPanel);
		m_Widgets.m_LeftPanel.SetVisible(!showEmptyPanel);

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

		// Items Found Message
		m_Widgets.m_FilterPanelComponent.SetItemsFoundMessage(m_iEntriesCurrent, m_iEntriesTotal, m_iEntriesCurrent != m_iEntriesTotal);
	}

	//------------------------------------------------------------------------------------------------
	array<MissionWorkshopItem> SearchScenarios(array<MissionWorkshopItem> scenarios, string searchStr)
	{
		array<MissionWorkshopItem> scenariosOut = {};

		if (!searchStr.IsEmpty())
		{
			string searchStrLower = searchStr;
			searchStrLower.ToLower();
			foreach (MissionWorkshopItem scenario : scenarios)
			{
				// Name
				if (SearchStringLocalized(scenario.Name(), searchStrLower))
				{
					scenariosOut.Insert(scenario);
					continue;
				}

				// Description
				if (SearchStringLocalized(scenario.Description(), searchStrLower))
				{
					scenariosOut.Insert(scenario);
					continue;
				}
			}
		}
		else
		{
			scenariosOut.Copy(scenarios);
		}

		return scenariosOut;
	}
}

// Classes to compare mission headers
// Sort by name
class SCR_CompareMissionName : SCR_SortCompare<MissionWorkshopItem>
{
	//------------------------------------------------------------------------------------------------
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
	{
		string name1 = ResolveName(left.Name());
		string name2 = ResolveName(right.Name());

		if (name1.Compare(name2) == -1)
			return 1;
		else
			return 0;
	}

	//------------------------------------------------------------------------------------------------
	static string ResolveName(string name)
	{
		if (name.StartsWith("#"))
			return WidgetManager.Translate(name);
		else
			return name;
	}
}

// Sort by name
class SCR_CompareMissionAddonName : SCR_SortCompare<MissionWorkshopItem>
{
	//------------------------------------------------------------------------------------------------
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
	{
		WorkshopItem ownerItem = left.GetOwner();

		string name1 = ResolveAddonName(left);
		string name2 = ResolveAddonName(right);

		if (name1.Compare(name2) == -1)
			return 1;
		else
			return 0;
	}

	//------------------------------------------------------------------------------------------------
	static string ResolveAddonName(MissionWorkshopItem scenario)
	{
		WorkshopItem sourceAddon = scenario.GetOwner();
		if (!sourceAddon)
			return WidgetManager.Translate("#AR-Editor_Attribute_OverlayLogo_Reforger");
		else
			return sourceAddon.Name();
	}
}

// Sort by player count
class SCR_CompareMissionPlayerCount : SCR_SortCompare<MissionWorkshopItem>
{
	//------------------------------------------------------------------------------------------------
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
	{
		return left.GetPlayerCount() < right.GetPlayerCount();
	}
}

// Sort by favouriteness
class SCR_CompareMissionFavourite : SCR_SortCompare<MissionWorkshopItem>
{
	//------------------------------------------------------------------------------------------------
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
	{
		return right.IsFavorite();
	}
}

// Sort by time since last played
class SCR_CompareMissionTimeSinceLastPlay : SCR_SortCompare<MissionWorkshopItem>
{
	//------------------------------------------------------------------------------------------------
	override static int Compare(MissionWorkshopItem left, MissionWorkshopItem right)
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
