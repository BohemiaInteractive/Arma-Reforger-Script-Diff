/*
Several classes for addon manager.

SCR_WorkshopItem

Wraps functionality of WorkshopItem and Dependency objects. The main purpose of this class is to
provide access to WorkshopItem from multiple menus at the same time. For everything to work correctly, perform all
actions with workshop items through this class, instead of accessing the WorkshopItem methods direcly.

To create a SCR_WorkshopItem object, use SCR_AddonManager.Register() methods.

Make sure you use strong refs (ref keyword) to store pointers to SCR_WorkshopItem object, because it might get garbage
collected by SCR_AddonManager if it is not needed by any other object.


SCR_AddonManager

Centralized storage of all SCR_WorkshopItem objects. It performs registration and updates of SCR_WorkshopItem objects.

It is an Entity and must be placed in the world to use it.


SCR_WorkshopItemAction

Base class for asynchronous operations of workshop items. Some methods of SCR_WorkshopItem return an action object,
which you can use to monitor the result of the operation, cancel/pause/resume it, or subscribe to its events. It's also the only way to know
if the action was canceled by something else.


!!! Methods marked with "Internal_" are not meant to be used publicly, although they are declared public.
*/

// Enum of bit flags used for doing queries
enum EWorkshopItemQuery
{
	ONLINE			= 1<<0,
	NOT_ONLINE		= 1<<1,

	OFFLINE			= 1<<2,
	NOT_OFFLINE		= 1<<3,

	ENABLED			= 1<<4,
	NOT_ENABLED		= 1<<5,

	BLOCKED			= 1<<6,
	REPORTED_BY_ME	= 1<<7,
	RESTRICTED 		= 1<<8,

	LOCAL_VERSION_MATCH_DEPENDENCY		= 1<<9,
	NOT_LOCAL_VERSION_MATCH_DEPENDENCY	= 1<<10,

	UPDATE_AVAILABLE			= 1<<11,

	DEPENDENCY_UPDATE_AVAILABLE	= 1<<12,

	NO_PROBLEMS		= 1<<13,

	DEPENDENCY_MISSING				= 1<<14,
	DEPENDENCY_NOT_MISSING			= 1<<15,
	ENABLED_AND_DEPENDENCY_DISABLED	= 1<<16,

	FAVOURITE		= 1<<17,
	
	AUTHOR_BLOCKED	= 1<<18,
	
	ONLY_WORKSHOP_ITEM =1<<19,
	ONLY_WORLD_SAVES = 1<<20, 
};

void ScriptInvokerSCRWorkshopItemMethod(WorkshopItem item);
typedef func ScriptInvokerSCRWorkshopItemMethod;
typedef ScriptInvokerBase<ScriptInvokerSCRWorkshopItemMethod> ScriptInvokerWorkshopItem;

[EntityEditorProps(category: "", description: "A centralized system which lets many users perform actions on addons. Most likely only needed in the main menu world.")]
class SCR_AddonManagerClass: GenericEntityClass
{
};

class SCR_AddonManager : GenericEntity
{
	// friend class SCR_WorkshopItem

	// Constants
	static const string ADDONS_CLI = "addons"; // Cli parameter used to load addons.
	static const string VERSION_DOT = ".";
	protected const static float ADDONS_ENABLED_UPDATE_INTERVAL_S = 1/30;
	protected const static float ADDONS_OUTDATED_UPDATE_INTERVAL_S = 1.0;
	
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// Old untyped invokers. This code is old and unmantained, and we also have a SCR_DonwloadManager class that seems to share some responsibilitites and that fires it's own invokers and some of them seem to happen on the same events as this one, making it unclear to which ones classes should listen to
	
	// Public callbacks
	ref ScriptInvoker m_OnAddonsChecked = new ScriptInvoker;

	// Called when a new addon is downloaded or uninstalled
	ref ScriptInvoker m_OnAddonOfflineStateChanged = new ScriptInvoker; // (SCR_WorkshopItem item, bool newState) - newState: true - addon downloaded, false - addon deleted

	// Called when reported state of addon is changed
	ref ScriptInvoker m_OnAddonReportedStateChanged = new ScriptInvoker; // (SCR_WorkshopItem item, bool newReported) - newReported: new reported value

	// Called as a result of NegotiateUgcPrivilege
	ref ScriptInvoker m_OnUgcPrivilegeResult = new ScriptInvoker;	// (bool result)

	// Called wherever set of enabled addons has changed
	ref ScriptInvoker m_OnAddonsEnabledChanged = new ScriptInvoker; //
	
	// Other
	protected ref array<WorkshopItem> m_aAddonsToRegister = {};
	protected ref map<string, ref SCR_WorkshopItem> m_mItems = new map<string, ref SCR_WorkshopItem>();
	protected static SCR_AddonManager s_Instance;		// Pointer to instance of this class
	protected ref SCR_WorkshopAddonManagerPresetStorage m_Storage;
	protected int m_iAddonsOutdated;
	protected float m_fAddonsOutdatedTimer = 0;
	protected float m_fAddonsEnabledTimer = 0;
	protected string m_sAddonsEnabledPrev;
	
