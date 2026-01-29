enum SCR_EServerHostingDialogTabs
{
	SETTINGS,
	MODS,
	ADVANCED_SETTINGS
}

class ServerHostingUI : SCR_TabDialog
{
	protected const string JSON_POSTFIX = ".json";

	protected const string DIALOG_OVERRIDE =	"#AR-ServerHosting_OverrideWarning";
	protected const string DIALOG_SAVED = 		"#AR-ServerHosting_SaveSuccessful";

	protected const string COLOR_TAG = 		"<color rgba='%1'>";
	protected const string COLOR_TAG_END = 	"</color>";

	protected ref SCR_DSConfig m_DSConfig = new SCR_DSConfig();
	protected static ref SCR_DSConfig s_TemporaryConfig;

	// Config widget wrapper components
	protected ref SCR_ServerConfigListComponent m_ConfigList;
	protected ref SCR_ServerHostingSettingsSubMenu m_ConfigListSubMenu;

	protected ref SCR_ServerHostingModSubMenu m_ConfigMods;

	protected ref SCR_ServerConfigAdvancedComponent m_AdvancedSettings;
	protected ref SCR_ServerHostingSettingsSubMenu m_AdvancedSubMenu;

	// Values
	protected string m_sUnifiedPort; // Unified port used acress sub menu to define port from basic in advanced settings and vice versa
	protected string m_sFileName;

