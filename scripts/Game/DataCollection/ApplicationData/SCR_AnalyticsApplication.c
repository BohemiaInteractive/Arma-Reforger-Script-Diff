[EnumLinear()]
enum SCR_EAnalyticalItemSlotType
{
	HORIZONTAL, // weapons, quick slots
	VERTICAL, // helmet, vest, bag, t-shirt, trausers, etc.
	CHARACTER_STORAGE, // actual items in vests, trausers, etc.
	VICINITY, // loot
	OTHER
}

[EnumLinear()]
enum SCR_EAnalyticsWorkshopTab
{
	MODS,
	MAKE_ARMA_NOT_WAR,
	GAME_SAVES, 
	DOWNLOADED,
	MOD_MANAGER
}

[EnumBitFlag()]
enum SCR_EAnalyticsApplicationLoggingFlags
{
	CONSOLE,
	FILE,
	API
}

[EnumLinear()]
enum SCR_EAnalyticsDataTable
{
	APPLICATION,
	PLAYER,
	SESSION,
	MOD
}

enum SCR_EAnalyticsCourseEndReason : SCR_ETutorialBreakType
{
	COMPLETED
}

class SCR_AnalyticsApplication : WorldSystem
{
	/// Settings
	protected static const string ANALYTICAL_EVENT_SETTINGS_CLOSED = "settingsClosed";
	protected ref SCR_SettingsTimeSpentData m_SettingsData = new SCR_SettingsTimeSpentData();
	protected ref SCR_AnalyticsSettings m_SettingsInteractions = new SCR_AnalyticsSettings();

	/// Scenarios
	protected static const string ANALYTICAL_EVENT_SCENARIOS_MENU_CLOSE = "scenariosMenuClosed";
	protected ref SCR_MenuInteractedData m_ScenariosData = new SCR_MenuInteractedData();
	protected ref SCR_AnalyticsInteractions m_ScenariosInteractions = new SCR_AnalyticsInteractions();

	/// Workshop
	protected static const string ANALYTICAL_EVENT_WORKSHOP_CLOSED = "workshopMenuClosed";
	protected ref SCR_MenuInteractedData m_WorkshopData = new SCR_MenuInteractedData();
	protected ref SCR_AnalyticsInteractions m_WorkshopInteractions = new SCR_AnalyticsInteractions();
	
	// Mod Details
	protected static const string ANALYTICAL_EVENT_MOD_CLOSED = "modMenuClosed";
	protected ref SCR_MenuInteractedData m_ModData = new SCR_MenuInteractedData();
	protected ref SCR_AnalyticsInteractions m_ModInteractions = new SCR_AnalyticsInteractions();

	/// Play Menu
	protected static const string ANALYTICAL_EVENT_PLAY_MENU_CLOSED = "playMenuClosed";
	protected ref SCR_AnalyticsInteractions m_PlayMenuInteractions = new SCR_AnalyticsInteractions();
	protected ref SCR_MenuTimeSpentData m_PlayMenuData = new SCR_MenuTimeSpentData();

	/// Multiplayer Menu
	protected static const string ANALYTICAL_EVENT_MULTIPLAYER_MENU_CLOSED = "multiplayerMenuClosed";
	protected ref SCR_MenuInteractedData m_MultiplayerMenuData = new SCR_MenuInteractedData();
	protected ref SCR_AnalyticsInteractions m_MultiplayerMenuInteractions = new SCR_AnalyticsInteractions();

	/// Field Manual
	protected static const string MANUAL_LOCATION_GAME = "Game";
	protected static const string MANUAL_LOCATION_MAIN_MENU = "Main Menu";
	protected static const string ANALYTICAL_EVENT_FIELD_MANUAL_CLOSED = "manualClosed";
	protected ref SCR_AnalyticsInteractions m_FieldManualInteractions = new SCR_AnalyticsInteractions();
	protected ref SCR_FieldManualTimeSpentData m_FieldManualData = new SCR_FieldManualTimeSpentData();

