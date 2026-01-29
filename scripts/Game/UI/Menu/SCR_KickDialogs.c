class SCR_KickDialogs
{
	protected const ResourceName KICK_DIALOGS_CONFIG = "{D3BFEE28E7D5B6A1}Configs/ServerBrowser/KickDialogs.conf";
	protected const ResourceName HOSTING_ERROR_DIALOG_CONFIG = "{F96212757F65C4A3}Configs/ServerBrowser/ServerHosting/Dialogs/ServerHostingErrors.conf";

	protected const string TAG_KICK_DEFAULT = "DEFAULT_ERROR";
	protected const int MAX_AUTO_REJOINS = 3;

	protected static SCR_ConfigurableDialogUi s_CurrentKickDialog;

	protected static string s_sErrorMessage;
	protected static string s_sErrorMessageGroup;
	protected static string s_sErrorMessageDetail;

	protected static bool m_bReconnectEnabled;

	protected static ref ScriptInvokerVoid s_OnErrorMessageSet;
	protected static ref ScriptInvokerVoid s_OnCancel;
	protected static ref ScriptInvokerVoid s_OnReconnect;
	
	protected static ref BackendCallback m_ProcessLastHostCallback = new BackendCallback();

	//------------------------------------------------------------------------------------------------
	//! Set message for error dialog and create it
	static void CreateKickErrorDialog(string msg, string group, string details = "")
	{
		s_sErrorMessage = msg;
		s_sErrorMessageGroup = group;
		s_sErrorMessageDetail = details;

		if (s_OnErrorMessageSet)
			s_OnErrorMessageSet.Invoke();

		DisplayKickErrorDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create and setup error dialog
	static void DisplayKickErrorDialog()
	{
		if (s_sErrorMessage.IsEmpty())
			return;

		// --- Setup Kick dialog ---
		SCR_ConfigurableDialogUi dialogUi = SCR_ConfigurableDialogUi.CreateFromPreset(KICK_DIALOGS_CONFIG, s_sErrorMessage);
		if (dialogUi && s_sErrorMessage == "PLATFORM_USER_SIGNED_OUT")
		{
			dialogUi.m_OnCancel.Insert(ForceMainMenu);
		}
		
		// Use group as fallback if no dialog found
		if (!dialogUi)
			dialogUi = SCR_ConfigurableDialogUi.CreateFromPreset(KICK_DIALOGS_CONFIG, s_sErrorMessageGroup);


		// Show default error if tag is not found
		if (!dialogUi)
			dialogUi = SCR_ConfigurableDialogUi.CreateFromPreset(KICK_DIALOGS_CONFIG, TAG_KICK_DEFAULT);

		if (!dialogUi)
		{
			Clear();
			return;
		}

		s_CurrentKickDialog = dialogUi;
		dialogUi.m_OnClose.Insert(OnDialogClose);

		// Set error details
		SCR_RejoinDialog errorDialogComp = SCR_RejoinDialog.FindRejoinComponent(dialogUi.GetRootWidget());
		if (errorDialogComp)
			errorDialogComp.SetErrorDetail(s_sErrorMessageDetail);

		// Check reconnect
		SCR_KickDialogUiPreset kickPreset = SCR_KickDialogUiPreset.Cast(dialogUi.GetDialogPreset());
		bool reconnect = m_bReconnectEnabled && kickPreset && kickPreset.m_bCanBeReconnected;

		SCR_InputButtonComponent confirmButton = dialogUi.FindButton(SCR_ConfigurableDialogUi.BUTTON_CONFIRM);
		if (confirmButton)
			confirmButton.SetVisible(reconnect);
		
		// --- Setup reconnect ---
		if (!reconnect)
		{
			Clear();
			return;
		}

		string errorStr = kickPreset.m_sMessage;
		errorDialogComp.SetErrorMessage(errorStr);

		// Check rejoin attempt
		string strAttempt = GameSessionStorage.s_Data["m_iRejoinAttempt"];
		int attempt = strAttempt.ToInt();

		if (attempt <= MAX_AUTO_REJOINS)
		{
			errorStr += "\n" + "#AR-ServerBrowser_JoinMessageDefault";
			errorDialogComp.SetErrorMessage(errorStr);

			dialogUi.SetMessage(errorStr + " " + errorDialogComp.GetTimer());

			// Setup timer
			errorDialogComp.GetEventOnTimerChanged().Insert(OnDialogTimerChange);

			errorDialogComp.ShowLoading(true);
			errorDialogComp.SetTimer(kickPreset.m_iAutomaticReconnectTime);
			errorDialogComp.RunTimer(true);
		}
		else
		{
			dialogUi.SetMessage(errorStr);

			// Block rejoin
			errorDialogComp.ShowLoading(false);
			if (confirmButton)
				confirmButton.SetEnabled(false);
		}

		// Reconnect button
		dialogUi.m_OnConfirm.Insert(OnReconnect);
		dialogUi.m_OnCancel.Insert(OnCancel);

		Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check last server hosting to find out if there was a bug
	static void CheckLastServerHost()
	{
		// Check for hosting errors 
		m_ProcessLastHostCallback.SetOnError(OnProcessLastHostError);
		GetGame().GetBackendApi().GetClientLobby().ProcessLastHostError(m_ProcessLastHostCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	protected static void OnProcessLastHostError(BackendCallback callback)
	{
		int apiCode = callback.GetApiCode();		

		string apiStr = typename.EnumToString(EApiCode, apiCode);
		
		switch (apiCode)
		{
			case EApiCode.EACODE_ERROR_ASSET_NOT_FOUND:
			{
				SCR_ConfigurableDialogUi dialogUi = SCR_ConfigurableDialogUi.CreateFromPreset(HOSTING_ERROR_DIALOG_CONFIG, apiStr);
				return;
			}
			default:
			{
				SCR_ConfigurableDialogUi dialogUi = SCR_ConfigurableDialogUi.CreateFromPreset(HOSTING_ERROR_DIALOG_CONFIG, "unknown");
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	static void SetReconnectEnabled(bool enabled)
	{
		m_bReconnectEnabled = enabled;
	}

	//------------------------------------------------------------------------------------------------
	static void Clear()
	{
		s_sErrorMessage = string.Empty;
		s_sErrorMessageGroup = string.Empty;
		s_sErrorMessageDetail = string.Empty;
	}

	// Getters
	//------------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi GetCurrentKickDialog()
	{
		return s_CurrentKickDialog;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerVoid GetOnCancel()
	{
		if (!s_OnCancel)
			s_OnCancel = new ScriptInvokerVoid();

		return s_OnCancel;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerVoid GetOnReconnect()
	{
		if (!s_OnReconnect)
			s_OnReconnect = new ScriptInvokerVoid();

		return s_OnReconnect;
	}

	// Internal
	//------------------------------------------------------------------------------------------------
	protected static void OnDialogTimerChange(SCR_RejoinDialog dialog, int time)
	{
		if (!s_CurrentKickDialog)
			return;
		
		s_CurrentKickDialog.SetMessage(dialog.GetErrorMessage() + " " + time);

		if (time == 0)
		{
			OnReconnect();
			s_CurrentKickDialog.Close();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnDialogClose()
	{
		s_CurrentKickDialog = null;
		Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnReconnect()
	{
		if (s_OnReconnect)
			s_OnReconnect.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnCancel()
	{
		if (s_OnCancel)
			s_OnCancel.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	static void ForceMainMenu()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.MainMenu);
	}
}
