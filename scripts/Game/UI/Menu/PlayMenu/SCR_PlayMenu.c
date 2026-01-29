enum EPlayMenuContentType
{
	FEATURED,
	RECOMMENDED,
	RECENT
}

class SCR_PlayMenuEntry : Managed
{
	EPlayMenuContentType m_eContentType;
	ref MissionWorkshopItem m_Item;

	void SCR_PlayMenuEntry(MissionWorkshopItem item, EPlayMenuContentType type)
	{
		m_Item = item;
		m_eContentType = type;
	}
}

class SCR_PlayMenu : MenuRootBase
{
	protected ResourceName m_sConfig = "{6409EA8EA4BFF7E6}Configs/PlayMenu/PlayMenuEntries.conf";
	protected ref Resource m_Config;
	protected BaseContainer m_ConfigEntries;

	const string TYPE_FEATURED = "m_aFeaturedScenarios";
	const string TYPE_RECOMMENDED = "m_aRecommendedScenarios";

	protected SCR_PlayMenuComponent m_Featured;
	protected SCR_PlayMenuComponent m_Recommended;
	protected SCR_PlayMenuComponent m_Recent;

	protected ref array<MissionWorkshopItem> m_aScenariosFeatured = {};
	protected ref array<MissionWorkshopItem> m_aScenariosRecommended = {};
	protected ref array<MissionWorkshopItem> m_aScenariosRecent = {};

	protected ref array<ref SCR_PlayMenuEntry> m_aEntriesFeatured = {};
	protected ref array<ref SCR_PlayMenuEntry> m_aEntriesRecommended = {};
	protected ref array<ref SCR_PlayMenuEntry> m_aEntriesRecent = {};

	protected ref MissionWorkshopItem m_ItemTutorial;
	protected WorkshopApi m_WorkshopAPI;
	protected SCR_PlayMenuTileComponent m_CurrentTile;
	protected Widget m_wRoot;
	protected bool m_bTutorialPlayed;
	protected bool m_bShowPlayTutorialDialog;
	protected int m_iPlayTutorialShowCount;
	protected int m_iPlayTutorialShowMax;

	const int THRESHOLD_RECENTLY_PLAYED = 3600 * 24 * 30;

	protected ref array<SCR_InputButtonComponent> m_aRightFooterButtons = {};

	protected SCR_PlayMenuTileComponent m_ClickedTile; // Cache last clicked line to trigger the correct dialog after the double click window

	protected MissionWorkshopItem m_SelectedScenario;
	
	protected SCR_MenuActionsComponent m_ActionsComponent;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		m_wRoot = GetRootWidget();
		m_Recent = SCR_PlayMenuComponent.GetComponent("Recent", m_wRoot);
		m_Recommended = SCR_PlayMenuComponent.GetComponent("Recommended", m_wRoot);
		m_Featured = SCR_PlayMenuComponent.GetComponent("Featured", m_wRoot);
		m_WorkshopAPI = GetGame().GetBackendApi().GetWorkshop();

		//! Inputs
		//! Buttons
		SCR_DynamicFooterComponent footer = GetFooterComponent();
		footer.GetOnButtonActivated().Insert(OnInteractionButtonPressed);
		
		m_aRightFooterButtons = footer.GetButtonsInFooter(SCR_EDynamicFooterButtonAlignment.RIGHT);

		//! Listeners
		m_ActionsComponent = SCR_MenuActionsComponent.FindComponent(GetRootWidget());
		if (m_ActionsComponent)
			m_ActionsComponent.GetOnAction().Insert(OnActionTriggered);

		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		// Backend
		WorkshopApi workshop = GetGame().GetBackendApi().GetWorkshop();
		
		// Scan offline items if needed
		if (workshop.NeedAddonsScan())
			workshop.ScanOfflineItems();
		
		// Read the PlayMenu config
		m_Config = BaseContainerTools.LoadContainer(m_sConfig);
		if (!m_Config)
			return;

		m_ConfigEntries = m_Config.GetResource().ToBaseContainer();
		if (!m_ConfigEntries)
			return;

		// Get tutorial
		ResourceName tutorial;
		m_ConfigEntries.Get("m_TutorialScenario", tutorial);
		m_ItemTutorial = m_WorkshopAPI.GetInGameScenario(tutorial);

		if (m_ItemTutorial)
			m_bTutorialPlayed = m_ItemTutorial.GetTimeSinceLastPlay() > -1;

		// Check how many times "Play tutorial" dialog was shown
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");

		if (settings)
		{
			settings.Get("m_iPlayTutorialShowCount", m_iPlayTutorialShowCount);
			settings.Get("m_iPlayTutorialShowMax", m_iPlayTutorialShowMax);
		}