	/// Inventory
	protected static const string ANALYTICAL_EVENT_INVENTORY_CLOSED = "inventoryClosed";
	protected ref SCR_AnalyticsInventory m_Inventory = new SCR_AnalyticsInventory();
	protected ref SCR_InventoryData m_InventoryData = new SCR_InventoryData();
	protected SCR_EAnalyticalItemSlotType m_eLastInteractedStorage = SCR_EAnalyticalItemSlotType.OTHER;
	protected int m_iLastInteractedSlotId = 0;
	protected ResourceName m_sLastInteractedResource = "";
	protected static const ref array<EMenuAction> MOVING_MENU_ACTIONS
		= { EMenuAction.ACTION_MOVEBETWEEN, EMenuAction.ACTION_DRAGGED, EMenuAction.ACTION_DROPPED, EMenuAction.ACTION_MOVEINSIDE };

	/// Voting
	protected static const string ANALYTICAL_EVENT_VOTE_ENDS = "voteEnd";
	protected ref SCR_VotingResultData m_VotingData = new SCR_VotingResultData();
	
	/// Tutorial
	protected static const string ANALYTICAL_EVENT_COURSE_ENDS = "courseEnd";
	protected ref SCR_CourseEndData m_CourseData = new SCR_CourseEndData();
	protected ref SCR_AnalyticsTimer m_CourseTimer = new SCR_AnalyticsTimer();
	protected SCR_ETutorialCourses m_StartedCourse = 0;
	
	/// Logging
	protected SCR_EAnalyticsApplicationLoggingFlags m_Logging
					= SCR_EAnalyticsApplicationLoggingFlags.CONSOLE
						| SCR_EAnalyticsApplicationLoggingFlags.API;

	/// Sending player events on dedicated severs, hardcoded constant used by Enfusion team.
	/// See https://bohemiainteractive.slack.com/archives/CUDBS472A/p1753345662387439
	protected static const int LOCAL_PLAYER_ID = 8888;

	/// PLAY MENU ///
	
	//------------------------------------------------------------------------------------------------
	//! User opens play menu user interface. Informs analytics we are starting to track actions and
	//! time spent. Use ClosePlayMenu() when user closes the interface to stop and send analytical event.
	//! This clears all previously saved data!
	void OpenPlayMenu()
	{
		m_PlayMenuInteractions.Start();
	}
	
	//------------------------------------------------------------------------------------------------
	//! User closes play menu user interface. Informs analytics we are stopping to track actions and
	//! time spent and sends analytical event. Use OpenPlayMenu() when user opens the interface again.
	void ClosePlayMenu()
	{
		m_PlayMenuInteractions.Close();
	}
	
	/// FIELD MANUAL ///
	
	//------------------------------------------------------------------------------------------------
	//! User opens field manual user interface. Informs analytics we are starting to track actions and
	//! time spent. Use CloseFieldManual() when user closes the interface to stop and send analytical event.
	//! This clears all previously saved data!
	void OpenFieldManual()
	{
		m_FieldManualInteractions.Start();
	}
	
	//------------------------------------------------------------------------------------------------
	//! User closes settings user interface. Informs analytics we are stopping to track actions and
	//! time spent and sends analytical event. Use OpenWorkshop() when user opens the interface again.
	void CloseFieldManual()
	{
		m_FieldManualInteractions.Close();
	}
	
	/// SETTINGS ///

	//------------------------------------------------------------------------------------------------
	//! User opens settings user interface. Informs analytics we are starting to track actions and
	//! time spent. Use CloseSettings() when user closes the interface to stop and send analytical event.
	//! This clears all previously saved data!
	void OpenSettings()
	{
		m_SettingsInteractions.Start();
	}

