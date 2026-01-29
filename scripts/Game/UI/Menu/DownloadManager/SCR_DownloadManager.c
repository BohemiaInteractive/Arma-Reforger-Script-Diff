/*
This entity is required for SCR_DownloadManager_Dialog to work.

It performs tracking of current downloads, while the Dialog class just visualizes them when opened.

!!! This entity relies on SCR_AddonManager, which also must be placed into the world.
*/

[EntityEditorProps(category: "", description: "Entity of the download manager. Most likely only needed in the main menu world.")]
class SCR_DownloadManagerClass: GenericEntityClass
{
};

void ScriptInvoker_DownloadManagerAction(SCR_WorkshopItemActionDownload action);
typedef func ScriptInvoker_DownloadManagerAction;

void ScriptInvoker_DownloadManagerActionError(SCR_WorkshopItemActionDownload action, int reason);
typedef func ScriptInvoker_DownloadManagerActionError;

class SCR_DownloadManager : GenericEntity
{
	protected const int DOWNLOAD_STUCK_DELAY = 60; // Max time in which download needs to progress in seconds
	
	protected const int ERROR_FULL_STORAGE = 19;
	
	protected static SCR_DownloadManager s_Instance;
	
	// All download actions
	protected ref array<ref SCR_DownloadManager_Entry> m_aDownloadActions = new array<ref SCR_DownloadManager_Entry>();
	
	// Download queue - it gets empty once all active downloads are complete, but keeps added up if new downloads are started
	// While previous are in progress.
	// This is mainly used by the panel.
	protected ref array<ref SCR_WorkshopItemActionDownload> m_aDownloadQueue = new array<ref SCR_WorkshopItemActionDownload>;
	protected int m_iQueueDownloadsCompleted; // Amount of completed downloads in the queue
	
	protected float m_fNoDownloadProgressTimer = 0; // Track how long there was no progress on downloading until stuck delay
	protected float m_fDownloadQueueSize;
	protected float m_fDownloadedSize;
	
	// Bool to pause all downloads
	protected bool m_bDownloadsPaused;
	
	protected const int FAIL_TIME = 500;
	protected ref array<ref SCR_WorkshopItemActionDownload> m_aFailedDownloads = {};

	//! Script invoker for new downloads
	//! ALways gets called after the new download is registered in the download manager.
	ref ScriptInvoker m_OnNewDownload = new ScriptInvoker; // (SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	
	ref ScriptInvokerBase<ScriptInvoker_DownloadManagerAction> m_OnDownloadComplete = new ScriptInvokerBase<ScriptInvoker_DownloadManagerAction>();
	ref ScriptInvokerBase<ScriptInvoker_DownloadManagerActionError> m_OnDownloadFailed = new ScriptInvokerBase<ScriptInvoker_DownloadManagerActionError>();
	protected ref ScriptInvokerBase<ScriptInvoker_ActionDownloadFullStorage> m_OnFullStorageError;
	ref ScriptInvokerBase<ScriptInvoker_DownloadManagerAction> m_OnDownloadCanceled = new ScriptInvokerBase<ScriptInvoker_DownloadManagerAction>();
	protected ref ScriptInvokerVoid m_OnDownloadQueueCompleted
	protected ref ScriptInvokerVoid m_OnAllDownloadsStopped;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_ActionDownloadFullStorage> GetOnFullStorageError()
	{
		if (!m_OnFullStorageError)
			m_OnFullStorageError = new ScriptInvokerBase<ScriptInvoker_ActionDownloadFullStorage>();
		
		return m_OnFullStorageError;
	}
	
	//-----------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnDownloadQueueCompleted()
	{
		if (!m_OnDownloadQueueCompleted)
			m_OnDownloadQueueCompleted = new ScriptInvokerVoid();
		
		return m_OnDownloadQueueCompleted;
	}
	
	//-----------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnAllDownloadsStopped()
	{
		if (!m_OnAllDownloadsStopped)
			m_OnAllDownloadsStopped = new ScriptInvokerVoid();
		
		return m_OnAllDownloadsStopped;
	}
	
