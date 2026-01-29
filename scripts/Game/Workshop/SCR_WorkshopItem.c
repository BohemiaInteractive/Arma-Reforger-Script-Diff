/*
!  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  
!  ! If you use SCR_WorkshopItem, make sure you hold a strong reference (ref) to it:
!  ! Example: ref SCR_WorkshopItem m_Item;
!  ! The object is shared by nature and exists as long as at least something holds a strong
!  ! reference to it, if it is offline or as if any download is active.
!  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  
*/
void ScriptInvokerMissionWorkshopItemMethod(MissionWorkshopItem item);
typedef func ScriptInvokerMissionWorkshopItemMethod;
typedef ScriptInvokerBase<ScriptInvokerMissionWorkshopItemMethod> ScriptInvokerMissionWorkshopItem;

// Enum for possible issues of addons
enum EWorkshopItemProblem
{
	NO_PROBLEM = 0,
	BROKEN,
	DEPENDENCY_MISSING,
	DEPENDENCY_DISABLED,
	UPDATE_AVAILABLE,
	DEPENDENCY_OUTDATED
};

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Confusing architecture: this class adds a huge layer of complexity on top of the backend API: it's a wrapper of multiple API (WorkshopItem, Dependency, Revision, MissionWorkshopItem), it's thigtly coupled with other script classes (SCR_AddonManager, SCR_WorkshopItemAction), acted upon by even more (SCR_DownloadManager, a plethora of UIs), and apparently it's cached data is not guaranteed to be up to date with backend's. Also, old untyped scrip invokers and auto keyword

class SCR_WorkshopItem
{
	// friend class SCR_WorkshopItemAction
	
	// ---- Public variables ----
	ref ScriptInvoker m_OnChanged = new ScriptInvoker();				// (SCR_WorkshopItem item) - Called on any change and any event which affects state. Might be called on next frame!
	ref ScriptInvoker m_OnGetAsset = new ScriptInvoker();				// (SCR_WorkshopItem item) - Called on failure to load details too.
	ref ScriptInvoker m_OnDependenciesLoaded = new ScriptInvoker();	// (SCR_WorkshopItem item)
	ref ScriptInvoker m_OnScenariosLoaded = new ScriptInvoker();		// (SCR_WorkshopItem item)
	ref ScriptInvoker m_OnError = new ScriptInvoker();				// (SCR_WorkshopItem item) - Called on any error event
	ref ScriptInvoker m_OnTimeout = new ScriptInvoker();				// (SCR_WorkshopItem item) - Called on any timeout event
	ref ScriptInvoker m_OnDownloadComplete = new ScriptInvoker();		// (SCR_WorkshopItem item) - Called when any download has been completed (including an update)
	ref ScriptInvoker m_OnOfflineStateChanged = new ScriptInvoker();	// (SCR_WorkshopItem item, bool newState) - Called when the addon was downloaded first time or deleted (OFFLINE flag changed its value)
	ref ScriptInvoker m_OnReportStateChanged = new ScriptInvoker();	// (SCR_WorkshopItem, bool newReported) - Called when reported state has changed
	ref ScriptInvoker m_OnMyReportLoaded = new ScriptInvoker();		// (SCR_WorkshopItem item) - called after report loading is done.
	ref ScriptInvoker m_OnMyReportLoadError = new ScriptInvoker();
	ref ScriptInvoker m_OnRedownload = new ScriptInvoker();
	ref ScriptInvoker m_OnCanceled = new ScriptInvoker();
	
	// ---- Protected / Private ----
	
	protected ref WorkshopItem m_Item;	// Strong reference!
	protected ref Dependency m_Dependency;
	protected ref Revision m_LatestRevision;
	
	// Revision to download and measure patch size when item is not dependency
	protected ref Revision m_ItemTargetRevision;
	protected float m_fTargetRevisionPatchSize; // cached size for target revision
	
	protected ref array<MissionWorkshopItem> m_aMissions;
	protected ref array<ref SCR_WorkshopItem> m_aDependencies;	// Strong references!
	protected ref array<SCR_WorkshopItem> m_aDependent = new array<SCR_WorkshopItem>;	// Weak refs - array of addons which depend on this addon
	protected ref array<Revision> m_aRevisions;
	
	// Name and id - just to check them in debugger
	#ifdef WORKSHOP_DEBUG
	protected string m_sName;
	protected string m_sId;
	#endif
	
	// Flags for state of partially loaded data
	protected bool m_bDetailsLoaded;	// Details are loaded - After LoadDetails
	protected bool m_bSizeRequesting;	// Patch Size is loaded - After ComputePatchSize
	
	// Backend callbacks
	protected ref SCR_WorkshopItemCallback_AskDetails m_CallbackAskDetails;
	protected ref BackendCallback m_CallbackLoadGallery;
	protected ref BackendCallback m_CallbackLoadDependencies;
	protected ref BackendCallback m_CallbackLoadScenarios;
	protected ref BackendCallback m_CallbackLoadMyReport;
	protected ref BackendCallback m_CallbackRequestFavourite;
	protected ref BackendCallback m_CancelDownloadCallback = new BackendCallback();
	protected ref BackendCallback m_ComputeCallback;
	
	// Various state flags
	protected bool m_bMyRating;
	protected bool m_bMyRatingSet;
	protected bool m_bFavourite;
	protected bool m_bFavouriteRequesting;
	protected bool m_bSubscribed;
	protected bool m_bChanged;		// Checked every frame, if something changes then this flag can be set to OnChanged script invoker is called during update.
	protected bool m_bRequestFailed;	// Set to true on HTTP request failures
	protected bool m_bOffline;
	protected int m_iAccessState;
	
	// Data loading flags
	protected bool m_bWaitingLoadDetails;		// With current API it's always true after a LoadDetails call, since we can't know when it's done for sure
	protected float m_fPrevDownloadProgress;	// Used to detect when download progress changes
	
	// Actions
	protected ref SCR_WorkshopItemActionDownload m_ActionDownload;
	protected ref SCR_WorkshopItemActionReport m_ActionReport;
	protected ref SCR_WorkshopItemActionCancelReport m_ActionCancelReport;
	protected ref SCR_WorkshopItemActionComposite m_ActionDependency; // Composite action we have started for dependencies
	protected ref SCR_WorkshopItemActionAddAuthorBlock m_ActionAddAuthorBlock;
	protected ref SCR_WorkshopItemActionRemoveAuthorBlock m_ActionRemoveAuthorBlock;
	
	
	//-----------------------------------------------------------------------------------------------
	// 						P U B L I C   M E T H O D S 
	//-----------------------------------------------------------------------------------------------
	
	//! Use these only if absoulutely necessary to intarract with game API.
	WorkshopItem GetWorkshopItem() { return m_Item; }
	Dependency GetDependency() { return m_Dependency; }
	
	//! Getters for loading state
	bool GetItemDataLoaded() { return m_Item != null; }
	bool GetDetailsLoaded() { return m_bDetailsLoaded; }				// GetAsset request was completed, doesn't mean that anything else related to it is loaded
	bool GetScenariosLoaded() { return m_aMissions != null; }			// Scenarios were loaded after GetAsset request
	bool GetRevisionsLoaded() { return m_aRevisions != null; }			// Revisions were loaded after GetAsset request
	bool GetDependenciesLoaded() { return m_aDependencies != null; }	// Latest dependencies were loaded after GetAsset request
	bool GetRequestFailed() { return m_bRequestFailed; }				// True when any request has failed
	
