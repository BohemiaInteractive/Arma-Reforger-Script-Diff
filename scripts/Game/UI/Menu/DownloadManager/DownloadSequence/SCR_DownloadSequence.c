/*!
Base scripted download seqeuence
Take care of starting of downloads from menu 
This class only solves functionality, not UI 
- load details, scenarios, dependencies, compute patch size 

Downloading is not started automatically at base
*/

//-----------------------------------------------------------------------------------------------
void ScriptInvoker_DownloadSequence(SCR_DownloadSequence sequence);
typedef func ScriptInvoker_DownloadSequence;

//-----------------------------------------------------------------------------------------------
void ScriptInvoker_DownloadSequenceDependencies(SCR_DownloadSequence sequence, array<ref SCR_WorkshopItem> dependencies);
typedef func ScriptInvoker_DownloadSequenceDependencies;

//------------------------------------------------------------------------------------------------
class SCR_DownloadSequence
{
	// When true, we will subscribe automatically when download is started
	protected bool m_bSubscribeToAddons;
	
	protected ref array<ref SCR_WorkshopItem> m_aDependencies = {};
	protected bool m_bWaitingResponse;	// Set to false when data fetching is complete
	protected bool m_bCanceled;	// When true, finishing data reception will not trigger further downloading or other actions.
	protected bool m_bFailed;	// True when failed due to timeour or error
	protected bool m_bRestrictedAddons;
	protected bool m_bBlockedAddons;
	protected bool m_bPatchSizeLoaded;
	
	// True - skip loading of addon details which speed up loading time, useful for getting downloading list like in Server broweser
	protected bool m_bSkipDetails;
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// No need for = 0, it's the default value for ints already
	
	protected int m_iDependencyDetailsLoadedCount = 0;
	protected bool m_bAllDependencyDetailsLoaded;
	
	protected int m_iPatchesLoaded = 0;
	
//---- REFACTOR NOTE END ----
	
	protected ref array<ref SCR_BackendCallbackWorkshopItem> m_aPatchSizeCallbacks = {};
	
	protected ref array<ref SCR_WorkshopItem> m_aDependenciesIssues = {};
	
	//------------------------------------------------------------------------------------------------
	// Invokers 
	//------------------------------------------------------------------------------------------------
	
	protected ref ScriptInvokerBase<ScriptInvoker_DownloadSequence> m_OnInit;
	protected ref ScriptInvokerBase<ScriptInvoker_DownloadSequence> m_OnDetailsLoaded;
	protected ref ScriptInvokerBase<ScriptInvoker_DownloadSequence> m_OnDependenciesLoaded;
	protected ref ScriptInvokerBase<ScriptInvoker_DownloadSequenceDependencies> m_OnRestrictedDependencies;
	protected ref ScriptInvokerBase<ScriptInvoker_DownloadSequenceDependencies> m_OnDependenciesLoadingPrevented;
	protected ref ScriptInvokerBase<ScriptInvoker_DownloadSequence> m_OnError;
	protected ref ScriptInvokerBase<ScriptInvoker_DownloadSequence> m_OnReady;
	