		m_bShowPlayTutorialDialog = !m_bTutorialPlayed && m_iPlayTutorialShowCount < m_iPlayTutorialShowMax;

		//PrintFormat("[OnMenuOpen] m_bShowPlayTutorialDialog: %1 | m_bTutorialPlayed: %2 | m_iPlayTutorialShowCount: %3", m_bShowPlayTutorialDialog, m_bTutorialPlayed, m_iPlayTutorialShowCount);

		// Get scenarios
		GetScenarios(m_aScenariosFeatured, EPlayMenuContentType.FEATURED);
		GetScenarios(m_aScenariosRecommended, EPlayMenuContentType.RECOMMENDED);
		GetScenarios(m_aScenariosRecent, EPlayMenuContentType.RECENT);

		// Get menu entries
		CreateMenuEntries(m_aEntriesFeatured, m_aScenariosFeatured, EPlayMenuContentType.FEATURED);
		CreateMenuEntries(m_aEntriesRecommended, m_aScenariosRecommended, EPlayMenuContentType.RECOMMENDED);
		CreateMenuEntries(m_aEntriesRecent, m_aScenariosRecent, EPlayMenuContentType.RECENT);

		SetupSectionTiles(m_Featured, m_aEntriesFeatured);
		SetupSectionTiles(m_Recommended, m_aEntriesRecommended);
		SetupSectionTiles(m_Recent, m_aEntriesRecent);

		// Set starting focused tile
		if (m_aEntriesRecommended.Count() > 0)
			m_Recommended.SetFocusedItem(0);
		
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
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();
		
		SCR_AnalyticsApplication.GetInstance().ClosePlayMenu();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		
		if (m_ActionsComponent)
			m_ActionsComponent.ActivateActions();
		
		SCR_AnalyticsApplication.GetInstance().OpenPlayMenu();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();

		if (m_ActionsComponent)
			m_ActionsComponent.ActivateActions();
		
		SCR_ServicesStatusHelper.RefreshPing();
		
