/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Backend Api instance
sealed class BackendApi
{
	private void BackendApi();
	private void ~BackendApi();

	//! Error code to string
	string GetErrorCode(int code)
	{
		string result;

		if (code == EBackendError.EBERR_OK)
			result = "OK";
		else if (code == EBackendError.EBERR_UNKNOWN)
			result = "Offline";
		else if (code == EBackendError.EBERR_DISABLED)
			result = "Communication Disabled";
		else if (code == EBackendError.EBERR_INVALID_STATE)
			result = "Cannot be called from current state";
		else if (code == EBackendError.EBERR_BUSY)
			result = "Busy processing requests";
		else if (code == EBackendError.EBERR_LOGIN_FAILED)
			result = "Failed to logon";
		else if (code == EBackendError.EBERR_AUTH_FAILED)
			result = "Failed to Authenticate";
		else
			result = " * ";

		return result;
	}

	[Obsolete("Use GetPlayerIdentityId() instead.")]
		string GetPlayerUID( int iPlayerId ) { return GetPlayerIdentityId(iPlayerId); }

	/*!
	\brief Set session callback to recieve all session related events
	\param callback - check all events and respective description upon object
	*/
	proto external void SetSessionCallback(DSSessionCallback callback);
	/*!
	\brief Return count of services
	*/
	proto external int GetStatusCount();
	/*!
	\brief Return specific status object
	*/
	proto external ServiceStatusItem GetStatusItem( int iIndex );
	/*!
	\brief Return Main status object
	*/
	proto external ServiceStatusItem GetMainStatus();
	/*!
	\brief Return count of News messages
	*/
	proto external int GetNewsCount();
	/*!
	\brief Return specific News message
	*/
	proto external NewsFeedItem GetNewsItem( int iIndex );
	/*!
	\brief Return count of Notification messages
	*/
	proto external int GetNotifyCount();
	/*!
	\brief Return specific Notification message
	*/
	proto external NewsFeedItem GetNotifyItem( int iIndex );
	/*!
	\brief Return count of Popups
	*/
	proto external int GetPopupCount();
	/*!
	\brief Return specific Popup
	*/
	proto external PopupFeedItem GetPopupItem( int iIndex );
	/*!
	\brief Triggers OnSuccess when check for popup image finished
	*/
	proto external void OnPopupReady(BackendCallback callback);
	/*!
	\brief Triggers OnSuccess when check for news image finished
	*/
	proto external void OnNewsReady(BackendCallback callback);
	/*!
	\brief Return specific Link by it's name
	*/
	proto external string GetLinkItem( string linkName );
	/*!
	\brief Shutdown backend - request processing
	*/
	proto external bool Shutdown();
	/*!
	\brief Invoke credentials update (authenticate with new name+password)
	*/
	[Obsolete("Use BohemiaAccountApi.Link() instead.")]
	proto external void VerifyCredentials(BackendCallback callback, bool storeCredentials);
	/*!
	\brief Unlink the bi-account and clear credentials
	*/
	[Obsolete("Use BohemiaAccountApi.Unlink() instead.")]
	proto external void Unlink(BackendCallback callback);
	/*!
	\brief The bi-account remains locked for X seconds
	*/
	[Obsolete("Use BohemiaAccountApi.GetSecondsUntilAccountUnlockTime() instead.")]
	proto external int RemainingAccountLockedTime();
	/*!
	\brief Client is Authenticated - relate requests may proceed
	*/
	[Obsolete("Use BackendAuthenticatorApi.IsAuthenticated() instead.")]
	proto external bool IsAuthenticated();
	/*!
	\brief Client is busy - Authentication in process
	*/
	[Obsolete("Use BackendAuthenticatorApi.IsAuthInProgress() instead.")]
	proto external bool IsAuthInProgress();
	/*!
	\brief True if HTTP communication enabled (initialization, runtime or shutdown may be pending at same time)
	*/
	proto external bool IsRunning();
	/*!
	\brief True if HTTP communication is being activated (initializing yet)
	*/
	proto external bool IsInitializing();
	/*!
	\brief True if HTTP communication active (initialized)
	*/
	proto external bool IsActive();
	/*!
	\brief Refresh status - ping services to obtain response time
	*/
	proto external void RefreshCommStatus();
	/*!
	\brief Get comm test status, 0==not executed, 1==running, 2==finished, 3==failed
	*/
	proto external int GetCommTestStatus();
	/*!
	\brief Read service response time in milliseconds, if value is subzero - test was not executed yet, call RefreshCommStatus()
	*/
	proto external float GetCommResponseTime();
	/*!
	\brief Get time in seconds since last successful request (limit is 1hr - does not count more)
	*/
	proto external float GetCommTimeLastSuccess();
	/*!
	\brief Get time in seconds since last successful request (limit is 1hr - does not count more)
	*/
	proto external float GetCommTimeLastFail();
	/*!
	\brief Ask specific request with callback result
	\param request Is type of request, which is EBackendRequest
	\param cb Is script callback where you will recieve result/ error or even data when request finsihes
	\param dataObject Is optional destination when request uses or response return Json data and you want to work with object
	*/
	proto external void Request( int request, BackendCallback cb, JsonApiStruct dataObject );
	/*!
	\brief Ask player request with callback result from controller (Lobby)
	\param request Is type of request, which is EBackendRequest
	\param cb Is script callback where you will recieve result/ error or even data when request finsihes
	\param dataObject Is optional destination when request uses or response return Json data and you want to work with object
	\param iPlayerId Is Player Id used on player identity
	*/
	[Obsolete()]
	proto external void PlayerRequest( int request, BackendCallback cb, JsonApiStruct dataObject, int iPlayerId );
	/*!
	\brief Expand player data upon defined structure, this is Server-Side only!
	\note Data are available only after player was successfully accepted into Room/ Lobby on server
	\param dataObject Is optional destination when request uses or response return Json data and you want to work with object
	\param iPlayerId Is Player Id used on player identity
	*/
	[Obsolete()]
	proto external void PlayerData( JsonApiStruct dataObject, int iPlayerId );
	/*!
	\brief Expand settings data upon defined structure, this is Server-Side only!
	\note Data are available only after game was successfully executed and connected to online services
	\param sFileName Is name of settings file you want to access, remember - it must exist!
	\param dataObject Is optional destination when request uses or response return Json data and you want to work with object
	*/
	proto external void SettingsData( string sFileName, JsonApiStruct dataObject );
	/*!
	\brief Send feedback message and/ or script object with whatever data on it (additionally it is possible to handle callback as well)
	\param cb Is script callback where you will recieve result/ error or even data when request finsihes
	\param dataObject Is optional destination when request uses or response return Json data and you want to work with object
	\param message Is custom
	\param data Raw image data of screenshot, if any
	\param width Width of the screenshot
	\param height Height of the screenshot
	\param channels How many channels does the image use (4 for RGBA)
	\param quality 1-100, the higher number the better quality but also bigger size of the image
	*/
	proto external void FeedbackMessage( BackendCallback cb, JsonApiStruct dataObject, string message, PixelRawData data = null, int width = 0, int height = 0, int channels = 0, int quality = 0 );
	/*!
	\brief Get Workshop Api
	*/
	proto external WorkshopApi GetWorkshop();
	/*!
	\brief Get Server Config Api
	*/
	proto external ServerConfigApi GetServerConfigApi();
	/*!
	\brief Get Ban Service Api
	*/
	proto external BanServiceApi GetBanServiceApi();
	/*!
	\brief Get Client Lobby Api
	*/
	proto external ClientLobbyApi GetClientLobby();
	/*!
	\brief Check if player is in list of admins defined in server config
	\param iPlayerId is Id of Player which is being verified
	*/
	proto external bool IsListedServerAdmin( int iPlayerId );
	/*!
	\brief Check if player is server owner defined by ownerToken
	\param iPlayerId is Id of Player which is being verified
	*/
	proto external bool IsServerOwner( int iPlayerId );
	/*!
	\brief Get Session Storage
	*/
	proto external SessionStorage GetStorage();
	/*!
	\brief Get DS server instance
	*/
	proto external DSSession GetDSSession();
	/*!
	\brief Get Player Identity ID by Player ID
	\param iPlayerId Is id of player in session
	*/
	[Obsolete()]
	proto external string GetPlayerIdentityId(int iPlayerId);
	/*!
	\brief Get Local Identity ID on client
	*/
	[Obsolete("Use BackendAuthenticatorApi.GetIdentityId() instead.")]
	proto external string GetLocalIdentityId();
	/*!
	\brief Get Player Platform Kind by Player ID
	\param iPlayerId Is id of player in session
	*/
	[Obsolete()]
	proto external PlatformKind GetPlayerPlatformKind(int iPlayerId);
	/*!
	\brief Get Player Platform ID by Player ID
	\param iPlayerId Is id of player in session
	\note Xbox and PS Ids are hashed in SHA-256
	*/
	[Obsolete()]
	proto external string GetPlayerPlatformId(int iPlayerId);
	/*!
	\brief Return true if local platform data are to be used for authentication/ persistency of client (meaningless on server)
	*/
	proto external bool IsLocalPlatformAssigned();
	/*!
	\brief Set credentials value per item
	\param item Is type of EBackendCredentials parameter you want to set
	\param str Is value itself
	*/
	[Obsolete("Use BohemiaAccountApi.Link() instead.")]
	proto external void SetCredentialsItem( EBackendCredentials item, string str );
	/*!
	\brief Get credentials value per item
	\param item Is type of EBackendCredentials parameter you want to read
	*/
	[Obsolete("Use BohemiaAccountApi.GetEmail() or BohemiaAccountApi.GetName() instead.")]
	proto external string GetCredentialsItem( EBackendCredentials item );
	/*!
	\brief Return true if BI Account is linked to local Identity
	*/
	[Obsolete("Use BohemiaAccountApi.IsLinked() instead.")]
	proto external bool IsBIAccountLinked();
	/*!
	\brief Get target backend environment
	*/
	proto external owned string GetBackendEnv();
	proto external bool GetRunningDSConfig(DSConfig config);
	proto external bool LoadDSConfig(DSConfig config, string fileName);
	proto external bool SaveDSConfig(DSConfig config, string fileName);
	proto external void SetDefaultIpPort(DSConfig config);
	proto external int GetAvailableConfigs(out notnull array<string> configs);
	proto external int GetAvailableConfigPaths(out notnull array<string> configs);
	/*!
	\brief Called when new server or client game session is started
	*/
	proto external void NewSession();
	/*!
	\brief Debugging API for internal builds
	*/
	proto external void SetDebugHandling(EBackendRequest eRequest, EBackendDebugHandling eHandlingType);

	// callbacks

	/*!
	\brief Called when platform ready and provided all data necessary to connect online
	*/
	event void OnPlatformActive();
	/*!
	\brief Called when platform is not ready - perhaps user is not signed-in
	*/
	event void OnPlatformMissing();
}

/*!
\}
*/
