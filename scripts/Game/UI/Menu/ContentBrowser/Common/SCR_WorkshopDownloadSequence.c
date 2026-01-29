/*!
Extended download sequence for workshop downloading 
Instead of just dependencies main m_Item is used for downloading single item
- Item dependencies are then stored in dependencies list 

Is displaying workshop donwload related UI and automatically starts download
*/

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Data handling class tangled with UI. Furthermore, it complicates the already messy cross referencing going on with SCR_WorkshopItem and related classes

class SCR_WorkshopDownloadSequence : SCR_DownloadSequence
{	
	protected static float LOADING_DELAY = 500;
	
	protected ref SCR_WorkshopItem m_Item;
	protected ref Revision m_ItemTargetRevision;

	protected SCR_LoadingOverlayDialog m_LoadingOverlay;
	
	//------------------------------------------------------------------------------------------------
	// Public 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Tries to create a new request if previous doesn't exist or finished. Otherwise returns the previous request.
	static SCR_WorkshopDownloadSequence Create(SCR_WorkshopItem item, Revision itemTargetRevision, SCR_WorkshopDownloadSequence previous)
	{
		if (previous && previous.m_bWaitingResponse)
			return previous;
		
		SCR_WorkshopDownloadSequence sq = new SCR_WorkshopDownloadSequence(null, item, itemTargetRevision);
		
		return sq;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateLoadingOverlay()
	{
		// Skip if already loaded
		if (!m_bWaitingResponse)
			return;
		
		// Show only if we are still waiting for data
		m_LoadingOverlay = SCR_LoadingOverlayDialog.Create();
		m_LoadingOverlay.m_OnCloseStarted.Insert(Cancel); // Cancel this when the loading overlay close is initiated by user
	}
	
	//------------------------------------------------------------------------------------------------
	// Protected  
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! After all dependencies patch loaded start loading patch size for main item
	override protected void AllPatchSizeLoaded()
	{
		// Setup callback and compute
		SCR_BackendCallbackWorkshopItem callback = new SCR_BackendCallbackWorkshopItem(m_Item);
		callback.SetOnSuccess(OnItemPatchLoaded);
		callback.SetOnError(OnItemPatchError);
		m_aPatchSizeCallbacks.Insert(callback);
		
		m_ItemTargetRevision.ComputePatchSize(callback);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unified behavior for error and timeout
	protected void HandleError()
	{
		if (m_bWaitingResponse || !m_bFailed)
		{
			if (m_LoadingOverlay)
				m_LoadingOverlay.CloseAnimated();

			// Add action to downloads
			SCR_WorkshopItemActionDownload action = new SCR_WorkshopItemActionDownload(m_Item, latestVersion: true);
			SCR_DownloadManager.GetInstance().AddDownloadManagerEntry(m_Item, action);
			action.Activate();
			
			// Display fail dialog in next frame to display it after action is really failed
			// TODO - improve logic and move to base
			if (!SCR_DownloadManager_Dialog.Cast(SCR_ConfigurableDialogUi.GetCurrentDialog()))
			{
				GetGame().GetCallqueue().CallLater(SetupAddonFail, 0, false, action);
			}
		}
		
		// Clear waiting
		m_bWaitingResponse = false;
		m_bFailed = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Open download manager and set action to fail
	protected void SetupAddonFail(SCR_WorkshopItemActionDownload action)
	{
		SCR_DownloadManager_Dialog.Create();
		//action.ForceFail();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show main addon restricted dialog
	protected void ShowRestrictedItemDialog()
	{
		m_bWaitingResponse = false;
		
		// Close the loading overlay
		if (m_LoadingOverlay)
			m_LoadingOverlay.CloseAnimated(false);
		
		SCR_WorkshopDialogs.CreateDialog("error_addon_blocked");
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show download confirm dialog base on main item size and dependencies
	protected void ShowConfirmationUI()
	{
		if (m_LoadingOverlay)
			m_LoadingOverlay.CloseAnimated();
		
		if (m_bRestrictedAddons)
			return;
		
		// Setup main item
		bool downloadMainItem = SCR_DownloadManager.IsLatestDownloadRequired(m_Item);
		
		// Get dependencies
		array<ref SCR_WorkshopItem> dependenciesToLoad = {};
		SCR_DownloadManager.SelectAddonsForLatestDownload(m_aDependencies, dependenciesToLoad);
		
		// Show confirmation only if there are dependencies
		if (!m_aDependencies.IsEmpty())
		{
			SCR_DownloadConfirmationDialog confirmDialog = SCR_DownloadConfirmationDialog.CreateForAddonAndDependencies(m_Item, downloadMainItem, dependenciesToLoad, m_bSubscribeToAddons);
		}
		else
		{
			// Start download immediately
			//SCR_DownloadManager.GetInstance().DownloadLatestWithDependencies(m_Item, downloadMainItem, dependenciesToLoad);
			SCR_DownloadManager.GetInstance().DownloadItems({m_Item});
			
			// Subscribe to items
			if (!m_bSubscribeToAddons)
				return;
			
			if (m_Item)
				m_Item.SetSubscribed(true);
			
			foreach (SCR_WorkshopItem dependency : dependenciesToLoad)
			{
				dependency.SetSubscribed(true);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Callbacks  
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override protected void OnItemError(SCR_WorkshopItem item)
	{
		HandleError();
		
		super.OnItemError(item);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when finally all the details of all dependencies are loaded
	override protected void OnAllDependenciesDetailsLoaded()
	{
		// Close the loading overlay
		if (m_LoadingOverlay)
			m_LoadingOverlay.CloseAnimated(false);
		
		GetOnRestrictedDependency().Insert(OnRestrictedDependenciesFound);
		
		super.OnAllDependenciesDetailsLoaded();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRestrictedDependenciesFound(SCR_DownloadSequence sequence, array<ref SCR_WorkshopItem> dependencies)
	{
		ShowRestrictedDependenciesDialog();
		GetOnRestrictedDependency().Remove(OnRestrictedDependenciesFound);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called from SCR_WorkshopItem.m_OnDependenciesLoaded
	protected void OnItemDependenciesLoaded(notnull SCR_WorkshopItem item)
	{	
		// Unsubscribe - this callback is not needed any more
		m_Item.m_OnDependenciesLoaded.Remove(OnItemDependenciesLoaded);
		
		m_aDependencies = m_Item.GetLatestDependencies();
		
		// Resticted 
		if (item.GetRestricted())
		{
			ShowRestrictedItemDialog();
			return;
		}
		
		// No dependencies
		if (m_aDependencies.IsEmpty())
		{
			OnAllDependenciesDetailsLoaded();
			return;
		}
		
		// Load
		//OnAllDependenciesDetailsLoaded();
		LoadDependenciesDetails();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call when main item (addon) is loaded
	protected void OnItemGetAsset(SCR_WorkshopItem item)
	{
		if (!item.GetRequestFailed())
			return;

		GetGame().GetCallqueue().Remove(CreateLoadingOverlay);
		if (m_LoadingOverlay)
			m_LoadingOverlay.Close();
		
		OnItemError(item);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call when main item patch size is loaded 
	//! Server as AllPatchSizeLoaded in base class
	protected void OnItemPatchLoaded()
	{	
		// Size 
		float size;
		m_ItemTargetRevision.GetPatchSize(size);
		m_Item.SetTargetRevisionPatchSize(size);
		
		// All ready
		m_bWaitingResponse = false;
		
		if (HasAllDetails())
		{			
			if (m_OnReady)
				m_OnReady.Invoke(this);
			
			ShowConfirmationUI();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call when main item patch size fails to load
	//! Server as AllPatchSizeLoaded in base class
	protected void OnItemPatchError()
	{
		HandleError();
	}
	
	//------------------------------------------------------------------------------------------------
	// Construct  
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SCR_WorkshopDownloadSequence(array<ref SCR_WorkshopItem> dependencies, SCR_WorkshopItem item, Revision itemTargetRevision)
	{
		// Setup
		m_Item = item;
		m_ItemTargetRevision = itemTargetRevision;
		m_Item.SetItemTargetRevision(itemTargetRevision);
		
		m_bWaitingResponse = true;
		
		// Load
		m_Item.m_OnDependenciesLoaded.Insert(OnItemDependenciesLoaded);
		
		m_Item.m_OnGetAsset.Insert(OnItemGetAsset);
		m_Item.m_OnError.Insert(OnItemError);
		m_Item.m_OnTimeout.Insert(OnItemError);
		
		m_Item.LoadDetails();
		
		GetGame().GetCallqueue().CallLater(CreateLoadingOverlay, LOADING_DELAY);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_WorkshopDownloadSequence()
	{
		#ifdef WORKSHOP_DEBUG
		ContentBrowserUI._print(string.Format("SCR_WorkshopDownloadSequence: Delete for: %1", m_Item.GetName()));
		#endif
		
		if (m_LoadingOverlay)
			m_LoadingOverlay.Close();
		
		// Unsubscribe from item's events
		m_Item.m_OnDependenciesLoaded.Remove(OnItemDependenciesLoaded);
	}
	
	//---- REFACTOR NOTE END ----
};