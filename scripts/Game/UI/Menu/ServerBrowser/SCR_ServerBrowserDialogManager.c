/*!
Class for handling server browser dialogs.
Server browser use multiple dialogs in joining process for checking servers, mod content and restrictinos.
*/

//------------------------------------------------------------------------------------------------
class SCR_ServerBrowserDialogManager
{
	// Resources
	const ResourceName CONFIG_DIALOGS = "{471EFCF445C3E9C6}Configs/ServerBrowser/JoiningDialogs.conf";
	protected const string OFFICIAL_SERVER_SCOPE = "officialServer";

	// Dialog tags references
	protected const string TAG_SEARCHING_SERVER 				= "SEARCHING_SERVER";
	protected const string TAG_JOIN 							= "JOIN";
	protected const string TAG_REJOIN 							= "REJOIN";
	protected const string TAG_SERVER_NOT_FOUND 				= "SERVER_NOT_FOUND";
	protected const string TAG_VERSION_MISMATCH 				= "VERSION_MISMATCH";
	protected const string TAG_CHECKING_CONTENT 				= "CHECKING_CONTENT";
	protected const string TAG_MOD_UGC_PRIVILEGE_MISSING 		= "MOD_UGC_PRIVILEGE_MISSING";
	protected const string TAG_MODS_DOWNLOADING 				= "MODS_DOWNLOADING";
	protected const string TAG_SERVER_FULL 						= "SERVER_FULL";
	protected const string TAG_PASSWORD_REQUIRED 				= "PASSWORD_REQUIRED";
	protected const string TAG_BACKEND_TIMEOUT					= "BACKEND_TIMEOUT";
	protected const string TAG_BANNED							= "JOIN_FAILED_BAN";
	protected const string TAG_HIGH_PING_SERVER					= "HIGH_PING_SERVER";
	protected const string TAG_UNRELATED_DOWNLOADS_CANCELING	= "UNRELATED_DOWNLOADS_CANCELING";

	protected const string STR_LIGHT_BAN = "#AR-LightBan";
	protected const string STR_HEAVY_BAN = "#AR-HeavyBan";

	// References
	protected Room m_JoinRoom;
	protected SCR_RoomModsManager m_ModManager;
	protected ServerBrowserMenuUI m_ServerBrowser;

	// States
	protected EJoinDialogState m_iDisplayState;
	protected SCR_ConfigurableDialogUi m_CurrentDialog;

	const string ERROR_TAG_DEFAULT = "JOIN_FAILED";

	// Invokers
	protected ref ScriptInvokerVoid m_OnConfirm;
	protected ref ScriptInvokerVoid m_OnCancel;
	protected ref ScriptInvokerVoid m_OnJoinProcessCancel;

	protected ref ScriptInvokerRoom m_OnDownloadComplete;
	protected ref ScriptInvokerVoid m_OnDownloadCancelDialogClose;

