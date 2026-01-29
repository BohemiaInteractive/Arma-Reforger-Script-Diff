/* 
Sub menu base for handlign scenario lines.
*/

class SCR_ScenarioMenuLoadContext
{
	ref array<MissionWorkshopItem> m_aScenarios;
	Widget m_wListRoot;
}

class SCR_ContentBrowser_ScenarioSubMenuBase : SCR_SubMenuBase
{
	[Attribute("{FEFDB7917AB8310F}UI/layouts/Menus/ContentBrowser/ScenariosMenu/ContentBrowser_ScenarioLine.layout", UIWidgets.ResourceNamePicker, ".layout for the scenario lines", params: "layout")]
	protected ResourceName m_sLinesLayout;
	
	protected ref array<SCR_ContentBrowser_ScenarioLineComponent> m_aScenarioLines = {};

	protected SCR_ContentBrowser_ScenarioLineComponent m_SelectedLine;
	protected SCR_ContentBrowser_ScenarioLineComponent m_LastSelectedLine;

	protected MissionWorkshopItem m_SelectedScenario;
	protected ref SCR_MissionHeader m_Header;
	
	protected SCR_ScenarioDetailsPanelComponent m_ScenarioDetailsPanel;
	protected SCR_AddonDetailsPanelComponent m_AddonDetailsPanel;

	// Line Actions
	protected EInputDeviceType m_eLastInputType;
	protected bool m_bWasLineSelected;
	protected SCR_ContentBrowser_ScenarioLineComponent m_ClickedLine; // Cache last clicked line to trigger the correct dialog after the double click window

	protected SCR_MenuActionsComponent m_ActionsComponent;
	
	// Nav buttons
	protected SCR_InputButtonComponent m_NavFavorite;
	protected ref array<SCR_InputButtonComponent> m_aRightFooterButtons = {};

	// Invokers
	protected ref ScriptInvokerBool m_OnLineFavorite;

	protected bool m_bIsListeningForCommStatus;
	protected WorkshopApi m_WorkshopApi;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		InitWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		// Listen for Actions
		m_ActionsComponent = SCR_MenuActionsComponent.FindComponent(GetRootWidget());
		if (m_ActionsComponent)
			m_ActionsComponent.GetOnAction().Insert(OnActionTriggered);

		// Right footer buttons
		// These are visible when using keyboard / gamepad and focusing a line
		m_DynamicFooter.GetOnButtonActivated().Insert(OnInteractionButtonPressed);

		m_NavFavorite = m_DynamicFooter.FindButton(SCR_ScenarioUICommon.BUTTON_FAVORITE);
		m_aRightFooterButtons = m_DynamicFooter.GetButtonsInFooter(SCR_EDynamicFooterButtonAlignment.RIGHT);

		UpdateNavigationButtons();
		
		// Backend
		m_WorkshopApi = GetGame().GetBackendApi().GetWorkshop();
		
