/*
The overview tab of the details view
*/

class SCR_ContentBrowserDetails_OverviewSubMenu : SCR_ContentBrowser_ScenarioSubMenuBase
{
	protected ref SCR_WorkshopItem m_Item;

	protected ref SCR_ContentBrowserDetails_OverviewSubMenuWidgets m_Widgets = new SCR_ContentBrowserDetails_OverviewSubMenuWidgets;

	protected ref SCR_WorkshopDownloadSequence m_DownloadRequest;
	protected SCR_LoadingOverlayDialog m_LoadingOverlay;
	protected SCR_InputButtonComponent m_Downloads;
	protected SCR_InputButtonComponent m_LicenseAndContributors;
	
	protected SCR_ERevisionAvailability m_eRevisionAvailability;
	protected SCR_EAddonPrimaryActionState m_ePrimaryActionState;
	
	protected SCR_ScriptedWidgetTooltip m_Tooltip;
	
	protected bool m_bUpdatingContinuously;
	protected bool m_MouseTriggeredPanelUpdate;
	
	//------------------------------------------------------------------------------------------------
	void SetWorkshopItem(SCR_WorkshopItem item)
	{
		if (!item)
			return;
		
		ClearWorkshopItemInvokers();
		
		m_Item = item;
		
		m_Item.m_OnGetAsset.Insert(Callback_OnGetAsset);
		m_Item.m_OnChanged.Insert(Callback_OnItemChanged);
		m_Item.m_OnScenariosLoaded.Insert(Callback_OnDownloadScenarios);
		m_Item.m_OnTimeout.Insert(Callback_OnTimeout);
		
		// Init the addon details panel
		m_Widgets.m_AddonDetailsPanelComponent.SetWorkshopItem(m_Item);
		
		// Load Item
		LoadWorkshopDetails();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
	
		SCR_DownloadManager downloadManager = SCR_DownloadManager.GetInstance();
		if (downloadManager)
		{
			downloadManager.m_OnNewDownload.Insert(OnAnyNewDownload);
			downloadManager.m_OnDownloadFailed.Insert(OnAnyDownloadError);
			downloadManager.m_OnDownloadCanceled.Insert(OnAnyDownloadCompleted);
			downloadManager.m_OnDownloadComplete.Insert(OnAnyDownloadCompleted);
		}
		
		m_Widgets.m_DescriptionHoverDetectorComponent.GetOnMouseEnter().Insert(OnDescriptionMouseEnter);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
		
		StopContinuousUpdate();	
		ClearWorkshopItemInvokers();
		
		SCR_DownloadManager downloadManager = SCR_DownloadManager.GetInstance();
		if (downloadManager)
		{
			downloadManager.m_OnNewDownload.Remove(OnAnyNewDownload);
			downloadManager.m_OnDownloadFailed.Remove(OnAnyDownloadError);
			downloadManager.m_OnDownloadCanceled.Remove(OnAnyDownloadCompleted);
			downloadManager.m_OnDownloadComplete.Remove(OnAnyDownloadCompleted);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		m_ScenarioDetailsPanel = m_Widgets.m_ScenarioDetailsPanelComponent;
		
		// Hide verrsion selection in release
		#ifndef WORKSHOP_DEBUG
			m_Widgets.m_VersionComboBoxComponent0.SetVisible(false, false);
		#endif

		// Hide scenario and gallery section
		m_Widgets.m_wGallery.SetVisible(false);
		m_Widgets.m_wScenarioSection.SetVisible(false);

		// Init event handlers last of all.
		// We don't want event handlers to get called while we are still initializing UI.
		m_Widgets.m_VoteUpButtonComponent0.m_OnToggled.Insert(OnVoteUpButton);
		m_Widgets.m_VoteDownButtonComponent0.m_OnToggled.Insert(OnVoteDownButton);
		
		m_Widgets.m_EnableButtonComponent.m_OnToggled.Insert(OnEnableButtonToggled);
		m_Widgets.m_FavoriteButtonComponent0.m_OnToggled.Insert(OnAddonFavouriteButtonToggled);
		m_Widgets.m_RepairButtonComponent0.m_OnClicked.Insert(OnRepairActionButton);
		m_Widgets.m_ReportButtonComponent0.m_OnClicked.Insert(OnReportButton);			
		m_Widgets.m_PrimaryActionButtonComponent0.m_OnClicked.Insert(OnPrimaryActionButton);
		m_Widgets.m_DeleteButtonComponent0.m_OnClicked.Insert(OnDeleteButton);
		m_Widgets.m_LicensesComponent0.m_OnClicked.Insert(OnLicensesButton);
		m_Widgets.m_DependenciesComponent0.m_OnClicked.Insert(OnDependenciesButton);
		m_Widgets.m_DependentComponent0.m_OnClicked.Insert(OnDependentButton);

		m_Downloads = m_DynamicFooter.FindButton("DownloadManager");
		
		m_LicenseAndContributors = m_DynamicFooter.FindButton("LicenseAndContributors");
		if (m_LicenseAndContributors)
			m_LicenseAndContributors.m_OnActivated.Insert(OnLicensesButton);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();

		if (m_LicenseAndContributors)
			m_LicenseAndContributors.SetVisible(true, false);
		
		UpdateAllWidgets();

		// Set the default focused widget to one we're sure is always present
		Widget defaultFocus = m_Widgets.m_wFavoriteButton;
		GetGame().GetCallqueue().Call(UpdateFocus, defaultFocus);
		
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabRemove()
	{
		super.OnTabRemove();

		ClearWorkshopItemInvokers();
		UpdateNavigationButtons(false);
		
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();
		
		// This button must be visible in other tabs as well
		if (m_Downloads)
			m_Downloads.SetVisible(true);
		
		if (m_LicenseAndContributors)
			m_LicenseAndContributors.SetVisible(false, false);
		
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		//Bring back focus to the last selected line after closing a pop-up dialog
		if (!GetGame().GetMenuManager().IsAnyDialogOpen())
		{
			Widget target;
			
			if (m_LastSelectedLine)
				target = m_LastSelectedLine.GetRootWidget();
			else if (m_Widgets && m_Widgets.m_wScenariosList)
				target = m_Widgets.m_wScenariosList.GetChildren();
			
			if (!target)
				target = m_Widgets.m_wFavoriteButton;
				
			UpdateFocus(target);
		}
		
		super.OnMenuFocusGained();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnLineFocus(SCR_ScriptedWidgetComponent entry)
	{
		super.OnLineFocus(entry);

		if (m_Downloads)
			m_Downloads.SetVisible(false, false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnLineFocusLost(SCR_ScriptedWidgetComponent entry)
	{
		super.OnLineFocusLost(entry);
		
		if (m_Downloads)
			m_Downloads.SetVisible(true, false);
	}
	
	//------------------------------------------------------------------------------------------------
	// Restore the scenario details display if we move the mouse back on an already focused line
	override void OnLineMouseEnter(SCR_ScriptedWidgetComponent entry)
	{
		super.OnLineMouseEnter(entry);
		
		SCR_ContentBrowser_ScenarioLineComponent lineComp = SCR_ContentBrowser_ScenarioLineComponent.Cast(entry);
		if (!lineComp)
			return;
		
		if (m_MouseTriggeredPanelUpdate && lineComp.IsFocused())
		{
			GetGame().GetCallqueue().Call(UpdateSidePanel);
			m_MouseTriggeredPanelUpdate = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateNavigationButtons(bool visible = true)
	{
		visible =
			visible &&
			m_Item &&
			m_Item.GetOffline() &&
			GetSelectedLine() &&
			GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE;

		super.UpdateNavigationButtons(visible);
	}

	//------------------------------------------------------------------------------------------------
	override void OnLineClick(SCR_ContentBrowser_ScenarioLineComponent lineComp)
	{
		if (!lineComp)
			return;

		//! Handle interaction with addon not downloaded
		if (!m_Item || !m_Item.GetOffline())
		{
			if (GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE)
				LineInteractionTryDownload(lineComp);
			
			return;
		}

		//! Interaction with addon downloaded
		super.OnLineClick(lineComp);
	}

	//------------------------------------------------------------------------------------------------
	override void OnLineDoubleClick(SCR_ContentBrowser_ScenarioLineComponent lineComp)
	{
		if (!lineComp)
			return;

		//! Handle interaction with addon not downloaded
		if (!m_Item || !m_Item.GetOffline())
		{
			LineInteractionTryDownload(lineComp);
			return;
		}

		//! Handle interaction with addon dowloaded
		super.OnLineDoubleClick(lineComp);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayInteraction(MissionWorkshopItem scenario)
	{
		if (!m_Item || !m_Item.GetOffline())
			return;

		super.OnPlayInteraction(scenario);
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
	override void Join(MissionWorkshopItem scenario)
	{
		if (!scenario || !SCR_ScenarioUICommon.CanJoin(scenario))
			return;

		// Interrupt when server config is being edited
		// just temporary solution to prevent endless opening of menus
		if (ServerHostingUI.GetTemporaryConfig() && SCR_ScenarioUICommon.CanHost(scenario))
		{
			SCR_MenuToolsLib.GetEventOnAllMenuClosed().Insert(OnAllMenuCloseJoin);
			SCR_MenuToolsLib.CloseAllMenus({MainMenuUI, ServerBrowserMenuUI});
			return;
		}

		super.Join(scenario);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets favorite state of the Scenario line
	override void SetFavorite(MissionWorkshopItem scenario)
	{
		if (!m_Item || !m_Item.GetOffline())
			return;

		super.SetFavorite(scenario);
	}

	//------------------------------------------------------------------------------------------------
	//! Called from scenario line component when scenario state changes
	override void OnScenarioStateChanged(SCR_ContentBrowser_ScenarioLineComponent comp)
	{
		HandleDownloadChanges();
		
		super.OnScenarioStateChanged(comp);
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitWidgets()
	{
		super.InitWidgets();
		m_Widgets.Init(m_wRoot);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateSidePanel()
	{
		super.UpdateSidePanel();
		
		GetGame().GetCallqueue().Remove(UpdateSidePanel);
		
		SCR_ContentBrowser_ScenarioLineComponent lineComp = GetSelectedLine();
		
		m_Widgets.m_wAddonDetailsPanel.SetVisible(!lineComp);
		m_Widgets.m_wScenarioDetailsPanel.SetVisible(lineComp != null);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HandleDownloadChanges()
	{
		if (!m_Item)
			return;

		UpdateStateFlags();
		
		UpdatePrimaryActionButton();
		UpdateDownloadProgressBar();
		UpdateDeleteButton();
		
		WorkshopItem item = m_Item.GetWorkshopItem();
		Revision itemRevision = item.GetActiveRevision();
		
		m_Widgets.m_wRepairButton.SetVisible(itemRevision && itemRevision.IsCorrupted());

		// Continuous update
		if (SCR_WorkshopUiCommon.IsDownloadingAddon(m_Item))
			StartContinuousUpdate();
		else
			StopContinuousUpdate();
	}

	//------------------------------------------------------------------------------------------------
	protected void StartContinuousUpdate()
	{
		if (m_bUpdatingContinuously || !m_Item)
			return;

		// Update download progress widgets
		GetGame().GetCallqueue().CallLater(ContinuousUpdate, SCR_WorkshopUiCommon.CONTINUOUS_UPDATE_DELAY, true);

		m_bUpdatingContinuously = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void StopContinuousUpdate()
	{
		if (m_bUpdatingContinuously)
			GetGame().GetCallqueue().Remove(ContinuousUpdate);

		m_bUpdatingContinuously = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void ContinuousUpdate()
	{
		if (!SCR_WorkshopUiCommon.IsDownloadingAddon(m_Item))
			StopContinuousUpdate();

		UpdatePrimaryActionButton();
		UpdateDownloadProgressBar();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearWorkshopItemInvokers()
	{
		if (!m_Item)
			return;

		m_Item.m_OnGetAsset.Remove(Callback_OnGetAsset);
		m_Item.m_OnChanged.Remove(Callback_OnItemChanged);
		m_Item.m_OnScenariosLoaded.Remove(Callback_OnDownloadScenarios);
		m_Item.m_OnTimeout.Remove(Callback_OnTimeout);
	}
	
	// ---- CALLBACKS ----
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnGetAsset(SCR_WorkshopItem item)
	{
		UpdateGallery();
		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnItemChanged(SCR_WorkshopItem item)
	{
		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	//! Called when we receive scenarios from backend
	protected void Callback_OnDownloadScenarios(SCR_WorkshopItem item)
	{
		array<MissionWorkshopItem> scenarios = {};
		m_Item.GetScenarios(scenarios);

		#ifdef WORKSHOP_DEBUG
		ContentBrowserUI._print(string.Format("Downloaded %1 scenarios", scenarios.Count()));
		#endif

		m_Widgets.m_wScenarioSection.SetVisible(!scenarios.IsEmpty());

		// Delete old entries
		while (m_Widgets.m_wScenariosList.GetChildren())
		{
			m_Widgets.m_wScenariosList.RemoveChild(m_Widgets.m_wScenariosList.GetChildren());
		}

		// Sort missions by name
		SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionName>.HeapSort(scenarios, false);

		if (CreateLines(scenarios, m_Widgets.m_wScenariosList))
			UpdateScenarioLines();
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnTimeout()
	{
		// Ignore if we are not connected to backend at all
		if (!SCR_ServicesStatusHelper.IsAuthenticated())
			return;

		SCR_ConfigurableDialogUi dlg = SCR_CommonDialogs.CreateTimeoutOkDialog();
		dlg.m_OnClose.Insert(RequestCloseMenu);
	}

	// --- Any downloads ---
	// We need to update the progress bar and main action display if we start downloading this item or any of its dependencies
	//------------------------------------------------------------------------------------------------
	protected void OnAnyNewDownload(SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	{
		GetGame().GetCallqueue().Call(HandleDownloadChanges);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAnyDownloadError(SCR_WorkshopItemActionDownload action, int reason)
	{
		HandleDownloadChanges();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAnyDownloadCompleted(SCR_WorkshopItemActionDownload action)
	{
		HandleDownloadChanges();
	}
	
	// ---- WIDGET CALLBACKS ----
	//------------------------------------------------------------------------------------------------
	protected void OnDeleteButton()
	{
		if (!m_Item)
			return;
		
		// Unsubscribe/delete any addon data is available
		if (m_Item.GetOffline() || m_Item.GetWorkshopItem().GetDownloadingRevision())
			SCR_WorkshopUiCommon.OnDeleteAddonButton(m_Item);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLicensesButton()
	{
		if (!m_Item)
			return;
		
		SCR_ModDetailsAdditionalInfoDialog.CreateDialog(m_Item);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPrimaryActionButton()
	{
		Revision revisionToDownload;

		#ifdef WORKSHOP_DEBUG
			string strSpecificVersion = m_Widgets.m_VersionComboBoxComponent0.GetCurrentItem();
			revisionToDownload = m_Item.FindRevision(strSpecificVersion);
		#endif
		
		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_Item, m_DownloadRequest, revisionToDownload);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDependenciesButton()
	{
		RequestTabChange(SCR_EModDetailsMenuTabs.DEPENDENCY);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDependentButton()
	{
		RequestTabChange(SCR_EModDetailsMenuTabs.DEPENDENT);
	}
	
	// --- Rating ---
	// We have two buttons: vote up and vote down. They have a toggled visual
	// When we interact with one of the buttons, we also need to update the other's toggled visual
	// They are mutually esclusive, so rating has three states: neutral, positive, negative
	//------------------------------------------------------------------------------------------------
	// By the time when this is called, the button has already been toggled by the widget library code
	protected void OnVoteUpButton()
	{
		bool toggled = m_Widgets.m_VoteUpButtonComponent0.GetToggled();
		OnVoteButtons(toggled, !toggled);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVoteDownButton()
	{
		bool toggled = m_Widgets.m_VoteDownButtonComponent0.GetToggled();
		OnVoteButtons(!toggled, !toggled);
	}

	//------------------------------------------------------------------------------------------------
	// resetAll - resets the rating
	// voteUp - if not reset rating, vote up or down
	protected void OnVoteButtons(bool voteUp, bool resetAll)
	{
		if (resetAll)
			m_Item.ResetMyRating();
		else
			m_Item.SetMyRating(voteUp);
		
		//TODO: delayed update based on callback
		UpdateRatingButtons();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEnableButtonToggled(SCR_ModularButtonComponent comp)
	{
		SCR_WorkshopUiCommon.OnEnableAddonToggleButton(m_Item, comp);
	}

	//------------------------------------------------------------------------------------------------
	//! Start the repairing process of the addon
	protected void OnRepairActionButton()
	{
		if (!m_Item || m_Item.GetRestricted())
			return;

		SCR_ValidateRepair_Dialog dialogValidator = SCR_CommonDialogs.CreateValidateRepairDialog();
		dialogValidator.LoadAddon(m_Item);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets favorite state of the Addon
	protected void OnAddonFavouriteButtonToggled()
	{
		if (!m_Item || m_Item.GetRestricted())
			return;

		m_Item.SetFavourite(!m_Item.GetFavourite());
		UpdateFavoriteButtonTooltip();
	}

	//------------------------------------------------------------------------------------------------
	//! This button used in reported and not reported state. It either sends a report or cancels it.
	protected void OnReportButton()
	{
		// Bail if not in backend
		if (!m_Item.GetOnline())
			return;

		// Individually reported mod - reported by me
		if (m_Item.GetReportedByMe())
		{
			if (m_LoadingOverlay)
				m_LoadingOverlay.Close();

			m_LoadingOverlay = SCR_LoadingOverlayDialog.Create();
			m_LoadingOverlay.m_OnCloseStarted.Insert(Callback_OnLoadMyReportCancel); // Called when user decides to cancel and not wait

			m_Item.m_OnMyReportLoaded.Insert(Callback_OnLoadMyReportSuccess);
			m_Item.m_OnMyReportLoadError.Insert(Callback_OnLoadMyReportError);
			m_Item.LoadReport();

			return;
		}

		// TODO: move to configurable dialog
		SCR_ModReportDialogComponent comp = SCR_ModReportDialogComponent.Cast(m_wRoot.FindHandler(SCR_ModReportDialogComponent));
		if (!comp)
			return;
		
		comp.GetOnItemReportSuccessDialogClose().Insert(RequestCloseMenu);
		
		// Mod with reported author
		if (m_Item.GetModAuthorReportedByMe())
		{
			comp.OpenSelectReportAuthor(m_Item);
			m_Item.m_OnReportStateChanged.Insert(OnReportStateChanged);
			
			return;
		}

		// Not reported
		comp.OpenSelectReport(m_Item);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnReportStateChanged()
	{
		LoadWorkshopDetails();
		m_Item.m_OnReportStateChanged.Remove(OnReportStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAllMenuCloseJoin()
	{
		SCR_MenuToolsLib.GetEventOnAllMenuClosed().Remove(OnAllMenuCloseJoin);
		SCR_CommonDialogs.CreateServerHostingDialog();
	}

	//------------------------------------------------------------------------------------------------
	// Restore the addon details display if we move the mouse in the description area
	protected void OnDescriptionMouseEnter()
	{
		m_Widgets.m_wAddonDetailsPanel.SetVisible(true);
		m_Widgets.m_wScenarioDetailsPanel.SetVisible(false);
		
		m_MouseTriggeredPanelUpdate = true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Unreporting, loading report
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnLoadMyReportCancel()
	{
		m_Item.m_OnMyReportLoaded.Remove(Callback_OnLoadMyReportSuccess);	// todo improve error handling :(
		m_Item.m_OnMyReportLoadError.Remove(Callback_OnLoadMyReportError);
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnLoadMyReportSuccess()
	{
		m_Item.m_OnMyReportLoaded.Remove(Callback_OnLoadMyReportSuccess);
		m_Item.m_OnMyReportLoadError.Remove(Callback_OnLoadMyReportError);

		m_LoadingOverlay.CloseAnimated();

		new SCR_CancelMyReportDialog(m_Item);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnLoadMyReportError()
	{
		m_Item.m_OnMyReportLoaded.Remove(Callback_OnLoadMyReportSuccess);
		m_Item.m_OnMyReportLoadError.Remove(Callback_OnLoadMyReportError);

		m_LoadingOverlay.CloseAnimated();
		SCR_CommonDialogs.CreateRequestErrorDialog();
	}

	//------------------------------------------------------------------------------------------------
	// Methods for updating state of individual components
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void LoadWorkshopDetails()
	{
		if (!m_Item)
			return;

		if (!m_Item.GetDetailsLoaded())
			m_Item.LoadDetails();
		
		Callback_OnDownloadScenarios(m_Item);
		Callback_OnItemChanged(m_Item);
		Callback_OnGetAsset(m_Item);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		if (!m_Item)
			return;
		
		HandleDownloadChanges();
		
		UpdateButtons();
		UpdateScenarioLines();
		//UpdateTimeInfoWidgets();
		UpdateSizeInfoWidgets();
		UpdateVersionInfoWidgets();
		
		UpdateVersionComboBox();
		ShowBackendEnv();
		
		m_Widgets.m_wDescriptionText.SetText(m_Item.GetDescription());
		m_Widgets.m_wSummaryText.SetText(m_Item.GetSummary());
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateStateFlags()
	{
		if (!m_Item)
			return;
		
		SCR_AddonManager addonManager = SCR_AddonManager.GetInstance();
		WorkshopItem item = m_Item.GetWorkshopItem();
		EWorkshopItemProblem itemProblem = m_Item.GetHighestPriorityProblem();
		
		// --- Revision Availability ---
		m_eRevisionAvailability = SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY;
		if (item)
			m_eRevisionAvailability = addonManager.ItemAvailability(item);

		// --- Primary Action State ---
		m_ePrimaryActionState = SCR_WorkshopUiCommon.GetPrimaryActionState(m_Item);
	}
	
	//------------------------------------------------------------------------------------------------
	//! updated m_Widgets which depend on state of the mod (so everything except for description, gallery)
	protected void UpdateButtons()
	{
		if (!m_bShown || !m_Item)
			return;
		
		// --- Enable button ---
		SCR_WorkshopUiCommon.UpdateEnableAddonToggleButton(m_Widgets.m_EnableButtonComponent, m_Item, m_ePrimaryActionState, false);
		UpdateEnableButtonTooltip();

		// --- Report button ---
		m_Widgets.m_ReportButtonComponent0.SetEnabled(m_Item.GetOnline());
		m_Widgets.m_ReportButtonComponent0.SetToggled(m_Item.GetReportedByMe() || m_Item.GetModAuthorReportedByMe(), false);
		UpdateReportButtonTooltip();

		// --- Vote up/down ---
		UpdateRatingButtons();

		// --- Favourite button ---
		m_Widgets.m_FavoriteButtonComponent0.SetToggled(m_Item.GetFavourite(), false);
		UpdateFavoriteButtonTooltip();
		
		// --- Dependencies ---
		UpdateDependencyCountButtons();
				
		// --- Navigation buttons ---
		UpdateNavigationButtons();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdatePrimaryActionButton()
	{
		if (!m_Item)
			return;

		m_Widgets.m_wPrimaryActionButton.SetVisible(m_ePrimaryActionState != SCR_EAddonPrimaryActionState.DOWNLOADED && !m_Item.GetRestricted());
		if (!m_Widgets.m_wPrimaryActionButton.IsVisible())
			return;

		UpdatePrimaryActionTooltip();
		
		SCR_ButtonEffectColor iconColorEffect = SCR_ButtonEffectColor.Cast(m_Widgets.m_PrimaryActionButtonComponent0.FindEffect(SCR_ListEntryHelper.EFFECT_ICON_COLOR));
		if (!iconColorEffect)
			return;
		
		SCR_ButtonEffectColor messageColorEffect = SCR_ButtonEffectColor.Cast(m_Widgets.m_PrimaryActionButtonComponent0.FindEffect(SCR_ListEntryHelper.EFFECT_MESSAGE_COLOR));
		if (!messageColorEffect)
			return;

		string icon;
		string message;
		Color iconColor;
		Color messageColor;
		SCR_WorkshopUiCommon.GetPrimaryActionLook(m_ePrimaryActionState, m_eRevisionAvailability, m_Item, icon, iconColor, message, messageColor);
	
		if (iconColor)
		{
			iconColorEffect.m_cDefault = iconColor;
			iconColorEffect.m_cFocusLost = iconColor;
		}
		
		if (messageColor)
		{
			messageColorEffect.m_cDefault = messageColor;
			messageColorEffect.m_cFocusLost = messageColor;
		}
		
		m_Widgets.m_PrimaryActionButtonComponent0.InvokeAllEnabledEffects(true);
		
		m_Widgets.m_PrimaryActionButtonComponent1.SetImage(icon);
		m_Widgets.m_PrimaryActionButtonComponent2.SetTitle(message);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateDeleteButton()
	{
		if (!m_Item)
			return;
		
		bool downloading = SCR_WorkshopUiCommon.IsDownloadingAddonOrDependencies(m_Item);
		bool differentEnvironment = m_Item.GetWorkshopItem().GetBackendEnv() != GetGame().GetBackendApi().GetBackendEnv();
		SCR_ERevisionAvailability availability = SCR_AddonManager.ItemAvailability(m_Item.Internal_GetWorkshopItem());
		
		m_Widgets.m_DeleteButtonComponent0.SetVisible(
			!downloading &&
			m_Item.GetOnline() &&
			(differentEnvironment || m_Item.GetOffline()) &&
			availability != SCR_ERevisionAvailability.ERA_DOWNLOAD_NOT_FINISHED
		)
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateDownloadProgressBar()
	{
		if (!m_Item)
			return;
		
		float progress;

		if (m_Item && m_ePrimaryActionState == SCR_EAddonPrimaryActionState.DOWNLOADING)
			progress = SCR_DownloadManager.GetItemDownloadActionsProgress(m_Item);

		m_Widgets.m_wDownloadProgressBar.SetCurrent(progress);
		m_Widgets.m_wDownloadProgressBarOverlay.SetVisible(progress > 0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateRatingButtons()
	{
		if (!m_Item)
			return;
		
		m_Widgets.m_VoteUpButtonComponent0.SetEnabled(m_Item.GetOnline());
		m_Widgets.m_VoteDownButtonComponent0.SetEnabled(m_Item.GetOnline());

		bool upvote, ratingSet;
		m_Item.GetMyRating(ratingSet, upvote);

		m_Widgets.m_VoteUpButtonComponent0.SetToggled(ratingSet && upvote, false);
		m_Widgets.m_VoteDownButtonComponent0.SetToggled(ratingSet && !upvote, false);
		
		bool restricted = m_Item.GetRestricted();
		int upVotes = SCR_WorkshopUiCommon.GetUpvotes(m_Item);
		int downVotes = SCR_WorkshopUiCommon.GetDownvotes(m_Item);
		
		WorkshopItem item = m_Item.GetWorkshopItem();
		bool showVotes;
		if (item)
			showVotes = item.IsAuthor() || item.IsContributor();
		
		m_Widgets.m_VoteUpButtonComponent2.SetTitleVisible(showVotes);
		m_Widgets.m_VoteDownButtonComponent2.SetTitleVisible(showVotes);

		if (showVotes)
		{
			m_Widgets.m_VoteUpButtonComponent2.SetTitle(upVotes.ToString());
			m_Widgets.m_VoteDownButtonComponent2.SetTitle(downVotes.ToString());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateDependencyCountButtons()
	{
		if (!m_Item)
			return;

		int nDependent = Math.ClampInt(SCR_WorkshopUiCommon.GetDownloadedDependentAddons(m_Item).Count(), 0, SCR_WorkshopUiCommon.MAX_DEPENDENCIES_SHOWN);
		int nDependencies = Math.ClampInt(m_Item.GetLatestDependencies().Count(), 0, SCR_WorkshopUiCommon.MAX_DEPENDENCIES_SHOWN);

		// Displays
		m_Widgets.m_wDependent.SetVisible(nDependent > 0);
		m_Widgets.m_wDependencies.SetVisible(nDependencies > 0);
		
		if (nDependent > 0)
		{
			if (nDependent == 1)
				m_Widgets.m_DependentComponent2.SetTitle(SCR_WorkshopUiCommon.LABEL_DEPENDENT_NUMBER_ONE);
			else
				m_Widgets.m_DependentComponent2.SetTitle(WidgetManager.Translate(SCR_WorkshopUiCommon.LABEL_DEPENDENT_NUMBER, nDependent));
		}
		
		if (nDependencies > 0)
		{
			if (nDependencies == 1)
				m_Widgets.m_DependenciesComponent2.SetTitle(SCR_WorkshopUiCommon.LABEL_DEPENDENCIES_NUMBER_ONE);
			else
				m_Widgets.m_DependenciesComponent2.SetTitle(WidgetManager.Translate(SCR_WorkshopUiCommon.LABEL_DEPENDENCIES_NUMBER, nDependencies));
		}
		
		UpdateDependenciesTooltip();
	}
	/* TODO: disabled for now, we might return to this display in the future
	//------------------------------------------------------------------------------------------------
	protected void UpdateTimeInfoWidgets()
	{
		if (!m_Item)
			return;
		
		// Time since first downloaded
		int time = m_Item.GetTimeSinceFirstDownload();
		m_Widgets.m_wLastDownloadedHorizontalLayout.SetVisible(time >= 0);
		
		if (time >= 0)
			m_Widgets.m_wLastDownloaded.SetText(SCR_FormatHelper.GetTimeSinceEventImprecise(time));
		
		// Time since last played
		time = m_Item.GetTimeSinceLastPlay();
		m_Widgets.m_wLastPlayedHorizontalLayout.SetVisible(time >= 0);
		
		if (time >= 0)
			m_Widgets.m_wLastPlayed.SetText(SCR_FormatHelper.GetTimeSinceEventImprecise(time));
	}*/
	
	//------------------------------------------------------------------------------------------------	
	protected void UpdateSizeInfoWidgets()
	{
		if (!m_Item)
			return;
		
		float sizef = m_Item.GetSizeBytes();
		string sizeStr = SCR_ByteFormat.GetReadableSize(sizef);
		m_Widgets.m_AddonSizeComponent1.SetTitle(sizeStr);
	}
	
	//------------------------------------------------------------------------------------------------	
	protected void UpdateVersionInfoWidgets()
	{
		bool showUpdateText;
		string currentVersionIcon;
		Color currentVersionIconColor;
		string currentVersionText;
		Color currentVersionTextColor;
		string updateVersionText;
		
		SCR_WorkshopUiCommon.GetVersionDisplayLook(
			m_Item, 
			showUpdateText, 
			currentVersionIcon, 
			currentVersionIconColor, 
			currentVersionText, 
			currentVersionTextColor, 
			updateVersionText);
		
		m_Widgets.m_wVersionUpdateHorizontalLayout.SetVisible(showUpdateText);
		m_Widgets.m_wCurrentVersion.SetColor(currentVersionTextColor);
		m_Widgets.m_wCurrentVersion.SetText(currentVersionText);
		m_Widgets.m_wUpdateVersion.SetText(updateVersionText);
		m_Widgets.m_wCurrentVersionIcon.SetColor(currentVersionIconColor);
		m_Widgets.m_wCurrentVersionIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, currentVersionIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateGallery()
	{
		if (!m_Item)
		{
			m_Widgets.m_wGallery.SetVisible(false);
			return;
		}
		
		array<BackendImage> galleryImages = m_Item.GetGallery();
		
		m_Widgets.m_wGallery.SetVisible(!galleryImages.IsEmpty() && !m_Item.GetRestricted());

		if (!galleryImages.IsEmpty())
			m_Widgets.m_GalleryComponent.SetImages(galleryImages);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowBackendEnv()
	{
		WorkshopItem item = m_Item.GetWorkshopItem();
		if (!item)
		{
			m_Widgets.m_wBackendSource.SetVisible(false);
			return;
		}
		
		string gameEnv = GetGame().GetBackendApi().GetBackendEnv();
		string modEnv = m_Item.GetWorkshopItem().GetBackendEnv();

		bool display = (modEnv != "local") && (modEnv != "ask") && (modEnv != "invalid");
		bool envMatch = gameEnv == modEnv;

		m_Widgets.m_wBackendSource.SetVisible(display && !envMatch);
		if (!display)
			return;

		m_Widgets.m_BackendSourceComponent1.SetTitle(modEnv);

		if (envMatch)
			m_Widgets.m_BackendSourceComponent0.SetIconColor(UIColors.CONFIRM);
		else
			m_Widgets.m_BackendSourceComponent0.SetIconColor(UIColors.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateVersionComboBox()
	{
		#ifdef WORKSHOP_DEBUG
			// Fill version combo box - only for testing!
			if (m_Widgets.m_VersionComboBoxComponent0.GetNumItems() == 0)
			{
				array<string> versions = m_Item.GetVersions();
				if (!versions.IsEmpty())
				{
					m_Widgets.m_VersionComboBoxComponent0.ClearAll();
					foreach (string version : versions)
					{
						m_Widgets.m_VersionComboBoxComponent0.AddItem(version);
					}
				}
			}
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateScenarioLines()
	{
		if (!m_Item)
			return;
		
		foreach (SCR_ContentBrowser_ScenarioLineComponent scenarioLine : m_aScenarioLines)
		{
			if (!scenarioLine)
				continue;
			
			// Show favorites and mouse buttons only when downloaded
			scenarioLine.NotifyScenarioUpdate(m_Item.GetOffline());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void LineInteractionTryDownload(SCR_ContentBrowser_ScenarioLineComponent line)
	{
		// Check if the addon is downloaded
		if (!line || (m_Item && m_Item.GetOffline()))
			return;

		//! START DOWNLOAD
		// Clicking a line without the addon installed starts the download
		bool inProgress, paused;
		float progress;
		Revision revision;
		m_Item.GetDownloadState(inProgress, paused, progress, revision);

		if (!inProgress && !paused)
			OnPrimaryActionButton();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateFocus(Widget target)
	{
		GetGame().GetWorkspace().SetFocusedWidget(target);
	}
	
	
	// ---- Tooltips ----
	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		m_Tooltip = tooltip;
		
		UpdatePrimaryActionTooltip();
		UpdateEnableButtonTooltip();
		UpdateReportButtonTooltip();
		UpdateFavoriteButtonTooltip();
		UpdateDependenciesTooltip();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdatePrimaryActionTooltip()
	{
		if (!m_Tooltip || !m_Tooltip.GetContent() || !m_Tooltip.IsValid("MainAction_Simple", m_Widgets.m_wPrimaryActionButton))
			return;
		
		m_Tooltip.GetContent().SetMessage(SCR_WorkshopUiCommon.GetPrimaryActionTooltipMessage(m_ePrimaryActionState, m_Item));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateEnableButtonTooltip()
	{
		if (!m_Item || !m_Tooltip || !m_Tooltip.GetContent() || !m_Tooltip.IsValid("Enable_Simple", m_Widgets.m_wEnableButton))
			return;
		
		if (m_Item.GetEnabled())
			m_Tooltip.GetContent().SetMessage(SCR_WorkshopUiCommon.LABEL_DISABLE);
		else
			m_Tooltip.GetContent().SetMessage(SCR_WorkshopUiCommon.LABEL_ENABLE);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateReportButtonTooltip()
	{
		if (!m_Item || !m_Tooltip || !m_Tooltip.GetContent() || !m_Tooltip.IsValid("Report", m_Widgets.m_wReportButton))
			return;
		
		if (m_Item.GetReportedByMe())
			m_Tooltip.GetContent().SetMessage(SCR_WorkshopUiCommon.LABEL_CANCEL_REPORT);
		else
			m_Tooltip.GetContent().SetMessage(SCR_WorkshopUiCommon.LABEL_REPORT);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateFavoriteButtonTooltip()
	{
		if (!m_Item || !m_Tooltip || !m_Tooltip.GetContent() || !m_Tooltip.IsValid("Favorite_Simple", m_Widgets.m_wFavoriteButton))
			return;
		
		m_Tooltip.GetContent().SetMessage(UIConstants.GetFavoriteLabel(m_Item.GetFavourite()));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateDependenciesTooltip()
	{
		if (!m_Item || !m_Tooltip || !m_Tooltip.GetContentRoot())
			return;
		
		SCR_ContentBrowser_AddonsTooltipComponent comp = SCR_ContentBrowser_AddonsTooltipComponent.FindComponent(m_Tooltip.GetContentRoot());
		if (!comp)
			return;
		
		if (m_Tooltip.IsValid("Dependent", m_Widgets.m_wDependent))
			comp.Init(SCR_WorkshopUiCommon.GetDownloadedDependentAddons(m_Item));
		else if (m_Tooltip.IsValid("Dependencies", m_Widgets.m_wDependencies))
			comp.Init(m_Item.GetLatestDependencies());
	}
}