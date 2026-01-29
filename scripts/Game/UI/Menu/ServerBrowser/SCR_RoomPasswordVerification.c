//------------------------------------------------------------------------------------------------
/*!
Solve joining to password protected room
*/
//------------------------------------------------------------------------------------------------
class SCR_RoomPasswordVerification
{
	protected string FALLBACK_ERROR = "#AR-Password_FailMessage";
	protected string FALLBACK_TIMEOUT = "#AR-Password_TimeoutMessage";

	protected Room m_Room;
	protected SCR_ConfigurableDialogUi m_Dialog;
	protected SCR_LoadingOverlay m_LoadingOverlay;

	protected ref BackendCallback m_Callback = new BackendCallback();

	protected ref ScriptInvokerVoid m_OnVerified = new ScriptInvokerVoid();
	protected ref ScriptInvokerString m_OnFailVerification = new ScriptInvokerString();

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnVerified()
	{
		return m_OnVerified;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerString GetOnFailVerification()
	{
		return m_OnFailVerification;
	}

	//------------------------------------------------------------------------------------------------
	//! Setup dialog UI for handling password and joining room
	void SetupDialog(notnull SCR_ConfigurableDialogUi dialog, notnull Room room, string message = "")
	{
		m_Dialog = dialog;
		m_Room = room;

		// Callbacks
		dialog.m_OnConfirm.Insert(OnPasswordConfirm);
		//dialog.m_OnCancel.Insert(PasswordClearInvokers);


		// Display dialog message - TODO: Check what is this for?
		SCR_EditboxDialogUi editDialog = SCR_EditboxDialogUi.Cast(dialog);
		if (editDialog)
			editDialog.SetWarningMessage(message);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitCheck()
	{
		// Show loading
		if (m_LoadingOverlay)
			m_LoadingOverlay.SetShown(true);
		else
			m_LoadingOverlay = SCR_LoadingOverlay.ShowForWidget(GetGame().GetWorkspace(), string.Empty);

		m_Callback.SetOnSuccess(OnPasswordCheckResponse);
		m_Callback.SetOnError(OnPasswordCheckError);
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckRejoinAuthorization(Room room)
	{
		if (!room)
		{
			Debug.Error("Room instance expected! Null found in room");
			return;
		}	
		m_Room = room;
		
		InitCheck();
		m_Room.CheckAuthorization(m_Callback);
		
	}

	//------------------------------------------------------------------------------------------------
	//! Start password verification on confirming password dialog
	protected void OnPasswordConfirm()
	{
		if (!m_Room)
		{
			Debug.Error("Room instance expected! Null found in m_Room");
			return;
		}	
	
		InitCheck();
		
		SCR_EditboxDialogUi editboxDialog = SCR_EditboxDialogUi.Cast(m_Dialog);

		// Get edit box value
		string value = editboxDialog.GetEditbox().GetValue();

		// Try join with password			
		m_Room.VerifyPassword(value, m_Callback);
	}

	//------------------------------------------------------------------------------------------------
	//! Handle password check request response
	protected void OnPasswordCheckResponse(BackendCallback callback)
	{
		// Hide loading
		if (m_LoadingOverlay)
			m_LoadingOverlay.SetShown(false);

		// Clear
		ClearInvokers();

		// Invoke verification with successful password struct
		m_OnVerified.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handle password check request response
	protected void OnPasswordCheckError(BackendCallback callback)
	{
		// Hide loading
		if (m_LoadingOverlay)
			m_LoadingOverlay.SetShown(false);

		// Clear
		ClearInvokers();

		// Reaction
		if (callback.GetRestResult() == ERestResult.EREST_ERROR_TIMEOUT)
		{
			m_OnFailVerification.Invoke(FALLBACK_TIMEOUT);
		}
		else
		{
			m_OnFailVerification.Invoke(FALLBACK_ERROR);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearInvokers()
	{
		if(m_Dialog && m_Dialog.m_OnConfirm)
			m_Dialog.m_OnConfirm.Remove(OnPasswordConfirm);
	}
}
