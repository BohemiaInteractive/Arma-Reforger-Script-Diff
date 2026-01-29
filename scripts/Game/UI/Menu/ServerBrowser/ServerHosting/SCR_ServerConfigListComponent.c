/*!
Script component for handling server config editing UI 
*/

class SCR_ServerConfigListComponent : SCR_ConfigListComponent
{
	protected const string NAME_ENTRY = "name";
	protected const string FILE_NAME_ENTRY = "fileName";
	protected const string SCENARIO_SELECTION_ENTRY = "scenarioId";
	protected const string SCENARIO_MOD_SELECTION_ENTRY = "scenarioModId";
	protected const string PLAYER_LIMIT_ENTRY = "maxPlayers";
	protected const string JOIN_QUEUE_MAX_SIZE = "joinQueueMaxSize";
	protected const string BATTLEYE = "battlEye";
	protected const string CROSSPLAY = "crossPlatform";
	protected const string SIMPLE_PORT = "publicPortSimple";
	
	// Properties content
	protected const string SERVER_NAME_BASE = "%1 %2";
	protected const string FILE_NAME_BASE = "Config_%1";

	protected const string CHAR_BLACK_LIST = "<>:\/\|?*.";
	
	protected SCR_WidgetListEntryEditBox m_NameEdit;
	protected SCR_WidgetListEntryEditBox m_SimplePortEdit;
	protected SCR_WidgetListEntrySelection m_ScenarioSelect;
	protected SCR_WidgetListEntrySelection m_ScenarioModSelect;
	protected SCR_WidgetListEntrySelection m_BattleyeSelect;
	protected SCR_WidgetListEntrySelection m_CrossplaySelect;
	
	protected SCR_WidgetListEntrySlider m_PlayerListSlider;
	protected SCR_WidgetListEntrySlider m_JoinQueueSizeSlider;
	
	protected bool m_bNameEdited;
	protected bool m_bFileNameEdited;
	
	// Mod + scenarios that mod is overriding
	protected ref array<ref SCR_ScenarioSources> m_aScenarioSources = {};

	// Invokers
	protected ref ScriptInvokerMissionWorkshopItem m_OnScenarioSelected;
	protected ref ScriptInvokerString m_OnPortChanged;

	//------------------------------------------------------------------------------------------------
	ScriptInvokerMissionWorkshopItem GetOnScenarioSelected()
	{
		if (!m_OnScenarioSelected)
			m_OnScenarioSelected = new ScriptInvokerMissionWorkshopItem();

		return m_OnScenarioSelected;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerString GetOnPortChanged()
	{
		if (!m_OnPortChanged)
			m_OnPortChanged = new ScriptInvokerString();

		return m_OnPortChanged;
	}
	
	//-------------------------------------------------------------------------------------------
	// Overridden widget API
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (GetGame().InPlayMode())
			FillScenarios();
		
		m_wScrollWidget = ScrollLayoutWidget.Cast(w);
	}

