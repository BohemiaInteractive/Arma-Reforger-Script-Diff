/* 
	Parent class of login/profile dialogs
*/
//------------------------------------------------------------------------------------------------
class SCR_LoginProcessDialogUI : SCR_ConfigurableDialogUi
{
	// Widgets
	protected const string TOS_BUTTON = "ProfileTOS";
	protected const string TOS_LINK = "Link_PrivacyPolicy";
	
	protected const string REGISTER_BUTTON = "createAccount";
	protected const string REGISTER_LINK = "Link_RegisterAccount";
	
	protected const string PID_TEXT_WIDGET = "PIDText";
	protected const string PID_BUTTON_WIDGET = "CopyPIDButton";
	
	// Buttons
	protected SCR_InputButtonComponent m_TOSButton;
	protected SCR_InputButtonComponent m_ConfirmButton;
	protected SCR_InputButtonComponent m_CreateAccount;
	
	protected SCR_ButtonImageComponent m_CopyButtonComponent;
	protected RichTextWidget m_wPIDText;
	
	// Other
	protected SCR_LoadingOverlay m_LoadingOverlay;
	protected SCR_SimpleWarningComponent m_Warning;
	
	protected bool m_bForceConfirmButtonDisabled;
	protected bool m_bIsLoading;
	
	protected ref BackendCallback m_Callback;
	
	protected const int ON_FAIL_DELAY = 2000;
	
	protected static const ResourceName DIALOG_CONFIG = "{9381BF296A0E273B}Configs/Dialogs/LoginDialogs.conf";

	// Dialog creation
	//------------------------------------------------------------------------------------------------
	static SCR_LoginDialogUI CreateLoginDialog()
	{
		if (GetGame().IsPlatformGameConsole())
		{
			SCR_LoginDialogConsoleUI dialog = new SCR_LoginDialogConsoleUI();
			SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "LOGIN_CONSOLE", dialog);
			
			return dialog;
		}
		