	//------------------------------------------------------------------------------------------------
	//! User closes settings user interface. Informs analytics we are stopping to track actions and
	//! time spent and sends analytical event. Use OpenWorkshop() when user opens the interface again.
	void CloseSettings()
	{
		m_SettingsData.amt_time_spent = m_SettingsInteractions.GetTimeSpent();
		m_SettingsData.array_settings_changed = m_SettingsInteractions.GetSettingsChanged();
		SendAnalyticalEvent(m_SettingsData, ANALYTICAL_EVENT_SETTINGS_CLOSED, SCR_EAnalyticsDataTable.APPLICATION);
	}

	//------------------------------------------------------------------------------------------------
	//! Used when user manually changes quality preset. If used after OpenSettings() and before
	//! CloseSettings() informs the analytics about the change.
	//! \param[in] index from combobox
	void SetQualityPresetSetting(int index)
	{
		m_SettingsInteractions.SetQualityPreset(index);
	}

	//------------------------------------------------------------------------------------------------
	//! Used when user manually changes hud visibility setting. If used after OpenSettings() and before
	//! CloseSettings() informs the analytics about the change.
	//! \param[in] index from spin box
	void SetHudVisibilitySetting(int index)
	{
		m_SettingsInteractions.SetHudVisibility(index);
	}

	//------------------------------------------------------------------------------------------------
	//! User changes some setting in some category. If used after OpenSettings() and before
	//! CloseSettings() informs the analytics about the change.
	//! \param[in] category to tweak setting in (e.g., "Audio", "Interface")
	//! \param[in] action concrete setting which will be changed (e.g., "Effect Volume")
	void ChangeSetting(string category, string action)
	{
		m_SettingsInteractions.ChangeSetting(category, action);
	}

	//------------------------------------------------------------------------------------------------
	//! User tweaks keybind for a specific action. If used after OpenSettings() and before
	//! CloseSettings() informs the analytics about the change.
	//! \param[in] action (e.g., "Forward", "Aim")
	void ChangeKeybind(string action, string preset)
	{
		m_SettingsInteractions.ChangeSetting("Controls", string.Format("%1_%2", action, preset));
	}

	//------------------------------------------------------------------------------------------------
	//! User presses "Reset All" button. If used after OpenSettings() and before
	//! CloseSettings() informs the analytics about the change.
	void ResetAllKeybinds()
	{
		m_SettingsInteractions.UseResetAllButton();
	}

	/// MULTIPLAYER MENU ///

	//------------------------------------------------------------------------------------------------
	//! User opens multiplayer menu user interface. Informs analytics we are starting to track actions and
	//! time spent. Use CloseMultiplayerMenu() when user closes the interface to stop and send analytical event.
	//! This clears all previously saved data!
	void OpenMultiplayerMenu()
	{
		m_MultiplayerMenuInteractions.Start();
	}

	//------------------------------------------------------------------------------------------------
	//! User closes server browser user interface. Informs analytics we are stopping to track actions and
	//! time spent and sends analytical event. Use OpenMultiplayerMenu() when user opens the interface again.
	void CloseMultiplayerMenu()
	{
		m_MultiplayerMenuInteractions.Close();	
	}

	//------------------------------------------------------------------------------------------------
	//! User chooses to go to a different tab in server browser. If used after OpenMultiplayerMenu()
	//! and before CloseSettings() informs the analytics about the change.
	//! \param[in] tab
	void MultiplayerMenuSetTab(SCR_EServerBrowserTabs tab)
	{
		m_MultiplayerMenuInteractions.SetTab(typename.EnumToString(SCR_EServerBrowserTabs, tab));
	}

	//------------------------------------------------------------------------------------------------
	//! User sets a different sorting of servers in server browser. If used after OpenMultiplayerMenu()
	//! and before CloseSettings() informs the analytics about the change.
	//! \param[in] sorting (e.g., "Name", "Ping")
	void MultiplayerMenuSetSorting(string sorting)
	{
		m_MultiplayerMenuInteractions.SetSorting(sorting);
	}