	//-----------------------------------------------------------------------------------------------
	bool IsDownloadRunning()
	{
		if (!m_ActionDownload)
			return false;
		
		return m_ActionDownload.IsActive();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Logs all properties of the object into console
	void LogState()
	{
		bool myRating, myRatingSet;
		GetMyRating(myRatingSet, myRating);
		
		string description = GetDescription();
		if (description.Length() > 32)
		{
			description = description.Substring(0, 32);
			description = description + " <truncated>";
		}
		
		string summary = GetSummary();
		if (summary.Length() > 32)
		{
			summary = summary.Substring(0, 32);
			summary = summary + " <truncated>";
		}
		
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		
		array<ref SCR_WorkshopItem> dependencies = GetLatestDependencies();
		
		_print("\nLogState:");
		_print(string.Format("   Name:                %1", GetName()));
		_print(string.Format("   ID:                  %1", GetId()));
		_print(string.Format("   Dependency:          %1", m_Dependency));
		_print(string.Format("   WorkshopItem:        %1", m_Item));
		_print(string.Format("   Subscribed:          %1", GetSubscribed()));
		_print(string.Format("   Enabled:             %1", GetEnabled()));
		_print(string.Format("   Online:              %1", GetOnline()));
		_print(string.Format("   Offline:             %1", GetOffline()));
		_print(string.Format("   Blocked:             %1", GetBlocked()));
		_print(string.Format("   ReportedByMe:        %1", GetReportedByMe()));
		if (GetCurrentLocalRevision())
			_print(string.Format("   CurrentLocalVersion: %1", GetCurrentLocalRevision().GetVersion()));
		
		if (GetLatestVersion())
			_print(string.Format("   LatestVersion:       %1", GetLatestVersion()));
		
		_print(string.Format("   ItemDataLoaded:      %1", GetItemDataLoaded()));
		_print(string.Format("   DetailsLoaded:       %1", GetDetailsLoaded()));
		_print(string.Format("   ScenariosLoaded:     %1", GetScenariosLoaded()));
		_print(string.Format("   DependenciesLoaded:  %1", GetDependenciesLoaded()));
		_print(string.Format("   RequestFailed:       %1", GetRequestFailed()));
		//_print(string.Format("   OfflineAndUpToDate:  %1", GetIsOfflineAndUpToDate()));
		_print(string.Format("   AuthorName:          %1", GetAuthorName()));
		_print(string.Format("   MyRating:            %1", myRating));
		_print(string.Format("   MyRatingSet:         %1", myRatingSet));
		_print(string.Format("   Favourite:           %1", GetFavourite()));
		_print(string.Format("   Corrupted:           %1", GetCorrupted()));
		_print(string.Format("   Description:         %1", description));
		_print(string.Format("   Summary:             %1", summary));
		_print(string.Format("   RatingCount:         %1", GetRatingCount()));
		_print(string.Format("   SizeBytes:           %1", GetSizeBytes()));
		_print(string.Format("   UpdateAvailable:     %1", GetUpdateAvailable()));
		_print(string.Format("   EnabledExternally:   %1", mgr.GetAddonEnabledExternally(this)));
		_print(string.Format("   TimeSinceLastPlay:   %1, %2", GetTimeSinceLastPlay(), SCR_FormatHelper.GetTimeSinceEventImprecise(GetTimeSinceLastPlay())));
		_print(string.Format("   TimeSinceFirstDl:    %1, %2", GetTimeSinceFirstDownload(), SCR_FormatHelper.GetTimeSinceEventImprecise(GetTimeSinceFirstDownload())));
		
		//if (m_Item)
		//_print(string.Format("   HasBackendThumbnail: %1", m_Item.HasBackendThumbnail()));
			
		_print(string.Format("   AnyDepDisabled:      %1", GetEnabledAndAnyDependencyDisabled()));
		_print(string.Format("   AnyDepOutdated:      %1", GetAnyDependencyUpdateAvailable()));
		_print(string.Format("   AnyDepMissing:       %1", GetAnyDependencyMissing()));
		
		_print(string.Format("   Dependencies:        %1", dependencies.Count()));
		foreach (SCR_WorkshopItem dep : dependencies)
		{
		_print(string.Format("      - %1, Offline: %2, Update Available: %3", dep.GetName(), dep.GetOffline(), dep.GetUpdateAvailable()));
		}
		
		// todo scenarios, dependencies
		
		_print("\n\n");
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Loads details from backend, or if they are already loaded, calls the callbacks immediately
	void LoadDetails()
	{
		#ifdef WORKSHOP_DEBUG
		_print("LoadDetails");
		#endif
		
		if (m_Item || m_Dependency || !m_bDetailsLoaded)
		{
			Internal_LoadDetails();
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns array of scenarios
	void GetScenarios(array<MissionWorkshopItem> scenarios)
	{	
		if (!m_aMissions)
			return;
		
		scenarios.Clear();
		scenarios.Copy(m_aMissions);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Returns array of dependencies
	array<ref SCR_WorkshopItem> GetLatestDependencies()
	{
		array<ref SCR_WorkshopItem> dependencies = new array<ref SCR_WorkshopItem>;
		
		if (!m_aDependencies)
			return dependencies;
		
		foreach (auto a : m_aDependencies)
			dependencies.Insert(a);
		
		return dependencies;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns array of dependendent addons
	array<ref SCR_WorkshopItem> GetDependentAddons()
	{
		array<ref SCR_WorkshopItem> dependent = new array<ref SCR_WorkshopItem>;
		
		if (!m_aDependent)
			return dependent;
		
		foreach (auto a : m_aDependent)
		{
			if (a)
				dependent.Insert(a);
		}
		
		return dependent;
	}
	
	//-----------------------------------------------------------------------------------------------
	string GetLatestVersion()
	{
		Revision rev = GetLatestRevision();
		
		if (!rev)
			return string.Empty;
		
		return rev.GetVersion();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns array of versions
	array<string> GetVersions()
	{	
		array<string> versions = new array<string>;
		
		if (!GetRevisionsLoaded())
			return versions;
		
		foreach (auto rev : m_aRevisions)
		{
			versions.Insert(rev.GetVersion());
		}
		
		return versions;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns true if downloaded revision was uploded after 1.0
	bool IsDownloadedVersionCompatible()
	{
		if (!GetOffline())
			return false;
		
		// Hard coded check for 1.0 - should be replaced with specific version 
		string targetClientVersion = "1.0.0";
		string addonClientVersion = m_Item.GetActiveRevision().GetGameVersion();
		
		array<string> targetNumbers = {};
		targetClientVersion.Split(".", targetNumbers, false);
		
		array<string> addonNumbers = {};
		addonClientVersion.Split(".", addonNumbers, false);
		
		for (int i = 0, count = targetNumbers.Count(); i < count; i++)
		{
			int targetNumber = targetNumbers[i].ToInt();
			int addonNumber = addonNumbers[i].ToInt();
			
			if (targetNumber > addonNumber)
				return false;
		} 
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetMyRating(bool newRating)
	{
		if (!m_Item)
			return;

		m_bMyRatingSet = true;
		m_bMyRating = newRating;
		
		#ifdef WORKSHOP_DEBUG
		_print("WorkshopItem.Rate(%1)", newRating);
		#endif
				
		m_Item.Rate(newRating, null);
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: SetMyRating()");
		#endif
			
		SetChanged();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	void ResetMyRating()
	{
		if (!m_Item)
			return;
		
		m_bMyRatingSet = false;
		
		#ifdef WORKSHOP_DEBUG
		_print("WorkshopItem.ResetRating()");
		#endif
		
		m_Item.ResetRating(null);
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: ResetMyRating()");
		#endif
		
		SetChanged();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	void GetMyRating(out bool ratingSet, out bool rating)
	{
		ratingSet = m_bMyRatingSet;
		rating = m_bMyRating;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	void SetFavourite(bool favourite)
	{
		if (!m_Item)
		{
			Debug.Error("SCR_WorkshopItem: Missing item to change favorite state!");
			return;
		}
		
		if (m_bFavouriteRequesting)
			return;
		
		m_CallbackRequestFavourite = new BackendCallback();
		m_CallbackRequestFavourite.SetOnSuccess(OnFavouriteSuccess);
		m_CallbackRequestFavourite.SetOnError(OnFavouriteFail);
		
		m_Item.SetFavorite(m_CallbackRequestFavourite, favourite);
		
		m_bFavourite = favourite;
		m_bFavouriteRequesting = true;
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: SetFavourite()");
		#endif
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetFavourite()
	{	
		if (m_Item)
			return m_Item.IsFavorite();
		
		return m_bFavourite;
	}
	
	//-----------------------------------------------------------------------------------------------
	void OnFavouriteSuccess()
	{		
		m_bFavouriteRequesting = false;
		SetChanged();
	}
	
	//-----------------------------------------------------------------------------------------------
	void OnFavouriteFail()
	{	
		m_bFavourite = !m_bFavourite;
		m_bFavouriteRequesting = false;
		SetChanged();
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetSubscribed(bool subscribe)
	{
		if (!m_Item)
			return;
		
		if (subscribe)
			m_Item.Subscribe(null);
		else
			m_Item.Unsubscribe(null);
		
		m_bSubscribed = subscribe;
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: Subscribe()");
		#endif
		
		SetChanged();
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetSubscribed()
	{
		return m_bSubscribed;
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetEnabled(bool enable)
	{
		if (!GetOffline())
			return;
		
		m_Item.Enable(enable);
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: SetEnabled()");
		#endif
		
		if (enable)
			m_Item.NotifyPlay();
		
		SetChanged();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Enables/disables all downloaded dependencies
	void SetDependenciesEnabled(bool enable)
	{
		if (!m_aDependencies)
			return;
		
		// Check for missing dependencies 
		array<ref SCR_WorkshopItem> missing = SCR_AddonManager.GetInstance().SelectItemsBasic(m_aDependencies, EWorkshopItemQuery.NOT_OFFLINE);
		if (!missing.IsEmpty())
		{
			
			return;
		}
		
		// Enable all
		foreach (auto dep : m_aDependencies)
		{
			if (dep.GetOffline() && dep.GetEnabled() != enable)
			{
				dep.SetEnabled(enable);
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetEnabled()
	{
		if (!m_Item)
			return false;
		
		if (!GetOffline())
			return false;
		
		return m_Item.IsEnabled();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! True when addon is loaded by engine. It means that the game is already running with this mod.
	bool GetLoaded()
	{
		if (!m_Item)
			return false;
		
		return m_Item.IsLoaded();
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetBlocked()
	{
		if (m_Item)
		{
			int accessState = m_Item.GetAccessState();
			return accessState & EWorkshopItemAccessState.EWASTATE_BLOCKED;
		}
		else
			return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetReportedByMe()
	{
		if (m_Item)
			return m_Item.GetAccessState() & EWorkshopItemAccessState.EWASTATE_REPORTED;
		else
			return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetModAuthorReportedByMe()
	{
		if (m_Item)
		{
			WorkshopAuthor author = m_Item.Author();
			
			if (author)
				return author.IsBlocked();
			
			return false;
		}
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool IsWorldSave()
	{
		if (!m_Item)
			return false;
		
		return WorldSaveItem.Cast(m_Item);
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns true when item is restricted for any reason (blocked or reported)
	bool GetRestricted()
	{
		return GetBlocked() || GetReportedByMe() || GetModAuthorReportedByMe();
	}
	
	//-----------------------------------------------------------------------------------------------
	bool DeleteLocally()
	{
		if (!m_Item)
		{
			Debug.Error("SCR_WorkshopItem: Missing item to delete!");
			return false;
		}
		

		if (!m_Item.GetPendingDownload() && !m_Item.GetActiveRevision())
		{
			Debug.Error("SCR_WorkshopItem: Attempt to delete item which is not downloaded!");
			return true; // Already deleted
		}
		
		m_Item.DeleteLocally();
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: DeleteLocally()");
		#endif
		
		SetChanged();
		
		return true;
	}
	//-----------------------------------------------------------------------------------------------
	void DeleteDownloadProgress()
	{
		if (!m_Item)
			return;
		m_Item.DeleteDownloadProgress();
		SetChanged();
	}
	//-----------------------------------------------------------------------------------------------
	string GetName()
	{
		if (m_Item)
			return m_Item.Name();
		else if (m_Dependency)
			return m_Dependency.GetName();
		else
			return string.Empty;
	}
	
	//-----------------------------------------------------------------------------------------------
	string GetId()
	{
		if (m_Item)
			return m_Item.Id();
		else if (m_Dependency)
			return m_Dependency.GetID();
		else
			return string.Empty;
	}
	
	//-----------------------------------------------------------------------------------------------
	string GetAuthorName()
	{
		if (m_Item)
		{
			WorkshopAuthor author = m_Item.Author();
			
			if (author)
				return author.Name();
		}
		
		return string.Empty;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! True when the item is stored in the backend
	bool GetOnline()
	{
		if (m_Item)
			return m_Item.GetLatestRevision() && m_Item.GetLatestRevision().GetBackendEnv() == GetGame().GetBackendApi().GetBackendEnv();
		else if (m_Dependency)		// TODO Apparently, if it's a dependency, then it's online
			return true;
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! True when we have the item on our local storage
	bool GetOffline()
	{
		if (m_Item)
			return m_Item.GetActiveRevision();
		else if (m_Dependency)
			return m_Dependency.IsOffline();
		else
			return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns the revision which we currently have on the local storage
	Revision GetCurrentLocalRevision()
	{
		if (m_Item && GetOffline())
			return m_Item.GetActiveRevision();
		return null;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Returns true if current local version matches version of dependency
	bool GetCurrentLocalVersionMatchDependency()
	{
		if (!m_Item || !m_Dependency)
			return false;
			
		// Does active version match required?
		Revision ActiveV = GetCurrentLocalRevision();
		Revision RequiredV = m_Dependency.GetRevision();
		
		return Revision.AreEqual(ActiveV, RequiredV);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	bool GetCorrupted()
	{
		if (m_Item)
			return m_Item.GetActiveRevision().IsCorrupted();
		else if (m_Dependency)
			return false; // TODO this is not valid for dependency object
		else
			return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	BackendImage GetThumbnail()
	{		
		if (!m_Item)
			return null;
		
		return m_Item.Thumbnail();
	}
	
	//-----------------------------------------------------------------------------------------------
	array<BackendImage> GetGallery()
	{
		array<BackendImage> gallery = {};
		
		if (!m_Item)
			return gallery;
		
		m_Item.Gallery(gallery);		
		return gallery;
	}
	
	//-----------------------------------------------------------------------------------------------
	string GetDescription()
	{
		if (!m_Item)
			return string.Empty;
		
		return m_Item.Description();
	}
	
	//-----------------------------------------------------------------------------------------------
	string GetSummary()
	{
		if (!m_Item)
			return string.Empty;
		
		return m_Item.Summary();
	}
	
	//-----------------------------------------------------------------------------------------------
	int GetRatingCount()
	{
		if (!m_Item)
			return 0;
		
		return m_Item.RatingCount();
	}
	
	//-----------------------------------------------------------------------------------------------
	float GetSizeBytes()
	{
		if (m_Dependency)
		{
			return m_Dependency.TotalFileSize();
		}
		else if (m_Item)
		{
			if (!m_LatestRevision)
				m_LatestRevision = m_Item.GetLatestRevision();
			
			return m_LatestRevision.GetTotalSize();
		}
		else
		{
			Debug.Error("SCR_WorkshopItem: Dependency or WorkshopItem are not set!");
			return 0;
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnPatchComputed()
	{
		m_ComputeCallback.SetOnError(OnPatchComputedFailed);
		m_bSizeRequesting = false;
		m_OnChanged.Invoke();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnPatchComputedFailed()
	{
		m_bSizeRequesting = false;
	}
	
	//-----------------------------------------------------------------------------------------------
	float GetAverageRating()
	{
		if (!m_Item)
			return 0;
		
		return m_Item.AverageRating();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns state of the download process
	void GetDownloadState(out bool inProgress, out bool paused, out float progress, out Revision targetRevision)
	{
		if (!m_ActionDownload || !m_Item)
		{
			inProgress = false;
			paused = false;
			progress = 0;
			targetRevision = null;
			return;
		}
		
		inProgress = m_ActionDownload.IsActive() || m_ActionDownload.IsPaused();
		paused = m_ActionDownload.IsPaused();
		progress = m_Item.GetProgress();
		targetRevision = m_ActionDownload.GetTargetRevision();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns current download action
	SCR_WorkshopItemActionDownload GetDownloadAction()
	{
		return m_ActionDownload;
	}

	//-----------------------------------------------------------------------------------------------
	//! Returns the current composite action performed for dependencies of this addon
	SCR_WorkshopItemActionComposite GetDependencyCompositeAction()
	{
		return m_ActionDependency;
	}
		
	//-----------------------------------------------------------------------------------------------
	//! Returns true when we have an old version offline, and the new version
	bool GetUpdateAvailable()
	{
		bool rawUpdateAvailable = false;
		if (!m_Item)
		{
			Debug.Error("GetUpdateAvailable() was called on null item.");
			return false;
		}
		
		Revision rev = m_Item.GetLatestRevision();		
		rawUpdateAvailable = rev && !rev.IsDownloaded();
		
		bool updateAvailableByVersions = Internal_GetUpdateAvailable();
		
		return rawUpdateAvailable || updateAvailableByVersions;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns dependencies which are missing.
	array<ref SCR_WorkshopItem> GetMissingDependencies()
	{	
		array<ref SCR_WorkshopItem> dependencies = GetLatestDependencies();
		
		return SCR_AddonManager.SelectItemsBasic(dependencies, EWorkshopItemQuery.NOT_OFFLINE);
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetAnyDependencyMissing()
	{
		if (!m_aDependencies)
			return false;
		
		bool count = SCR_AddonManager.CountItemsBasic(m_aDependencies, EWorkshopItemQuery.NOT_OFFLINE, true);
		return count;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetAnyDependencyUpdateAvailable()
	{
		if (!m_aDependencies)
			return false;
		
		bool count = SCR_AddonManager.CountItemsBasic(m_aDependencies, EWorkshopItemQuery.UPDATE_AVAILABLE, true);
		return count;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns true if any dependency is offline and disabled
	bool GetEnabledAndAnyDependencyDisabled()
	{
		if (!m_aDependencies)
			return false;
		
		if (!GetEnabled())
			return false;
		
		bool count = SCR_AddonManager.CountItemsAnd(m_aDependencies, EWorkshopItemQuery.NOT_ENABLED | EWorkshopItemQuery.OFFLINE, true);
		return count;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns highest priority issue of this addon if it's downloaded,
	//! Addon might have all issue types at once, but this method returns only the most important issue
	//! In the order most suitable for UI
	EWorkshopItemProblem GetHighestPriorityProblem()
	{
		if (!GetOffline())
			return EWorkshopItemProblem.NO_PROBLEM;
		
		if (GetAnyDependencyMissing())
			return EWorkshopItemProblem.DEPENDENCY_MISSING;
		else if (GetEnabledAndAnyDependencyDisabled())
			return EWorkshopItemProblem.DEPENDENCY_DISABLED;
		else if (GetUpdateAvailable())
			return EWorkshopItemProblem.UPDATE_AVAILABLE;
		else if (GetAnyDependencyUpdateAvailable())
			return EWorkshopItemProblem.DEPENDENCY_OUTDATED;
		
		return EWorkshopItemProblem.NO_PROBLEM;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns dependencies which can be updated.
	array<ref SCR_WorkshopItem> GetOutdatedDependencies()
	{	
		array<ref SCR_WorkshopItem> dependencies = GetLatestDependencies();
		
		if (dependencies.IsEmpty())
			return new array<ref SCR_WorkshopItem>;
		
		return SCR_AddonManager.SelectItemsBasic(dependencies, EWorkshopItemQuery.UPDATE_AVAILABLE);
	}
	
	//-----------------------------------------------------------------------------------------------
	int GetTimeSinceLastPlay()
	{
		if (!m_Item)
			return -1;
		
		return m_Item.GetTimeSinceLastPlay();
	}
	
	//-----------------------------------------------------------------------------------------------
	int GetTimeSinceFirstDownload()
	{
		if (!m_Item)
			return -1;
		
		return m_Item.GetTimeSinceFirstDownload();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Unified download to target revision 
	//! There can be different revisions and reasons to setup new download 
	//! No target revision = latest
	protected SCR_WorkshopItemActionDownload DownloadRevision(Revision targetRevision, bool isDownloadRunning)
	{
		if (!m_Item && !m_Dependency)
			return null;
		
		bool latestRevision = (targetRevision == null);
		
		// Create new download 
		if (!m_ActionDownload)
		{
			m_ActionDownload = new SCR_WorkshopItemActionDownload(this, latestRevision, targetRevision);
			SCR_AddonManager.GetInstance().Internal_OnNewDownload(this, m_ActionDownload);
			
			return m_ActionDownload;
		}
		
		// Check if same download is running 
		if (isDownloadRunning)
		{
			// Same kind of download is already running
			return m_ActionDownload; // TODO if action was canceled or failed, create a new one
		}
		
		// Another download is running, but downloading a different version
		m_ActionDownload.Cancel();
		m_ActionDownload.Internal_Detach();
		m_ActionDownload = null;
		
		m_ActionDownload = new SCR_WorkshopItemActionDownload(this, latestVersion: latestRevision, targetRevision);
		SCR_AddonManager.GetInstance().Internal_OnNewDownload(this, m_ActionDownload);
		return m_ActionDownload;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Starts download of a specific version.
	//! Returns an action object, use it to control the download process or
	//! To get notified about its events.
	//!
	//! !!! If a download of same version is already running, it doesn't create a new action
	//! !!! But returns a previous one instead.
	//! 
	//! !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  
	//! ! You must call Activate() of the returned action yourself.
	//! ! This is done this way so that you can subscribe to events before the action is started.
	//! !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  
	SCR_WorkshopItemActionDownload Download(notnull Revision targetRevision)
	{
		#ifdef WORKSHOP_DEBUG
		if (targetRevision)
			_print(string.Format("Download: %1", targetRevision.GetVersion()));
		else
			_print(string.Format("Download: the latest one"));
		#endif
		
		bool running = false;
		if (m_ActionDownload && targetRevision)
			running = Revision.AreEqual(m_ActionDownload.GetTargetRevision(), targetRevision);
		
		return DownloadRevision(targetRevision, running);
	}
	
	//-----------------------------------------------------------------------------------------------
	SCR_WorkshopItemActionDownload DownloadLatestVersion()
	{
		#ifdef WORKSHOP_DEBUG
		_print("DownloadLatestVersion()");
		#endif
		
		if (!m_LatestRevision)
			m_LatestRevision = GetLatestRevision(); // Might return empty string if revs not loaded!
		
		bool running = false;
		if (m_ActionDownload && m_LatestRevision)
			running = m_ActionDownload.GetTargetedAtLatestVersion() || Revision.AreEqual(m_LatestRevision, m_ActionDownload.GetTargetRevision());
		
		return DownloadRevision(null, running);
	}
	
	//-----------------------------------------------------------------------------------------------
	SCR_WorkshopItemActionDownload RetryDownload(Revision targetRevision)
	{
		if (!m_ActionDownload)
			m_ActionDownload = new SCR_WorkshopItemActionDownload(this, latestVersion: false, targetRevision);
		
		SCR_AddonManager.GetInstance().Internal_OnNewDownload(this, m_ActionDownload);
		
		m_ActionDownload.Activate();
		
		return m_ActionDownload;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Starts downloading latest version of dependencies which are offline
	//! !!! If a previous same action is running, it doesn't create a new action but
	//! !!! Returns a previous action instead.
	SCR_WorkshopItemActionDownloadDependenciesLatest DownloadDependenciesLatest(array<ref SCR_WorkshopItem> dependencies)
	{
		#ifdef WORKSHOP_DEBUG
		_print("DownloadDependenciesLatest()");
		#endif
		
		if (!m_Item)
			return null;
		
		if (m_ActionDependency)
		{
			auto action = SCR_WorkshopItemActionDownloadDependenciesLatest.Cast(m_ActionDependency);
			if (action)
			{
				// Download action with dependencies is running
				return action;
			}
			else
			{
				// Another kind of action with dependencies is running, cancel it
				m_ActionDependency.Cancel();
				m_ActionDependency.Internal_Detach();
				m_ActionDependency = null;
				
				action = new SCR_WorkshopItemActionDownloadDependenciesLatest(this, dependencies);
				m_ActionDependency = action;
				return action;
			}
		}
		else
		{
			auto action = new SCR_WorkshopItemActionDownloadDependenciesLatest(this, dependencies);
			m_ActionDependency = action;
			return action;
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Pauses current download, if possible
	bool PauseDownload()
	{
		if (!m_Item)
			return false;
		
		if (m_ActionDownload)
		{
			return m_ActionDownload.Pause();
		}
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Cancels current download, if possible
	bool CancelDownload()
	{
		if (!m_Item)
			return false;
		
		if (m_ActionDownload)
		{
			return m_ActionDownload.Cancel();
		}
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Resumes current download, if it's already started
	bool ResumeDownload()
	{
		if (!m_Item)
			return false;
		
		if (m_ActionDownload)
		{
			return m_ActionDownload.Resume();
		}
		
		return false;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Sends a report about this workshop item
	//!
	//! !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  
	//! ! You must call Activate() of the returned action yourself.
	//! ! This is done this way so that you can subscribe to events before the action is started.
	//! !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  !  
	SCR_WorkshopItemActionReport Report(EWorkshopReportType eReport, string sMessage)
	{
		if (!m_Item)
			return null;
		
		if (m_ActionReport)
		{
			// We can't abort it or change while it's being sent anyway,
			// Return the existing action
			return m_ActionReport;
		}
		else
		{
			m_ActionReport = new SCR_WorkshopItemActionReport(this, eReport, sMessage);
			return m_ActionReport;
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Loads report of current user from backend. Subscribe to m_OnMyReportLoaded to get result.
	void LoadReport()
	{
		if (!m_Item)
			return;
		
		if (!m_CallbackLoadMyReport)
		{
			m_CallbackLoadMyReport = new BackendCallback();
			m_CallbackLoadMyReport.SetOnSuccess(Callback_LoadMyReport_OnSuccess);
			m_CallbackLoadMyReport.SetOnError(Callback_LoadMyReport_OnError);
		}
		
		m_Item.LoadReport(m_CallbackLoadMyReport);
	}

	
	//-----------------------------------------------------------------------------------------------
	//! Returns our own report. We must load it first with LoadMyReport.
	//! How the function behaves immediately relative to data passed through Report() is undefined.
	void GetReport(out EWorkshopReportType reportType, out string reportDescription)
	{
		if (!m_Item)
		{
			reportType = EWorkshopReportType.EWREPORT_OTHER;
			reportDescription = string.Empty;
			return;
		}
		
		reportType = m_Item.GetReportType();
		reportDescription = m_Item.GetReportDescription();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Sends a cancel report about this workshop item
	SCR_WorkshopItemActionCancelReport CancelReport()
	{
		if (!m_Item)
			return null;
		
		if (m_ActionReport)
		{
			// We can't abort it or change while it's being sent anyway,
			// Return the existing action
			return m_ActionCancelReport;
		}
		else
		{
			m_ActionCancelReport = new SCR_WorkshopItemActionCancelReport(this);
			return m_ActionCancelReport;
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Add block for workshop author and all author mods
	SCR_WorkshopItemActionAddAuthorBlock AddAuthorBlock()
	{
		if (!m_Item)
			return null;
		
		if (!m_ActionAddAuthorBlock)
			m_ActionAddAuthorBlock = new SCR_WorkshopItemActionAddAuthorBlock(this);
		
		return m_ActionAddAuthorBlock;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Add block for workshop author and all author mods
	SCR_WorkshopItemActionRemoveAuthorBlock RemoveAuthorBlock()
	{
		if (!m_Item)
			return null;
		
		if (!m_ActionRemoveAuthorBlock)
			m_ActionRemoveAuthorBlock = new SCR_WorkshopItemActionRemoveAuthorBlock(this);
		
		return m_ActionRemoveAuthorBlock;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Finds revision object by string version
	Revision FindRevision(string version)
	{
		if (!m_aRevisions)
			return null;
		
		foreach (Revision r : m_aRevisions)
		{
			if (r.GetVersion() == version)
				return r;
		}
		
		return null;
	}
	
	//-----------------------------------------------------------------------------------------------
	// 					P R O T E C T E D   /   P R I V A T E
	//-----------------------------------------------------------------------------------------------
		
	
	//-----------------------------------------------------------------------------------------------
	Revision GetLatestRevision()
	{
		
		if (!m_Item)
		{
			Debug.Error("SCR_WorkshopItem: Item is null!");
			return null;
		}
		

		Revision rev = m_Item.GetLatestRevision();
		if (!rev && GetOnline() )
			Debug.Error("SCR_WorkshopItem: Item is present but LatestRevision is null!");

		return rev;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	// 					C A L L B A C K S
	//-----------------------------------------------------------------------------------------------
	
	
	//  --- Ask Details ---
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_AskDetails_OnGetAsset()
	{
		#ifdef WORKSHOP_DEBUG
		_print("Callback_AskDetails_OnGetAsset()");
		#endif
		
		TryLoadItemFromDependency();
		
		TryLoadRevisions();
		
		if (!m_aRevisions || m_aRevisions.IsEmpty())
		{
			#ifdef WORKSHOP_DEBUG
			_print("Callback_AskDetails_OnGetAsset(): Received no revisions", LogLevel.ERROR);
			#endif
		}
		else
		{
			#ifdef WORKSHOP_DEBUG
			_print("Callback_AskDetails_OnGetAsset(): Received revisions:");
			array<string> versions = GetVersions();
			foreach (string ver : versions)
			{
				_print(string.Format("Callback_AskDetails_OnGetAsset():   %1", ver));
			}
			#endif
			
			if (!m_CallbackLoadDependencies)
			{
				m_CallbackLoadDependencies = new BackendCallback;
				m_CallbackLoadDependencies.SetOnSuccess(Callback_LoadDependencies_OnSuccess);
				m_CallbackLoadDependencies.SetOnError(Callback_LoadDependencies_OnError);
			}
			
			if (!m_CallbackLoadScenarios)
			{
				m_CallbackLoadScenarios = new BackendCallback;
				m_CallbackLoadScenarios.SetOnSuccess(Callback_LoadScenarios_OnSuccess);
				m_CallbackLoadScenarios.SetOnError(Callback_LoadScenarios_OnError);
			}
			
			if (m_Item)
			{
				if (!m_LatestRevision)
					m_LatestRevision = GetLatestRevision();
				
				if (m_LatestRevision)
				{
					m_LatestRevision.LoadDependencies(m_CallbackLoadDependencies);
					m_LatestRevision.LoadScenarios(m_CallbackLoadScenarios);
				}
			}
		}
		
		m_bDetailsLoaded = true;
		m_OnGetAsset.Invoke(this);
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: Callback_AskDetails_OnGetAsset");
		#endif
		
		SetChanged();
		m_bWaitingLoadDetails = false;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_AskDetails_OnError(BackendCallback callback)
	{
		m_bWaitingLoadDetails = false;
		if (callback.GetRestResult() == ERestResult.EREST_ERROR_TIMEOUT)
		{
			#ifdef WORKSHOP_DEBUG
			_print("Callback_AskDetails_OnTimeout()");
			#endif

			m_OnTimeout.Invoke(this);
			return;
		}
		
		m_bRequestFailed = true;
		
		// We still have received a response, but with an error.
		m_OnGetAsset.Invoke(this);
		
		SetChanged();

		//CustomDebugError("SCR_WorkshopItem failed to load details");
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_LoadDependencies_OnSuccess()
	{
		#ifdef WORKSHOP_DEBUG
		_print("Callback_LoadDependencies_OnSuccess()");
		#endif
		
		TryLoadDependencies(); // For now we support only dependencies of latest version
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_LoadDependencies_OnError(BackendCallback cb)
	{
		#ifdef WORKSHOP_DEBUG
		_print("Callback_LoadDependencies_OnError()");
		#endif
		
		if (cb.GetRestResult() == ERestResult.EREST_ERROR_TIMEOUT)
		{
			m_OnTimeout.Invoke(this);
		}
		else
		{
			m_OnError.Invoke(this);
		}

		CustomDebugError("SCR_WorkshopItem failed to load dependencies");
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_LoadScenarios_OnSuccess()
	{
		#ifdef WORKSHOP_DEBUG
		_print("Callback_LoadScenarios_OnSuccess()");
		#endif
		
		TryLoadScenarios();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_LoadScenarios_OnError(BackendCallback cb)
	{
		#ifdef WORKSHOP_DEBUG
		_print("Callback_LoadScenarios_OnError()");
		#endif
		
		if (cb.GetRestResult() == ERestResult.EREST_ERROR_TIMEOUT)
		{
			m_OnTimeout.Invoke(this);
		}
		else
		{
			m_OnError.Invoke(this);
		}
		
		CustomDebugError("SCR_WorkshopItem failed to load scenarios");
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_LoadMyReport_OnSuccess()
	{
		m_OnMyReportLoaded.Invoke(this);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_LoadMyReport_OnError()
	{
		m_OnMyReportLoadError.Invoke(this);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void TryLoadItemFromDependency()
	{
		// Try to get WorkshopItem from Dependency, if we have only Dependency
		if (!m_Item && m_Dependency)
		{
			m_Item = m_Dependency.GetCachedItem();
			
			if (m_Item)
			{
				UpdateStateFromWorkshopItem();
			}
			
			#ifdef WORKSHOP_DEBUG
			if (m_Item)
				_print(string.Format("WorkshopItem was missing, but was loaded: %1", m_Item.Name()));
			#endif
		}
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void TryLoadRevisions(bool log = true)
	{
		if (!m_Item)
			return;
		
		array<Revision> revisionsTemp = new array<Revision>;
		m_Item.GetRevisions(revisionsTemp);
		
		if (revisionsTemp.IsEmpty() && m_Dependency)
			revisionsTemp.Insert(m_Dependency.GetRevision());
		
		if (revisionsTemp.IsEmpty())
		{
			#ifdef WORKSHOP_DEBUG
			if (log)
				_print(string.Format("Revisions are not loaded yet. Revision array is empty."));
			#endif
			
			return;
		}
			
		// The revision array is not empty, so we can store it
		// When m_aRevisions is not null, it means revisions are loaded
		m_aRevisions = revisionsTemp;
	}
	
	//-----------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItem> GetLoadedDependencies()
	{
		return m_aDependencies;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void TryLoadDependencies(bool log = true)
	{
		#ifdef WORKSHOP_DEBUG
		if (log)
			_print("TryLoadDependencies()");
		#endif
		

		// Get array of dependencies - TODO: differentiate between dependencies version to load
		m_LatestRevision = GetLatestRevision();
		array<WorkshopItem> dependencies = new array<WorkshopItem>;
		
		if (m_LatestRevision && !SCR_Enum.HasFlag(m_LatestRevision.GetLoadFlags(), EPendingLoadState.ELS_DEPENDENCIES))
			m_LatestRevision.GetDependencies(dependencies);
		else
		{
			m_OnDependenciesLoaded.Invoke(this);
			return;
		}

		if (!m_aDependencies)
			m_aDependencies = new array<ref SCR_WorkshopItem>;
		
		m_aDependencies.Clear();
		
		foreach (WorkshopItem dep : dependencies)
		{
			SCR_WorkshopItem registeredItem = SCR_AddonManager.GetInstance().Register(dep);
			RegisterDependency(registeredItem);
			registeredItem.RegisterDependent(this);
		}
		
		m_OnDependenciesLoaded.Invoke(this);
	}
	
	
	
	//-----------------------------------------------------------------------------------------------
	protected void TryLoadScenarios(bool log = true)
	{
		#ifdef WORKSHOP_DEBUG
		if (log)
			_print("TryLoadScenarios()");
		#endif			
		
		// Load scenarios of latest revision
		m_LatestRevision = GetLatestRevision();
		
		m_aMissions = new array<MissionWorkshopItem>;
		
		if (m_LatestRevision && !SCR_Enum.HasFlag(m_LatestRevision.GetLoadFlags(), EPendingLoadState.ELS_SCENARIOS))
			m_LatestRevision.GetScenarios(m_aMissions);

		#ifdef WORKSHOP_DEBUG
		_print(string.Format("TryLoadScenarios(): Received %1 scenarios", m_aMissions.Count()));
		_print("OnChanged: Callback_AskDetails_OnGetScenarios");
		#endif
		
		SetChanged();
		
		m_OnScenariosLoaded.Invoke(this);
	}
	
	
	
	//-----------------------------------------------------------------------------------------------
	protected void RegisterDependency(SCR_WorkshopItem item)
	{
		int id = m_aDependencies.Find(item);
		if (id == -1)
			m_aDependencies.Insert(item);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void UnregisterDependency(SCR_WorkshopItem item)
	{
		m_aDependencies.RemoveItem(item);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void RegisterDependent(SCR_WorkshopItem item)
	{
		int id = m_aDependent.Find(item);
		if (id == -1)
			m_aDependent.Insert(item);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	protected void UnregisterDependent(SCR_WorkshopItem item)
	{
		m_aDependent.RemoveItem(item);
	}
	
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	
	
	
	
	
	
	
	
	
	
	//-----------------------------------------------------------------------------------------------
	// 			I N T E R A C T I O N S   W I T H   W O R K S H O P   A P I 
	//-----------------------------------------------------------------------------------------------
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_StartDownload(notnull Revision targetRevision, BackendCallback callback)
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Internal_StartDownload(): %1, %2", targetRevision.GetVersion(), callback));
		#endif
		
		// Check privileges
		if (!SCR_AddonManager.GetInstance().GetUgcPrivilege())
		{
			#ifdef WORKSHOP_DEBUG
			_print("Internal_StartDownload(): UGC privileges were not checked or false!");
			#endif
			return false;
		}
		
		
		#ifdef WORKSHOP_DEBUG
		_print("Internal_StartDownload(): Calling WorkshopItem.Download ...");
		#endif
		
		if (!m_Item)
		{
			if (!m_Dependency)
			{
				Debug.Error("SCR_WorkshopItem: Cannot download because Item and Dependency are missing!");
				return false;
			}
			
			m_Item = m_Dependency.GetCachedItem();
			
			if (!m_Item)
			{
				Debug.Error("SCR_WorkshopItem: Cannot download because Item was not cached!");
				return false;
			}
		}
		
		
		m_Item.Download(callback, targetRevision);
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_CancelDownload()
	{
		// Check
		#ifdef WORKSHOP_DEBUG
		_print("Internal_CancelDownload()");
		#endif
		
		if (!m_Item)
		{
			Debug.Error("SCR_WorkshopItem: Cannot cancel download because Item is missing!");
			return false;
		}
			
		
		// Pause
		m_CancelDownloadCallback.SetOnSuccess(OnCancelDownloadResponse);
		m_CancelDownloadCallback.SetOnError(OnCancelDownloadError);
		m_Item.Cancel(m_CancelDownloadCallback);
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnCancelDownloadResponse(BackendCallback callback)
	{		
		m_OnCanceled.Invoke(this);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnCancelDownloadError(BackendCallback callback)
	{
		m_OnError.Invoke(this);
		m_OnCanceled.Invoke(this);
	}
	
	/*
	protected ref SCR_BackendCallback m_PauseDownloadCallback = new SCR_BackendCallback();
	protected bool m_bRunningRequested;
	protected bool m_bRunningProccessed;
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_PauseDownload()
	{
		// Check
		#ifdef WORKSHOP_DEBUG
		_print("Internal_PauseDownload()");
		#endif
		
		if (!m_Item)
			return false;
		
		// Pause
		m_bRunningRequested = false;
		
		m_PauseDownloadCallback.GetEventOnResponse().Insert(OnPauseDownloadResponse);
		m_Item.PauseDownload(m_PauseDownloadCallback);
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnPauseDownloadResponse(SCR_BackendCallback callback)
	{
		m_PauseDownloadCallback.GetEventOnResponse().Remove(OnPauseDownloadResponse);
		m_bRunningProccessed = false;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_ResumeDownload(BackendCallback callback)
	{
		#ifdef WORKSHOP_DEBUG
		_print("Internal_ResumeDownload()");
		#endif
		
		if (!m_Item)
			return false;
		
		// Resume 
		m_bRunningRequested = true;
		
		m_Item.ResumeDownload(callback);
		return true;
	}
	*/
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_Report(EWorkshopReportType eReport, string sMessage, BackendCallback callback)
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Internal_Report(): %1, %2, %3", eReport, sMessage, callback));
		#endif
		
		if (!m_Item)
			return false;
		
		m_Item.Report(eReport, sMessage, callback);
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: Internal_Report()");
		#endif
		
		SetChanged();
		
		return true;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_CancelReport(BackendCallback callback)
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Internal_CancelReport(): %1", callback));
		#endif
		
		if (!m_Item)
			return false;
		
		m_Item.CancelReport(callback);
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: Internal_CancelReport()");
		#endif
		
		SetChanged();
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_AddAuthorBlock(BackendCallback callback)
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Internal_AddAuthorBlock(): %1", callback));
		#endif
		
		if (!m_Item)
			return false;
		
		WorkshopAuthor author = m_Item.Author();
		if (!author)
			return false;
		
		author.AddBlock(callback);
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: Internal_AddAuthorBlock()");
		#endif
		
		SetChanged();
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_RemoveAuthorBlock(BackendCallback callback)
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Internal_RemoveAuthorBlock(): %1", callback));
		#endif
		
		if (!m_Item)
			return false;
		
		WorkshopAuthor author = m_Item.Author();
		if (!author)
			return false;
		
		author.RemoveBlock(callback);
		
		#ifdef WORKSHOP_DEBUG
		_print("OnChanged: Internal_RemoveAuthorBlock()");
		#endif
		
		SetChanged();
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Called each frame by Addon Manager.
	void Internal_Update(float timeSlice)
	{	
		// Check if we are waiting for dependencies or revisions
		/*
		if (!GetRevisionsLoaded())
			TryLoadRevisions(false);
		*/
		//if (!GetDependenciesLoaded())
		//	TryLoadDependencies(false);		
		
		
		// Update all our current actions
		// If our own action are completed, we unref and detach them
		// We don't delete the actions so that users can still access them
		if (m_ActionDownload)
		{
			

			m_ActionDownload.Internal_Update(timeSlice);
			
			float newProgress;
			if (m_Item)
				newProgress = m_Item.GetProgress();
			if (newProgress != m_fPrevDownloadProgress)
			{
				/*
				// Too spammy
				#ifdef WORKSHOP_DEBUG
				_print(string.Format("OnChanged: Internal_Update: Download Progress Changed: %1", newProgress));
				#endif
				*/
				m_fPrevDownloadProgress = newProgress;
			}
			
			if (m_ActionDownload.IsCompleted())
			{
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Download Is Completed");
				#endif
				
				m_ActionDownload.Internal_Detach();
				m_ActionDownload = null;
				m_OnDownloadComplete.Invoke(this);
				SetChanged();
			}
			else if (m_ActionDownload.IsCanceled())
			{
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Download was canceled, unregistering");
				#endif
				
				m_ActionDownload.Internal_Detach();
				m_ActionDownload = null;
				SetChanged();
			}
			else if (m_ActionDownload.IsFailed())
			{
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Download has failed, unregistering");
				#endif
				
				m_ActionDownload.Internal_Detach();
				m_ActionDownload = null;
				SetChanged();
			}
		}
			
		if (m_ActionReport)
		{
			m_ActionReport.Internal_Update(timeSlice);
			
			if (m_ActionReport.IsCompleted())
			{
				m_ActionReport.Internal_Detach();
				m_ActionReport = null;
				
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Report Action Was Completed");
				#endif
				
				SetChanged();
				m_OnReportStateChanged.Invoke(this, true);
			}
		}
		
		if (m_ActionCancelReport)
		{
			m_ActionCancelReport.Internal_Update(timeSlice);
			
			if (m_ActionCancelReport.IsCompleted())
			{
				m_ActionCancelReport.Internal_Detach();
				m_ActionCancelReport = null;
				
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Cancel report Action Was Completed");
				#endif
				
				SetChanged();
				m_OnReportStateChanged.Invoke(this, false);
			}
		}
			
		if (m_ActionDependency)
		{
			m_ActionDependency.Internal_Update(timeSlice);
			
			if (m_ActionDependency.IsCompleted())
			{
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Dependency action is completed, unregistering");
				#endif
				
				m_ActionDependency.Internal_Detach();
				m_ActionDependency = null;
				SetChanged();
				return;
			}
			else if (m_ActionDependency.IsCanceled())
			{
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Dependency action is canceled, unregistering");
				#endif
				
				m_ActionDependency.Internal_Detach();
				m_ActionDependency = null;
				SetChanged();
			}
			else if (m_ActionDependency.IsFailed())
			{
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Dependency action has failed, unregistering");
				#endif
				
				m_ActionDependency.Internal_Detach();
				m_ActionDependency = null;
				SetChanged();
			}
		}
		
		// Update mod author block state
		UpdateAuthorBlock();
		
		// Update the offline state
		bool offlineNew = GetOffline();
		if (offlineNew != m_bOffline)
		{
			m_OnOfflineStateChanged.Invoke(this, offlineNew);
		}
		m_bOffline = offlineNew;
		
		// Update the access state
		if (m_Item)
		{
			int accessState = m_Item.GetAccessState();
			if (accessState != m_iAccessState)
			{
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Access state has changed");
				#endif			
				SetChanged();
				m_iAccessState = accessState;
			}
		}
		
		// Invoke changed script invoker
		if (m_bChanged)
		{
			
			#ifdef WORKSHOP_DEBUG
			_print(string.Format("Invoking OnChanged: ID: %1, Name: %2", GetId(), GetName()));
			#endif
			
			m_OnChanged.Invoke(this);
			m_bChanged = false;
			
			// Invoke it for dependent addons too
			foreach (auto i : m_aDependent)
			{
				if (i)
				{
					#ifdef WORKSHOP_DEBUG
					_print(string.Format("Invoking OnChanged: ID: %1, Name: %2, for dependent: %3, %4", GetId(), GetName(), i.GetId(), i.GetName()));
					#endif
					i.m_OnChanged.Invoke(i);
				}
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Check state of action to add and remove author block in update
	protected void UpdateAuthorBlock()
	{
		// Add block
		if (m_ActionAddAuthorBlock)
		{
			m_ActionAddAuthorBlock.Internal_Update(0);
			
			if (m_ActionAddAuthorBlock.IsCompleted())
			{
				m_ActionAddAuthorBlock.Internal_Detach();
				m_ActionAddAuthorBlock = null;
				
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Add author block action Was Completed");
				#endif
				
				SetChanged();
				m_OnReportStateChanged.Invoke(this, true);
			}
		}
		
		// Remove block 
		if (m_ActionRemoveAuthorBlock)
		{
			m_ActionRemoveAuthorBlock.Internal_Update(0);
			
			if (m_ActionRemoveAuthorBlock.IsCompleted())
			{
				m_ActionRemoveAuthorBlock.Internal_Detach();
				m_ActionRemoveAuthorBlock = null;
				
				#ifdef WORKSHOP_DEBUG
				_print("OnChanged: Internal_Update: Add author block action Was Completed");
				#endif
				
				SetChanged();
				m_OnReportStateChanged.Invoke(this, false);
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Called by actions when some change happens
	void Internal_OnChanged()
	{
		SetChanged();
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Returns true when it can be unregistered bu addon manager
	//! We don't unregister items if they are offline or have any actions in progress, or if we have reported them (to avoid flushing item cache)
	bool Internal_GetCanBeUnregistered()
	{
		if (m_ActionDownload || m_ActionReport || m_ActionDependency || m_ActionCancelReport || GetOffline() 
			|| m_ActionAddAuthorBlock || m_ActionRemoveAuthorBlock)
			return false;
		else
			return true;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Returns true when it can be unregistered bu addon manager
	WorkshopItem Internal_GetWorkshopItem()
	{
		if (!m_Item)
			m_Item = m_Dependency.GetCachedItem();
		return m_Item;
	}
	
	//-----------------------------------------------------------------------------------------------
	float Internal_GetDownloadProgress()
	{
		if (!m_Item)
			return 0;
		
		return m_Item.GetProgress();
	}
	
	//-----------------------------------------------------------------------------------------------
	bool Internal_GetIsProcessing()
	{
		if (!m_Item)
			return false;
		
		Revision rev = m_Item.GetDownloadingRevision();
		if (!rev)
			return false;
		
		return rev.IsProcessing();
	}
	
	//-----------------------------------------------------------------------------------------------
	float Internal_GetProcessingProgress()
	{
		if (!m_Item)
			return 0;
		
		Revision rev = m_Item.GetDownloadingRevision();
		if (!rev)
			return 0;
		
		return rev.GetProcessingProgress();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Creates a callback object, requests workshop API to load details
	void Internal_LoadDetails()
	{
		if (!m_CallbackAskDetails)
		{
			m_CallbackAskDetails = new SCR_WorkshopItemCallback_AskDetails(m_Item);
			m_CallbackAskDetails.SetOnSuccess(Callback_AskDetails_OnGetAsset);
			m_CallbackAskDetails.SetOnError(Callback_AskDetails_OnError);
		}
		
		//Restarts waiting for details? - TODO review logic
		if (!m_bWaitingLoadDetails)
		{
			if (m_Item)
			{
				if (m_Item.GetLatestRevision() && m_Item.GetLatestRevision().GetBackendEnv() == GetGame().GetBackendApi().GetBackendEnv())
				{
					m_bWaitingLoadDetails = true;
					m_Item.AskDetail(m_CallbackAskDetails);			// Internally both methods perform the same request
				}
			}
			else if (m_Dependency)
			{
				m_bWaitingLoadDetails = true;
				m_Dependency.LoadItem(m_CallbackAskDetails);
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetItemTargetRevision(Revision revision)
	{
		m_ItemTargetRevision = revision;
	}
	
	//-----------------------------------------------------------------------------------------------
	Revision GetItemTargetRevision()
	{
		return m_ItemTargetRevision;
	}
	
	//-----------------------------------------------------------------------------------------------
	void SetTargetRevisionPatchSize(float size)
	{
		m_fTargetRevisionPatchSize = size;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Get dependency target revision patch size 
	//! Dependency has to be define and patch has to be computed
	float GetTargetRevisionPatchSize()
	{
		return m_fTargetRevisionPatchSize;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void SetChanged()
	{
		m_bChanged = true;
	}
	
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	
	
	
	
	
	
	
	
	
	
	//-----------------------------------------------------------------------------------------------
	//						C O N S T R U C T I O N
	//-----------------------------------------------------------------------------------------------
	
	//-----------------------------------------------------------------------------------------------
	//! Private constructor - don't instantiate this yourself! Use SCR_AddonManager instead.
	private void SCR_WorkshopItem(WorkshopItem item, Dependency dependency)
	{
		m_Item = item;
		
		m_Dependency = dependency;
		
		if (m_Item)
		{
			#ifdef WORKSHOP_DEBUG
			_print(string.Format("NEW for WorkshopItem: ID: %1, Name: %2", m_Item.Id(), m_Item.Name()));
			#endif
			
			UpdateStateFromWorkshopItem();
		}
		
		if(m_Dependency)
		{
			#ifdef WORKSHOP_DEBUG
			_print(string.Format("NEW for Dependency: ID: %1, Name: %2", m_Dependency.GetID(), m_Dependency.GetName()));
			#endif
		}
		
		if (!m_Item && !m_Dependency)
			Print("Can't be set!");
		
		m_bOffline = GetOffline();
		
		#ifdef WORKSHOP_DEBUG
		m_sName = GetName();
		m_sId = GetId();
		#endif
	}
	
	//-----------------------------------------------------------------------------------------------
	void ~SCR_WorkshopItem()
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("DELETE for: ID: %1, Name: %2", this.GetId(), this.GetName()));
		#endif
		
		if (m_aDependencies)
		{
			foreach (SCR_WorkshopItem dependency : m_aDependencies)
			{
				if (dependency)
				{
					// Unregister this from dependency, so that it doesn't point back at a null
					dependency.UnregisterDependent(this);
				}
			}
		}
	}
	
	
	//-----------------------------------------------------------------------------------------------
	void Internal_UpdateObjects(WorkshopItem item, Dependency dependency)
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Internal_UpdateObjects: %1, %2", item, dependency));
		#endif
		
		if (!m_Item && item)
		{
			m_Item = item;
			UpdateStateFromWorkshopItem();
		}
			
		// Alwasys update dependency - because it might be required to join a different server and thus dependency object
		// will be different!
		if (dependency)
			m_Dependency = dependency;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Called by SCR_AddonManager when the game checks addons after game start.
	void Internal_OnAddonsChecked()
	{
		UpdateStateFromWorkshopItem();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected bool Internal_GetUpdateAvailable()
	{
		if (GetCurrentLocalRevision())
			return GetCurrentLocalRevision().CompareTo(GetLatestRevision()) < 0;
		else 
			return false;
	}
	
	//-----------------------------------------------------------------------------------------------
	static SCR_WorkshopItem Internal_CreateFromWorkshopItem(WorkshopItem item)
	{
		SCR_WorkshopItem wrapper = new SCR_WorkshopItem(item, null);
		return wrapper;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	static SCR_WorkshopItem Internal_CreateFromDependency(Dependency dependency)
	{

		SCR_WorkshopItem wrapper = new SCR_WorkshopItem(null, dependency);
		return wrapper;
	}
	
	//-----------------------------------------------------------------------------------------------
	void UpdateStateFromWorkshopItem()
	{	
		if (m_Item)
		{
			m_bSubscribed = m_Item.IsSubscribed();
			m_bMyRating = m_Item.MyRating();
			m_bMyRatingSet = m_Item.IsRatingSet();
			m_bFavourite = m_Item.IsFavorite();
			
			Revision pendingDownload = m_Item.GetPendingDownload();
			if (pendingDownload)
				SetItemTargetRevision(pendingDownload);
			
			if (m_Item.IsEnabled())
				m_Item.NotifyPlay();
		}
		
		TryLoadDependencies();
		TryLoadScenarios();
	}
	
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	static void _sprint(string str, LogLevel logLevel = LogLevel.DEBUG)
	{
		Print(string.Format("[SCR_AddonManager] %1", str), logLevel);
	}
	
	//------------------------------------------------------------------------------------------------
	void _print(string str, LogLevel logLevel = LogLevel.DEBUG)
	{
		Print(string.Format("[SCR_AddonManager] %1 %2", this, str), logLevel);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void CustomDebugError(string message)
	{	
		string addonEnv = "unknown due to missing item";
		if (m_Item)
			addonEnv = m_Item.GetBackendEnv();
		
		string CurrentEnv = GetGame().GetBackendApi().GetBackendEnv();
		//Debug.Error(string.Format("Addon: %1 [id: %2, from: %3] - %4 [current env: %5]", GetName(), GetId(), addonEnv, message, CurrentEnv));
	}
};

//---- REFACTOR NOTE END ----