		// Restore focus to the last accessed tile
		if (m_CurrentTile)
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_CurrentTile.GetRootWidget());
			return;
		}

		// Fallback to the 1st item in the *recent items* list
		if (m_Recent)
			m_Recent.SetFocusedItem(0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupSectionTiles(SCR_PlayMenuComponent section, array<ref SCR_PlayMenuEntry> entries)
	{
		ref array<Widget> widgets = section.GetWidgets();

		// Remove entries that do not fit into the layout
		if (entries.Count() > widgets.Count())
			entries.Resize(widgets.Count());

		// Setup grid tiles
		foreach (int i, SCR_PlayMenuEntry entry : entries)
		{
			Widget w = widgets.Get(i);
			if (!w)
				continue;

			SCR_PlayMenuTileComponent tile = SCR_PlayMenuTileComponent.Cast(w.FindHandler(SCR_PlayMenuTileComponent));
			if (!tile)
				continue;

			tile.GetOnMouseInteractionButtonClicked().Insert(OnInteractionButtonPressed);

			tile.m_OnFocused.Insert(OnTileFocused);
			tile.m_OnFocusLost.Insert(OnTileFocusLost);

			tile.Setup(entry.m_Item, entry.m_eContentType);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateMenuEntries(out array<ref SCR_PlayMenuEntry> entries, array<MissionWorkshopItem> scenarios, EPlayMenuContentType type)
	{
		foreach (MissionWorkshopItem scenario : scenarios)
		{
			entries.Insert(new SCR_PlayMenuEntry(scenario, type));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void GetScenarios(out array<MissionWorkshopItem> scenarios, EPlayMenuContentType type)
	{
		if (type == EPlayMenuContentType.RECENT)
		{
			GetRecentScenarios(scenarios);
			return;
		}

		array<ResourceName> aResources = {};

		if (type == EPlayMenuContentType.FEATURED)
			m_ConfigEntries.Get(TYPE_FEATURED, aResources);
		else
			m_ConfigEntries.Get(TYPE_RECOMMENDED, aResources);

		foreach (ResourceName sResource : aResources)
		{
			MissionWorkshopItem scenario = m_WorkshopAPI.GetInGameScenario(sResource);

			if (scenario && !SCR_ScenarioUICommon.IsMissingDependencies(scenario))
				scenarios.Insert(scenario);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void GetRecentScenarios(out array<MissionWorkshopItem> scenarios)
	{
		// Get missions from Workshop API
		m_WorkshopAPI.GetPageScenarios(scenarios, 0, SCR_WorkshopUiCommon.PAGE_SCENARIOS);

		int count = scenarios.Count();
		int elapsed;
		MissionWorkshopItem scenario;

		// Remove scenarios from disabled addons
		for (int i = count - 1; i >= 0; i--)
		{
			scenario = scenarios[i];
			elapsed = scenario.GetTimeSinceLastPlay();

			if (elapsed == -1 || elapsed > THRESHOLD_RECENTLY_PLAYED || SCR_ScenarioUICommon.IsMissingDependencies(scenario))
			{
				//PrintFormat("[GetRecentScenarios] removed: %1 | last played: %2 | missing dependency: %3", scenario.Name(), elapsed, SCR_ScenarioUICommon.IsMissingDependencies(scenario));
				scenarios.Remove(i);
				continue;
			}
		};

		SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionTimeSinceLastPlay>.HeapSort(scenarios, false);
	}

	//! Input Events
	//------------------------------------------------------------------------------------------------
	protected void OnActionTriggered(string action, float multiplier)
	{
		//! TODO: set which input modes should trigger the actions in the component itself
		if (GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE || !GetTileUnderCursor())
			return;

		MissionWorkshopItem scenario = GetSelectedScenario();
		
		switch (action)
		{
			case UIConstants.MENU_ACTION_DOUBLE_CLICK:		OnTileClickInteraction(multiplier); break;
			case SCR_ScenarioUICommon.ACTION_RESTART:		Restart(scenario); break;
			case SCR_ScenarioUICommon.ACTION_FIND_SERVERS:	Join(scenario); break;
			case SCR_ScenarioUICommon.ACTION_HOST:			Host(scenario); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInteractionButtonPressed(string action)
	{
		SwitchOnButton(action, GetSelectedScenario());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnConfirmationDialogButtonPressed(SCR_ScenarioConfirmationDialogUi dialog, string tag)
	{
		if (!dialog)
			return;

		SwitchOnButton(tag, dialog.GetScenario());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SwitchOnButton(string tag, MissionWorkshopItem scenario)
	{
		switch (tag)
		{
			case SCR_ConfigurableDialogUi.BUTTON_CONFIRM:	Play(scenario); break;
			case SCR_ScenarioUICommon.BUTTON_PLAY:			Play(scenario); break;
			case SCR_ScenarioUICommon.BUTTON_CONTINUE:		Continue(scenario); break;
			case SCR_ScenarioUICommon.BUTTON_RESTART:		Restart(scenario); break;
			case SCR_ScenarioUICommon.BUTTON_FIND_SERVERS:	Join(scenario); break;
			case SCR_ScenarioUICommon.BUTTON_HOST:			Host(scenario); break;
			case SCR_ScenarioUICommon.BUTTON_SCENARIOS: 	OnScenarios(); break;
			case UIConstants.BUTTON_BACK: 					OnBack(); break;
		}
	}

	// CLICKS
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void OnTileClickInteraction(float multiplier)
	{
		//! multiplier value in the action is used to differentiate between single and double click

		SCR_PlayMenuTileComponent tile = m_ClickedTile;
		if (!tile || !GetTileUnderCursor())
			return;

		MissionWorkshopItem scenario = tile.m_Item;
		if (!scenario)
			return;

		switch (Math.Floor(multiplier))
		{
			case 1: OnClickInteraction(scenario); break;
			case 2: OnDoubleClickInteraction(scenario); break;
		}

		m_ClickedTile = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClickInteraction(MissionWorkshopItem scenario)
	{
		//! Confirmation Dialog
		SCR_ScenarioConfirmationDialogUi scenarioConfirmationDialog = SCR_ScenarioDialogs.CreateScenarioConfirmationDialog(scenario);
		if (!scenarioConfirmationDialog)
		{
			OnPlayInteraction(scenario);
			return;
		}
	
		//! Bind dialog delegates
		scenarioConfirmationDialog.m_OnButtonPressed.Insert(OnConfirmationDialogButtonPressed);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDoubleClickInteraction(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		OnPlayInteraction(scenario);
	}
	
	// BUTTONS
	//------------------------------------------------------------------------------------------------
	protected void OnTileMouseClick(SCR_ScriptedWidgetComponent tile)
	{
		m_ClickedTile = SCR_PlayMenuTileComponent.Cast(tile);
	}

	// INTERACTIONS
	//------------------------------------------------------------------------------------------------
	protected void OnPlayInteraction(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		if (SCR_ScenarioUICommon.HasSave(scenario))
			Continue(scenario);
		else
			Play(scenario);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBack()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnScenarios()
	{
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);

		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ScenarioMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void Play(MissionWorkshopItem scenario)
	{
		if (!scenario || !SCR_ScenarioUICommon.CanPlay(scenario))
			return;
		
		m_SelectedScenario = scenario;
		
		if (m_bShowPlayTutorialDialog && m_ItemTutorial && m_ItemTutorial != m_CurrentTile.m_Item)
		{
			BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");

			if (settings)
			{
				m_iPlayTutorialShowCount++;

				settings.Set("m_iPlayTutorialShowCount", m_iPlayTutorialShowCount);
				GetGame().UserSettingsChanged();
			}

			// Tutorial confirmation dialog
			SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateTutorialDialog();
			if (dialog)
			{
				dialog.m_OnConfirm.Insert(OnPlayTutorial);
				dialog.m_OnCancel.Insert(PlayCurrentScenario);
			}
		}
		else
		{
			PlayCurrentScenario();
		}

		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void Continue(MissionWorkshopItem scenario)
	{
	 	SCR_ScenarioUICommon.LoadSave(scenario, m_CurrentTile.m_Header, ChimeraMenuPreset.PlayMenu);
		m_SelectedScenario = scenario;
		PlayCurrentScenario();
	}

	//------------------------------------------------------------------------------------------------
	protected void Restart(MissionWorkshopItem scenario)
	{
		if (!scenario || !SCR_ScenarioUICommon.CanPlay(scenario))
			return;
		
		m_SelectedScenario = scenario;

		SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateDialog(SCR_ScenarioUICommon.DIALOG_RESTART);
		dialog.m_OnConfirm.Insert(OnRestartConfirmed);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestartConfirmed()
	{
		PlayCurrentScenario();
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void Host(MissionWorkshopItem scenario)
	{
		if (!scenario || !SCR_ScenarioUICommon.CanHost(scenario))
			return;
		
		m_SelectedScenario = scenario;
		
		HostCurrentScenario();

		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void Join(MissionWorkshopItem scenario)
	{
		if (!scenario || !SCR_ScenarioUICommon.CanJoin(scenario))
			return;
		
		m_SelectedScenario = scenario;
		
		FindCurrentScenarioServers();

		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileFocused(SCR_PlayMenuTileComponent tile)
	{
		m_CurrentTile = tile;

		UpdateNavigationButtons();
		
		tile.m_OnClick.Insert(OnTileMouseClick);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileFocusLost(SCR_PlayMenuTileComponent tile)
	{
		UpdateNavigationButtons(false);
		
		tile.m_OnClick.Remove(OnTileMouseClick);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateNavigationButtons(bool show = true)
	{
		if (!m_CurrentTile)
			return;
		
		show = show && GetSelectedTile() && GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE;
		SCR_ScenarioUICommon.UpdateInputButtons(m_CurrentTile.m_Item, m_aRightFooterButtons, show);
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayCurrentScenario()
	{
		SCR_ScenarioUICommon.TryPlayScenario(m_SelectedScenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void FindCurrentScenarioServers()
	{
		if (m_SelectedScenario)
			ServerBrowserMenuUI.TryOpenServerBrowserWithScenarioFilter(m_SelectedScenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void HostCurrentScenario()
	{
		if (!m_SelectedScenario)
			return;
		
		ServerHostingUI dialog = SCR_CommonDialogs.CreateServerHostingDialog();

		if (!dialog)
			return;

		dialog.SelectScenario(m_SelectedScenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayTutorial()
	{
		SCR_ScenarioUICommon.TryPlayScenario(m_ItemTutorial);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_PlayMenuTileComponent GetSelectedTile()
	{
		// We are not over a line, use currently focused line
		Widget wfocused = GetGame().GetWorkspace().GetFocusedWidget();
		SCR_PlayMenuTileComponent comp;
		if (wfocused)
			comp = SCR_PlayMenuTileComponent.Cast(wfocused.FindHandler(SCR_PlayMenuTileComponent));

		EInputDeviceType inputDevice = GetGame().GetInputManager().GetLastUsedInputDevice();
		bool isCursorOnInnerButton = m_CurrentTile && m_CurrentTile.m_bIsMouseInteraction;
		
		if (inputDevice == EInputDeviceType.MOUSE && (GetTileUnderCursor() || isCursorOnInnerButton))
			return m_CurrentTile;
		else
			return comp;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_PlayMenuTileComponent GetTileUnderCursor()
	{
		Widget w = WidgetManager.GetWidgetUnderCursor();

		if (!w)
			return null;

		return SCR_PlayMenuTileComponent.Cast(w.FindHandler(SCR_PlayMenuTileComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	protected MissionWorkshopItem GetSelectedScenario()
	{
		SCR_PlayMenuTileComponent comp = GetSelectedTile();

		if (!comp)
			return null;

		return comp.m_Item;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateNavigationButtons();
	}
}