	//------------------------------------------------------------------------------------------------
	//! User sets a different filter of servers in server browser. If used after OpenMultiplayerMenu()
	//! and before CloseSettings() informs the analytics about the change.
	//! \param[in] filterCategory (e.g., "Population", "Ping")
	//! \param[in] filterName (e.g., "Low", "Medium", "Public", "Private")
	void MultiplayerMenuSetFilter(string filterCategory, string filterName)
	{
		m_MultiplayerMenuInteractions.SetFilter(filterCategory, filterName);
	}

	//------------------------------------------------------------------------------------------------
	//! User uses search box in server browser. If used after OpenMultiplayerMenu()
	//! and before CloseSettings() informs the analytics about the change.
	void MultiplayerMenuUseSearch()
	{
		m_MultiplayerMenuInteractions.UseSearch();
	}

	//------------------------------------------------------------------------------------------------
	//! User uses favorite button in server browser. If used after OpenMultiplayerMenu()
	//! and before CloseSettings() informs the analytics about the change.
	void MultiplayerMenuUseFavorite()
	{
		m_MultiplayerMenuInteractions.UseFavorite();
	}

	//------------------------------------------------------------------------------------------------
	//! User uses filter toggle button in server browser. If used after OpenMultiplayerMenu()
	//! and before CloseSettings() informs the analytics about the change.
	void MultiplayerMenuUseFilterOn()
	{
		m_MultiplayerMenuInteractions.UseFilterOn();
	}

	/// WORKSHOP ///

	//------------------------------------------------------------------------------------------------
	//! User opens workshop user interface. Informs analytics we are starting to track actions and
	//! time spent. Use CloseWorkshop() when user closes the interface to stop and send analytical event.
	//! This clears all previously saved data!
	void OpenWorkshop()
	{
		m_WorkshopInteractions.Start();
	}