		InitWorkshopApi()
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		//! Update selected scenario
		MissionWorkshopItem selectedMission = GetSelectedScenario();
		if (selectedMission && m_SelectedScenario != selectedMission)
		{
			m_SelectedScenario = selectedMission;
			m_Header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(selectedMission.Id()));
		}

		//! Update buttons
		EInputDeviceType inputDeviceType = GetGame().GetInputManager().GetLastUsedInputDevice();
		bool isLineSelected = GetSelectedLine();
		bool shouldUpdateButtons = inputDeviceType != m_eLastInputType || isLineSelected != m_bWasLineSelected;

		if (shouldUpdateButtons)
			UpdateNavigationButtons();

		m_eLastInputType = inputDeviceType;
		m_bWasLineSelected = isLineSelected;
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		SCR_ServicesStatusHelper.RefreshPing();
		
		if (!m_bIsListeningForCommStatus)
			SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		m_bIsListeningForCommStatus = true;
		
		if (m_ActionsComponent)
			m_ActionsComponent.ActivateActions();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();

		UpdateNavigationButtons(false);
		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
		m_bIsListeningForCommStatus = false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();

		SCR_ServicesStatusHelper.RefreshPing();
		
		UpdateNavigationButtons();
		
		if (m_bShown && m_ActionsComponent)
			m_ActionsComponent.ActivateActions();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		
		if (m_bShown && m_ActionsComponent)
			m_ActionsComponent.ActivateActions();
	}
	
	// ---- EVENTS ----
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateNavigationButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineFocus(SCR_ScriptedWidgetComponent entry)
	{
		SCR_ContentBrowser_ScenarioLineComponent lineComp = SCR_ContentBrowser_ScenarioLineComponent.Cast(entry);

		m_SelectedLine = lineComp;
		m_SelectedLine.m_OnClick.Insert(OnLineMouseClick);

		UpdateNavigationButtons();
		UpdateSidePanel();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineFocusLost(SCR_ScriptedWidgetComponent entry)
	{
		m_LastSelectedLine = m_SelectedLine;
		m_SelectedLine.m_OnClick.Remove(OnLineMouseClick);
		
		UpdateSidePanel();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineMouseEnter(SCR_ScriptedWidgetComponent entry);
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineMouseClick(SCR_ScriptedWidgetComponent button)
	{
		m_ClickedLine = SCR_ContentBrowser_ScenarioLineComponent.Cast(button);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineFavorite(SCR_BrowserListMenuEntryComponent entry, bool favorite)
	{
		OnScenarioStateChanged(SCR_ContentBrowser_ScenarioLineComponent.Cast(entry));
	}

	//------------------------------------------------------------------------------------------------
	//! Called from scenario line component when scenario state changes
	protected void OnScenarioStateChanged(SCR_ContentBrowser_ScenarioLineComponent comp)
	{
		UpdateSidePanel();
	}

	// ---- INPUTS ----
	// SWITCHES
	//------------------------------------------------------------------------------------------------
	protected void OnActionTriggered(string action, float multiplier)
	{
		//! TODO: set which input modes should trigger the actions in the component itself
		if (GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE)
			return;
		
		MissionWorkshopItem scenario = GetSelectedScenario();
		
		switch (action)
		{
			case UIConstants.MENU_ACTION_DOUBLE_CLICK:		OnLineClickInteraction(multiplier); break;
			case SCR_ScenarioUICommon.ACTION_RESTART:		Restart(scenario); break;
			case SCR_ScenarioUICommon.ACTION_FIND_SERVERS:	Join(scenario); break;
			case UIConstants.MENU_ACTION_FAVORITE:			OnFavouriteButton(); break;
			case SCR_ScenarioUICommon.ACTION_HOST:
			{
				if (!GetGame().IsPlatformGameConsole())
					Host(scenario);
				break;
			}
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
			case SCR_ScenarioUICommon.BUTTON_FAVORITE:		OnFavouriteButton(); break;
		}
	}
	
	// CLICKS
	//------------------------------------------------------------------------------------------------
	protected void OnLineClickInteraction(float multiplier)
	{
		//! multiplier value in the action is used to differentiate between single and double click

		SCR_ContentBrowser_ScenarioLineComponent lineComp = m_ClickedLine;
		if (!lineComp || !GetLineUnderCursor())
			return;

		switch (Math.Floor(multiplier))
		{
			case 1: OnLineClick(lineComp); break;
			case 2: OnLineDoubleClick(lineComp); break;
		}

		m_ClickedLine = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineClick(SCR_ContentBrowser_ScenarioLineComponent lineComp)
	{
		if (!lineComp)
			return;

		//! CONFIRMATION DIALOG
		MissionWorkshopItem scenario = lineComp.GetScenario();
		if (!scenario)
			return;

		//! If using Mouse single click opens confirmation dialog, double click goes straight to the play interaction
		if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE)
		{
			SCR_ScenarioConfirmationDialogUi scenarioConfirmationDialog;
			
			// Save details dialog
			SCR_ContentBrowser_GMSaveLineComponent saveLineComp = SCR_ContentBrowser_GMSaveLineComponent.Cast(lineComp);
			
			if (saveLineComp)
			{
				// zzz Issue is that this call just create plain SCR_ConfigurableDialog
				SCR_GMSaveDialog saveDialog = SCR_SaveWorkshopManagerUI.CreateSaveDetailsnDialog(scenario, GetOnLineFavorite());
				scenarioConfirmationDialog = saveDialog;

				if (saveDialog)
					saveDialog.SetSave(saveLineComp.GetSave());
			}
			
			// Scenario confirm dialog
			if (!scenarioConfirmationDialog)
				scenarioConfirmationDialog = SCR_ScenarioDialogs.CreateScenarioConfirmationDialog(scenario, GetOnLineFavorite());
			
			if (!scenarioConfirmationDialog)
			{
				OnPlayInteraction(scenario);
				return;
			}

			//! Bind dialog delegates
			scenarioConfirmationDialog.m_OnButtonPressed.Insert(OnConfirmationDialogButtonPressed);
			scenarioConfirmationDialog.GetOnFavorite().Insert(SetFavorite);
		}
		//! If using Gamepad or Keyboard there's no confirmation dialog and single click starts the play interaction
		else
		{
			OnPlayInteraction(scenario);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineDoubleClick(SCR_ContentBrowser_ScenarioLineComponent lineComp)
	{
		if (!lineComp)
			return;

		OnPlayInteraction(lineComp.GetScenario());
	}

	// BUTTONS
	//------------------------------------------------------------------------------------------------
	protected void OnFavouriteButton()
	{
		SetFavorite(GetSelectedScenario());
		if (m_NavFavorite && GetSelectedLine())
			m_NavFavorite.SetLabel(UIConstants.GetFavoriteLabel(GetSelectedScenario().IsFavorite()));
	}

	// INTERACTIONS
	//------------------------------------------------------------------------------------------------
	protected void OnPlayInteraction(MissionWorkshopItem scenario)
	{
		if (SCR_ScenarioUICommon.HasSave(scenario))
			Continue(scenario);
		else
			Play(scenario);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Play(MissionWorkshopItem scenario)
	{
		if (!SCR_ScenarioUICommon.CanPlay(scenario))
			return false;

		// Save behavior
		/*
		WorldSaveItem save = WorldSaveItem.Cast(scenario.GetOwner());
		if (save)
		{
			// Find save by id
			string id = save.Id();
			string fileName = ""; //GetGame().GetSaveManager().FindFileNameById(id);
			
			if (!fileName)
			{
				Print("Save to play was not found", LogLevel.WARNING);
				return;
			}
			
			GetGame().GetSaveManager().SetFileNameToLoad(fileName);
		}
		*/
		
		// Play scenario 
		SCR_ScenarioUICommon.TryPlayScenario(scenario);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Continue(MissionWorkshopItem scenario)
	{
		SCR_ScenarioUICommon.LoadSave(scenario, m_Header, ChimeraMenuPreset.ScenarioMenu);
		SCR_ScenarioUICommon.TryPlayScenario(scenario);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Restart(MissionWorkshopItem scenario)
	{
		if (!scenario || !SCR_ScenarioUICommon.CanPlay(scenario))
			return;

		m_SelectedScenario = scenario;
		m_Header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(scenario.Id()));

		if (!SCR_ScenarioUICommon.HasSave(scenario))
			return;

		SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateDialog(SCR_ScenarioUICommon.DIALOG_RESTART);
		dialog.m_OnConfirm.Insert(OnRestartConfirmed);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestartConfirmed()
	{
		SCR_ScenarioUICommon.RestartScenario(m_SelectedScenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void Join(MissionWorkshopItem scenario)
	{
		if (!scenario || !SCR_ScenarioUICommon.CanJoin(scenario))
			return;

		ServerBrowserMenuUI.TryOpenServerBrowserWithScenarioFilter(scenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void Host(MissionWorkshopItem scenario)
	{
		if (!scenario || !SCR_ScenarioUICommon.CanHost(scenario))
			return;

		// Open server hosting dialog
		ServerHostingUI dialog = SCR_CommonDialogs.CreateServerHostingDialog();

		if (dialog)
			dialog.SelectScenario(scenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetFavorite(MissionWorkshopItem scenario)
	{
		if (!scenario || !m_SelectedLine || m_SelectedLine.GetScenario() != scenario)
			return;

		bool isFavorite = !scenario.IsFavorite();

		//Update the scenario
		scenario.SetFavorite(isFavorite);

		//Update the widgets
		m_SelectedLine.NotifyScenarioUpdate(true);

		//Delegate to update the dialog
		if (m_OnLineFavorite)
			m_OnLineFavorite.Invoke(isFavorite);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CreateLines(array<MissionWorkshopItem> scenarios, Widget parent)
	{
		array<ResourceName> missionFilter();
		
		SCR_ScenarioMenuLoadContext context();
		context.m_aScenarios = scenarios;
		context.m_wListRoot = parent;
		
		foreach (MissionWorkshopItem scenario : scenarios)
		{
			missionFilter.Insert(scenario.Id())
		}

		return GetGame().GetSaveGameManager().RetrieveSaveGameInfo(missionFilter, new SaveGameOperationCb(OnSaveGameDataLoaded, context));
	}

	//------------------------------------------------------------------------------------------------
	protected Widget CreateLine(ResourceName layout, Widget parent, MissionWorkshopItem scenario)
	{
		Widget w = GetGame().GetWorkspace().CreateWidgets(layout, parent);
		if (!w)
			return null;

		SCR_ContentBrowser_ScenarioLineComponent comp = SCR_ContentBrowser_ScenarioLineComponent.FindComponent(w);
		if (!comp)
			return null;

		comp.SetScenario(scenario);
		m_aScenarioLines.Insert(comp);

		comp.GetOnFavorite().Insert(OnLineFavorite);
		comp.GetOnMouseInteractionButtonClicked().Insert(OnInteractionButtonPressed);
		comp.GetOnFocus().Insert(OnLineFocus);
		comp.GetOnFocusLost().Insert(OnLineFocusLost);
		comp.GetOnMouseEnter().Insert(OnLineMouseEnter);
		
		return w;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSaveGameDataLoaded(bool success, Managed context)
	{
		if (!success)
		{
			OnScenariosLoadFailed();
			return;
		}

		auto ctx = SCR_ScenarioMenuLoadContext.Cast(context);
		if (!ctx.m_wListRoot)
			return; // Menu closed already before callback could invoke

		foreach (MissionWorkshopItem scenario : ctx.m_aScenarios)
		{
			if (scenario)
				CreateLine(m_sLinesLayout, ctx.m_wListRoot, scenario);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnScenariosLoadFailed();
	
	//------------------------------------------------------------------------------------------------
	protected void InitWidgets();
	
	//------------------------------------------------------------------------------------------------
	// Inits workshop API according to current mode
	protected void InitWorkshopApi()
	{
		// Scan offline items if needed
		if (m_WorkshopApi.NeedAddonsScan())
			m_WorkshopApi.ScanOfflineItems();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateSidePanel()
	{
		SCR_ContentBrowser_ScenarioLineComponent lineComp = GetSelectedLine();
		if (!lineComp)
			return;
		
		// Display panel
		SCR_ContentBrowser_GMSaveLineComponent saveLineComp = SCR_ContentBrowser_GMSaveLineComponent.Cast(lineComp); 
		bool isSave = saveLineComp != null;
		
		if (m_AddonDetailsPanel)
			m_AddonDetailsPanel.GetRootWidget().SetVisible(isSave);
		
		if (m_ScenarioDetailsPanel)
			m_ScenarioDetailsPanel.GetRootWidget().SetVisible(!isSave);
		
		// Fill from save 
		if (isSave)
		{
			if (m_AddonDetailsPanel)
				m_AddonDetailsPanel.SetWorkshopItem(saveLineComp.GetSaveScrWorkshopItem());
			
			return;
		}
		
		// Fill scenario 
		MissionWorkshopItem scenario = lineComp.GetScenario();
		if (scenario && m_ScenarioDetailsPanel)
			m_ScenarioDetailsPanel.SetScenario(scenario);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateNavigationButtons(bool visible = true)
	{
		MissionWorkshopItem scenario = GetSelectedScenario();
		SCR_ScenarioUICommon.UpdateInputButtons(scenario, m_aRightFooterButtons, visible);
		
		if (m_NavFavorite)
		{
			m_NavFavorite.SetVisible(visible, false);
			if (visible)
				m_NavFavorite.SetLabel(UIConstants.GetFavoriteLabel(GetSelectedScenario().IsFavorite()));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ContentBrowser_ScenarioLineComponent GetSelectedLine()
	{
		// We are not over a line, use currently focused line
		Widget wfocused = GetGame().GetWorkspace().GetFocusedWidget();
		SCR_ContentBrowser_ScenarioLineComponent comp;
		if (wfocused)
			comp = SCR_ContentBrowser_ScenarioLineComponent.Cast(wfocused.FindHandler(SCR_ContentBrowser_ScenarioLineComponent));

		EInputDeviceType inputDevice = GetGame().GetInputManager().GetLastUsedInputDevice();
		bool isCursorOnInnerButton = m_SelectedLine && m_SelectedLine.IsInnerButtonInteraction();

		if (inputDevice == EInputDeviceType.MOUSE && (GetLineUnderCursor() || isCursorOnInnerButton))
			return m_SelectedLine;
		else
			return comp;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ContentBrowser_ScenarioLineComponent GetLineUnderCursor()
	{
		// Note that this returns null if the cursor is on a button nested inside the line main button

		Widget w = WidgetManager.GetWidgetUnderCursor();

		if (!w)
			return null;

		return SCR_ContentBrowser_ScenarioLineComponent.Cast(w.FindHandler(SCR_ContentBrowser_ScenarioLineComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected MissionWorkshopItem GetSelectedScenario()
	{
		SCR_ContentBrowser_ScenarioLineComponent comp = GetSelectedLine();

		if (!comp)
			return null;

		return comp.GetScenario();
	}

	// ---- PUBLIC ----
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBool GetOnLineFavorite()
	{
		if (!m_OnLineFavorite)
			m_OnLineFavorite = new ScriptInvokerBool();

		return m_OnLineFavorite;
	}
}
