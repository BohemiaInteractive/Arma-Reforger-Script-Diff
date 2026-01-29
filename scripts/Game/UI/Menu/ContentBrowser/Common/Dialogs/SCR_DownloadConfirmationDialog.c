//------------------------------------------------------------------------------------------------
//! Dialog for confirming multiple downloads in workshop
class SCR_DownloadConfirmationDialog : SCR_ConfigurableDialogUi
{	
	protected const ResourceName DOWNLOAD_LINE_LAYOUT = "{BB5AEDDA3C4134FD}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManager_AddonDownloadLineConfirmation.layout";
	protected ref SCR_WorkshopItem m_Item;
	protected bool m_bDownloadMainItem;
	protected ref array<ref SCR_WorkshopItem> m_aDependencies;
	protected ref array<ref Revision> m_aDependencyVersions;	// Array with specific versions of dependencies
	protected bool m_bSubscribeToAddons; // When true, if user confirms the download, addons will be subscribed
	protected ref SCR_DownloadConfirmationDialogContentWidgets m_Widgets = new SCR_DownloadConfirmationDialogContentWidgets();
	protected ref array<ref SCR_WorkshopItemAction> m_aCreatedActions = {};		// Actions which were created when the user confirmed the action
	
	protected ref SCR_AddonPatchSizeLoader m_Loader = new SCR_AddonPatchSizeLoader();
	
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// Old script invoker. Auto
		
	protected ref ScriptInvoker m_OnDownloadConfirmed; // (SCR_DownloadConfirmationDialog dlg)
	
	//------------------------------------------------------------------------------------------------
	//! The created download action will be bound to one main item and its dependencies.
	//! mainItem - the main addon for which the dialog is created.
	//! downloadMainItem - when true, main item will be considered for download.
	//! dependencies -array with dependencies. Cannot be null, but can be empty.
	static SCR_DownloadConfirmationDialog CreateForAddonAndDependencies(notnull SCR_WorkshopItem mainItem, bool downloadMainItem, notnull array<ref SCR_WorkshopItem> dependencies, bool subscribeToAddons)
	{
		SCR_DownloadConfirmationDialog dlg = new SCR_DownloadConfirmationDialog();
		
		dlg.m_bSubscribeToAddons = subscribeToAddons;
		dlg.m_Item = mainItem;
		dlg.m_bDownloadMainItem = downloadMainItem;
		dlg.m_aDependencies = new array<ref SCR_WorkshopItem>;
		foreach (auto i : dependencies)
			dlg.m_aDependencies.Insert(i);
		
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, "download_confirmation", dlg);
		
		dlg.SetStyleDownloadAddonsWithDependencies();
		