	//-----------------------------------------------------------------------------------------------
	// 				P U B L I C   A P I 
	//-----------------------------------------------------------------------------------------------
	
	
	//-----------------------------------------------------------------------------------------------
	static SCR_DownloadManager GetInstance()
	{
		return s_Instance;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns an array of all downloads regardless of their state
	void GetAllDownloads(array<ref SCR_DownloadManager_Entry> downloads)
	{
		downloads.Clear();
		
		foreach (auto dl : m_aDownloadActions)
			downloads.Insert(dl);
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Might get delayed by a frame! Just use it for UI.
	void GetDownloadQueueState(out int nCompleted, out int nTotal)
	{
		nTotal = m_aDownloadQueue.Count();
		nCompleted = m_iQueueDownloadsCompleted;
	}
	
	//-----------------------------------------------------------------------------------------------
	bool HasRunningDownloads()
	{
		return !m_aDownloadQueue.IsEmpty();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Return true if there is addon with given id in download queue
	SCR_WorkshopItemActionDownload DownloadingActionAddonById(string id, bool runningOnly = true)
	{
		foreach (SCR_WorkshopItemActionDownload download : m_aDownloadQueue)
		{
			if (download.m_Wrapper.GetId() != id)
				continue;
			
			if (runningOnly && download.IsActive() && !download.IsCompleted() && !download.IsFailed() && !download.IsCanceled())
				return download;
		}
		
		return null;
	}
	
	//-----------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItemActionDownload> GetDownloadQueue()
	{
		array<ref SCR_WorkshopItemActionDownload> downloads = {};
		
		if (!m_aDownloadQueue)
			return downloads;
		
		foreach (SCR_WorkshopItemActionDownload action : m_aDownloadQueue)
		{
			downloads.Insert(action);
		}
		
		return downloads;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Return item action for given workshop item 
	SCR_WorkshopItemActionDownload GetActionOfItem(SCR_WorkshopItem item)
	{
		foreach (SCR_DownloadManager_Entry entry : m_aDownloadActions)
		{
			if (entry.m_Item == item)
				return entry.m_Action;
		}
		
		return null;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Paused or resumes all downloads globally
	//! Pause/resume of all downloads is called recursivelly to prevent freezing  
	//! Count = -1 setup the intial pause and starts late call
	void SetDownloadsPaused(bool pause, int count = -1)
	{
		if (pause == m_bDownloadsPaused)
			return;
		
		m_bDownloadsPaused = pause;

		// Setup calls count
		if (count == -1)
			count = m_aDownloadQueue.Count() - 1;
		
		// Paseu/resume
		PauseAction(m_aDownloadQueue[count], pause);
		
		// Late call 
		if (count > 0)
			GetGame().GetCallqueue().CallLater(SetDownloadsPaused, 0, false, pause, count - 1);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void PauseAction(SCR_WorkshopItemActionDownload action, bool pause)
	{
		// Pause 
		if (pause)
		{
			if (action.IsActive())
				action.Pause();
			
			return;
		}
		
		// Resume
		if (action.IsPaused())
			action.Resume();
		else if (action.IsInactive())
			action.Activate();
	}
	
	//-----------------------------------------------------------------------------------------------
	bool GetDownloadsPaused()
	{
		return m_bDownloadsPaused;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//! Ends all downloads in a safe manner.
	//! It is meant to be caused when we start a scenario or exit the game.
	//! The function should handle the termination of all downloads in safest way currently possible.
	void EndAllDownloads()
	{
		foreach (SCR_WorkshopItemActionDownload a : m_aDownloadQueue)
		{
			a.Cancel();
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	// Given an array of required items, returns all those queued items that are outside of it, including mismatching versions
	array<ref SCR_WorkshopItemActionDownload> GetUnrelatedDownloads(array<ref SCR_WorkshopItem> requiredItems)
	{
		array<ref SCR_WorkshopItemActionDownload> unrelatedDownloads = {};
	
		foreach (SCR_WorkshopItemActionDownload action : m_aDownloadQueue)
		{
			if (!IsDownloadingActionRequired(action, requiredItems))
				unrelatedDownloads.Insert(action);
		}
		
		return unrelatedDownloads;
	}
	
	//-----------------------------------------------------------------------------------------------
	// Given an array of required items, returns true if the input action's item is part of it and is the correct version
	bool IsDownloadingActionRequired(SCR_WorkshopItemActionDownload action, array<ref SCR_WorkshopItem> requiredItems)
	{
		SCR_WorkshopItem item = action.GetWorkshopItem();
		if (!item)
			return true;
		
		bool required;
		bool sameId;
		bool sameVersion;		
		
		foreach (SCR_WorkshopItem requiredItem : requiredItems)
		{	
			sameId = requiredItem.GetId() == item.GetId();
			sameVersion = requiredItem.GetDependency().GetVersion() == action.GetTargetRevision().GetVersion();
			required = sameId && sameVersion;

			if (required)
				return true;
		}
		
		return false;	
	}
	
	// --- Helper functions for generic download functionality ---
	
	//-----------------------------------------------------------------------------------------------
	//! Checks if we need to start a new download of some addon
	//! Compared to just checking if it's offline, it is more contextual
	//! Because it also checks current download
	static bool IsLatestDownloadRequired(SCR_WorkshopItem item)
	{
		if (!item.GetOffline())
			return true;
		
		Revision latestRevision = item.GetLatestRevision();
		Revision targetRevision;
		bool downloading, paused;
		float progress;
		item.GetDownloadState(downloading, paused, progress, targetRevision);
		
		if (!downloading)
		{
			return !Revision.AreEqual(item.GetCurrentLocalRevision(), latestRevision);
		}
		else
		{
			// We are already downloading this, but what versoin?
			if (Revision.AreEqual(item.GetLatestRevision(), targetRevision))
				return false; // Already downloading this version, no need to start a new download
			else
				return true;	// Downloading a different version, we must download another one
		}
	}
	
	
	//-----------------------------------------------------------------------------------------------
	static void SelectAddonsForLatestDownload(array<ref SCR_WorkshopItem> arrayIn, array<ref SCR_WorkshopItem> arrayOut)
	{
		foreach (SCR_WorkshopItem addon : arrayIn)
		{
			if (IsLatestDownloadRequired(addon))
				arrayOut.Insert(addon);
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Creates new download actions for downloading latest version of main item and all its dependencies.
	//! The actions are started if downloads are not paused in the download manager (GetDownloadsPaused())
	//! If the GetDownloadsPaused() is true, the actions are created but not activated.
	array<ref SCR_WorkshopItemAction> DownloadLatestWithDependencies(notnull SCR_WorkshopItem mainItem, bool downloadMainItem, array<ref SCR_WorkshopItem> dependencies)
	{
		array<ref SCR_WorkshopItemAction> actions = {};
		
		if (IsLatestDownloadRequired(mainItem) && downloadMainItem)
		{
			auto actionMain = mainItem.DownloadLatestVersion();
			if (actionMain)
			{
				actions.Insert(actionMain);
				if (!m_bDownloadsPaused)
					actionMain.Activate();
			}
		}
		
		if (dependencies)
		{
			if (dependencies.Count() > 0)
			{
				auto actionDependencies = mainItem.DownloadDependenciesLatest(dependencies);
				if (actionDependencies)
				{
					if (!m_bDownloadsPaused)
						actionDependencies.Activate();
					
					actions.Insert(actionDependencies);
				}
			}
		}
		
		return actions;
	}
	
	//-----------------------------------------------------------------------------------------------
	
	//-----------------------------------------------------------------------------------------------
	//! Start downloading list of scripted workshop items
	//! Mainly used for server browser content download
	array<ref SCR_WorkshopItemActionDownload> DownloadItems(array<ref SCR_WorkshopItem> items)
	{
		array<ref SCR_WorkshopItemActionDownload> actions = {};
		
		for (int i = 0, count = items.Count(); i < count; i++)
		{
			Revision target = items[i].GetItemTargetRevision();
			
			if (!target)
				target = items[i].GetDependency().GetRevision();
			
			if (!target)
			{
				return null;
			}
			
			// Create download action from dependency and depdency revision
			SCR_WorkshopItemActionDownload action = items[i].Download(target);
			actions.Insert(action);	
			action.Activate();
		}
		
		return actions; 
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Start downloading list of dependencies of given revision
	//! Mainly used for server browser content download
	void DownloadDependecies(array<Dependency> dependencies)
	{
		for (int i = 0, count = dependencies.Count(); i < count; i++)
		{
			SCR_WorkshopItem item = SCR_WorkshopItem.Internal_CreateFromDependency(dependencies[i]);
			
			// Create download action from dependency and depdency revision
			SCR_WorkshopItemActionDownload action = item.Download(dependencies[i].GetRevision());	
			action.Activate();
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	static float GetTotalSizeBytes(array<ref SCR_WorkshopItem> arrayIn, SCR_WorkshopItem extraItem = null)
	{
		float sizeOut = 0;
		
		foreach (SCR_WorkshopItem addon : arrayIn)
		{
			float s = addon.GetTargetRevisionPatchSize();
			sizeOut += s;
		}
		
		if (extraItem)
		{
			float s = extraItem.GetTargetRevisionPatchSize();
			sizeOut += s;
		}
		
		return sizeOut;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected float DownloadQueueSize()
	{
		array<ref SCR_WorkshopItem> downloading = {};
		for (int i = 0, count = m_aDownloadQueue.Count(); i < count; i++)
		{
			downloading.Insert(m_aDownloadQueue[i].m_Wrapper);
		}
		
		return GetTotalSizeBytes(downloading);
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Returns overall progress of all download actions, from 0 to 1
	static float GetDownloadActionsProgress(array<ref SCR_WorkshopItemActionDownload> actions)
	{
		float totalSizeBytes = 0;
		float totalBytesDownloaded = 0;
		
		foreach (SCR_WorkshopItemActionDownload dl : actions)
		{
			float dlsize = dl.GetSizeBytes();
			totalSizeBytes += dlsize;
			totalBytesDownloaded += dl.GetProgress() * dlsize;
		}
		
		if (totalSizeBytes == 0) // Don't divide by 0
			return 0;
		
		float progress = totalBytesDownloaded / totalSizeBytes;
		return progress;
	}

	//-----------------------------------------------------------------------------------------------
	//! Returns overall download progress for all actions of a workshop item
	static float GetItemDownloadActionsProgress(SCR_WorkshopItem item)
	{
		// Create an array of all download actions started from this tile
		// Get their aggregated progress
		
		SCR_WorkshopItemActionDownload actionThisItem = item.GetDownloadAction();					// Action for downloading this item
		SCR_WorkshopItemActionComposite actionDependencies = item.GetDependencyCompositeAction();	// Action for downloading dependencies
		
		array<ref SCR_WorkshopItemAction> allActions;
		if (actionDependencies)
			allActions = actionDependencies.GetActions();
		else
			allActions = new array<ref SCR_WorkshopItemAction>;
		
		if (actionThisItem)
			allActions.Insert(actionThisItem);
		
		// Cast all actions to download actions...
		auto allDownloadActions = new array<ref SCR_WorkshopItemActionDownload>;
		foreach (auto a : allActions)
		{
			auto downloadAction = SCR_WorkshopItemActionDownload.Cast(a);
			if (downloadAction)
				allDownloadActions.Insert(downloadAction);
		}
		
		float progress = SCR_DownloadManager.GetDownloadActionsProgress(allDownloadActions);
		
		return progress;
	}
	
	//-----------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItemActionDownload> GetFailedDownloads()
	{
		array<ref SCR_WorkshopItemActionDownload> actions = {};
		for (int i = 0, count = m_aFailedDownloads.Count(); i < count; i++)
		{
			actions.Insert(m_aFailedDownloads[i]);	
		}
		
		return actions;
	}
	
	//-----------------------------------------------------------------------------------------------
	void ClearFailedDownloads()
	{
		m_aFailedDownloads.Clear();
	}
	
	//-----------------------------------------------------------------------------------------------
	void AddDownloadManagerEntry(notnull SCR_WorkshopItem item, notnull SCR_WorkshopItemActionDownload action)
	{
		RemoveSameAddonFromDownloads(item);
		
		SCR_DownloadManager_Entry entry = new SCR_DownloadManager_Entry(item, action);
		m_aDownloadActions.Insert(entry);
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Remove previous download action if downloaded addon (item) has same ID as parameter item
	//! Use to prevent displaying same addon in downloading multiple times
	protected void RemoveSameAddonFromDownloads(notnull SCR_WorkshopItem item)
	{
		// Release download actions
		for (int i = 0, count = m_aDownloadActions.Count(); i < count; i++)
		{
			// Same addon ids?
			if (m_aDownloadActions[i].m_Item.GetId() == item.GetId())
			{
				m_aDownloadActions.Remove(i);
				break;
			}
		}
		
		// Release download queue 
		for (int i = 0, count = m_aDownloadQueue.Count(); i < count; i++)
		{
			// Same addon ids?
			if (m_aDownloadQueue[i].m_Wrapper.GetId() == item.GetId())
			{
				m_aDownloadQueue.Remove(i);
				break;
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	float GetDownloadQueueSize()
	{
		return m_fDownloadQueueSize;
	}
	
	//-----------------------------------------------------------------------------------------------
	float GetDownloadedSize()
	{
		return m_fDownloadedSize;
	}
	
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	
	
	
	
	

	
	
	//-----------------------------------------------------------------------------------------------
	// 					P R O T E C T E D   /   P R I V A T E
	//-----------------------------------------------------------------------------------------------
	
	
	
	//-----------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		int pausedCount = 0;
		
		// Remove failed or canceled actions from the download queue
		for (int i = m_aDownloadQueue.Count() - 1; i >= 0; i--)
		{
			SCR_WorkshopItemActionDownload action = m_aDownloadQueue[i];

			if (action.IsCanceled() || action.IsFailed())
				ClearUnfinishedAction(action);
			else if (action.IsPaused()) 
				pausedCount++;
		}
		
		// Restart no progress timer check
		if (pausedCount == m_aDownloadQueue.Count())
		{
			m_fNoDownloadProgressTimer = 0;
		}
		
		// If all are completed, clear the queue entirely
		// And reset 'all downloads paused' state.
		if (!m_aDownloadQueue.IsEmpty() && m_iQueueDownloadsCompleted == m_aDownloadQueue.Count())
		{
			m_aDownloadQueue.Clear();
			m_iQueueDownloadsCompleted = 0;
			m_fDownloadedSize = 0;
			m_bDownloadsPaused = false;

			if (m_OnDownloadQueueCompleted)
				m_OnDownloadQueueCompleted.Invoke();
			
			m_fNoDownloadProgressTimer = 0;
		}
		
		/*
		else 
		{
			// Increase time from last download progress
			m_fNoDownloadProgressTimer += timeSlice;
			
			// Quit downloads if nothing is progressing
			if (m_fNoDownloadProgressTimer >= DOWNLOAD_STUCK_DELAY)
			{
				// TODO: Display error if no progress for 1 min istead of force fail
			}
		}
		*/
	}
	
	//-----------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_AddonManager addonManager = SCR_AddonManager.GetInstance();
		
		if (!addonManager)
		{
			Print("SCR_DownloadManager_Entity: SCR_AddonManager was not found. It must be placed in the world for download manager to work.", LogLevel.ERROR);
			return;
		}
		
		addonManager.m_OnNewDownload.Insert(Callback_OnNewDownload);
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Gets called when a new download action is created.
	//! !!! Action creation doesn't mean start of a download. At this point the action is created but
	//! it is not activated yet.
	protected void Callback_OnNewDownload(SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	{
		#ifdef WORKSHOP_DEBUG
		Revision rev = action.GetTargetRevision();
		if (rev)
			_print(string.Format("Callback_OnNewDownloadStarted: %1, %2", item.GetName(), action.GetTargetRevision().GetVersion()));
		else
			_print(string.Format("Callback_OnNewDownloadStarted: %1", item.GetName()));
		#endif
		
		RemoveSameAddonFromDownloads(item);
		
		// Create a new entry
		SCR_DownloadManager_Entry entry = new SCR_DownloadManager_Entry(item, action);
		m_aDownloadActions.Insert(entry);
		
		// Add the action to the queue
		m_aDownloadQueue.Insert(action);
		m_fDownloadQueueSize = DownloadQueueSize();
		
		m_OnNewDownload.Invoke(item, action);
		
		action.GetOnDownloadProgress().Insert(OnDownloadProgress);
		action.m_OnCompleted.Insert(Callback_OnDownloadCompleted);
		action.m_OnFailed.Insert(Callback_OnFailed);
		action.GetOnFullStorageError().Insert(Callback_OnFullStorageError);
		action.m_OnCanceled.Insert(Callback_OnCanceled);
		
		m_fNoDownloadProgressTimer = 0;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnDownloadProgress(SCR_WorkshopItemActionDownload action, float progressSize)
	{
		m_fDownloadedSize += progressSize;
		
		// Restart progress timer
		m_fNoDownloadProgressTimer = 0;
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Called from aciton when a download is completed
	protected void Callback_OnDownloadCompleted(SCR_WorkshopItemActionDownload action)
	{	
		// Check user's settings if we need to enable the addon automatically
		// Ignore this if it was an update of an addon
		SCR_WorkshopSettings settings = SCR_WorkshopSettings.Get();
		if (settings.m_bAutoEnableDownloadedAddons && !action.GetUpdate())
		{	
			SCR_WorkshopItem item = action.GetWorkshopItem();
			
			if (!item)
				return;
			
			item.SetEnabled(true);
		}
		
		// Invoke for UI
		m_iQueueDownloadsCompleted++;
		m_OnDownloadComplete.Invoke(action);
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Call on downloading fail to show download manager dialog with problematic
	protected void Callback_OnFailed(SCR_WorkshopItemActionDownload action, int reason)
	{	
		m_aFailedDownloads.Insert(action);
		
		// TODO: hack, download manager should not open UI, but we lack a proper system to handle this better
		SCR_FailedModsDownloadDialog.ShowFailedModsDialog(action, reason);
		
		// Prevent error on full storage error
		if (reason == ERROR_FULL_STORAGE)
			return;
		
		// Generic error 
		m_OnDownloadFailed.Invoke(action, reason);
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Call on full addons storage reached to display storage limit reached error 
	protected void Callback_OnFullStorageError(SCR_WorkshopItemActionDownload action, float size)
	{	
		// TODO: hack, download manager should not open UI, but we lack a proper system to handle this better
		SCR_NotEnoughStorageDialog.ShowNotEnoughStorageDialog(size, action);
		
		if (m_OnFullStorageError)
			m_OnFullStorageError.Invoke(action, size);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_OnCanceled(SCR_WorkshopItemActionDownload action)
	{	
		m_aFailedDownloads.Insert(action);
		m_OnDownloadCanceled.Invoke(action);
	}
	
	//-----------------------------------------------------------------------------------------------
	//! End all running downloads as fail
	protected void ForceFailRunningDownloads()
	{
		for (int i = 0, count = m_aDownloadQueue.Count(); i < count; i++)
		{
			m_aDownloadQueue[i].ForceFail();
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Start downloading list of given addons
	void DownloadAddons(array<ref SCR_WorkshopItem> items)
	{
		SCR_DownloadSequence sequence = SCR_DownloadSequence.Create(items, null, true);
		sequence.GetOnReady().Insert(OnDownloadAddonsReady);
		sequence.Init();
		
		/*
		array<ref SCR_WorkshopItem> offline = SCR_AddonManager.GetInstance().GetOfflineAddons();
		array<SCR_WorkshopItem> unfinished = {};
		
		// Filter out unfinished 
		foreach (SCR_WorkshopItem item : offline)
		{
			bool inProgress, paused;
			int progress;
			Revision targetRevision;
			
			item.GetDownloadState(inProgress, paused, progress, targetRevision);
			Print("item: " + item.GetName());
			Print("in progress: " + inProgress);
			Print("----------------------------------");
			
			//Print("item: " + item.GetDownloadState());
			//if (offline.ge)
		}
		*/
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnDownloadAddonsReady(SCR_DownloadSequence sequence)
	{
		sequence.GetOnReady().Remove(OnDownloadAddonsReady);
		//sequence.
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void ClearUnfinishedAction(SCR_WorkshopItemActionDownload action)
	{
		m_aDownloadQueue.RemoveItem(action);
					
		// Remove canceled and failed from size count 
		m_fDownloadQueueSize = Math.Clamp(m_fDownloadQueueSize - action.GetDownloadSize(), 0, float.MAX);
		m_fDownloadedSize = Math.Clamp(m_fDownloadedSize - action.GetDownloadSize(), 0, float.MAX);
		
		if (m_OnAllDownloadsStopped && m_aDownloadQueue.IsEmpty())
			m_OnAllDownloadsStopped.Invoke();
	}
	
	//-----------------------------------------------------------------------------------------------
	private void SCR_DownloadManager(IEntitySource src, IEntity parent)
	{
		SetEventMask( EntityEvent.FRAME | EntityEvent.INIT);
		SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);
		
		s_Instance = this;
	}
	
	//-----------------------------------------------------------------------------------------------
	private void ~SCR_DownloadManager()
	{
		s_Instance = null;
	}
	
	
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------

	
	//------------------------------------------------------------------------------------------------
	void _print(string str, LogLevel logLevel = LogLevel.DEBUG)
	{
		Print(string.Format("[SCR_DownloadManager] %1 %2", this, str), logLevel);
	}	
};


//------------------------------------------------------------------------------------------------
//! Helper class to store current downloads and their attributes.
class SCR_DownloadManager_Entry
{
	ref SCR_WorkshopItem m_Item;					// We hold strong refs to both item and action!
	ref SCR_WorkshopItemActionDownload m_Action;
	
	//------------------------------------------------------------------------------------------------
	void SCR_DownloadManager_Entry(SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	{
		m_Item = item;
		m_Action = action;
	}
};

//------------------------------------------------------------------------------------------------
//! Enum describing current state of downloading action
enum EDownloadManagerActionState
{
	INACTIVE,
	RUNNING,
	FAILED,
	DOWNLOADED,
};