	//-------------------------------------------------------------------------------------------
	// Protected 
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------
	//! Fill scenario selection with list of available scenarios
	protected void FillScenarios()
	{
		// Find widgets
		m_NameEdit = SCR_WidgetListEntryEditBox.Cast(FindEntry(NAME_ENTRY));
		
		m_SimplePortEdit = SCR_WidgetListEntryEditBox.Cast(FindEntry(SIMPLE_PORT));
		
		m_ScenarioSelect = SCR_WidgetListEntrySelection.Cast(FindEntry(SCENARIO_SELECTION_ENTRY));
		
		m_ScenarioModSelect = SCR_WidgetListEntrySelection.Cast(FindEntry(SCENARIO_MOD_SELECTION_ENTRY));
		
		m_PlayerListSlider = SCR_WidgetListEntrySlider.Cast(FindEntry(PLAYER_LIMIT_ENTRY));
		m_JoinQueueSizeSlider = SCR_WidgetListEntrySlider.Cast(FindEntry(JOIN_QUEUE_MAX_SIZE));
		
		m_BattleyeSelect = SCR_WidgetListEntrySelection.Cast(FindEntry(BATTLEYE));
		
		m_CrossplaySelect = SCR_WidgetListEntrySelection.Cast(FindEntry(CROSSPLAY));
		
		// Setup names 
		if (m_NameEdit)
			m_NameEdit.GetEditBoxComponent().m_OnChanged.Insert(OnNameChanged);
		
		// Simple port 
		if (m_SimplePortEdit)
			m_SimplePortEdit.GetEditBoxComponent().m_OnWriteModeLeave.Insert(OnSimplePortChanged);
		
		// Scenarios
		if (!m_ScenarioSelect)
			return;
		
		// Fill scenario sources 
		FillScenarioSources();
		
		// Fill scenario options 
		array<ref SCR_LocalizedProperty> options = {};
		
		foreach (SCR_ScenarioSources source : m_aScenarioSources)
		{
			MissionWorkshopItem scenario = source.m_Scenario;
			if (!scenario)
				return;
			
			SCR_LocalizedProperty scenarioOption = new SCR_LocalizedProperty(scenario.Name(), scenario.Id());
			options.Insert(scenarioOption);
		}
		
		m_ScenarioSelect.SetOptions(options);
		
		// Callback 
		m_ScenarioSelect.GetSelectionComponent().m_OnChanged.Insert(OnSelectScenario);
		
		// Select default scenario
		SelectDefaultScenario();
		
		// Platform specific setup 
		ConsoleSetup();
	}
	