	// Connected to WorkshopApi.OnItemsChecked, which automatically runs at start up.
	protected bool m_bAddonsChecked			= false;
	protected ref BackendCallback m_CallbackCheckAddons;
	protected ref SCR_ScriptPlatformRequestCallback m_CallbackGetPrivilege;
	protected bool m_bInitFinished 			= false;

	protected ref BackendCallback m_AddonCheckCallback = new BackendCallback();

	protected SCR_LoadingOverlay m_LoadingOverlay;
	
	protected ref ScriptInvoker<SCR_WorkshopItem, int> Event_OnAddonEnabled;
	protected ref ScriptInvoker<> Event_OnAllAddonsEnabled;
		
	// Called when a new download have been started
	ref ScriptInvoker m_OnNewDownload = new ScriptInvoker; // (SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	
	//---- REFACTOR NOTE END ----
	
	//-----------------------------------------------------------------------------------------------
	// 				P U B L I C   A P I
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	static SCR_AddonManager GetInstance()
	{
		return s_Instance;
	}


	//-----------------------------------------------------------------------------------------------
	//! Returns array with all addons currently in the system - online and offline.
	array<ref SCR_WorkshopItem> GetAllAddons()
	{
		array<ref SCR_WorkshopItem> ret = {};
		foreach (string id, SCR_WorkshopItem item : m_mItems)
		{
			ret.Insert(item);
		}
		return ret;
	}

	//-----------------------------------------------------------------------------------------------
	//! Returns array with all offline addons.
	array<ref SCR_WorkshopItem> GetOfflineAddons()
	{
		array<ref SCR_WorkshopItem> ret = {};
		foreach (string id, SCR_WorkshopItem item : m_mItems)
		{
			if (item.GetOffline())
				ret.Insert(item);
		}
		return ret;
	}

	//-----------------------------------------------------------------------------------------------
	//! Returns a SCR_WorkshopItem. If it's not registered, creates a new one and registers it.
	SCR_WorkshopItem Register(WorkshopItem item)
	{
		string id = GetItemId(item);
		SCR_WorkshopItem existingItem = m_mItems.Get(id);

		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Register WorkshopItem: ID: %1, Name: %2, Found in map: %3", id, item.Name(), existingItem != null));
		#endif

		if (existingItem)
		{
			existingItem.Internal_UpdateObjects(item, null);

			#ifdef WORKSHOP_DEBUG
			//existingItem.LogState();
			#endif

			return existingItem;
		}

		SCR_WorkshopItem newItem = SCR_WorkshopItem.Internal_CreateFromWorkshopItem(item);
		RegisterNewItem(id, newItem);

		#ifdef WORKSHOP_DEBUG
		newItem.LogState();
		#endif

		return newItem;
	}

	//-----------------------------------------------------------------------------------------------
	//! Returns a SCR_WorkshopItem. If it's not registered, creates a new one and registers it.
	SCR_WorkshopItem Register(Dependency item)
	{
		string id = GetItemId(item);
		SCR_WorkshopItem existingItem = m_mItems.Get(id);

		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Register Dependency: ID: %1, Name: %2, Found in map: %3", id, item.GetName(), existingItem != null));
		#endif

		if (existingItem)
		{
			existingItem.Internal_UpdateObjects(null, item);

			#ifdef WORKSHOP_DEBUG
			//existingItem.LogState();
			#endif

			return existingItem;
		}

		SCR_WorkshopItem newItem = SCR_WorkshopItem.Internal_CreateFromDependency(item);
		RegisterNewItem(id, newItem);

		#ifdef WORKSHOP_DEBUG
		newItem.LogState();
		#endif

		return newItem;
	}

	//-----------------------------------------------------------------------------------------------
	//! When true, we can fully use Workshop. All async checks are finished successfully.
	//! CheckAddons is not reliable and might never finish, so be careful with this.
	bool GetReady()
	{
		return m_bInitFinished && m_bAddonsChecked;
	}

	//-----------------------------------------------------------------------------------------------
	//! True when all async checks are done. Doesn't indicate that all checks were successful!
	//bool GetAllAsyncChecksDone() { return m_bAddonsChecked ; }

	//-----------------------------------------------------------------------------------------------
	bool GetAddonsChecked()
	{
		return m_bAddonsChecked;
	}

