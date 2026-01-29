//! This class may become obsolete on BackendAPI update
enum SCR_ECommStatus
{
	NOT_EXECUTED = 0,	// No backend
	RUNNING,			// Still pinging
	FINISHED,			// Success
	FAILED				// Lost connection
}

void ScriptInvokerCommStatusMethod(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime);
typedef func ScriptInvokerCommStatusMethod;
typedef ScriptInvokerBase<ScriptInvokerCommStatusMethod> ScriptInvokerCommStatus;

//------------------------------------------------------------------------------------------------
class SCR_ServicesStatusHelper
{
	protected static int s_iLastPingUpdate = -1;
	protected static int s_iLastStatusUpdate = -1;
	protected static ServiceStatusItem s_MainStatus;
	protected static const ref array<ServiceStatusItem> SERVICE_STATUSES = {};
	
	// Timers
	protected static const int COMM_STATUS_CHECK_FREQUENCY_MS = 500;	// frequency of checking for comm test response
	protected static const int FIRST_REFRESH_DELAY = 1000;				// wait a bit for the very first refresh
	
	static const int REFRESH_COOLDOWN = 10000;							// minimum ping age to allow for refreshing
	static const int CONNECTION_CHECK_EXPIRE_TIME = 15000;				// fallback in case pinging hangs, considered failed after this time has passed
	static const int AUTOMATIC_REFRESH_RATE = 60000;					// time after which a refresh request is automatically triggered
	
	// Statuses
	static const string STATUS_OK = "ok";
	static const string STATUS_ERROR = "error";
	
	// Services
	static const string SERVICE_ACCOUNT_PROFILE = 			"game-identity";
	static const string SERVICE_XBOX =						"game-identity";
	static const string SERVICE_BI_BACKEND_MULTIPLAYER =	"game-api";
	static const string SERVICE_WORKSHOP =					"reforger-workshop-api";
	
	// Ping thresholds
	static const int PING_MAX = 999;
	static const int PING_THRESHOLD_GOOD = 100;
	static const int PING_THRESHOLD_BAD = 300;
	static const int PING_THRESHOLD_AWFUL = 600;
	
	protected static SCR_ECommStatus s_eLastReceivedCommStatus = SCR_ECommStatus.RUNNING;
	protected static bool s_bIsCheckingCommStatus;
	protected static float s_fLastReceivedCommResponseTime;
	
	protected static float s_fPingStartTime;
	
	protected static bool s_bFirstRefresh = true;
	
