class SCR_WorkshopAddonBarComponent : SCR_ScriptedWidgetComponent
{
	protected ref SCR_WorkshopAddonBarWidgets m_Widgets = new SCR_WorkshopAddonBarWidgets();

	[Attribute("20", UIWidgets.EditBox, desc: "After how many character should be display preset name cut")]
	protected int m_iPresetNameCap;

	protected static SCR_ConfigurableDialogUi m_FailDialog;

	protected ref SCR_AddonPatchSizeLoader m_Loader = new SCR_AddonPatchSizeLoader();
	protected SCR_LoadingOverlayDialog m_LoadingOverlay;

	protected const int UPDATE_DELAY = 250;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (!GetGame().InPlayMode())
			return;

		m_Widgets.Init(w);

		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		if (!mgr)
			return;

		mgr.m_OnAddonsEnabledChanged.Insert(Callback_OnAddonsEnabledChanged);
		mgr.GetPresetStorage().GetEventOnUsedPresetChanged().Insert(OnUsedPresetChanged);

		OnUsedPresetChanged(mgr.GetPresetStorage().GetUsedPreset());

		UpdateAllWidgets();

		m_Widgets.m_PresetsButtonComponent.GetButton().m_OnClicked.Insert(SCR_CommonDialogs.CreateModPresetsDialog);
		m_Widgets.m_UpdateButtonComponent.GetButton().m_OnClicked.Insert(OnUpdateButtonClicked);

		HandleUpdatesButton();
		GetGame().GetCallqueue().CallLater(HandleUpdatesButton, UPDATE_DELAY, true);
		
		SCR_MenuHelper.GetOnMenuOpen().Insert(OnMenuShow);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();

		if (mgr)
			SCR_AddonManager.GetInstance().m_OnAddonsEnabledChanged.Remove(Callback_OnAddonsEnabledChanged);
		
		if (m_LoadingOverlay)
			m_LoadingOverlay.Close();
		
		GetGame().GetCallqueue().Remove(HandleUpdatesButton);
		
		SCR_MenuHelper.GetOnMenuOpen().Remove(OnMenuShow);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuShow(ChimeraMenuBase menu)
	{
		if (ChimeraMenuBase.GetOwnerMenu(m_wRoot) == menu)
			HandleUpdatesButton();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnAddonsEnabledChanged()
	{
		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	//! Show actual preset name on preset name change
	protected void OnUsedPresetChanged(string name)
	{
		// Cap
		if (!name.IsEmpty() && name.Length() > m_iPresetNameCap)
			name = name.Substring(0, m_iPresetNameCap) + "...";

		m_Widgets.m_PresetsButtonComponent.SetLabelText(name);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();

		if (!mgr)
			return;

		// Mod count text
		int nAddonsEnabled = SCR_AddonManager.CountItemsAnd(SCR_AddonManager.GetInstance().GetOfflineAddons(), EWorkshopItemQuery.ENABLED | EWorkshopItemQuery.ONLY_WORKSHOP_ITEM);
		m_Widgets.m_PresetsButtonComponent.SetCountText(nAddonsEnabled.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUpdateButtonClicked()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		array<ref SCR_WorkshopItem> addonsOutdated = SCR_AddonManager.SelectItemsBasic(mgr.GetOfflineAddons(), EWorkshopItemQuery.UPDATE_AVAILABLE);

		// Load patch sizes for latest revision
		m_Loader = new SCR_AddonPatchSizeLoader();

		foreach (SCR_WorkshopItem item : addonsOutdated)
		{
			Revision rev = item.GetLatestRevision();
			item.SetItemTargetRevision(rev);
			m_Loader.InsertItem(item);
		}

		m_LoadingOverlay = SCR_LoadingOverlayDialog.Create();

		m_Loader.GetOnAllPatchSizeLoaded().Insert(OnUpdatePatchSizeLoaded);
		m_Loader.LoadPatchSizes();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUpdatePatchSizeLoaded(SCR_AddonPatchSizeLoader loader, bool allLoaded)
	{
		// Cleanup
		m_Loader.GetOnAllPatchSizeLoaded().Remove(OnUpdatePatchSizeLoaded);
		if (m_LoadingOverlay)
			m_LoadingOverlay.Close();

		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		array<ref SCR_WorkshopItem> addonsOutdated = SCR_AddonManager.SelectItemsBasic(mgr.GetOfflineAddons(), EWorkshopItemQuery.UPDATE_AVAILABLE);

		// Open download confirmation dialog
		array<ref Tuple2<SCR_WorkshopItem, ref Revision>> addonsAndVersions = {};
		foreach (SCR_WorkshopItem item : addonsOutdated)
		{
			addonsAndVersions.Insert(new Tuple2<SCR_WorkshopItem, ref Revision>(item, item.GetLatestRevision()));

		}

		SCR_AddonUpdateConfirmationDialog dialog = SCR_AddonUpdateConfirmationDialog.CreateForUpdates(addonsAndVersions, false);
		if (dialog)
			dialog.m_OnClose.Insert(OnAddonUpdateDialogClose);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAddonUpdateDialogClose(SCR_ConfigurableDialogUi dialog)
	{
		HandleUpdatesButton();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HandleUpdatesButton()
	{
		if (!SCR_MenuHelper.IsInTopMenu(GetRootWidget()))
		{
			m_Widgets.m_UpdateButtonComponent.SetVisible(false);
			return;
		}
		
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		array<ref SCR_WorkshopItem> addonsOutdated = {};
		
		if (mgr)
			addonsOutdated = SCR_AddonManager.SelectItemsBasic(mgr.GetOfflineAddons(), EWorkshopItemQuery.UPDATE_AVAILABLE);

		int nOutdated;
		
		foreach (SCR_WorkshopItem item : addonsOutdated)
		{
			if (!item.IsDownloadRunning())
				nOutdated++;
		}

		m_Widgets.m_UpdateButtonComponent.SetVisible(nOutdated > 0);

		if (nOutdated > 0)
			m_Widgets.m_UpdateButtonComponent.SetCountText(nOutdated.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void DisplayFailDialog()
	{
		// Check if dialog is not already opened
		if (m_FailDialog)
			return;

		array<ref Tuple2<SCR_WorkshopItem, ref Revision>> failed = {};
		array<ref SCR_WorkshopItemActionDownload> failedActions = SCR_DownloadManager.GetInstance().GetFailedDownloads();

		foreach (SCR_WorkshopItemActionDownload action : failedActions)
		{
			SCR_WorkshopItem item = action.m_Wrapper;
			Revision version = item.GetDependency().GetRevision();

			failed.Insert(new Tuple2<SCR_WorkshopItem, ref Revision>>(item, version));
		}

		// Setup dialog
		SCR_DownloadFailDialog dialog = SCR_DownloadFailDialog.CreateFailedAddonsDialog(failed, false);
		if (!dialog)
			return;

		m_FailDialog = dialog;
		m_FailDialog.m_OnConfirm.Insert(Callback_OnFailDialogConfirm);
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnFailDialogConfirm(SCR_ConfigurableDialogUi dialog)
	{
		// Clear dialog
		m_FailDialog.m_OnConfirm.Clear();
		m_FailDialog.Close();
		m_FailDialog = null;
		SCR_DownloadManager.GetInstance().ClearFailedDownloads();
	}
}
