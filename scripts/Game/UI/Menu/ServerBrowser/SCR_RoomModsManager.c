// Mods callback
//------------------------------------------------------------------------------------------------
class SCR_BackendCallbackRoomMods : BackendCallback
{
	protected Room m_Room;
	
	//-----------------------------------------------------------------------------------------------
	Room GetRoom()
	{
		return m_Room;
	}
	
	//-----------------------------------------------------------------------------------------------
	void SCR_BackendCallbackRoomMods(Room room)
	{
		m_Room = room;
	}
};

//-----------------------------------------------------------------------------------------------
void ScriptInvoker_RoomModsDependencies(array<ref SCR_WorkshopItem> dependencies);
typedef func ScriptInvoker_RoomModsDependencies;

//------------------------------------------------------------------------------------------------
//! This class will take care of handling all room mods & dependencies
//! Receiveing, sorting, downloading
class SCR_RoomModsManager
{	
	protected Room m_Room;
	protected Dependency m_ScenarioDependency;
	
	protected ref SCR_DownloadSequence m_DownloadSequence;
	
	protected bool m_bModsMatching = false;
	protected bool m_bBlockedMods;
	protected ref array<ref SCR_WorkshopItem> m_aCollidingDependencies = {};
	
	// Arrays 
	protected ref array<ref SCR_WorkshopItem> m_aRoomItemsScripted = {};
	
	protected ref array<ref SCR_WorkshopItem> m_aItemsUpdated = {};
	protected ref array<ref SCR_WorkshopItem> m_aItemsToUpdate = {};
	
	// Callbacks 
	protected ref SCR_BackendCallbackRoomMods m_ModsCallback;

	
	//------------------------------------------------------------------------------------------------
	// Invokers
	//------------------------------------------------------------------------------------------------
	
	protected ref ScriptInvokerRoom m_OnGetAllDependencies = new ScriptInvokerRoom();
	
	protected ref ScriptInvokerRoom m_OnGetScenario = new ScriptInvokerRoom();
	