	// TODO: move tab specific stuff to a child of SCR_SuperMenuComponent (low priority)
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);

		// Server setting list
		m_ConfigListSubMenu = SCR_ServerHostingSettingsSubMenu.Cast(m_SuperMenuComponent.GetSubMenu(SCR_EServerHostingDialogTabs.SETTINGS));
		m_ConfigList = SCR_ServerConfigListComponent.Cast(m_ConfigListSubMenu.GetConfigList());
		
		// Mods
		m_ConfigMods = SCR_ServerHostingModSubMenu.Cast(m_SuperMenuComponent.GetSubMenu(SCR_EServerHostingDialogTabs.MODS));

		m_ConfigMods.GetEventOnWorkshopButtonActivate().Insert(OnWorkshopOpenActivate);
		m_ConfigMods.GetOnRequestDefaultScenario().Insert(SelectDefaultScenario);

		// Advanced settings
		m_AdvancedSubMenu = SCR_ServerHostingSettingsSubMenu.Cast(m_SuperMenuComponent.GetSubMenu(SCR_EServerHostingDialogTabs.ADVANCED_SETTINGS));
		m_AdvancedSettings = SCR_ServerConfigAdvancedComponent.Cast(m_AdvancedSubMenu.GetConfigList());
		
		// Default IP and port
		GetGame().GetBackendApi().SetDefaultIpPort(m_DSConfig);
		m_AdvancedSettings.SetIPPort(m_DSConfig);

		// Setup buttons
		m_ConfigListSubMenu.GetOnHost().Insert(OnHostServerClick);
		m_ConfigListSubMenu.GetOnSave().Insert(OnSaveTemplateClick);

		m_AdvancedSubMenu.GetOnHost().Insert(OnHostServerClick);
		m_AdvancedSubMenu.GetOnSave().Insert(OnSaveTemplateClick);

		// Restore config
		if (s_TemporaryConfig)
		{
			m_ConfigList.FillFromDSConfig(s_TemporaryConfig);
			m_AdvancedSettings.FillFromDSConfig(s_TemporaryConfig);
			m_ConfigMods.EnableModsFromDSConfig(s_TemporaryConfig);
			s_TemporaryConfig = null;
		}
		else
		{
			SelectDefaultScenario();
		}
		
		// Setup menu listeners
		m_ConfigList.GetOnPortChanged().Insert(OnSubMenuChangePort);
		m_ConfigList.GetOnScenarioSelected().Insert(OnScenarioSelected);
		
		m_AdvancedSettings.GetOnPortChanged().Insert(OnSubMenuChangePort);
	}

	//------------------------------------------------------------------------------------------------
	//! Verify and store all info into config - return true if all config properties are valid
	protected bool VerifyAndStoreConfig()
	{
		if (!m_DSConfig || !m_ConfigList || !m_ConfigMods || !m_AdvancedSettings)
		{
			#ifdef SB_DEBUG
			Print("Missing config references");
			#endif

			return false;
		}

		// Check all values validity
		Widget invalidEntry = m_ConfigList.GetInvalidEntry();
		int invalidTab = 0;

		if (!invalidEntry)
		{
			invalidEntry = m_AdvancedSettings.GetInvalidEntry();
			invalidTab = 2;
		}

		if (invalidEntry)
		{
			// Show problematic tab
			if (m_SuperMenuComponent.GetTabView().m_iSelectedTab != invalidTab)
				m_SuperMenuComponent.GetTabView().ShowTab(invalidTab);

			// Move to invalid entry
			GetGame().GetWorkspace().SetFocusedWidget(invalidEntry);
			m_ConfigList.ScrollToTheEntry(invalidEntry);

			return false;
		}

		// Store
		array<ref SCR_WidgetListEntry> properties = m_ConfigList.GetInitialEntryList();
		array<ref SCR_WidgetListEntry> advanced = m_AdvancedSettings.GetInitialEntryList();
		for (int i = 0, count = advanced.Count(); i < count; i++)
		{
			properties.Insert(advanced[i]);
		}

		array<ref DSMod> mods = m_ConfigMods.SelectedModsList();
		WorkshopItem scenarioMod = m_ConfigList.GetScenarioOwnerMod();

		m_DSConfig.StoreFullJson(properties, mods, scenarioMod);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnOverrideConfirm(SCR_ConfigurableDialogUi dialog)
	{
		dialog.m_OnConfirm.Remove(OnOverrideConfirm);
		SaveConfig();
	}

	//------------------------------------------------------------------------------------------------
	// Callbacks
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void OnHostServerClick()
	{
		if (!m_DSConfig || !m_ConfigList)
			return;

		// Check properties
		if (!VerifyAndStoreConfig())
			return;

		// Check connection - prevent host
		if (!GetGame().GetBackendApi().IsActive() || !SCR_ServicesStatusHelper.AreMultiplayerServicesAvailable())
		{
			SCR_ServerHostingDialogs.CreateNoConnectionDialog();
			return;
		}

		// Host scenario
		protected ref MissionWorkshopItem hostedScenario = m_ConfigList.GetSelectedScenario();

		if (hostedScenario)
			SCR_ScenarioUICommon.TryHostScenario(hostedScenario, m_DSConfig);

		GameSessionStorage.s_Data["m_iRejoinAttempt"] = "0";

		// Save current menu
		MenuBase lastMenu = GetGame().GetMenuManager().GetTopMenu();

		if (ServerBrowserMenuUI.Cast(lastMenu))
			SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ServerBrowserMenu, -1, SCR_EMenuLoadingAdditionalDialog.SERVER_HOSTING);
		else if (SCR_ScenarioMenu.Cast(lastMenu))
			SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ScenarioMenu, -1, SCR_EMenuLoadingAdditionalDialog.SERVER_HOSTING);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSaveTemplateClick()
	{
		if (!VerifyAndStoreConfig())
			return;

		DisplaySaveDialog();
	}

	//------------------------------------------------------------------------------------------------
	//! Show dialog with file name settings and confirmation
	protected void DisplaySaveDialog()
	{
		SCR_ServerConfigSaveDialog dialog = SCR_ServerHostingDialogs.CreateSaveConfirmDialog();
		dialog.m_OnConfirm.Insert(OnSaveDialogConfirm);

		// Generate
		string name = m_ConfigList.GenerateFileName(m_ConfigList.GetSelectedScenario());
		dialog.SetFileNameText(name);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSaveDialogConfirm(SCR_ConfigurableDialogUi dialog)
	{
		SCR_ServerConfigSaveDialog saveDialog = SCR_ServerConfigSaveDialog.Cast(dialog);
		m_sFileName = saveDialog.GetFileNameText();

		// Check if name is in config
		array<string> configs = {};
		GetGame().GetBackendApi().GetAvailableConfigs(configs);
		string name = m_sFileName;

		// Go though locally saved configs
		for (int i = 0, count = configs.Count(); i < count; i++)
		{
			if (name + JSON_POSTFIX == configs[i])
			{
				// Display config exists
				SCR_ConfigurableDialogUi overrideDialog = SCR_ServerHostingDialogs.CreateSaveOverrideDialog();

				string color = string.Format(COLOR_TAG, UIColors.FormatColor(UIColors.CONTRAST_COLOR));
				string msg = WidgetManager.Translate(DIALOG_OVERRIDE, name, color, COLOR_TAG_END);
				overrideDialog.SetMessage(msg);
				overrideDialog.m_OnConfirm.Insert(OnOverrideConfirm);

				return;
			}
		}

		// Save
		SaveConfig();
	}

	//------------------------------------------------------------------------------------------------
	void SaveConfig()
	{
		string configName = m_sFileName + JSON_POSTFIX;
		bool saved = GetGame().GetBackendApi().SaveDSConfig(m_DSConfig, configName);

		if (saved)
		{
			// Show success dialog
			SCR_ConfigurableDialogUi dialog = SCR_ServerHostingDialogs.CreateSaveSuccessDialog();

			array<string> paths = {};
			GetGame().GetBackendApi().GetAvailableConfigPaths(paths);

			// Find right path and show message about file path
			array<string> configs = {};
			GetGame().GetBackendApi().GetAvailableConfigs(configs);
			string name = m_sFileName;

			foreach (int i, string config : configs)
			{
				if (name + JSON_POSTFIX != config)
					continue;
				
				string color = string.Format(COLOR_TAG, UIColors.FormatColor(UIColors.CONTRAST_COLOR));
				string msg = WidgetManager.Translate(DIALOG_SAVED, paths[i], color, COLOR_TAG_END);

				if (dialog)
					dialog.SetMessage(msg);

				break;
			}

		}
		else
		{
			SCR_ServerHostingDialogs.CreateSaveFailedDialog();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWorkshopOpenActivate()
	{
		SCR_MenuToolsLib.GetEventOnAllMenuClosed().Insert(AllMenuClosed);
		SCR_MenuToolsLib.CloseAllMenus({MainMenuUI, ContentBrowserUI});
	}

	//------------------------------------------------------------------------------------------------
	//! Callback for when all additional menu are closed and top menu is main menu
	protected void AllMenuClosed()
	{
		SCR_MenuToolsLib.GetEventOnAllMenuClosed().Remove(AllMenuClosed);

		// Store and close
		array<ref SCR_WidgetListEntry> properties = m_ConfigList.GetInitialEntryList();
		array<ref SCR_WidgetListEntry> advanced = m_AdvancedSettings.GetInitialEntryList();
		for (int i = 0, count = advanced.Count(); i < count; i++)
			properties.Insert(advanced[i]);

		array<ref DSMod> mods = m_ConfigMods.SelectedModsList();
		WorkshopItem scenarioMod = m_ConfigList.GetScenarioOwnerMod();

		m_DSConfig.StoreFullJson(properties, mods, scenarioMod);
		s_TemporaryConfig = m_DSConfig;

		Close();

		// Open workshop
		ContentBrowserUI workshopUI = ContentBrowserUI.Cast(GetGame().GetMenuManager().GetTopMenu());
		if (!workshopUI)
			SCR_WorkshopUiCommon.TryOpenWorkshop();
	}

	//------------------------------------------------------------------------------------------------
	//! Call when sub menu change port setting to propagete unified port on multiple places
	protected void OnSubMenuChangePort(string port)
	{
		m_sUnifiedPort = port;

		m_ConfigList.SetPort(m_sUnifiedPort);
		m_AdvancedSettings.SetPorts(m_sUnifiedPort);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnScenarioSelected(MissionWorkshopItem scenario)
	{
		if (m_ConfigMods)
			m_ConfigMods.SetScenario(scenario);
	}
	
	//------------------------------------------------------------------------------------------------
	// API
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Select scenario in scenario selection dropdown
	void SelectScenario(notnull MissionWorkshopItem scenario)
	{
		if (!m_ConfigList)
			return;

		m_ConfigList.SelectScenario(scenario);
	}

	//------------------------------------------------------------------------------------------------
	//! Select scenario in scenario selection dropdown
	void SelectDefaultScenario()
	{
		if (!m_ConfigList)
			return;

		m_ConfigList.SelectDefaultScenario();
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_DSConfig GetTemporaryConfig()
	{
		return s_TemporaryConfig;
	}
}