	//-------------------------------------------------------------------------------------------
	protected void FillScenarioSources()
	{
		// Add scenarios and sources to map 
		foreach (MissionWorkshopItem mission : GetDefaultScenarios())
		{
			InsertScenarioToMap(mission, mission.GetOwner());
		}
		
		foreach (MissionWorkshopItem mission : GetModdedScenarios())
		{
			InsertScenarioToMap(mission, mission.GetOwner());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Insert scenario and sorces to scenario sources mod map 
	//! If scenario name is reapeate, insert source mod in to existing scenario 
	protected void InsertScenarioToMap(MissionWorkshopItem scenario, WorkshopItem modOwner)
	{
		// Scenario id is in map - there stil could be repeating names 
		int scenarioIndex = -1;
		
		foreach (int i, SCR_ScenarioSources source : m_aScenarioSources)
		{		
			MissionWorkshopItem mission = source.m_Scenario;
			
			// Scenario structure - label: name, propertyName: id	
			if (mission && scenario.Id() == mission.Id())
			{
				scenarioIndex = i;
				break;
			}
		}
		
		// Scenario is in map 
		if (scenarioIndex != -1) // Add source to existing scenario
			m_aScenarioSources[scenarioIndex].m_aMods.Insert(modOwner);
		else // Add new 
			m_aScenarioSources.Insert(new SCR_ScenarioSources(scenario, {modOwner}));
	}
	
	//-------------------------------------------------------------------------------------------
	protected int GetScenarioIndexById(string scenarioId)
	{
		array<ref SCR_LocalizedProperty> entries = {};
		m_ScenarioSelect.GetOptions(entries);
		
		foreach (int i, SCR_LocalizedProperty entry : entries)
		{
			// Property name = scenario id
			if (entry.m_sPropertyName == scenarioId)
				return i;
		}

		// Fallback
		return 0;
	}
	
	//-------------------------------------------------------------------------------------------
	//! Apply console specific properties
	protected void ConsoleSetup()
	{
		// Check platform 
		if (!GetGame().IsPlatformGameConsole())
			return;
		
		// Default values 
		m_BattleyeSelect.SetValue("0");
		m_CrossplaySelect.SetValue("1");
		
		// Hide limited on consoles 
		m_BattleyeSelect.GetEntryRoot().SetVisible(false);
		m_CrossplaySelect.GetEntryRoot().SetVisible(false);
	}
	
	//-------------------------------------------------------------------------------------------
	// Public
	//-------------------------------------------------------------------------------------------
	void SelectDefaultScenario()
	{
		if (!m_ScenarioSelect)
			return;
		
		array<MissionWorkshopItem> defaultMissions = GetDefaultScenarios();
		if (!defaultMissions.IsEmpty())
			SelectScenario(defaultMissions[0]);
	}
	
	//-------------------------------------------------------------------------------------------
	void SelectScenario(notnull MissionWorkshopItem scenario)
	{
		if (!m_ScenarioSelect)
			return;
			
		int selected = GetScenarioIndexById(scenario.Id());
			
		m_ScenarioSelect.SelectOption(selected);
		OnSelectScenario(SCR_ComboBoxComponent.Cast(m_ScenarioSelect.GetSelectionComponent()), selected);
	}
	
	//-------------------------------------------------------------------------------------------
	MissionWorkshopItem GetSelectedScenario()
	{
		if (!m_ScenarioSelect)
			return null;
	
		array<MissionWorkshopItem> missionItemsAll = {};
		GetGame().GetBackendApi().GetWorkshop().GetPageScenarios(missionItemsAll, 0, SCR_WorkshopUiCommon.PAGE_SCENARIOS);
		
		foreach (MissionWorkshopItem scenario : missionItemsAll)
		{
			SCR_LocalizedProperty selected = m_ScenarioSelect.GetSelectedOption();
			string name = selected.m_sPropertyName;
			
			if (scenario.Id() == name)
				return scenario;
		}
		
		return null;
	}
	
	//-------------------------------------------------------------------------------------------
	//! Return currently selected scenario owner mod as workshop item
	WorkshopItem GetScenarioOwnerMod()
	{
		if (!m_ScenarioSelect)
			return null;
	
		array<MissionWorkshopItem> missionItemsAll = {};
		GetGame().GetBackendApi().GetWorkshop().GetPageScenarios(missionItemsAll, 0, SCR_WorkshopUiCommon.PAGE_SCENARIOS);
		
		// Check current selection 
		SCR_LocalizedProperty property = m_ScenarioModSelect.GetSelectedOption();
		
		if (!property)
			return null;
		
		// Select owner mod
		foreach (MissionWorkshopItem scenario : missionItemsAll)
		{
			WorkshopItem ownerItem = scenario.GetOwner();
			if (!ownerItem)
				continue;
			
			if (ownerItem.Id() == property.m_sPropertyName)
				return ownerItem;
		}
		
		return null;
	}
	
	//-------------------------------------------------------------------------------------------
	// Default Reforger scenarios lack a owner mod
	array<MissionWorkshopItem> GetDefaultScenarios()
	{
		array<MissionWorkshopItem> allMissions = {};
		GetGame().GetBackendApi().GetWorkshop().GetPageScenarios(allMissions, 0, SCR_WorkshopUiCommon.PAGE_SCENARIOS);
		
		array<MissionWorkshopItem> defaultMissions = {};
		foreach (MissionWorkshopItem mission : allMissions)
		{
			if (!mission.GetOwner() && SCR_ScenarioUICommon.IsMultiplayer(mission))
				defaultMissions.Insert(mission);
		}
		
		// Order scenarios alphabetically
		SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionName>.HeapSort(defaultMissions);
		return defaultMissions;
	}
	
	//-------------------------------------------------------------------------------------------
	array<MissionWorkshopItem> GetModdedScenarios()
	{
		// Get all scenarios 
		array<MissionWorkshopItem> allMissions = {};
		GetGame().GetBackendApi().GetWorkshop().GetPageScenarios(allMissions, 0, SCR_WorkshopUiCommon.PAGE_SCENARIOS);
		
		// Filter scenarios 
		array<MissionWorkshopItem> missions = {};
		
		foreach (MissionWorkshopItem mission : allMissions)
		{
			// Exclude default and single player scenarios
			if (!mission.GetOwner() || !SCR_ScenarioUICommon.IsMultiplayer(mission))
				continue;
			
			// Exclude scenarios coming from incompatible addon 
			if (SCR_AddonManager.ItemAvailability(mission.GetOwner()) != SCR_ERevisionAvailability.ERA_AVAILABLE)
				continue;
			
			// Exclude unhostable scenarios (hacky, as it includes connection issues unrelated to the scenario)
			if (!SCR_ScenarioUICommon.CanHost(mission))
				continue;
			
			// Exclude broken mod scenarios
			SCR_WorkshopItem item = SCR_AddonManager.GetInstance().Register(mission.GetOwner());
			if (item && item.GetAnyDependencyMissing() || item.GetCorrupted())
				continue;
			
			missions.Insert(mission);
		}
		
		// Order scenarios alphabetically
		SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionName>.HeapSort(missions);
		return missions;
	}
	
	//-------------------------------------------------------------------------------------------
	//! Fill all entries with values from given DS config 
	void FillFromDSConfig(notnull SCR_DSConfig config)
	{
		// Game
		FindEntry("name").SetValue(config.game.name);
		FindEntry("maxPlayers").SetValue(config.game.maxPlayers.ToString());
		FindEntry("password").SetValue(config.game.password);
		FindEntry("passwordAdmin").SetValue(config.game.passwordAdmin);
		FindEntry("visible").SetValue(SCR_JsonApiStructHandler.BoolToString(config.game.visible));
		FindEntry("crossPlatform").SetValue(SCR_JsonApiStructHandler.BoolToString(config.game.crossPlatform));
		
		// Game properties
		SCR_DSGameProperties gamePropertiesSCr = SCR_DSGameProperties.Cast(config.game.gameProperties);
		
		if (gamePropertiesSCr)
		{
			FindEntry("battlEye").SetValue(SCR_JsonApiStructHandler.BoolToString(gamePropertiesSCr.battlEye));
			FindEntry("disableThirdPerson").SetValue(SCR_JsonApiStructHandler.BoolToString(gamePropertiesSCr.disableThirdPerson));
			FindEntry("VONDisableUI").SetValue(SCR_JsonApiStructHandler.BoolToString(gamePropertiesSCr.VONDisableUI));
			FindEntry("VONDisableDirectSpeechUI").SetValue(SCR_JsonApiStructHandler.BoolToString(gamePropertiesSCr.VONDisableDirectSpeechUI));
			FindEntry("VONCanTransmitCrossFaction").SetValue(SCR_JsonApiStructHandler.BoolToString(gamePropertiesSCr.VONCanTransmitCrossFaction));
			FindEntry("serverMaxViewDistance").SetValue(gamePropertiesSCr.serverMaxViewDistance.ToString());
			FindEntry("networkViewDistance").SetValue(gamePropertiesSCr.networkViewDistance.ToString());
			FindEntry("serverMinGrassDistance").SetValue(gamePropertiesSCr.serverMinGrassDistance.ToString());
		}

		// Scenario
		int index = GetScenarioIndexById(config.game.scenarioId);
		
		m_ScenarioSelect.SelectOption(index);
		OnSelectScenario(SCR_ComboBoxComponent.Cast(m_ScenarioSelect.GetSelectionComponent()), index);
	}
	
	//-------------------------------------------------------------------------------------------
	//! Scroll to the selected entry
	void ScrollToTheEntry(notnull Widget w)
	{
		// Entry position
		float entryX, entryY;
		w.GetScreenPos(entryX, entryY);
		
		// Vertical list size
		float listW, listH;
		m_wList.GetScreenSize(listW, listH);
		
		// Vertical list size
		float listX, listY;
		m_wList.GetScreenPos(listX, listY);
		
		// Scroll (view) size
		float scrollW, scrollH; 
		m_wScrollWidget.GetScreenSize(scrollW, scrollH);
		
		// Scroll to entry
		float outView = listH - scrollH; // What size can't be seen 
		float relPos = entryY - listY;

		float pos = 1 - (outView - relPos) / outView;
		m_wScrollWidget.SetSliderPos(0, pos);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPort(string port)
	{
		m_SimplePortEdit.SetValue(port);
	}
	
	//-------------------------------------------------------------------------------------------
	// Callbacks 
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------
	protected void OnNameChanged(SCR_EditBoxComponent edit, string text)
	{
		m_bNameEdited = !text.IsEmpty();
	}

	//-------------------------------------------------------------------------------------------	
	protected void OnSimplePortChanged(string text)
	{
		if (m_OnPortChanged)
			m_OnPortChanged.Invoke(m_SimplePortEdit.ValueAsString());
	}
	
	//-------------------------------------------------------------------------------------------
	protected void OnFileNameChanged(SCR_EditBoxComponent edit, string text)
	{
		m_bFileNameEdited = !text.IsEmpty();
	}
	
	//-------------------------------------------------------------------------------------------
	protected void OnSelectScenario(SCR_ComboBoxComponent comboBox, int selected)
	{
		if (!m_ScenarioSelect || !m_ScenarioModSelect)
			return;
		
		int selecteScenario = m_ScenarioSelect.GetSelectedOption();
		array<ref SCR_LocalizedProperty> ownerMods = {};
		
		int count = 0;
		
		if (m_aScenarioSources[selected].m_aMods)
			count = m_aScenarioSources[selected].m_aMods.Count();
		
		for (int i = 0; i < count; i++)
		{
			WorkshopItem mod = m_aScenarioSources[selected].m_aMods[i];
			if (!mod)
				continue;
			
			string name = m_aScenarioSources[selected].m_aMods[i].Name();
			string id = m_aScenarioSources[selected].m_aMods[i].Id();
			
			ownerMods.Insert(new SCR_LocalizedProperty(name, id));
		}
		
		// Is vanilla?
		if (!ownerMods || ownerMods.IsEmpty() || ownerMods[0] == null)
		{
			// Display source mod as Arma Reforger
			ownerMods.Clear();
			ownerMods.Insert(new SCR_LocalizedProperty("#AR-Editor_Attribute_OverlayLogo_Reforger", ""));
		}
		
		m_ScenarioModSelect.SetOptions(ownerMods);
		m_ScenarioModSelect.SelectOption(0);
		
		// Enable selection interactivity only if scenario has multiple sources 
		m_ScenarioModSelect.SetInteractive(ownerMods.Count() > 1);
		
		// Update properties
		MissionWorkshopItem scenario = GetSelectedScenario();
		
		if (!m_bNameEdited)
			GenerateName(scenario);
		
		if (!m_bFileNameEdited)
			GenerateFileName(scenario);
		
		SetupPlayerLimit(scenario.GetPlayerCount());
		
		if (m_OnScenarioSelected)
			m_OnScenarioSelected.Invoke(scenario);
	}
	
	//-------------------------------------------------------------------------------------------
	//! Generate server name to the name editbox in case name wasn't edited
	protected void GenerateName(notnull MissionWorkshopItem scenario)
	{
		if (!m_NameEdit)
			return;
		
		string name = string.Format(SERVER_NAME_BASE, SCR_Global.GetProfileName(), scenario.Name());
		
		m_NameEdit.SetValue(name);
		m_NameEdit.ClearInvalidInput();
	}
	
	//-------------------------------------------------------------------------------------------
	//! Generate server name to the name editbox in case name wasn't edited
	string GenerateFileName(notnull MissionWorkshopItem scenario)
	{
		string scenarioTranslation = WidgetManager.Translate(scenario.Name());
		string fileName =  string.Format(FILE_NAME_BASE, scenarioTranslation);
		fileName.Replace(" ", "");
		
		// Remove special characters 
		for (int i = 0, count = CHAR_BLACK_LIST.Length(); i < count; i++)
		{
			fileName.Replace(CHAR_BLACK_LIST[i], "");
		}
		
		return fileName;
	}
	
	//-------------------------------------------------------------------------------------------
	protected void SetupPlayerLimit(int limit)
	{
		if (!m_PlayerListSlider)
			return;
		
		m_PlayerListSlider.SetRange(1, limit);
		
		// Set value 
		int value = m_PlayerListSlider.ValueAsString().ToInt();
		
		if (value > limit)
			m_PlayerListSlider.SetValue(limit.ToString());
	}
}

//! Stores scenario with scenario source
//-------------------------------------------------------------------------------------------
class SCR_ScenarioSources
{
	MissionWorkshopItem m_Scenario;
	
	ref array<WorkshopItem> m_aMods;
	
	void SCR_ScenarioSources(MissionWorkshopItem scenario, array<WorkshopItem> mods)
	{
		m_Scenario = scenario;
		m_aMods = mods;
	}
}