	protected ref ScriptInvokerBase<ScriptInvoker_DownloadSequence> m_OnCancel;
	protected ref ScriptInvokerBase<ScriptInvoker_DownloadSequence> m_OnDestroyed;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_DownloadSequenceDependencies> GetOnRestrictedDependency()
	{
		if (!m_OnRestrictedDependencies)
			m_OnRestrictedDependencies = new ScriptInvokerBase<ScriptInvoker_DownloadSequenceDependencies>();
		
		return m_OnRestrictedDependencies;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_DownloadSequenceDependencies> GetOnDependenciesLoadingPrevented()
	{
		if (!m_OnDependenciesLoadingPrevented)
			m_OnDependenciesLoadingPrevented = new ScriptInvokerBase<ScriptInvoker_DownloadSequenceDependencies>();
		
		return m_OnDependenciesLoadingPrevented;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_DownloadSequence> GetOnError()
	{
		if (!m_OnError)
			m_OnError = new ScriptInvokerBase<ScriptInvoker_DownloadSequence>();
		
		return m_OnError;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_DownloadSequence> GetOnReady()
	{
		if (!m_OnReady)
			m_OnReady = new ScriptInvokerBase<ScriptInvoker_DownloadSequence>();
		
		return m_OnReady;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_DownloadSequence> GetOnCancel()
	{
		if (!m_OnCancel)
			m_OnCancel = new ScriptInvokerBase<ScriptInvoker_DownloadSequence>();
		
		return m_OnCancel;
	}
	
	//------------------------------------------------------------------------------------------------
	// Public 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	static SCR_DownloadSequence Create(array<ref SCR_WorkshopItem> dependencies, SCR_DownloadSequence previous, bool skipDetails = false)
	{
		if (previous && previous.m_bWaitingResponse)
			return previous;

		SCR_DownloadSequence sequence = new SCR_DownloadSequence();
		sequence.Setup(dependencies, skipDetails);
		
		return sequence;
	}
	
	//------------------------------------------------------------------------------------------------
	// Call on creating to start download sequence 
	// Start load details
	void Init()
	{
		m_bWaitingResponse = true;
		m_bRestrictedAddons = false;
		m_bBlockedAddons = false;
		
		// Skip details loading 
		if (m_bSkipDetails)
		{
			OnAllDependenciesDetailsLoaded();
			return;
		}
		
		// Load
		LoadDependenciesDetails();
	}
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// A comment on top of the class states "This class only solves functionality, not UI " ...and then it does UI
	
	//------------------------------------------------------------------------------------------------
	//! Show dialog with list of resctricted
	//! In dialog is possible to cancel reports 
	SCR_ReportedAddonsDialog ShowRestrictedDependenciesDialog()
	{
		array<ref SCR_WorkshopItem> restrictedDependencies = SCR_AddonManager.SelectItemsBasic(m_aDependencies, EWorkshopItemQuery.RESTRICTED);
		SCR_AddonListDialog addonsDialog = SCR_AddonListDialog.CreateRestrictedAddonsDownload(restrictedDependencies);
	
		// Handle cancel reports done 
		SCR_ReportedAddonsDialog reportedDialog = SCR_ReportedAddonsDialog.Cast(addonsDialog);
		if (reportedDialog)
			reportedDialog.GetOnAllReportsCanceled().Insert(OnAllReportsCanceled);
		
		return reportedDialog;
	}
	
//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	//! Cancels this download request.
	void Cancel()
	{
		m_bCanceled = true;
		m_bWaitingResponse = false;
		
		if (m_OnCancel)
			m_OnCancel.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	// Protected  
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Setup new created download sequence 
	protected void Setup(notnull array<ref SCR_WorkshopItem> dependencies, bool skipDetails)
	{
		m_bSubscribeToAddons = true;
		m_bSkipDetails = skipDetails;
		
		foreach (SCR_WorkshopItem dependency : dependencies)
		{
			m_aDependencies.Insert(dependency);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if all details were loaded successfully
	protected bool HasAllDetails()
	{
		if (m_bFailed || m_bCanceled || m_bWaitingResponse)
			return false;
		
		if (m_bSkipDetails && !m_bAllDependencyDetailsLoaded)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Load all dependencies details in list
	protected void LoadDependenciesDetails()
	{
		m_iDependencyDetailsLoadedCount = 0;
		
		// No dependenciees - Skip dependency loading
		if (m_aDependencies.IsEmpty())
		{
			OnAllDependenciesDetailsLoaded();
			return;
		}
		
		// Load details of all dependencies
		foreach (SCR_WorkshopItem dep : m_aDependencies)
		{
			// Skip loaded 
			if (dep.GetDetailsLoaded() || dep.GetOffline())
			{
				OnDependencyDetailsLoaded(dep);
				continue;
			}
			
			// Setup callbacks and load details
			dep.m_OnGetAsset.Insert(OnDependencyDetailsLoaded);
			dep.m_OnError.Insert(OnItemError);
			dep.m_OnTimeout.Insert(OnItemError);
			
			dep.LoadDetails();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when finally all the details of all dependencies are loaded
	protected void OnAllDependenciesDetailsLoaded()
	{		
		m_bAllDependencyDetailsLoaded = true;
		
		// Bail if this was canceled or failed
		if (m_bFailed || m_bCanceled)
			return;
		
		// Check if any dependencies are restricted
		array<ref SCR_WorkshopItem> restricted = SCR_AddonManager.SelectItemsBasic(m_aDependencies, EWorkshopItemQuery.RESTRICTED);
		if (!restricted.IsEmpty())
		{
			m_bRestrictedAddons = true;
			
			array<ref SCR_WorkshopItem> blocked = SCR_AddonManager.SelectItemsBasic(restricted, EWorkshopItemQuery.BLOCKED);
			m_bBlockedAddons = !blocked.IsEmpty();
			
			if (m_OnRestrictedDependencies)
				m_OnRestrictedDependencies.Invoke(this, restricted);
		}
		
		// Skip dependencies patch size loading
		if (m_aDependencies.IsEmpty())
		{
			AllPatchSizeLoaded();
			return;
		}
		
		// Load dependencies patch sizes 
		m_iPatchesLoaded = 0;
		
		foreach (SCR_WorkshopItem dependency : m_aDependencies)
		{
			// Skip blocked (banned) 
			if (dependency.GetBlocked())
			{
				OnDependencyPatchSizeLoadResponse(null);
				continue;
			}
			
			// Catch missing
			Revision revision;
			// server browser still uses Dependency class
			if (dependency.GetDependency())
				revision = dependency.GetDependency().GetRevision();
			// workshop uses WorkshopItem only
			else			 
				revision = dependency.GetLatestRevision();
			
			// Pending download
			if (!revision)
				revision = dependency.GetWorkshopItem().GetPendingDownload();
			
			if (!revision)
				continue;
			
			// Skip patch compute if dependency is running download
			SCR_WorkshopItemActionDownload downloading = SCR_DownloadManager.GetInstance().DownloadingActionAddonById(dependency.GetId());
			
			if (downloading)
			{	
				// Proccess with same version
				OnDependencyPatchSizeLoadResponse(null);
				
				continue;
			}

			// Skip patch size compute if addon is not downloaded 
			if (!dependency.GetOffline())
			{
				SetDependencySize(dependency, dependency.GetSizeBytes());
				continue;
			}
			
			// Setup callback and compute
			SCR_BackendCallbackWorkshopItem callback = new SCR_BackendCallbackWorkshopItem(dependency);
			callback.SetOnSuccess(OnDependencyPatchSizeLoadResponse);
			callback.SetOnError(OnDependencyPatchSizeLoadError);
			m_aPatchSizeCallbacks.Insert(callback);
			
			revision.ComputePatchSize(callback);
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	protected array<ref SCR_WorkshopItem> MissingWorkshopItems()
	{
		array<ref SCR_WorkshopItem> missingItems = {};
		
		foreach (SCR_WorkshopItem dependency : m_aDependencies)
		{
			if (!dependency.GetWorkshopItem())
				missingItems.Insert(dependency);
		}
		
		return missingItems;
	}

	//------------------------------------------------------------------------------------------------
	protected void AllPatchSizeLoaded()
	{
		m_bWaitingResponse = false;
		
		if (m_OnReady && HasAllDetails())
			m_OnReady.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call when all reports from dialog are cancled to clear invoker actions and display download dialog 
	void OnAllReportsCanceled(SCR_ReportedAddonsDialog dialog)
	{
		dialog.GetOnAllReportsCanceled().Remove(OnAllReportsCanceled);
		
		m_bRestrictedAddons = false;
		OnAllDependenciesDetailsLoaded();
	}
	
	//------------------------------------------------------------------------------------------------
	// Callbacks  
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Call when dependency details are successfully loaded
	protected void OnDependencyDetailsLoaded(notnull SCR_WorkshopItem item)
	{
		item.m_OnGetAsset.Remove(OnDependencyDetailsLoaded);
		
		m_iDependencyDetailsLoadedCount++;
		
		// All details loaded
		if (m_iDependencyDetailsLoadedCount == m_aDependencies.Count())
			OnAllDependenciesDetailsLoaded();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call on any response to dependency patch size 
	protected void OnDependencyPatchSizeLoadResponse(SCR_BackendCallbackWorkshopItem callback)
	{			
		float size;
		callback.GetItem().GetDependency().GetRevision().GetPatchSize(size);
		callback.GetItem().SetTargetRevisionPatchSize(size);

		m_iPatchesLoaded++;
		CheckAllPatchSizeLoaded();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call on error response to dependency patch size 
	protected void OnDependencyPatchSizeLoadError(SCR_BackendCallbackWorkshopItem callback)
	{
		OnItemError(callback.GetItem());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetDependencySize(SCR_WorkshopItem item, float size)
	{
		item.SetTargetRevisionPatchSize(size);
		
		m_iPatchesLoaded++;
		CheckAllPatchSizeLoaded();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckAllPatchSizeLoaded()
	{	
		// All loaded 
		if (m_iPatchesLoaded == m_aDependencies.Count())
		{
			m_aPatchSizeCallbacks.Clear();
			AllPatchSizeLoaded();
		}
		
		// Check issues
		if (m_OnDependenciesLoadingPrevented && !m_aDependenciesIssues.IsEmpty() && m_aDependencies.Count() == m_iPatchesLoaded + m_aDependenciesIssues.Count())
			m_OnDependenciesLoadingPrevented.Invoke(this, m_aDependenciesIssues);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemError(SCR_WorkshopItem item)
	{
		if (m_OnError)
			m_OnError.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemTimeout(SCR_WorkshopItem item)
	{
		if (m_OnError)
			m_OnError.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	// Get set  
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	bool GetRestrictedAddons()
	{
		return m_bRestrictedAddons;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetBlockedAddons()
	{
		return m_bBlockedAddons;
	}
	
	//------------------------------------------------------------------------------------------------
	// Construct  
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_DownloadSequence()
	{
		// Unsubscribe
		
		if (m_OnDestroyed)
			m_OnDestroyed.Invoke(this);
	}
}