		SCR_LoginDialogUI dialog = new SCR_LoginDialogUI();
		SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "LOGIN", dialog);
		
		return dialog;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_PlayerProfileDialogUI CreateProfileDialog()
	{
		SCR_PlayerProfileDialogUI dialog = new SCR_PlayerProfileDialogUI();
		SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "PLAYER_PROFILE", dialog);

		return dialog;
	}

	//------------------------------------------------------------------------------------------------
	static void Create2FADialog(string name, string code)
	{
		if (GetGame().IsPlatformGameConsole())
		{
			SCR_Login2FADialogConsoleUI dialog = new SCR_Login2FADialogConsoleUI(name, code);
			SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "2FA_CONSOLE", dialog);
			
			return;
		}
		
		SCR_Login2FADialogUI dialog = new SCR_Login2FADialogUI(name, code);
		SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "2FA", dialog);
	}
	
	// Generic account related feedback dialogs
	// We use the SCR_ConfigurableDialogUi because confirming must close the dialog and there's no need for SCR_LoginProcessDialogUI functionalities
	//------------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateLoginSuccessDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "LOGIN_SUCCESS");
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateLoginTimeoutDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "LOGIN_TIMEOUT");
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_AccountLockedDialogUi CreateAccountLockedDialog()
	{
		SCR_AccountLockedDialogUi dialog = new SCR_AccountLockedDialogUi();
		SCR_ConfigurableDialogUi.CreateFromPreset(DIALOG_CONFIG, "ACCOUNT_LOCKED", dialog);
		
		return dialog;
	}
	
	// Overrides
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{	
		super.OnMenuOpen(preset);
		
		m_TOSButton = FindButton(TOS_BUTTON);
		if (m_TOSButton)
			m_TOSButton.m_OnActivated.Insert(OnTOSButton);
		
		m_CreateAccount = FindButton(REGISTER_BUTTON);
		if (m_CreateAccount)
			m_CreateAccount.m_OnActivated.Insert(OnCreateAccount);
		
		m_ConfirmButton = FindButton(BUTTON_CONFIRM);
		
		m_wPIDText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(PID_TEXT_WIDGET));
		
		m_CopyButtonComponent = SCR_ButtonImageComponent.GetButtonImage(PID_BUTTON_WIDGET, m_wRoot);
		if (m_CopyButtonComponent)
			m_CopyButtonComponent.m_OnClicked.Insert(CopyPID);

		m_Warning = SCR_SimpleWarningComponent.FindComponentInHierarchy(m_wRoot);
		ShowWarningMessage(false);
		
		// Status check
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		UpdateButtons();
		
		// Callback events
		m_Callback = new BackendCallback();
		
		m_Callback.SetOnSuccess(OnSuccess);
		m_Callback.SetOnError(OnFail);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
		
		GetGame().GetCallqueue().Remove(OnTimeoutScripted);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		ShowLoadingAnim(true);
		ShowWarningMessage(false);
		
		// fallback timeout should the backend callback not work
		GetGame().GetCallqueue().Remove(OnTimeoutScripted);
		GetGame().GetCallqueue().CallLater(OnTimeoutScripted, SCR_ServicesStatusHelper.AUTOMATIC_REFRESH_RATE);
	}
	
	// Methods
	//------------------------------------------------------------------------------------------------
	protected bool UpdateButtons()
	{	
		string service = SCR_ServicesStatusHelper.SERVICE_ACCOUNT_PROFILE;
		
		SCR_ConnectionUICommon.ForceConnectionButtonEnabled(m_TOSButton, SCR_ServicesStatusHelper.IsBackendConnectionAvailable(), false);
		SCR_ConnectionUICommon.ForceConnectionButtonEnabled(m_CreateAccount, SCR_ServicesStatusHelper.IsBackendConnectionAvailable(), false);

		SCR_ConnectionUICommon.SetConnectionButtonEnabled(m_ConfirmButton, service, m_bForceConfirmButtonDisabled || ! SCR_ServicesStatusHelper.IsBackendConnectionAvailable(), false);
	
		string identity = BackendAuthenticatorApi.GetIdentityId();
		
		if (m_wPIDText)
			m_wPIDText.SetText(identity);
		
		if (m_CopyButtonComponent)
			m_CopyButtonComponent.SetVisible(!GetGame().IsPlatformGameConsole() && !identity.IsEmpty(), false);
		
		if (m_wPIDText)
			m_wPIDText.SetVisible(!identity.IsEmpty());
		
		return SCR_ServicesStatusHelper.IsServiceActive(service);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowLoadingAnim(bool show)
	{
		m_bIsLoading = show;

		if (show)
			m_LoadingOverlay = SCR_LoadingOverlay.ShowForWidget(GetDialogBaseOverlay(), string.Empty, false, true);
		else if (m_LoadingOverlay)
			m_LoadingOverlay.HideAndDelete();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowWarningMessage(bool show)
	{
		if (m_Warning)
			m_Warning.SetWarningVisible(show);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Copy playerID to clipboard
	protected void CopyPID()
	{
		System.ExportToClipboard(m_wPIDText.GetText());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Override in children
	protected bool VerifyFormatting(string text)
	{
		return true;
	}
	
	// Events
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTOSButton()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PrivacyPolicyMenu);
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnCreateAccount()
	{
		GetGame().GetPlatformService().OpenBrowser(GetGame().GetBackendApi().GetLinkItem(REGISTER_LINK));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSuccess(BackendCallback callback)
	{
		CreateLoginSuccessDialog();
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFail(BackendCallback callback)
	{
		GetGame().GetCallqueue().Remove(OnTimeoutScripted);
		
		if (callback.GetRestResult() != ERestResult.EREST_ERROR_TIMEOUT)
		{
			// Add a delay to prevent the backend from being overwhelmed
			// TODO: change this into a cooldown after the response has been received to make the UI as responsive as possible
			GetGame().GetCallqueue().Remove(OnFailDelayed);
			GetGame().GetCallqueue().CallLater(OnFailDelayed, ON_FAIL_DELAY, false, callback);
			return;
		}
		
		// Timeout handling
		ShowLoadingAnim(false);
		ShowWarningMessage(false);
		
		CreateLoginTimeoutDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTimeout()
	{
		GetGame().GetCallqueue().Remove(OnTimeoutScripted);
		
		ShowLoadingAnim(false);
		ShowWarningMessage(false);
		
		CreateLoginTimeoutDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFailDelayed(BackendCallback callback)
	{
		GetGame().GetCallqueue().Remove(OnFailDelayed);

		ShowLoadingAnim(false);
		ShowWarningMessage(false);
		
		// Based on restCode
		// wrong credentials: 401
		if (callback.GetHttpCode() == SCR_ELoginFailReason.INVALID_CREDENTIALS)
		{
			ShowWarningMessage(true);
			return;
		}
		
		// bad request: 400
		// user not found: EApiCode.EACODE_ERROR_USER_NOT_FOUND -> player wrote wrong credentials
		if (callback.GetApiCode() == EApiCode.EACODE_ERROR_USER_NOT_FOUND)
			ShowWarningMessage(true);
		
		// account locked: EApiCode.EACODE_ERROR_USER_LOCKED
		else if (callback.GetApiCode() == EApiCode.EACODE_ERROR_USER_LOCKED)
			CreateAccountLockedDialog();
		
		else
			CreateLoginTimeoutDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTimeoutScripted()
	{
		OnTimeout();
	}
	
	//------------------------------------------------------------------------------------------------
	BackendCallback GetCallback()
	{
		return m_Callback;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_AccountLockedDialogUi : SCR_ConfigurableDialogUi
{
	protected const string MAIN_MESSAGE = "#AR-Account_Locked_Message";
	protected const string CONTENT_MESSAGE = "#AR-CoreMenus_Support";
	protected const string MINUTES = "#AR-ValueUnit_Short_Minutes";
	protected const string SECONDS = "#AR-ValueUnit_Short_Seconds";
	
	protected const string SUPPORT = "Link_SupportEmail";
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);
		
		if (!SCR_ServicesStatusHelper.IsBackendConnectionAvailable())
			return;

		RichTextWidget widget = RichTextWidget.Cast(GetContentWidget(m_wRoot).FindAnyWidget("ContentText"));
		if (!widget)
			return;
		
		string message = string.Format("<color rgba=%1>%2</color>", UIColors.FormatColor(UIColors.CONTRAST_COLOR), GetGame().GetBackendApi().GetLinkItem(SUPPORT));
		widget.SetText(WidgetManager.Translate(CONTENT_MESSAGE, message));
		
		UpdateMessage();
		GetGame().GetCallqueue().CallLater(UpdateMessage, 1000, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		GetGame().GetCallqueue().Remove(UpdateMessage);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateMessage()
	{
		int seconds = BohemiaAccountApi.GetSecondsUntilAccountUnlockTime();
		int minutes = seconds / 60;
		
		string time = WidgetManager.Translate(MINUTES, minutes) + " " + WidgetManager.Translate(SECONDS, seconds - (60 * minutes));
		if (seconds < 60)
			time = WidgetManager.Translate(SECONDS, seconds);
		
		SetMessage(WidgetManager.Translate(MAIN_MESSAGE, time));
	}
}

//------------------------------------------------------------------------------------------------
enum SCR_ELoginFailReason
{
	BAD_REQUEST = 400,
	INVALID_CREDENTIALS = 401,
	TWO_FACTOR_AUTHENTICATION = 422,
}