	//------------------------------------------------------------------------------------------------
	//! User closes workshop user interface. Informs analytics we are stopping to track actions and
	//! time spent and sends analytical event. Use OpenWorkshop() when user opens the interface again.
	void CloseWorkshop()
	{
		m_WorkshopInteractions.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	//! User goes to a different tab in workshop. If used after OpenWorkshop() and before CloseWorkshop()
	//! informs the analytics about the change.
	//! \param[in] tab
	void WorkshopSetTab(SCR_EAnalyticsWorkshopTab tab)
	{
		m_WorkshopInteractions.SetTab(typename.EnumToString(SCR_EAnalyticsWorkshopTab, tab));
	}

	//------------------------------------------------------------------------------------------------
	//! User goes to a different tab in workshop. If used after OpenWorkshop() and before CloseWorkshop()
	//! informs the analytics about the change.
	//! \param[in] sorting
	void WorkshopSetSorting(string sorting)
	{
		m_WorkshopInteractions.SetSorting(sorting);
	}

	//------------------------------------------------------------------------------------------------
	//! User goes to a sets a different filter in workshop. If used after OpenWorkshop() and before
	//! CloseWorkshop() informs the analytics about the change.
	//! \param[in] filterCategory
	//! \param[in] filterName
	void WorkshopSetFilter(string filterCategory, string filterName)
	{
		m_WorkshopInteractions.SetFilter(filterCategory, filterName);
	}

	//------------------------------------------------------------------------------------------------
	//! User uses search box in the workshop. If used after OpenWorkshop() and before CloseWorkshop()
	//! informs the analytics about the change.
	void WorkshopUseSearch()
	{
		m_WorkshopInteractions.UseSearch();
	}

	//------------------------------------------------------------------------------------------------
	//! User goes to a uses filter toggle in workshop. If used after OpenWorkshop() and before
	//! CloseWorkshop() informs the analytics about the change.
	void WorkshopUseFilterOn()
	{
		m_WorkshopInteractions.UseFilterOn();
	}
	
	/// MOD DETAILS	///

	//------------------------------------------------------------------------------------------------
	//! User opens mod details user interface. Informs analytics we are starting to track actions and
	//! time spent. Use CloseModDetails() when user closes the interface to stop and send analytical event.
	//! This clears all previously saved data!
	void OpenModDetails()
	{
		m_ModInteractions.Start();
	}
	
	//------------------------------------------------------------------------------------------------
	//! User closes workshop user interface. Informs analytics we are stopping to track actions and
	//! time spent and sends analytical event. Use OpenWorkshop() when user opens the interface again.
	void CloseModDetails()
	{
		m_ModInteractions.Close();
	}

	//------------------------------------------------------------------------------------------------
	//! User goes to a particular tab in mod details panel in workshop. If used after OpenWorkshop() 
	//! and before CloseWorkshop() informs the analytics about the change.
	//! \param[in] tab
	void ModDetailsSetTab(SCR_EModDetailsMenuTabs tab)
	{
		m_ModInteractions.SetTab(typename.EnumToString(SCR_EModDetailsMenuTabs, tab));
	}

	/// SCENARIOS ///

	//------------------------------------------------------------------------------------------------
	//! User opens scenarios user interface. Informs analytics we are starting to track actions and
	//! time spent. Use CloseScenarios() when user closes the interface to stop and send analytical event.
	//! This clears all previously saved data!
	void OpenScenarios()
	{
		m_ScenariosInteractions.Start();
	}

	//------------------------------------------------------------------------------------------------
	//! User closes scenarios user interface. Informs analytics we are stopping to track actions and
	//! time spent and sends analytical event. Use OpenScenarios() when user opens the interface again.
	void CloseScenarios()
	{
		m_ScenariosInteractions.Close();
	}

	//------------------------------------------------------------------------------------------------
	//! User changes tab in scenarios. If used after OpenScenarios() and before CloseScenarios()
	//! informs the analytics about the change.
	//! \param[in] tab
	void ScenariosSetTab(EScenarioSubMenuMode tab)
	{
		m_ScenariosInteractions.SetTab(typename.EnumToString(EScenarioSubMenuMode, tab));
	}

	//------------------------------------------------------------------------------------------------
	//! User sets a different sorting in scenarios. If used after OpenScenarios() and before
	//! CloseScenarios() informs the analytics about the change.
	//! \param[in] sorting
	void ScenariosSetSorting(string sorting)
	{
		m_ScenariosInteractions.SetSorting(sorting);
	}

	//------------------------------------------------------------------------------------------------
	//! User uses search box in scenarios. If used after OpenScenarios() and before CloseScenarios()
	//! informs the analytics about the change.
	void ScenariosUseSearch()
	{
		m_ScenariosInteractions.UseSearch();
	}
	
	/// INVENTORY ///
	
	//------------------------------------------------------------------------------------------------
	//! User opens inventory in game with a specific weight. Call CloseInventory() after user closes 
	//! inventory to send analytical event (after its delay runs out).
	//! \param[in] currentWeight current weight upon openning the inventory
	void OpenInventory(float currentWeight)
	{		
		m_Inventory.OpenInventory(currentWeight);
	}
	
	//------------------------------------------------------------------------------------------------
	//! User closes inventory in game with a specific weight. Call after `OpenInventory()`.
	//! Sends analytical event if inventory is not open within ANALYTICS_INVENTORY_DELAY_SECONDS seconds.
	//! \param[in] currentWeight current weight upon closing the inventory
	void CloseInventory(float currentWeight)
	{
		m_Inventory.CloseInventory(currentWeight);
	}
	
	//------------------------------------------------------------------------------------------------
	//! User interacts with some item in inventory. Interaction = Pressing that item. If used after
	//! OpenInventory() and before CloseInventory() informs the analytics about the change.
	void InteractWithItem(SCR_InventorySlotUI focusedItem, SCR_InventorySlotUI selectedItem, EMenuAction action, bool isUsingGamepad)
	{
		// interaction is any move action or item selection on controller
		bool isInteraction = MOVING_MENU_ACTIONS.Contains(action) || (action == EMenuAction.ACTION_SELECT && isUsingGamepad);		
		if ((!focusedItem && !selectedItem) || !isInteraction)
			return;
		
		SCR_InventorySlotUI currentItem;
		if (focusedItem)
			currentItem = focusedItem;
		else
			currentItem = selectedItem;
		
	 	// prevent spamming interactions with the same items
		if (m_eLastInteractedStorage == currentItem.GetAnalyticalItemSlotType() 
			&& m_iLastInteractedSlotId == currentItem.GetStorageUI().GetSlotId(currentItem)
			&& m_sLastInteractedResource == currentItem.GetItemResource())
			return;
		
		m_eLastInteractedStorage = currentItem.GetAnalyticalItemSlotType();
		m_iLastInteractedSlotId = currentItem.GetStorageUI().GetSlotId(currentItem);
		m_sLastInteractedResource = currentItem.GetItemResource();
		
		m_Inventory.InteractWithItem(m_eLastInteractedStorage);
	}

	//------------------------------------------------------------------------------------------------
	//! User uses healing item from inventory. Currently only bandages count as healing. If used after
	//! OpenInventory() and before CloseInventory() informs the analytics about the change.
	void UseHealingFromInventory()
	{
		m_Inventory.UseHealing();
	}

	//------------------------------------------------------------------------------------------------
	//! User uses "Examine" button in inventory. If used after OpenInventory() and before 
	//! CloseInventory() informs the analytics about the change.
	void UseExaminationFromInventory()
	{
		m_Inventory.UseExamination();
	}
	
	/// VOTING ///
	
	//------------------------------------------------------------------------------------------------
	//! Voting about kicking a player ends with a success, inform analytics.
	void VoteToKickSucessful()
	{
		m_VotingData.flag_vote_success = true;
		SendAnalyticalEvent(m_VotingData, ANALYTICAL_EVENT_VOTE_ENDS, SCR_EAnalyticsDataTable.SESSION);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Voting about kicking a player ends unsucessfully, inform analytics.
	void VoteToKickFailed()
	{
		m_VotingData.flag_vote_success = false;
		SendAnalyticalEvent(m_VotingData, ANALYTICAL_EVENT_VOTE_ENDS, SCR_EAnalyticsDataTable.SESSION);
	}
	
	/// COURSE END ///
	
	//------------------------------------------------------------------------------------------------
	//! Player started a course in tutorial, start tracking time.
	void TutorialCourseStarts(SCR_ETutorialCourses course)
	{
		m_StartedCourse = course;
		m_CourseTimer.Start();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Player completed a course, inform analytics.
	//! \param[in] course name to double check we ended the same course as we started
	//! \param[in] reason was it ended successfully or was there some problem
	void TutorialCourseEnds(SCR_ETutorialCourses course, SCR_EAnalyticsCourseEndReason reason)
	{		
		if (course != m_StartedCourse)
		{
			Debug.Error(
				string.Format("Can't send course end to analytics, as course which ended (%1) is not equal to course that was started (%2)!", course, m_StartedCourse)
			);
			return;
		}

		m_StartedCourse = 0;  // reset to prevent multiple same sends
		m_CourseData.amt_time_spent = m_CourseTimer.GetTimeSpent();
		m_CourseData.amt_course_name = typename.EnumToString(SCR_ETutorialCourses, course);
		m_CourseData.amt_course_end_reason = typename.EnumToString(SCR_EAnalyticsCourseEndReason, reason);		
		SendAnalyticalEvent(m_CourseData, ANALYTICAL_EVENT_COURSE_ENDS, SCR_EAnalyticsDataTable.PLAYER);
	}
	
	/// LOGGING ///

	//------------------------------------------------------------------------------------------------
	//! Set a different logging functionality. SCR_EAnalyticsApplicationLoggingFlags is a flagged enum.
	//! \param[in] logging
	void SetLogging(SCR_EAnalyticsApplicationLoggingFlags logging)
	{
		m_Logging = logging;
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieves current logging functionality. SCR_EAnalyticsApplicationLoggingFlags is a flagged enum.
	//! \return SCR_EAnalyticsApplicationLoggingFlags logging
	SCR_EAnalyticsApplicationLoggingFlags GetLogging()
	{
		return m_Logging;
	}

	/// PUBLIC ////

	//------------------------------------------------------------------------------------------------
	//! Returns a pointer to analytical events, mostly from evt_app and some minor events from evt_player
	//! If you want to disable analytics programatically, use SetLogging(SCR_EAnalyticsApplicationLoggingFlags.NONE)
	//! If you want to enable analytics programatically, use for example SetLogging(SCR_EAnalyticsApplicationLoggingFlags.CONSOLE | SCR_EAnalyticsApplicationLoggingFlags.API);
	static SCR_AnalyticsApplication GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;

		return SCR_AnalyticsApplication.Cast(world.FindSystem(SCR_AnalyticsApplication));
	}

	//------------------------------------------------------------------------------------------------
	//! Create new analytics module, gets automatically called if class is added to ChimeraSystemsConfig.conf
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo.SetAbstract(false);
		outInfo.SetLocation(WorldSystemLocation.Both);
		outInfo.AddPoint(WorldSystemPoint.Frame);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update our interactions based on current time slice
	override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		super.OnUpdatePoint(args);
		
		float deltaSeconds = args.GetTimeSliceSeconds();		
		UpdateData(m_ScenariosInteractions, m_ScenariosData, ANALYTICAL_EVENT_SCENARIOS_MENU_CLOSE, deltaSeconds);
		UpdateData(m_WorkshopInteractions, m_WorkshopData, ANALYTICAL_EVENT_WORKSHOP_CLOSED, deltaSeconds);
		UpdateData(m_MultiplayerMenuInteractions, m_MultiplayerMenuData, ANALYTICAL_EVENT_MULTIPLAYER_MENU_CLOSED, deltaSeconds);
		UpdateData(m_ModInteractions, m_ModData, ANALYTICAL_EVENT_MOD_CLOSED, deltaSeconds);		
		UpdateInventory(deltaSeconds);
		UpdatePlayMenu(deltaSeconds);
		UpdateFieldManual(deltaSeconds);
	}
	
	/// PROTECTED ///
	
	//------------------------------------------------------------------------------------------------
	//! Updates generic interactions and send them it to analytics when time is right.
	protected void UpdateData(SCR_AnalyticsInteractions interactions, SCR_MenuInteractedData data, string analytical_event, float deltaSeconds)
	{
		interactions.Update(deltaSeconds);
		if (interactions.IsReadyToSend())
		{
			data.amt_time_spent = interactions.GetTimeSpent();
			data.array_menu_interactions = interactions.GetInteractions();
			data.cnt_menu_reopen = interactions.GetReopenTimes();
			SendAnalyticalEvent(data, analytical_event, SCR_EAnalyticsDataTable.APPLICATION);
			interactions.ResetReadyToSend();
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates inventory interactions and send them it to analytics when time is right.
	protected void UpdateInventory(float deltaSeconds)
	{
		m_Inventory.Update(deltaSeconds);
		if (m_Inventory.IsReadyToSend())
		{
			m_InventoryData = m_Inventory.GetClonedData();
			SendAnalyticalEvent(m_InventoryData, ANALYTICAL_EVENT_INVENTORY_CLOSED, SCR_EAnalyticsDataTable.PLAYER);
			m_Inventory.ResetReadyToSend();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates play menu interactions and send themto analytics when time is right.
	protected void UpdatePlayMenu(float deltaSeconds)
	{		
		m_PlayMenuInteractions.Update(deltaSeconds);
		if (m_PlayMenuInteractions.IsReadyToSend())
		{
			m_PlayMenuData.amt_time_spent = m_PlayMenuInteractions.GetTimeSpent();
			m_PlayMenuData.cnt_menu_reopen = m_PlayMenuInteractions.GetReopenTimes();
			SendAnalyticalEvent(m_PlayMenuData, ANALYTICAL_EVENT_PLAY_MENU_CLOSED, SCR_EAnalyticsDataTable.APPLICATION);
			m_PlayMenuInteractions.ResetReadyToSend();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates field manual interactions and send them to analytics when time is right.
	protected void UpdateFieldManual(float deltaSeconds)
	{		
		m_FieldManualInteractions.Update(deltaSeconds);
		if (m_FieldManualInteractions.IsReadyToSend())
		{
			m_FieldManualData.amt_time_spent = m_FieldManualInteractions.GetTimeSpent();
			m_FieldManualData.cnt_menu_reopen = m_FieldManualInteractions.GetReopenTimes();
			if (GetGame().m_bIsMainMenuOpen)
				m_FieldManualData.amt_name_location = MANUAL_LOCATION_MAIN_MENU;
			else
				m_FieldManualData.amt_name_location = MANUAL_LOCATION_GAME;
			SendAnalyticalEvent(m_FieldManualData, ANALYTICAL_EVENT_FIELD_MANUAL_CLOSED, SCR_EAnalyticsDataTable.APPLICATION);		
			m_FieldManualInteractions.ResetReadyToSend();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sends analytical event to the console or writes it to the file or sends it through the API
	//! Use SetLogging() and GetLogging() to alter the way how are the data currently stored.
	protected void SendAnalyticalEvent(SCR_AnalyticsData object, string eventName, SCR_EAnalyticsDataTable dataTable)
	{
		string logString = string.Format("%1 %2: %3", SCR_DateTimeHelper.GetDateTimeUTC(), eventName, object.ToPrettyString());

		if (SCR_Enum.HasFlag(m_Logging, SCR_EAnalyticsApplicationLoggingFlags.CONSOLE))
			PrintFormat("%1", logString);

		if (SCR_Enum.HasFlag(m_Logging, SCR_EAnalyticsApplicationLoggingFlags.FILE))
		{
			// Open file for every write separately. If we would open it only once, then we would lose it after a crash or a hard kill.
			FileHandle analyticsFile = FileIO.OpenFile("$profile:analytics_uniform.txt", FileMode.APPEND);
			if (analyticsFile)
			{
				analyticsFile.WriteLine(logString);
				analyticsFile.Close();
			}
		}

		if (SCR_Enum.HasFlag(m_Logging, SCR_EAnalyticsApplicationLoggingFlags.API))
		{
			switch (dataTable)
			{
				case SCR_EAnalyticsDataTable.APPLICATION:
					GetGame().GetStatsApi().CreateApplicationEvent(object, eventName);
					break;

				case SCR_EAnalyticsDataTable.PLAYER:
					GetGame().GetStatsApi().CreatePlayerEvent(LOCAL_PLAYER_ID, object, eventName, false);
					break;

				case SCR_EAnalyticsDataTable.SESSION:
					GetGame().GetStatsApi().CreateSessionEvent(object, eventName);
					break;

				case SCR_EAnalyticsDataTable.MOD:
					GetGame().GetStatsApi().CreateModEvent(object, eventName);
					break;

				default:
					Debug.Error(string.Format("Unrecognised SCR_EAnalyticsDataTable.%1! Please include it in 'SendAnalyticalEvent()'.", typename.EnumToString(SCR_EAnalyticsDataTable, dataTable)));
					break;
			}
		}
	}
}