	// Invokers
	protected static ref ScriptInvokerCommStatus s_OnCommStatusCheckFinished;
	protected static ref ScriptInvokerVoid s_OnCommStatusCheckStart;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static bool IsBackendEnabled()
	{
		return GetGame().GetBackendApi() && GetGame().GetBackendApi().IsRunning();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static bool IsBackendReady()
	{
		return GetGame().GetBackendApi() && GetGame().GetBackendApi().IsActive();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static bool IsAuthenticated()
	{
		return IsBackendReady() && GetGame().GetBackendApi().IsAuthenticated();
	}
	
	//------------------------------------------------------------------------------------------------
	// PING
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! \return
	protected static bool IsPinging()
	{
		return GetGame().GetBackendApi() && GetGame().GetBackendApi().GetCommTestStatus() == 1;
	}

	//------------------------------------------------------------------------------------------------
	//! ---> This is the function to call if you want to update the connection state <---
	//! Refresh ping value - won't do anything if is already pinging and waiting for the result
	// TODO: add a force refresh for when certain important menus are opened? eg we want to disable buttons to open multiplayer or workshop
	static void RefreshPing()
	{
		if (!GetGame() || !GetGame().InPlayMode())
			return;
		
		if (!s_bFirstRefresh && !CanRefresh())
		{
			// If we receive a refresh request while in cooldown (for example when a new menu is opened), refresh automatically to update the UI asap
			if (IsInRefreshCooldown())
			{
				ClearRefreshQueue();
				GetGame().GetCallqueue().CallLater(RefreshPing, (REFRESH_COOLDOWN - GetPingAge()) + 100);
			}
			
			return;
		}

		ClearRefreshQueue();
		
		if (s_bFirstRefresh)
		{
			s_bFirstRefresh = false;
			s_bIsCheckingCommStatus = true;
			
			// TODO: when testing without workbench, immediatly refreshing will cause GetCommTestStatus() to return 0 even if backend is initialized?!
			GetGame().GetCallqueue().CallLater(RefreshPing_Internal, FIRST_REFRESH_DELAY);
			return;
		}
		
		RefreshPing_Internal();
	}
	
	//------------------------------------------------------------------------------------------------
	protected static void RefreshPing_Internal()
	{
		// Start pinging to get the current status
		if (IsBackendReady())
			GetGame().GetBackendApi().RefreshCommStatus();
		else
			s_eLastReceivedCommStatus = SCR_ECommStatus.RUNNING;

		s_fPingStartTime = System.GetTickCount();

		StartStatusCheck();
	}

	//------------------------------------------------------------------------------------------------
	protected static void StartStatusCheck()
	{
		s_bIsCheckingCommStatus = true;

		if (s_OnCommStatusCheckStart)
			s_OnCommStatusCheckStart.Invoke();
		
		// This will perform regular checks for backend initialization or communication status, untill a result is obtained or a set amount of time has passed
		GetGame().GetCallqueue().CallLater(CheckStatus, COMM_STATUS_CHECK_FREQUENCY_MS);
	}
	
	//------------------------------------------------------------------------------------------------
	protected static void CheckStatus()
	{
		s_bIsCheckingCommStatus = false;
		
		if (IsBackendReady())
			CheckCommStatus();
		else
			CheckBackendStatus();
	}
	
	//------------------------------------------------------------------------------------------------
	// Try to give a definitive result
	// Once a result is out, this is also where the refresh cooldown starts, as it's based on s_iLastPingUpdate
	protected static void CheckCommStatus()
	{	
		BackendApi backend = GetGame().GetBackendApi();
		
		// Still pinging, need to wait some more to get definitive results
		if (IsPinging() && !IsPingingStuck())
		{
			s_eLastReceivedCommStatus = SCR_ECommStatus.RUNNING;
			
			StartStatusCheck();
			return;
		}
		
		// At this point, the comm test result should be either 0, 2 or 3
		if (!backend || IsPingingStuck() || !IsBackendReady())
			s_eLastReceivedCommStatus = SCR_ECommStatus.NOT_EXECUTED;
		else
			s_eLastReceivedCommStatus = backend.GetCommTestStatus();

		// If the comm test went well, update statuses
		if (s_eLastReceivedCommStatus == SCR_ECommStatus.FINISHED)
			RefreshStatuses();
		
		s_iLastPingUpdate = System.GetTickCount();
		s_fLastReceivedCommResponseTime = backend.GetCommResponseTime();
		
		// Notify that results are out
		if (s_OnCommStatusCheckFinished)
			s_OnCommStatusCheckFinished.Invoke(s_eLastReceivedCommStatus, s_fLastReceivedCommResponseTime, backend.GetCommTimeLastSuccess(), backend.GetCommTimeLastFail());
	}
	
	//------------------------------------------------------------------------------------------------
	// Wait for backend to be ready and then try again
	protected static void CheckBackendStatus()
	{
		// After some time stop checking and move on to giving a result
		if (!GetGame().GetBackendApi() || IsPingingStuck())
		{
			CheckCommStatus();
			return;
		}
		
		if (IsBackendReady())
			RefreshPing_Internal();
		else
			StartStatusCheck();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	static void ClearRefreshQueue()
	{
		GetGame().GetCallqueue().Remove(RefreshPing);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_ECommStatus GetLastReceivedCommStatus()
	{
		return s_eLastReceivedCommStatus;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return ping value
	//! \return ping in milliseconds
	static int GetPingValue()
	{
		if (!IsBackendReady() || s_eLastReceivedCommStatus == SCR_ECommStatus.FAILED)
			return -1;

		return Math.Round(s_fLastReceivedCommResponseTime);
	}

	//------------------------------------------------------------------------------------------------
	//! Return the last ping's age in milliseconds
	static int GetPingAge()
	{
		return System.GetTickCount() - s_iLastPingUpdate;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	// We want to still allow attempting a refresh if backend is not initialized
	static bool CanRefresh()
	{
		return !s_bIsCheckingCommStatus && !IsInRefreshCooldown();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static bool IsInRefreshCooldown()
	{
		return GetPingAge() < REFRESH_COOLDOWN;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static bool IsPingingStuck()
	{
		return System.GetTickCount() - s_fPingStartTime >= CONNECTION_CHECK_EXPIRE_TIME;
	}
	
	//------------------------------------------------------------------------------------------------
	// STATUSES
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//!
	static void RefreshStatuses()
	{
		if (!IsBackendReady())
			return;

		s_MainStatus = GetGame().GetBackendApi().GetMainStatus();

		SERVICE_STATUSES.Clear();
		ServiceStatusItem item;
		for (int i, cnt = GetGame().GetBackendApi().GetStatusCount(); i < cnt; i++)
		{
			item = GetGame().GetBackendApi().GetStatusItem(i);
			if (item)
				SERVICE_STATUSES.Insert(item);
		}

		s_iLastStatusUpdate = System.GetTickCount();

		#ifdef SERVICES_DEBUG
			DEBUG();
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static ServiceStatusItem GetMainStatus()
	{
		return s_MainStatus;
	}

	//------------------------------------------------------------------------------------------------
	//! finds a service by name
	//! \param statusName name of the wanted status, case INsensitive
	static ServiceStatusItem GetStatusByName(string statusName)
	{
		ServiceStatusItem result;

		statusName.ToLower();
		string serviceNameLC;

		foreach (ServiceStatusItem statusItem : SERVICE_STATUSES)
		{
			serviceNameLC = statusItem.Name();
			serviceNameLC.ToLower();

			if (serviceNameLC == statusName)
				return statusItem;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] statuses
	static void GetStatuses(notnull array<ServiceStatusItem> statuses)
	{
		statuses.Copy(SERVICE_STATUSES);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static bool AreServicesReady()
	{
		return s_MainStatus && !SERVICE_STATUSES.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Return statuses age IN SECONDS
	static int GetStatusesAge()
	{
		if (!IsBackendReady())
			return -1;

		return Math.Round((System.GetTickCount() - s_iLastStatusUpdate) * 0.001);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] service
	//! \return
	static bool DisplayServiceOnCurrentPlatform(SCR_BackendServiceDisplay service)
	{
		switch (System.GetPlatform())
		{
			case EPlatform.UNKNOWN:
			{
				return false;
			}
			case EPlatform.WINDOWS:
			case EPlatform.LINUX:
			{
				return SCR_Enum.HasFlag(service.m_ePlatforms, SCR_EBackendServicePlatforms.PC);
			}
			case EPlatform.XBOX_ONE:
			case EPlatform.XBOX_ONE_S:
			case EPlatform.XBOX_ONE_X:
			case EPlatform.XBOX_SERIES_S:
			case EPlatform.XBOX_SERIES_X:
			{
				return SCR_Enum.HasFlag(service.m_ePlatforms, SCR_EBackendServicePlatforms.XBOX);
			}
			case EPlatform.PS4:
			case EPlatform.PS5:
			case EPlatform.PS5_PRO:
			{
				return SCR_Enum.HasFlag(service.m_ePlatforms, SCR_EBackendServicePlatforms.PS);
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] serviceName
	//! \return
	static bool IsServiceActive(string serviceName)
	{
		if (s_eLastReceivedCommStatus != SCR_ECommStatus.FINISHED)
			return false;
		
		ServiceStatusItem serviceStatus = GetStatusByName(serviceName);
		if (!serviceStatus)
			return false;
		
		return serviceStatus.Status() == STATUS_OK;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	// Check if all conditions for multiplayer are fine in here
	static bool AreMultiplayerServicesAvailable()
	{
		return IsServiceActive(SERVICE_BI_BACKEND_MULTIPLAYER);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static bool IsBackendConnectionAvailable()
	{
		return s_eLastReceivedCommStatus != SCR_ECommStatus.NOT_EXECUTED && s_eLastReceivedCommStatus != SCR_ECommStatus.RUNNING && IsBackendReady();
	}
	
	//------------------------------------------------------------------------------------------------
	// INVOKERS
	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvokerCommStatus GetOnCommStatusCheckFinished()
	{
		if (!s_OnCommStatusCheckFinished)
			s_OnCommStatusCheckFinished = new ScriptInvokerCommStatus();
		
		return s_OnCommStatusCheckFinished;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvokerVoid GetOnCommStatusCheckStart()
	{
		if (!s_OnCommStatusCheckStart)
			s_OnCommStatusCheckStart = new ScriptInvokerVoid();
		
		return s_OnCommStatusCheckStart;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	static void DEBUG()
	{
		Print("--------------------------------------------------");
		Print("SERVICE STATUS DEBUG");
		Print("--------------------------------------------------");
		if (!GetGame().GetBackendApi())
		{
			Print("no backend API found.");
			return;
		}

		Print("is main service found: " + (GetGame().GetBackendApi().GetMainStatus() != null));
		Print("number of services found: " + GetGame().GetBackendApi().GetStatusCount());

		if (s_MainStatus)
			PrintFormat("Service %1: %2 (Name/Status/Message: %3/%4/%5)", -1, s_MainStatus, s_MainStatus.Name(), s_MainStatus.Status(), s_MainStatus.Message());
		else
			Print("Main Status not found");

		foreach (int index, ServiceStatusItem statusItem : SERVICE_STATUSES)
		{
			if (statusItem)
				PrintFormat("Service %1: %2 (Name/Status/Message: %3/%4/%5)", index, statusItem, statusItem.Name(), statusItem.Status(), statusItem.Message());
		}

		Print("--------------------------------------------------");
	}
}

//------------------------------------------------------------------------------------------------
//! Class for a .conf file with multiple service display presets.
[BaseContainerProps(configRoot : true)]
class SCR_BackendServiceDisplayPresets
{
	[Attribute()]
	protected ref array<ref SCR_BackendServiceDisplay> m_aServices;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_BackendServiceDisplay> GetServices()
	{
		array<ref SCR_BackendServiceDisplay> services = {}; 
		if (m_aServices)
			services = m_aServices;
		
		return services;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sId")]
class SCR_BackendServiceDisplay
{
	[Attribute(desc: "internal tag")]
	string m_sId;

	[Attribute(desc: "the id used to query backend")]
	string m_sServiceId;

	[Attribute(desc: "displayed name")]
	string m_sTitle;
	
	[Attribute(typename.EnumToString(SCR_EBackendServicePlatforms, SCR_EBackendServicePlatforms.PC), UIWidgets.Flags, "", "Platforms this service should be displayed on", ParamEnumArray.FromEnum(SCR_EBackendServicePlatforms))]
	SCR_EBackendServicePlatforms m_ePlatforms;
}

//------------------------------------------------------------------------------------------------
enum SCR_EBackendServicePlatforms
{
	PC		= 1 << 0,
	XBOX	= 1 << 1,
	PS		= 1 << 2
}