	//------------------------------------------------------------------------------------------------
	// Public functions
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Display dialog by state and setup current state
	void DisplayDialog(EJoinDialogState state)
	{
		// Set
		m_iDisplayState = state;

		if (m_CurrentDialog)
			m_CurrentDialog.Close();

		// Visual set
		switch (state)
		{
			// Searching server
			case EJoinDialogState.SEARCHING_SERVER:
			{
				SetDialogByTag(TAG_SEARCHING_SERVER);
				break;
			}

			// Joining
			case EJoinDialogState.JOIN:
			{
				SetDialogByTag(TAG_JOIN);
				break;
			}

			// Joining
			case EJoinDialogState.REJOIN:
			{
				SetDialogByTag(TAG_REJOIN);
				break;
			}

			// Server by given parameters wasn't found
			case EJoinDialogState.SERVER_NOT_FOUND:
			{
				SetDialogByTag(TAG_SERVER_NOT_FOUND);
				break;
			}
			
			// Wrong version restriction
			case EJoinDialogState.VERSION_MISMATCH:
			{
				SetDialogByTag(TAG_VERSION_MISMATCH);

				// Display version difference text
				string clientV = GetGame().GetBuildVersion();
				string RoomV = m_JoinRoom.GameVersion();

				string msg = string.Format("%1: %2 \n%3: %4", "#AR-ServerBrowser_Server", RoomV, "#AR-Editor_AttributeCategory_GameSettings_Name", clientV);
				m_CurrentDialog.SetMessage(msg);
				break;
			}

			// Checking mod content
			case EJoinDialogState.CHECKING_CONTENT:
			{
				SetDialogByTag(TAG_CHECKING_CONTENT);
				break;
			}

			// Client can't access user generated content
			case EJoinDialogState.MOD_UGC_PRIVILEGE_MISSING:
			{
				SetDialogByTag(TAG_MOD_UGC_PRIVILEGE_MISSING);
				break;
			}

			// Password input dialog
			case EJoinDialogState.PASSWORD_REQUIRED:
			{
				DisplayPasswordRequired();
				break;
			}

			// Password input dialog
			case EJoinDialogState.BACKEND_TIMEOUT:
			{
				SetDialogByTag(TAG_BACKEND_TIMEOUT);
				break;
			}
			
			// High ping
			case EJoinDialogState.HIGH_PING_SERVER:
			{
				SetDialogByTag(TAG_HIGH_PING_SERVER);
				break;
			}
			
			// Unrelated Downloads canceling filler dialog
			case EJoinDialogState.UNRELATED_DOWNLOADS_CANCELING:
			{
				SetDialogByTag(TAG_UNRELATED_DOWNLOADS_CANCELING);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetDialogMessage(string msg)
	{
		if (!m_CurrentDialog)
			return;

		m_CurrentDialog.SetMessage(msg);
	}

	//------------------------------------------------------------------------------------------------
	void CloseCurrentDialog()
	{
		if (m_CurrentDialog)
			m_CurrentDialog.Close();
	}

	//------------------------------------------------------------------------------------------------
	//! Quick short code to open dialog by tag and cache it
	protected void SetDialogByTag(string tag, SCR_ConfigurableDialogUi dialog = null)
	{
		// Check dialog resource
		if (CONFIG_DIALOGS.IsEmpty())
			return;

		// Remove invokers actions from old dialog
		if (m_CurrentDialog)
			m_CurrentDialog.m_OnConfirm.Remove(OnDialogConfirm);

		// Create dialog
		m_CurrentDialog = SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOGS, tag, dialog);

		// Add invoker actions to current dialog
		m_CurrentDialog.m_OnConfirm.Insert(OnDialogConfirm);
		m_CurrentDialog.m_OnCancel.Insert(OnDialogCancel);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDialogConfirm()
	{
		if (m_OnConfirm)
			m_OnConfirm.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDialogCancel()
	{
		if (m_OnCancel)
			m_OnCancel.Invoke();
		
		// Separate invoker for final cleanup (the other OnCancel might be used for specific functionalities still)
		if (m_OnJoinProcessCancel)
			m_OnJoinProcessCancel.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	// Spefic dialog handling
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Show join fail message in dialog
	void DisplayJoinFail(EApiCode apiError)
	{
		CloseCurrentDialog();
		
		string errorTag = ERROR_TAG_DEFAULT;

		// TODO@wernerjak - setup dialogs

		switch (apiError)
		{
			case EApiCode.EACODE_ERROR_P2P_USER_JOIN_BAN: errorTag = TAG_BANNED; break;
			case EApiCode.EACODE_ERROR_DS_USER_JOIN_BAN: errorTag = TAG_BANNED; break;
			case EApiCode.EACODE_ERROR_USER_IS_BANNED_FROM_SHARED_GAME: errorTag = TAG_BANNED; break;
			case EApiCode.EACODE_ERROR_PLAYER_IS_BANNED: errorTag = TAG_BANNED; break;
			//case EApiCode.EACODE_ERROR_MAINTENANCE_IN_PROGRESS: errorTag = "JOIN_FAILED_MAITANANCE"; break;
			//case EApiCode.EACODE_ERROR_MP_ROOM_IS_NOT_JOINABLE: errorTag = "JOIN_FAILED_NOT_JOINABLE"; break;
		}

		SetDialogByTag(errorTag);		
		
		// Show additional message
		if (m_CurrentDialog)
		{
			SCR_ErrorDialog errorDialog = SCR_ErrorDialog.Cast(m_CurrentDialog.GetRootWidget().FindHandler(SCR_ErrorDialog));
			if (errorDialog)
			{
				string strApiError = typename.EnumToString(EApiCode, apiError);
				errorDialog.SetErrorDetail("#AR-Workshop_Error: " + strApiError);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void DisplayJoinBan(RoomJoinData data)
	{	
		CloseCurrentDialog();
		
		SetDialogByTag(TAG_BANNED);
		
		if (!m_CurrentDialog)
			return;
		
		if (data.expiresAt < 0)
			return;
		
		int time = data.expiresAt - System.GetUnixTime();
		
		// Show message with time 	
		if (time <= 0)
			return;
		
		string message = STR_LIGHT_BAN;
		
		// Is ban for official servers
		if (data.scope.Contains(OFFICIAL_SERVER_SCOPE))
			message = STR_HEAVY_BAN;
		
		int timeMinutes = time / 60; // show minutes
		m_CurrentDialog.SetMessage(WidgetManager.Translate(message, timeMinutes.ToString()));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when all reports from dialog are cancled to clear invoker actions and display download dialog
	protected void OnAllReportsCanceled(SCR_ReportedAddonsDialog dialog)
	{
		dialog.GetOnAllReportsCanceled().Remove(OnAllReportsCanceled);
	}

	//------------------------------------------------------------------------------------------------
	// Display a dialog asking for downloads cancel confirmation
	void DisplayJoinDownloadsWarning(array<ref SCR_WorkshopItemActionDownload> downloads, SCR_EJoinDownloadsConfirmationDialogType type)
	{
		// Remove invokers actions from old dialog
		if (m_CurrentDialog)
			m_CurrentDialog.m_OnConfirm.Remove(OnDialogConfirm);
		
		m_CurrentDialog = SCR_ServerJoinDownloadsConfirmationDialog.Create(downloads, type);

		// Add invoker actions to current dialog
		m_CurrentDialog.m_OnConfirm.Insert(OnDialogConfirm);
		m_CurrentDialog.m_OnCancel.Insert(OnDialogCancel);
		
		if (type == SCR_EJoinDownloadsConfirmationDialogType.REQUIRED)
			SCR_DownloadManager.GetInstance().GetOnDownloadQueueCompleted().Insert(OnDownloadingDone);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDownloadingDone()
	{
		// Check if any of required addon is not failed
		array<ref SCR_WorkshopItem> required = m_ModManager.GetRoomItemsScripted();

		foreach (SCR_WorkshopItem item : required)
		{
			if (!item.GetOffline())
				return;
		}

		SCR_DownloadManager.GetInstance().GetOnDownloadQueueCompleted().Remove(OnDownloadingDone);
		
		if (m_OnDownloadComplete)
			m_OnDownloadComplete.Invoke(m_JoinRoom);
	}

	//------------------------------------------------------------------------------------------------
	//! Display dialog for password insert whenever password is required
	protected void DisplayPasswordRequired()
	{
		SCR_EditboxDialogUi editboxDialog = new SCR_EditboxDialogUi();
		SetDialogByTag(TAG_PASSWORD_REQUIRED, editboxDialog);

		editboxDialog.m_OnWriteModeLeave.Insert(OnPasswordEditboxChanged);
		editboxDialog.FindButton(SCR_ConfigurableDialogUi.BUTTON_CONFIRM).SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this whenever password dialog editbox is being edited
	protected void OnPasswordEditboxChanged(string text)
	{
		if (!m_CurrentDialog || !SCR_EditboxDialogUi.Cast(m_CurrentDialog))
			return;

		SCR_InputButtonComponent navButton = m_CurrentDialog.FindButton(SCR_ConfigurableDialogUi.BUTTON_CONFIRM);
		if (navButton)
			navButton.SetEnabled(!text.IsEmpty());

		navButton = m_CurrentDialog.FindButton(SCR_ConfigurableDialogUi.BUTTON_CANCEL);
		if (navButton)
			navButton.SetEnabled(true);
	}
	
	//! DECOUPLED DIALOGS
	//------------------------------------------------------------------------------------------------
	MultiplayerDialogUI CreateManualJoinDialog()
	{
		MultiplayerDialogUI multiplayerDialogUI = new MultiplayerDialogUI();
		MultiplayerDialogUI dialog = MultiplayerDialogUI.Cast(SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOGS, "MANUAL_CONNECT", multiplayerDialogUI));
		m_CurrentDialog = dialog;

		return dialog;
	}

	// Server details
	//------------------------------------------------------------------------------------------------
	SCR_ServerDetailsDialog CreateServerDetailsDialog(Room room, array<ref SCR_WorkshopItem> items, ScriptInvokerVoid onFavoritesResponse = null)
	{
		SCR_ServerDetailsDialog dialog = SCR_ServerDetailsDialog.CreateServerDetailsDialog(room, items, "SERVER_DETAILS", CONFIG_DIALOGS, onFavoritesResponse);
		m_CurrentDialog = dialog;
		m_CurrentDialog.m_OnCancel.Insert(OnDialogCancel);
		
		return dialog;
	}

	//------------------------------------------------------------------------------------------------
	void FillRoomDetailsMods(array<ref SCR_WorkshopItem> items, SCR_RoomModsManager modsManager = null)
	{
		SCR_ServerDetailsDialog dialog = SCR_ServerDetailsDialog.Cast(m_CurrentDialog);
		dialog.FillModList(items, modsManager);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateRoomDetailsScenarioImage(MissionWorkshopItem scenario)
	{
		SCR_ServerDetailsDialog dialog = SCR_ServerDetailsDialog.Cast(m_CurrentDialog);
		if (dialog)
			dialog.SetScenarioImage(scenario);
	}

	// Server Full
	//------------------------------------------------------------------------------------------------
	SCR_ServerFullDialog CreateServerFullDialog()
	{
		SCR_ServerFullDialog dialog = new SCR_ServerFullDialog();
		SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOGS, TAG_SERVER_FULL, dialog);

		m_CurrentDialog = dialog;

		if (m_CurrentDialog)
			m_CurrentDialog.m_OnCancel.Insert(OnDialogCancel);
		
		return dialog;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateServerFullScenarioImage(MissionWorkshopItem scenario)
	{
		SCR_ServerFullDialog dialog = SCR_ServerFullDialog.Cast(m_CurrentDialog);
		if (dialog)
			dialog.SetScenarioImage(scenario);
	}
	
	//------------------------------------------------------------------------------------------------
	// Update wating queue with actual player count
	void UpdateServerFullDialog()
	{
		SCR_ServerFullDialog dialog = SCR_ServerFullDialog.Cast(m_CurrentDialog);
		if (dialog)
			dialog.UpdateInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateBlockedPlayersWarningDialog(array<BlockedRoomPlayer> blockedPlayers)
	{
		SCR_BlockedPlayerPresentDialog dialog = new SCR_BlockedPlayerPresentDialog(blockedPlayers);
		
		m_CurrentDialog = dialog;
		
		m_CurrentDialog.m_OnCancel.Insert(OnDialogCancel);
		m_CurrentDialog.m_OnConfirm.Insert(OnDialogConfirm);
	}
	
	//------------------------------------------------------------------------------------------------
	// Get & Set API
	//------------------------------------------------------------------------------------------------
	
	// Invokers
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnConfirm()
	{
		if (!m_OnConfirm)
			m_OnConfirm = new ScriptInvokerVoid();

		return m_OnConfirm;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnCancel()
	{
		if (!m_OnCancel)
			m_OnCancel = new ScriptInvokerVoid();

		return m_OnCancel;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnJoinProcessCancel()
	{
		if (!m_OnJoinProcessCancel)
			m_OnJoinProcessCancel = new ScriptInvokerVoid();

		return m_OnJoinProcessCancel;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerRoom GetOnDownloadComplete()
	{
		if (!m_OnDownloadComplete)
			m_OnDownloadComplete = new ScriptInvokerRoom();

		return m_OnDownloadComplete;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnDownloadCancelDialogClose()
	{
		if (!m_OnDownloadCancelDialogClose)
			m_OnDownloadCancelDialogClose = new ScriptInvokerVoid();

		return m_OnDownloadCancelDialogClose;
	}
	
	// Helpers
	//------------------------------------------------------------------------------------------------
	EJoinDialogState GetDisplayState()
	{
		return m_iDisplayState;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ConfigurableDialogUi GetCurrentDialog()
	{
		return m_CurrentDialog;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsDialogOpen()
	{
		return GetCurrentDialog();
	}

	//------------------------------------------------------------------------------------------------
	void SetJoinRoom(Room room)
	{
		m_JoinRoom = room;
	}

	//------------------------------------------------------------------------------------------------
	void SetModManager(SCR_RoomModsManager modManager)
	{
		m_ModManager = modManager;
	}
}

//------------------------------------------------------------------------------------------------
//! Enum for tracking current state of joining process
enum EJoinDialogState
{
	// Basic states
	SEARCHING_SERVER, // Basic msg dialog with loading
	JOIN, // Basic msg dialog with loading
	REJOIN, // Rejoin dialog if client is kicked

	// Basic issues states
	SERVER_NOT_FOUND, // Basic error msg dialog
	VERSION_MISMATCH, // Basic error msg dialog
	JOIN_FAILED,

	// Content states
	CHECKING_CONTENT, // Basic msg dialog with loading
	MODS_DOWNLOADING, // Progress bar dialog
	MOD_RESTRICTED, // Basic error msg dialog
	MOD_UGC_PRIVILEGE_MISSING, // Client can't use user generated content

	// Additional actions states
	PASSWORD_REQUIRED, // Editbox dialog

	// Issues
	SERVICE_DOWN, // Backend service is not available
	SERVER_DOWN, // Joined server is not running
	BACKEND_TIMEOUT,
	
	// Warnings
	HIGH_PING_SERVER,
	
	// Fillers
	UNRELATED_DOWNLOADS_CANCELING // Waiting for all downloads to cancel
};

//------------------------------------------------------------------------------------------------
//! Enum for confirmation dialogs that guide the player through the download processes required to join the servers
enum SCR_EJoinDownloadsConfirmationDialogType
{
	ALL,
	UNRELATED,
	REQUIRED
};