	protected ref ScriptInvokerVoid m_OnModsFail;
	protected ref ScriptInvokerBase<ScriptInvoker_RoomModsDependencies> m_OnDependenciesLoadingPrevented;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerRoom GetOnGetAllDependencies()
	{
		return m_OnGetAllDependencies;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerRoom GetOnGetScenario()
	{
		return m_OnGetScenario;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnModsFail()
	{
		if (!m_OnModsFail)
			m_OnModsFail = new ScriptInvokerVoid();
		
		return m_OnModsFail;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_RoomModsDependencies> GetOnDependenciesLoadingPrevented()
	{
		if (!m_OnDependenciesLoadingPrevented)
			m_OnDependenciesLoadingPrevented = new ScriptInvokerBase<ScriptInvoker_RoomModsDependencies>();
		
		return m_OnDependenciesLoadingPrevented;
	}
	
	//------------------------------------------------------------------------------------------------
	// Public
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		// Default for loading mods

		m_bModsMatching = false;
		m_bBlockedMods = false;
		m_aCollidingDependencies.Clear();
		m_aRoomItemsScripted.Clear();
		m_aItemsUpdated.Clear();
		m_aItemsToUpdate.Clear();
		m_ModsCallback = null;
		m_DownloadSequence = null;
		m_ScenarioDependency = null;
		m_Room = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getting all mods for selected room 
	//! Checkign dependencies and loading workshop items of mods 
	void ReceiveRoomMods(notnull Room room)
	{
		if (room != m_Room)
			m_Room = room;
		
		array<Dependency> deps = {};  
		m_Room.GetItems(deps);
		
		m_bBlockedMods = false;
		m_aCollidingDependencies.Clear();
		
		// Load full list 
		m_ModsCallback = new SCR_BackendCallbackRoomMods(room);
		m_ModsCallback.SetOnSuccess(OnRoomCallbackResponse);
		m_ModsCallback.SetOnError(OnRoomCallbackError);
		m_Room.LoadDownloadList(m_ModsCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	// Try to receive scenario data without looking for mods 
	void ReceiveRoomScenario(notnull Room room)
	{
		if (room != m_Room)
		{
			Debug.Error("Received data for non active room");
			return;		
		}
		OnScenarioModLoaded();
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Invoke mods loading error 
	protected void OnLoadingModsFail(Room room)
	{
		// Don't notify fail if failed is not for current
		if (room != m_Room)
			return;
		
		if (m_OnModsFail)
			m_OnModsFail.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	// Protected
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Return only all updated dependecies from dependencies param
	//! Save all up to date to give out reference array 
	//! Save all up missing or outdate to out reference array 
	protected void UpdatedDependencies(notnull array<ref SCR_WorkshopItem> items)
	{	
		m_aItemsUpdated.Clear();
		m_aItemsToUpdate.Clear();
		
		m_aItemsUpdated = SCR_AddonManager.SelectItemsBasic(items, EWorkshopItemQuery.LOCAL_VERSION_MATCH_DEPENDENCY);
		m_aItemsToUpdate = SCR_AddonManager.SelectItemsBasic(items, EWorkshopItemQuery.NOT_LOCAL_VERSION_MATCH_DEPENDENCY);
	} 
	
	//------------------------------------------------------------------------------------------------
	protected void AllItemsLoaded()
	{
		if (!m_Room)
			return;
		
		UpdatedDependencies(m_aRoomItemsScripted);

		// Pass list of updated & outdated mods
		if (m_OnGetAllDependencies)
		{
			m_OnGetAllDependencies.Invoke(m_Room);
			
			m_bModsMatching = (m_aItemsToUpdate.IsEmpty());
		}
		
		ReceiveRoomScenario(m_Room);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when sceario is loaded 
	protected void LoadScenario()
	{
		if (!m_Room)
			return;
					
		m_ScenarioDependency = m_Room.HostScenarioMod();	
		OnScenarioDetails();
	}

	//------------------------------------------------------------------------------------------------
	//! Show restricted addons and return true if there are restricted addons in room content
	SCR_ReportedAddonsDialog DisplayRestrictedAddonsList()
	{
		/*
		if (m_DownloadSequence && m_DownloadSequence.GetRestrictedAddons())
			return m_DownloadSequence.ShowRestrictedDependenciesDialog();
		*/
		
		array<ref SCR_WorkshopItem> restrictedDependencies = SCR_AddonManager.SelectItemsBasic(m_aRoomItemsScripted, EWorkshopItemQuery.RESTRICTED);
		SCR_AddonListDialog addonsDialog = SCR_AddonListDialog.CreateRestrictedAddonsJoinServer(restrictedDependencies);
	
		// Handle cancel reports done 
		SCR_ReportedAddonsDialog reportedDialog = SCR_ReportedAddonsDialog.Cast(addonsDialog);
		if (reportedDialog)
			reportedDialog.GetOnAllReportsCanceled().Insert(OnAllReportsCanceled);
		
		return reportedDialog;
		
		// All can be downloaded
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call when all reports from dialog are cancled to clear invoker actions and display download dialog 
	protected void OnAllReportsCanceled(SCR_ReportedAddonsDialog dialog)
	{
		dialog.GetOnAllReportsCanceled().Remove(OnAllReportsCanceled);
		
		m_DownloadSequence.OnAllReportsCanceled(dialog);
	}
	
	//------------------------------------------------------------------------------------------------
	// Callbacks
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Call this when mod containing scenario is loaded  
	protected void OnScenarioModLoaded()
	{
		// Remove callback action
		LoadScenario();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when scenario has got complete info 
	protected void OnScenarioDetails()
	{
		m_OnGetScenario.Invoke(m_Room);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRoomCallbackResponse(SCR_BackendCallbackRoomMods callback)
	{
		// Load list success
		OnRoomFullList();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRoomCallbackError(SCR_BackendCallbackRoomMods callback)
	{
		// Fail/timeout
		OnLoadingModsFail(callback.GetRoom());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when room receive list of all mod dependencies 
	protected void OnRoomFullList()
	{
		if (!m_Room)
		{
			OnLoadingModsFail(null);
			return;
		}
		
		if (!m_Room.IsAuthorized())
		{
			OnLoadingModsFail(m_Room);
			return;
		}
		
		array<Dependency> deps = new array<Dependency>;
		m_Room.GetItems(deps);
		
		array<Dependency> depsItems = new array<Dependency>;
		
		// Clearup
		m_aRoomItemsScripted.Clear();
			
		// Skip loadig of content if none 
		if (deps.IsEmpty())
		{
			AllItemsLoaded();
			return;
		}
		
		SCR_AddonManager mngr = SCR_AddonManager.GetInstance();
		
		// Loaded dependencies as items
		foreach (Dependency dep : deps)
		{
			//ref SCR_WorkshopItem item = SCR_WorkshopItem.Internal_CreateFromDependency(dep);
			SCR_WorkshopItem item = mngr.Register(dep);
			item.SetItemTargetRevision(dep.GetRevision());
			m_aRoomItemsScripted.Insert(item);
		}
		
		// Clear previous download sequence reactions
		if (m_DownloadSequence)
		{
			m_DownloadSequence.GetOnReady().Remove(OnDownloadSequenceReady);
			m_DownloadSequence.GetOnError().Remove(OnDownloadSequenceError);
		}
		
		// Setup new download sequence
		InitDownloadSequence();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitDownloadSequence()
	{
		m_DownloadSequence = SCR_DownloadSequence.Create(m_aRoomItemsScripted, null, true);
		m_DownloadSequence.GetOnReady().Insert(OnDownloadSequenceReady);
		m_DownloadSequence.GetOnDependenciesLoadingPrevented().Insert(OnDependenciesLoadingPrevented);
		m_DownloadSequence.GetOnError().Insert(OnDownloadSequenceError);
		m_DownloadSequence.GetOnRestrictedDependency().Insert(OnDownloadSequenceRestrictedAddons);
		
		m_DownloadSequence.Init();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDownloadSequenceReady(SCR_DownloadSequence sequence)
	{
		AllItemsLoaded();
		
		// Cleanup
		sequence.GetOnReady().Remove(OnDownloadSequenceReady);
		sequence.GetOnDependenciesLoadingPrevented().Remove(OnDependenciesLoadingPrevented);
		sequence.GetOnError().Remove(OnDownloadSequenceError);
		sequence.GetOnRestrictedDependency().Remove(OnDownloadSequenceRestrictedAddons);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback when any of depeendencies are preventing loading or donwloading 
	//! Show dialog to cancel downloads blocking loading details
	protected void OnDependenciesLoadingPrevented(SCR_DownloadSequence sequence, array<ref SCR_WorkshopItem> dependencies)
	{	
		m_aCollidingDependencies = dependencies;
		
		if (m_OnDependenciesLoadingPrevented)
			m_OnDependenciesLoadingPrevented.Invoke(dependencies);
		
		// Cleanup
		sequence.GetOnDependenciesLoadingPrevented().Remove(OnDependenciesLoadingPrevented);
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItem> GetCollidingDependencies()
	{
		array<ref SCR_WorkshopItem> dependencies = {};
		
		foreach (SCR_WorkshopItem item : m_aCollidingDependencies)
		{
			dependencies.Insert(item);
		}
		
		return dependencies;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelDownloadConfirm(SCR_ConfigurableDialogUi dialog)
	{
		SCR_DownloadManager.GetInstance().EndAllDownloads();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelDownloadDialogClose(SCR_ConfigurableDialogUi dialog)
	{
		InitDownloadSequence();
		
		dialog.m_OnConfirm.Remove(OnCancelDownloadConfirm);
		dialog.m_OnClose.Remove(OnCancelDownloadDialogClose);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDownloadSequenceError(SCR_DownloadSequence sequence)
	{
		// Cleanup
		sequence.GetOnReady().Remove(OnDownloadSequenceReady);
		sequence.GetOnDependenciesLoadingPrevented().Remove(OnDependenciesLoadingPrevented);
		sequence.GetOnError().Remove(OnDownloadSequenceError);
		sequence.GetOnRestrictedDependency().Remove(OnDownloadSequenceRestrictedAddons);
		if (m_OnModsFail)
			m_OnModsFail.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDownloadSequenceRestrictedAddons(SCR_DownloadSequence sequence, array<ref SCR_WorkshopItem> items)
	{
		m_bBlockedMods = m_DownloadSequence.GetBlockedAddons();
	}
	
	//------------------------------------------------------------------------------------------------
	// Get set
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	string GetModListSizeString(array<ref SCR_WorkshopItem> items)
	{
		float size = GetModsSize(items); 

		if (size <= 0)
			return string.Empty;
		
		// Size string
		string str = SCR_ByteFormat.GetReadableSize(size);
		
		return str;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetModsSize(array<ref SCR_WorkshopItem> items) 
	{ 
		return SCR_ModHandlerLib.GetModListSize(items);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get size with unit based on pach sizes from addons list
	string GetModListPatchSizeString(array<ref SCR_WorkshopItem> items)
	{
		float size = GetModListPatchSize(items);

		if (size <= 0)
			return string.Empty;
		
		// Size string
		return SCR_ByteFormat.GetReadableSize(size);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetModListPatchSize(array<ref SCR_WorkshopItem> items) 
	{ 
		float size;
		for (int i = 0, count = items.Count(); i < count; i++)
		{
			size += items[i].GetTargetRevisionPatchSize();
		}
		
		return size;
	}
	
	//------------------------------------------------------------------------------------------------
	bool HasBlockedMods()
	{
		return m_bBlockedMods;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItem> GetRoomItemsScripted() 
	{ 
		return m_aRoomItemsScripted; 
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItem> GetRoomItemsUpdated()
	{ 
		return m_aItemsUpdated; 
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopItem> GetRoomItemsToUpdate() 
	{ 
		array<ref SCR_WorkshopItem> items = {};
		
		foreach (SCR_WorkshopItem item : m_aItemsToUpdate)
		{
			items.Insert(item);
		}
		
		return items; 
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetModsMatching() 
	{ 
		return m_bModsMatching; 
	}
};