		return dlg;
	}
	
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	protected static void SetupDownloadDialogAddons(notnull out SCR_DownloadConfirmationDialog dialog, notnull array<ref Tuple2<SCR_WorkshopItem, ref Revision>> addonsAndVersions, bool subscribeToAddons)
	{
		dialog.m_bDownloadMainItem = false;
		dialog.m_bSubscribeToAddons = subscribeToAddons;
		dialog.m_aDependencies = new array<ref SCR_WorkshopItem>;
		dialog.m_aDependencyVersions = new array<ref Revision>;
		
		foreach (Tuple2<SCR_WorkshopItem, ref Revision> i : addonsAndVersions)
		{
			dialog.m_aDependencies.Insert(i.param1);
			dialog.m_aDependencyVersions.Insert(i.param2);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Download addons with specific versions
	static SCR_DownloadConfirmationDialog CreateForAddons(notnull array<ref Tuple2<SCR_WorkshopItem, ref Revision>> addonsAndVersions, bool subscribeToAddons)
	{
		SCR_DownloadConfirmationDialog dlg = new SCR_DownloadConfirmationDialog();
	
		SetupDownloadDialogAddons(dlg, addonsAndVersions, subscribeToAddons);
		
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, "download_confirmation", dlg);
		
		dlg.SetStyleDownloadAddons();
			
		return dlg;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		m_Widgets.Init(GetContentLayoutRoot());
		
		// Update line of the main addon
		m_Widgets.m_MainAddonSection.SetVisible(m_bDownloadMainItem);
		if (m_Item != null && m_bDownloadMainItem)
		{
			m_Widgets.m_MainAddonLineComponent0.InitForWorkshopItem(m_Item);
		}
		
		// Load patch size 
		OnAllPatchSizeLoaded(m_Loader, true);
		
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Overreliance on strings, weird naming
		
		// Buttons
		SCR_InputButtonComponent confirmButton  = FindButton("confirm");
		SCR_InputButtonComponent confirmAlternativeButton  = FindButton("confirm2");
		
//---- REFACTOR NOTE END ----
		
		if (confirmAlternativeButton)
			confirmAlternativeButton.m_OnActivated.Insert(OnConfirmAlternative);
		
		if (m_aDependencies.IsEmpty())
		{
			// When we are downloading only one addon, 'download all' button becomes 'download'
			if (confirmAlternativeButton)
				confirmAlternativeButton.SetVisible(false);
			
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Hardcoded string
			
			confirmButton.SetLabel("#AR-Workshop_Dialog_ConfirmDownload_ButtonDownload");

//---- REFACTOR NOTE END ----			

		}
		else if (!m_bDownloadMainItem)
		{
			// When we are not downloading the main addon, 'download one' button is hidden
			if (confirmAlternativeButton)
				confirmAlternativeButton.SetVisible(false);
		}
			
		// Hide other optional text
		m_Widgets.m_OtherAddonsMessage.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadAddonPatchSizes()
	{
		array<ref SCR_WorkshopItem> versionDifferences = SCR_AddonManager.SelectItemsAnd(
			m_aDependencies, 
			EWorkshopItemQuery.NOT_OFFLINE | EWorkshopItemQuery.NOT_LOCAL_VERSION_MATCH_DEPENDENCY);
		
		// Skip if there are no version differences 
		if (versionDifferences.IsEmpty())
		{
			OnAllPatchSizeLoaded(m_Loader, true);
			return;
		}
		
		// Setup loading
		m_Loader.ClearItems();
		
		foreach (SCR_WorkshopItem item : versionDifferences)
		{
			Revision rev = item.GetDependency().GetRevision();
			item.SetItemTargetRevision(rev);
			m_Loader.InsertItem(item);
		}
		
		m_Loader.GetOnAllPatchSizeLoaded().Insert(OnAllPatchSizeLoaded);
		m_Loader.LoadPatchSizes();
		
		// Show loading overlay 
		m_Widgets.m_LoadingOverlayComponent.SetShown(true);
		m_Widgets.m_AddonContentVertical.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAllPatchSizeLoaded(SCR_AddonPatchSizeLoader loader, bool allLoaded)
	{
		CreateAddonList();
		
		loader.GetOnAllPatchSizeLoaded().Remove(OnAllPatchSizeLoaded);
		
		// Hide loading overlay 
		m_Widgets.m_LoadingOverlayComponent.SetShown(false);
		m_Widgets.m_AddonContentVertical.SetVisible(true);
		
		// Total download size - only visible if we have any dependencies
		m_Widgets.m_SummarySection.SetVisible(!m_aDependencies.IsEmpty());
		if (!m_aDependencies.IsEmpty())
		{
			float totalSize = SCR_DownloadManager.GetTotalSizeBytes(m_aDependencies, m_Item);
			string totalSizeStr = SCR_ByteFormat.GetReadableSize(totalSize);
			m_Widgets.m_TotalAddonSizeText.SetText(totalSizeStr);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create and setup addon widget text and verions
	protected void CreateAddonWidget(notnull SCR_WorkshopItem dep, Widget listRoot)
	{
		Widget w = GetGame().GetWorkspace().CreateWidgets(DOWNLOAD_LINE_LAYOUT, listRoot);
		SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
		
		// Setup addon revision
		Dependency dependency = dep.GetDependency();
		
		Revision revision = null;
		if (dependency)
			revision = dependency.GetRevision();
		
		//Revision revision = dep.GetDependency().GetRevision();
		comp.InitForWorkshopItem(dep, revision);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateAddonList()
	{
		// Create lines for other addons
		m_Widgets.m_OtherAddonsSection.SetVisible(!m_aDependencies.IsEmpty());
		if (!m_aDependencies.IsEmpty())
		{
			// Not downloaded - not avaiable offline 
			array<ref SCR_WorkshopItem> missingAddons = SCR_AddonManager.SelectItemsBasic(m_aDependencies, EWorkshopItemQuery.NOT_OFFLINE);
			
			foreach (SCR_WorkshopItem missing : missingAddons)
			{
				CreateAddonWidget(missing, m_Widgets.m_OtherAddonsList);
			}
			
			// Version difference
			array<ref SCR_WorkshopItem> versionDifferences = SCR_AddonManager.SelectItemsAnd(
				m_aDependencies, 
				EWorkshopItemQuery.NOT_LOCAL_VERSION_MATCH_DEPENDENCY | EWorkshopItemQuery.OFFLINE);
			
			int vDifCount = versionDifferences.Count();
			bool hasDifference = vDifCount > 0;
			
			// Display version difference widgets 
			m_Widgets.m_UpdateAddonsList.SetVisible(hasDifference);
			m_Widgets.m_UpdateAddonsMessage.SetVisible(hasDifference);
			
			if (hasDifference)
			{
				// Create woidgets 
				foreach (SCR_WorkshopItem addon : versionDifferences)
				{
					CreateAddonWidget(addon, m_Widgets.m_UpdateAddonsList);
				}
				
				// Version change text
				m_Widgets.m_UpdateAddonsMessage.SetTextFormat("#AR-Workshop_Dialog_ConfirmDownload_VersionDifferences", vDifCount);
			}
		}
	}
	
	//! This dialog can be used in a few contexts.
	//! These SetStyle methods set up the widgets according to specific use cases.
	//------------------------------------------------------------------------------------------------
	//! Sets text: "%1 dependencies will be downloaded:" over the dependencies list
	void SetStyleDownloadAddonsWithDependencies()
	{
		if (!m_aDependencies.IsEmpty())
		{
			m_Widgets.m_OtherAddonsMessage.SetVisible(true);
			m_Widgets.m_OtherAddonsMessage.SetTextFormat(
				"#AR-Workshop_Dialog_ConfirmDownload_DependenciesWillBeDownloaded",
				m_aDependencies.Count());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Hides all explanation text
	void SetStyleDownloadAddons()
	{
		m_Widgets.m_OtherAddonsMessage.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		if (m_Item)
		{
			// If main item is provided, we (maybe) start downloading this item and its dependencies.
			m_aCreatedActions = SCR_DownloadManager.GetInstance().DownloadLatestWithDependencies(m_Item, m_bDownloadMainItem, m_aDependencies);
		}
		else if (m_aDependencies)
		{
			// If main item was not provided, all the downloads are started separately.
			foreach (int i, SCR_WorkshopItem dep : m_aDependencies)
			{
				if (dep.IsDownloadRunning())
					continue;
				
				Revision targetVersion;
				if (m_aDependencyVersions != null)
					targetVersion = m_aDependencyVersions[i];
				
				SCR_WorkshopItemActionDownload action;
				
				if (targetVersion) //why? it's never true
					action = dep.Download(targetVersion);
				else
					action = dep.DownloadLatestVersion();
				
				if (action)
				{
					action.Activate();
					m_aCreatedActions.Insert(action);
				}
			}
		}
		
		// Subscription behavior	
		if (m_bSubscribeToAddons)
		{
			if (m_Item)
				m_Item.SetSubscribed(true);
			
			foreach (SCR_WorkshopItem dependency : m_aDependencies)
			{
				dependency.SetSubscribed(true);
			}
		}
		
		super.OnConfirm();
	}
	
	//------------------------------------------------------------------------------------------------
	//! This is only valid when we can download the main addon
	void OnConfirmAlternative()
	{
		m_aDependencies.Clear(); // Don't download any dependencies
		
		if (!m_Item)
			return;
		
		m_aCreatedActions = SCR_DownloadManager.GetInstance().DownloadLatestWithDependencies(m_Item, true, m_aDependencies);
		
		if (m_bSubscribeToAddons)
		{
			if (m_Item)
				m_Item.SetSubscribed(true);
		}
		
		super.OnConfirm();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the internal array with created download actions
	array<ref SCR_WorkshopItemAction> GetActions()
	{
		array<ref SCR_WorkshopItemAction> a = {};
		
		foreach (SCR_WorkshopItemAction action : m_aCreatedActions)
		{
			a.Insert(action);
		}
		
		return a;
	}
}