	//-----------------------------------------------------------------------------------------------
	//! Returns immediate value of UserPrivilege.USER_GEN_CONTENT
	bool GetUgcPrivilege()
	{
		return SocialComponent.IsPrivilegedTo(EUserInteraction.UserGeneratedContent);
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------------
	//! Returns count of outdated addons
	int GetCountAddonsOutdated()
	{
		return m_iAddonsOutdated;
	}

	//-----------------------------------------------------------------------------------------------
	//! Check user's workshop privilege (related to xbox profile)
	//! As a result on X-Box a system dialog will open and ask for confirmation.
	//! Subscribe to m_OnUgcPrivilegeResult to get the result.
	void NegotiateUgcPrivilegeAsync()
	{
		_print("CheckPrivilege()", LogLevel.NORMAL);

		if (!m_CallbackGetPrivilege)
		{
			m_CallbackGetPrivilege = new SCR_ScriptPlatformRequestCallback();
			m_CallbackGetPrivilege.m_OnResult.Insert(Callback_GetPrivilege_OnPrivilegeResult);
		}
		
		SocialComponent.RequestSocialPrivilege(EUserInteraction.UserGeneratedContent, m_CallbackGetPrivilege);
	}

	// Handling of addons loaded through external configuration
	//-----------------------------------------------------------------------------------------------
	//! Returns true when external addon configuration is used (through CLI or other means)
	protected static bool GetAddonsEnabledExternally()
	{
		return System.IsCLIParam(ADDONS_CLI);
	}

	//-----------------------------------------------------------------------------------------------
	//! Returns true if addon was enabled through external configuration (CLI or other).
	static bool GetAddonEnabledExternally(SCR_WorkshopItem item)
	{
		if (!GetAddonsEnabledExternally())
			return false;

		return item.GetLoaded();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Selects items which match query. Only one flag is tested!
	//! EWorkshopItemQuery.OFFLINE - will return items which are offline AND enabled.
	static array<ref SCR_WorkshopItem> SelectItemsBasic(array<ref SCR_WorkshopItem> items, EWorkshopItemQuery query)
	{
		array<ref SCR_WorkshopItem> ret = {};
		foreach (SCR_WorkshopItem item : items)
		{
			if (CheckQueryFlag(item, query))
				ret.Insert(item);
		}

		return ret;
	}

	//-----------------------------------------------------------------------------------------------
	//! Counts items which match query. Only one flag is tested!
	//! EWorkshopItemQuery.OFFLINE - will count items which are offline AND enabled.
	static int CountItemsBasic(array<ref SCR_WorkshopItem> items, EWorkshopItemQuery query, bool returnOnFirstMatch = false)
	{
		int count = 0;
		foreach (SCR_WorkshopItem item : items)
		{
			if (CheckQueryFlag(item, query))
			{
				count++;
				if (returnOnFirstMatch)
					return count;
			}
		}

		return count;
	}

	//-----------------------------------------------------------------------------------------------
	//! Selects items which match query. All flags must match.
	//! EWorkshopItemQuery.OFFLINE | EWorkshopItemQuery.ENABLED - will return items which are offline AND enabled.
	static array<ref SCR_WorkshopItem> SelectItemsAnd(array<ref SCR_WorkshopItem> items, EWorkshopItemQuery query)
	{
		array<ref SCR_WorkshopItem> ret = {};
		foreach (SCR_WorkshopItem item : items)
		{
			if (CheckQueryFlagsAnd(item, query))
				ret.Insert(item);
		}

		return ret;
	}


	//-----------------------------------------------------------------------------------------------
	//! Counts items which match query. All flags must match.
	static int CountItemsAnd(array<ref SCR_WorkshopItem> items, EWorkshopItemQuery query, bool returnOnFirstMatch = false)
	{
		int count = 0;
		foreach (SCR_WorkshopItem item : items)
		{
			if (CheckQueryFlagsAnd(item, query))
			{
				count++;
				if (returnOnFirstMatch)
					return count;
			}
		}

		return count;
	}

	//-----------------------------------------------------------------------------------------------
	//! Selects items which match query. Any flag must match.
	//! EWorkshopItemQuery.OFFLINE | EWorkshopItemQuery.ENABLED - will return items which are offline OR enabled.
	static array<ref SCR_WorkshopItem> SelectItemsOr(array<ref SCR_WorkshopItem> items, EWorkshopItemQuery query)
	{
		array<ref SCR_WorkshopItem> ret = {};
		foreach (SCR_WorkshopItem item : items)
		{
			if (CheckQueryFlagsOr(item, query))
				ret.Insert(item);
		}

		return ret;
	}

	//-----------------------------------------------------------------------------------------------
	//! Counts items which match query.Any flag must match.
	static int CountItemsOr(array<ref SCR_WorkshopItem> items, EWorkshopItemQuery query, bool returnOnFirstMatch = false)
	{
		int count = 0;
		foreach (SCR_WorkshopItem item : items)
		{
			if (CheckQueryFlagsOr(item, query))
			{
				count++;
				if (returnOnFirstMatch)
					return count;
			}
		}

		return count;
	}

	//-----------------------------------------------------------------------------------------------
	//! Return difference type between current version from current to target
	//OBSOLETE => use Revision.CompareTo
	static SCR_ComparerOperator DifferenceBetweenVersions(string vFrom, string vTo)
	{
		// Get versions numbers
		array<string> fromNums = {};
		vFrom.Split(VERSION_DOT, fromNums, false);

		array<string> toNums = {};
		vTo.Split(VERSION_DOT, toNums, false);

		// Setup count by smaller array
		int count = fromNums.Count();
		if (count > toNums.Count())
			count = toNums.Count();

		for (int i; i < count; i++)
		{
			int iFrom = fromNums[i].ToInt();
			int iTo = toNums[i].ToInt();

			// Compare version
			if (iFrom < iTo)
			{
				// Current is old
				return SCR_ComparerOperator.LESS_THAN;
			}
			else if (iFrom > iTo)
			{
				 // Current is newer
				return SCR_ComparerOperator.GREATER_THAN;
			}
			else if (iFrom == iTo)
			{
				// Versions are same
				int end = vTo.IndexOfFrom(i, VERSION_DOT) - 1;

				if (end == -1)
					return SCR_ComparerOperator.EQUAL;
			}
		}

		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Compare availability state for item current revision 
	//! Returns state base on if current and latest revision is avaiable in workshop
	static SCR_ERevisionAvailability ItemAvailability(notnull WorkshopItem item)
	{
		Revision currentRev = item.GetActiveRevision();
		if (!currentRev)
		{
			// Downloading was not finished 
			if (item.GetDownloadingRevision())
				return SCR_ERevisionAvailability.ERA_DOWNLOAD_NOT_FINISHED;
			
			return SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY;
		}
		
		ERevisionAvailability currentAvailability = currentRev.GetAvailability();
		
		// Addon is ok 
		if (currentAvailability == ERevisionAvailability.ERA_AVAILABLE)
			return SCR_ERevisionAvailability.ERA_AVAILABLE;
		
		Revision latestRev = item.GetLatestRevision();
		ERevisionAvailability latestAvailability = latestRev.GetAvailability();
		
		if (!latestRev)
		{
			Print("ItemAvailability() - Can't compare availability of item " + item.Name() + " latest revision", LogLevel.WARNING);
			return SCR_ERevisionAvailability.ERA_DELETED;
		}
		
		// Deleted 
		if (currentAvailability == ERevisionAvailability.ERA_DELETED)
		{
			// Update is available 
			if (latestAvailability == ERevisionAvailability.ERA_AVAILABLE)
				return SCR_ERevisionAvailability.ERA_COMPATIBLE_UPDATE_AVAILABLE;
			
			return SCR_ERevisionAvailability.ERA_DELETED;
		}
		
		// Current addon is obsolete (not compatible with client version)
		if (currentAvailability == ERevisionAvailability.ERA_OBSOLETE)
		{
			// Update is available 
			if (latestAvailability == ERevisionAvailability.ERA_AVAILABLE)
				return SCR_ERevisionAvailability.ERA_COMPATIBLE_UPDATE_AVAILABLE;
			
			// No compatible update available
			return SCR_ERevisionAvailability.ERA_OBSOLETE;
		}
		
		return SCR_ERevisionAvailability.ERA_AVAILABLE;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnAddonEnabled(SCR_WorkshopItem arg0, int arg1)
	{
		if (Event_OnAddonEnabled)
			Event_OnAddonEnabled.Invoke(arg0, arg1);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnAddonEnabled()
	{
		if (!Event_OnAddonEnabled)
			Event_OnAddonEnabled = new ScriptInvoker();

		return Event_OnAddonEnabled;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnAllAddonsEnabled()
	{
		if (Event_OnAllAddonsEnabled)
			Event_OnAllAddonsEnabled.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnAllAddonsEnabled()
	{
		if (!Event_OnAllAddonsEnabled)
			Event_OnAllAddonsEnabled = new ScriptInvoker();

		return Event_OnAllAddonsEnabled;
	}
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// addon manager system doing UI
	
	//-----------------------------------------------------------------------------------------------
	//! Enable/disable multiple mods recursively to save performance 
	void EnableMultipleAddons(array<ref SCR_WorkshopItem> items, bool enable)
	{
		int count = items.Count() - 1;
		
		GetGame().GetCallqueue().CallLater(EnableAddonsRecursively, 0, false, items, count, enable);
		
		//m_LoadingOverlay = SCR_LoadingOverlayDialog.Create();
		if (!m_LoadingOverlay)
			m_LoadingOverlay = SCR_LoadingOverlay.ShowForWidget(GetGame().GetWorkspace(), string.Empty);
		else
			m_LoadingOverlay.SetShown(true);
	}
	
//---- REFACTOR NOTE END ----
	
	//-----------------------------------------------------------------------------------------------
	protected void EnableAddonsRecursively(array<ref SCR_WorkshopItem> addons, out int remaining, bool enable)
	{
		if (remaining > -1)
		{
			addons[remaining].SetEnabled(enable);
			InvokeEventOnAddonEnabled(addons[remaining], remaining);
			remaining--;
			
			GetGame().GetCallqueue().CallLater(EnableAddonsRecursively, 0, false, addons, remaining, enable);
		}
		else
		{
			if (m_LoadingOverlay)
			{
				m_LoadingOverlay.SetShown(false);
				InvokeEventOnAllAddonsEnabled();
			}
			else 
			{
				InvokeEventOnAllAddonsEnabled();
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	// 					P R O T E C T E D   /   P R I V A T E
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//! When internet is disabled, might take a lot of time to complete the request.
	protected void Internal_CheckAddons()
	{
		_print("Internal_CheckAddons()", LogLevel.NORMAL);

		
		if (!m_AddonCheckCallback)
			m_AddonCheckCallback = new BackendCallback();
		
		WorkshopApi api = GetGame().GetBackendApi().GetWorkshop();

		if (api)
		{
			m_AddonCheckCallback.SetOnSuccess(AddonCheckResponse);
			m_AddonCheckCallback.SetOnError(AddonCheckError);
			api.OnItemsChecked(m_AddonCheckCallback);
		}
	}

	//-----------------------------------------------------------------------------------------------
	protected void RegisterNewItem(string id, SCR_WorkshopItem itemWrapper)
	{
		#ifdef WORKSHOP_DEBUG
		_print(string.Format("Registered new item: %1", itemWrapper.GetName()));
		#endif

		itemWrapper.m_OnOfflineStateChanged.Insert(Callback_OnAddonOfflineStateChanged);
		itemWrapper.m_OnReportStateChanged.Insert(Callback_OnAddonReportStateChanged);
		itemWrapper.m_OnDownloadComplete.Insert(CountOutdatedAddons);
		//

		m_mItems.Insert(id, itemWrapper);
		
		CountOutdatedAddons();
	}

	//-----------------------------------------------------------------------------------------------
	protected string GetItemId(WorkshopItem item)
	{
		return item.Id();
	}

	//-----------------------------------------------------------------------------------------------
	protected string GetItemId(Dependency item)
	{
		return item.GetID();
	}

	//-----------------------------------------------------------------------------------------------
	protected static bool CheckQueryFlag(SCR_WorkshopItem item, EWorkshopItemQuery flag)
	{
		switch(flag)
		{
			case EWorkshopItemQuery.ONLINE:								return item.GetOnline();
			case EWorkshopItemQuery.NOT_ONLINE: 						return !item.GetOnline();
			case EWorkshopItemQuery.OFFLINE: 							return item.GetOffline();
			case EWorkshopItemQuery.NOT_OFFLINE: 						return !item.GetOffline();
			case EWorkshopItemQuery.ENABLED: 							return item.GetEnabled();
			case EWorkshopItemQuery.NOT_ENABLED: 						return !item.GetEnabled();
			case EWorkshopItemQuery.BLOCKED: 							return item.GetBlocked();
			case EWorkshopItemQuery.RESTRICTED:							return item.GetRestricted();
			case EWorkshopItemQuery.REPORTED_BY_ME: 					return item.GetReportedByMe();
			case EWorkshopItemQuery.LOCAL_VERSION_MATCH_DEPENDENCY:		return item.GetCurrentLocalVersionMatchDependency();
			case EWorkshopItemQuery.NOT_LOCAL_VERSION_MATCH_DEPENDENCY:	return !item.GetCurrentLocalVersionMatchDependency();
			case EWorkshopItemQuery.UPDATE_AVAILABLE:					return item.GetUpdateAvailable();
			case EWorkshopItemQuery.DEPENDENCY_UPDATE_AVAILABLE:		return item.GetAnyDependencyUpdateAvailable();
			case EWorkshopItemQuery.NO_PROBLEMS:						return item.GetHighestPriorityProblem() == EWorkshopItemProblem.NO_PROBLEM;
			case EWorkshopItemQuery.DEPENDENCY_MISSING:					return item.GetAnyDependencyMissing();
			case EWorkshopItemQuery.DEPENDENCY_NOT_MISSING:				return !item.GetAnyDependencyMissing();
			case EWorkshopItemQuery.ENABLED_AND_DEPENDENCY_DISABLED:	return item.GetEnabledAndAnyDependencyDisabled();
			case EWorkshopItemQuery.FAVOURITE:							return item.GetFavourite();
			case EWorkshopItemQuery.AUTHOR_BLOCKED:						return item.GetModAuthorReportedByMe();
			case EWorkshopItemQuery.ONLY_WORKSHOP_ITEM:					return !item.IsWorldSave();
			case EWorkshopItemQuery.ONLY_WORLD_SAVES:					return item.IsWorldSave();
			default: return false;
		}
		return false;
	}

	//-----------------------------------------------------------------------------------------------
	//! Checks if all flags are satisfied
	protected static bool CheckQueryFlagsAnd(SCR_WorkshopItem item, EWorkshopItemQuery flags)
	{
		// Could optimize to `if ((~item.m_Flags & flags) == 0)`, if we had these flags easily available.

		int currentFlag = 1;
		for (int bit = 0; bit < 32; bit++)
		{
			if (flags & currentFlag)	// If this flag is set in query
			{
				if (!CheckQueryFlag(item, currentFlag))	// False if any is false
					return false;
			}
			currentFlag = currentFlag << 1;
		}
		return true;	// True when all set flags are true
	}

	//-----------------------------------------------------------------------------------------------
	//! Checks if any flag is satisfied
	protected static bool CheckQueryFlagsOr(SCR_WorkshopItem item, EWorkshopItemQuery flags)
	{
		int currentFlag = 1;
		for (int bit = 0; bit < 32; bit++)
		{
			if (flags & currentFlag)	// If this flag is set in query
			{
				if (CheckQueryFlag(item, currentFlag))	// False if any is false
					return true;
			}
			currentFlag = currentFlag << 1;
		}
		return false;	// False when no conditions are true
	}

	//-----------------------------------------------------------------------------------------------
	private void SCR_AddonManager(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.FRAME | EntityEvent.INIT);
		SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);

		s_Instance = this;
	}

	//-----------------------------------------------------------------------------------------------
	private void ~SCR_AddonManager()
	{
		s_Instance = null;
	}

	//-----------------------------------------------------------------------------------------------
	//! Finishes init after all async checks are done
	protected void FinalizeInitAfterAsyncChecks()
	{
		if (m_bInitFinished || !m_bAddonsChecked )
			return;

		_print("FinalizeInitAfterAsyncChecks()", LogLevel.NORMAL);

		m_bInitFinished = true;

		_print("Init Finished", LogLevel.NORMAL);
		_print(string.Format("  User Workshop access:           %1", GetReady()), LogLevel.NORMAL);
	}
	
	//-----------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		// Don't perform scan if not in actual game
		if (SCR_Global.IsEditMode(this))
			return;

		// Scan all offline items and register them
		// Scan offline items if needed
		WorkshopApi api = GetGame().GetBackendApi().GetWorkshop();
		if (api)
		{
			Internal_CheckAddons(); // sets callback for finished scan
			api.ScanOfflineItems(); // request scan of already downloaded addons
		}

		// If user's UGC privilege is disabled, disable all addons
		// If UGC is blocked, disable addons which are already downloaded
		if (!GetUgcPrivilege())
		{
			foreach (SCR_WorkshopItem item : GetOfflineAddons())
				item.SetEnabled(false);
		}
		
		
		
		if(GetGame().InPlayMode())
			m_Storage = new SCR_WorkshopAddonManagerPresetStorage();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddonCheckResponse(BackendCallback callback)
	{		
		// Success
		Callback_CheckAddons_OnSuccess();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddonCheckError(BackendCallback callback)
	{
		// TODO: Notify error!
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void Callback_CheckAddons_OnSuccess()
	{
		m_bAddonsChecked = true;
		_print("Callback_CheckAddons_OnSuccess()", LogLevel.NORMAL);
		
		WorkshopApi api = GetGame().GetBackendApi().GetWorkshop();
		
		// Get all offline addons and register them
		array<WorkshopItem> items = {};
		api.GetOfflineItems(items);
		
		// Prepare items to registration
		foreach (WorkshopItem item : items)
		{
			m_aAddonsToRegister.Insert(item);
		}

		array<ref SCR_WorkshopItem> pendingDownloads = {};
		
		// Update state of items after the check is complete
		// This will make the addons instantly update list of dependencies and revisions
		for (int i = 0; i < m_aAddonsToRegister.Count(); i++)
		{
			SCR_WorkshopItem scrItem = Register(m_aAddonsToRegister[i]);
			scrItem.Internal_OnAddonsChecked();
			array<ref SCR_WorkshopItem> dependencies = scrItem.GetLoadedDependencies();
			
			if (m_aAddonsToRegister[i].GetPendingDownload())
				pendingDownloads.Insert(scrItem);
		}
		
		m_aAddonsToRegister.Clear();
		
		FinalizeInitAfterAsyncChecks();
		SCR_DownloadManager.GetInstance().DownloadItems(pendingDownloads);
		
		// do not automatically start download without users knowledge
		// addons will be added to download dialog as paused download
		foreach (SCR_WorkshopItem download : pendingDownloads)
			download.PauseDownload();
		
		m_OnAddonsChecked.Invoke();
		
		//show reload failure
		array<string> addonIds = {};
		GetGame().ReloadFailureAddons(addonIds);
		array<ref SCR_WorkshopItem> arr = {};
		foreach(auto addon: addonIds)
		{
			SCR_WorkshopItem wi = GetItem(addon);
			if (wi)
				arr.Insert(wi);
		}
		
		if (!arr.IsEmpty())
			SCR_AddonListDialog.CreateFailedToStartWithMods(arr);
	}

	//-----------------------------------------------------------------------------------------------
	protected void Callback_GetPrivilege_OnPrivilegeResult(UserPrivilege privilege, UserPrivilegeResult result)
	{
		_print("Callback_GetPrivilege_OnPrivilegeResult()", LogLevel.NORMAL);

		if (privilege == UserPrivilege.USER_GEN_CONTENT)
		{
			bool allowed = result == UserPrivilegeResult.ALLOWED;

			_print(string.Format("  UserPrivilege.USER_GEN_CONTENT: %1", allowed), LogLevel.NORMAL);

			m_OnUgcPrivilegeResult.Invoke(allowed);
		}
	}


	//-----------------------------------------------------------------------------------------------
	//! Called from the specific SCR_WorkshopItem
	protected void Callback_OnAddonOfflineStateChanged(SCR_WorkshopItem item, bool newState)
	{
		m_OnAddonOfflineStateChanged.Invoke(item, newState);
	}

	//-----------------------------------------------------------------------------------------------
	//! Called from the specific SCR_WorkshopItem
	protected void Callback_OnAddonReportStateChanged(SCR_WorkshopItem item, bool newReport)
	{
		m_OnAddonReportedStateChanged.Invoke(item, newReport);
	}
	
	//-----------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		array<string> unregisterIds;
		array<SCR_WorkshopItem> updateItems;

		//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
		// Reliance on reference counting
		
		// Iterate all items
		// Existins utems are updated
		// Non-existant items are marked for deletion from the map
		foreach (string id, SCR_WorkshopItem item : m_mItems)
		{
			// Item can be unregistered when no actions are running
			// And when the addon manager holds the only reference to it
			int refCount = item.GetRefCount();
			bool canBeUnregistered = item.Internal_GetCanBeUnregistered();
			if (canBeUnregistered && item.GetRefCount() == 2) // SCR_WorkshopItem item above also holds one reference! The other reference is the map entry.
			{
				if (!unregisterIds)
					unregisterIds = {};

				unregisterIds.Insert(id);
			}
			else
			{
				if (!updateItems)
					updateItems = {};

				// Don't update items directly in case an update causes an insertion of a new item
				updateItems.Insert(item);
			}
		}

		//---- REFACTOR NOTE END ----
		
		// Update the marked entries
		if (updateItems)
		{
			foreach (SCR_WorkshopItem item : updateItems)
				item.Internal_Update(timeSlice);
		}

		// Delete the marked entries
		if (unregisterIds)
		{
			#ifdef WORKSHOP_DEBUG
			_print(string.Format("Unregistering items: %1", unregisterIds.Count()));
			#endif

			foreach (string id : unregisterIds)
			{
				#ifdef WORKSHOP_DEBUG
				SCR_WorkshopItem item = m_mItems.Get(id);
				_print(string.Format("Unregistered item: %1", item.GetName()));
				#endif

				m_mItems.Remove(id);
			}
		}
		
		// Detection of case when addons get enabled/disabled
		// Iterate all addons and invoke event whenever current list of enabled addons is different from prev. list
		// It's not a perfect implementation but it's fine in main menu
		m_fAddonsEnabledTimer += timeSlice;
		if (m_fAddonsEnabledTimer > ADDONS_ENABLED_UPDATE_INTERVAL_S)
		{
			string addonsEnabled;
			foreach (string id, SCR_WorkshopItem item : m_mItems)
			{
				if (item.GetEnabled())
					addonsEnabled = addonsEnabled + id + " ";
			}
			
			if (addonsEnabled != m_sAddonsEnabledPrev)
			{
				m_OnAddonsEnabledChanged.Invoke();
				m_sAddonsEnabledPrev = addonsEnabled;
			}
			
			m_fAddonsEnabledTimer = 0;
		}
		
		// Count and cache amount of outdated addons
		// This doesn't need to happen often, 1s is enough
			
		/*
		m_fAddonsOutdatedTimer += timeSlice;
		if (m_fAddonsOutdatedTimer > ADDONS_OUTDATED_UPDATE_INTERVAL_S)
		{
			m_iAddonsOutdated = CountItemsBasic(GetOfflineAddons(), EWorkshopItemQuery.UPDATE_AVAILABLE);
				
			m_fAddonsOutdatedTimer = 0;
		}
		*/
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// Public method called "Internal" because we don't allow friend classes: now SCR_WorkshopItems and SCR_AddonManager both call public methods on each other, hold references and duplicate cached data, and have bloated public APIs
	
	//-----------------------------------------------------------------------------------------------
	//! Called by SCR_WorkshopItem when it starts a new download
	void Internal_OnNewDownload(SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	{
		m_OnNewDownload.Invoke(item, action);
	}
	
	//---- REFACTOR NOTE END ----
	
	//-----------------------------------------------------------------------------------------------
	//! Go throught all offline addons and count size
	protected void CountOutdatedAddons()
	{	
		m_iAddonsOutdated = 0;
		array<ref SCR_WorkshopItem> items = GetOfflineAddons();
		
		foreach (SCR_WorkshopItem item : items)
		{
			auto wi = item.Internal_GetWorkshopItem();
			if (!wi || (wi.GetLatestRevision() && !wi.GetLatestRevision().IsDownloaded()))
				m_iAddonsOutdated++;
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	// Handling of addon presets
	//----------------------------------------------------------------------------------------------------------------------------------------------------
	SCR_WorkshopAddonManagerPresetStorage GetPresetStorage()
	{
		return m_Storage;
	}
	
	
	//----------------------------------------------------------------------------------------------------------------------------------------------------
	// Looks up addon from internal map
	SCR_WorkshopItem GetItem(string id)
	{
		return m_mItems.Get(id);
	}
	
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// Declare variables at the top of the class
	
	protected SCR_WorkshopAddonPreset m_SelectedPreset;
	protected ref array<ref SCR_WorkshopAddonPresetAddonMeta> m_aAddonsNotFound = {};
	
	//---- REFACTOR NOTE END ----
	
	//----------------------------------------------------------------------------------------------------------------------------------------------------
	// Enables addons from preset, disables all addons from other presets
	void SelectPreset(SCR_WorkshopAddonPreset preset, notnull array<ref SCR_WorkshopAddonPresetAddonMeta> addonsNotFound)
	{	
		array<ref SCR_WorkshopItem> addons = GetOfflineAddons();
		GetEventOnAllAddonsEnabled().Insert(OnOfflineAddonsDisabled);
		m_SelectedPreset = preset;
		EnableMultipleAddons(addons, false);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnOfflineAddonsDisabled()
	{
		GetEventOnAllAddonsEnabled().Remove(OnOfflineAddonsDisabled);
		
		array<ref SCR_WorkshopAddonPresetAddonMeta> enabledAddons = m_SelectedPreset.GetAddons();
		array<ref SCR_WorkshopItem> addons = {};
		
		foreach (SCR_WorkshopAddonPresetAddonMeta meta : enabledAddons)
		{
			string guid = meta.GetGuid();
			SCR_WorkshopItem item = GetItem(guid);
		
			if (item && item.GetOffline())
				addons.Insert(item);
			else 
				m_aAddonsNotFound.Insert(meta);
		}
		
		GetEventOnAllAddonsEnabled().Insert(OnAllAddonsEnabledCorrupted);
		EnableMultipleAddons(addons, true);
		m_SelectedPreset = null;
	}
	
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// AddonManager system doing UI stuff
	
	//-----------------------------------------------------------------------------------------------
	//! Call this when all dialog are enabled, but some are missing to show which one
	protected void OnAllAddonsEnabledCorrupted()
	{
		if (!m_aAddonsNotFound.IsEmpty())
		{
			new SCR_WorkshopErrorPresetLoadDialog(m_aAddonsNotFound);
			m_aAddonsNotFound.Clear();
		}	
		
		GetEventOnAllAddonsEnabled().Remove(OnAllAddonsEnabledCorrupted);
	}
	
	//---- REFACTOR NOTE END ----
	
	//----------------------------------------------------------------------------------------------------------------------------------------------------
	SCR_WorkshopAddonPreset CreatePresetFromEnabledAddons(string presetName)
	{		
		// Get GUIDs of addons which are enabled
		
		array<ref SCR_WorkshopItem> enabledAddons = SCR_AddonManager.SelectItemsBasic(GetOfflineAddons(), EWorkshopItemQuery.ENABLED);
		
		array<ref SCR_WorkshopAddonPresetAddonMeta> addonsMeta = {};
		foreach (SCR_WorkshopItem item : enabledAddons)
		{
			string guid = item.GetId();			
			addonsMeta.Insert((new SCR_WorkshopAddonPresetAddonMeta()).Init(guid, item.GetName()) );
		}
		
		if (addonsMeta.IsEmpty())
			return null;
		
		SCR_WorkshopAddonPreset preset = (new SCR_WorkshopAddonPreset()).Init(presetName, addonsMeta);
		
		return preset;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------------
	//! Return int count of all enabled mods
	int CountOfEnabledAddons()
	{
		array<ref SCR_WorkshopItem> enabledAddons = SCR_AddonManager.SelectItemsAnd(GetOfflineAddons(), EWorkshopItemQuery.ENABLED | EWorkshopItemQuery.ONLY_WORKSHOP_ITEM);
		return enabledAddons.Count();
	} 

	//-----------------------------------------------------------------------------------------------
	map<string, ref SCR_WorkshopItem> GetItemsMap()
	{
		return m_mItems;
	}
	
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------


	//------------------------------------------------------------------------------------------------
	void _print(string str, LogLevel logLevel = LogLevel.DEBUG)
	{
		Print(string.Format("[SCR_AddonManager] %1 %2", this, str), logLevel);
	}
}

enum SCR_ERevisionAvailability : ERevisionAvailability
{
	ERA_COMPATIBLE_UPDATE_AVAILABLE,
	ERA_DOWNLOAD_NOT_FINISHED,
	ERA_UNKNOWN_AVAILABILITY,
}