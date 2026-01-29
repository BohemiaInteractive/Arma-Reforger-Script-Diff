//------------------------------------------------------------------------------------------------
class SCR_LoginDialogUI : SCR_LoginProcessDialogUI
{
	protected const string USERNAME_WIDGET = "UserName";
	protected const string PASSWORD_WIDGET = "Password";
	
	protected SCR_EditBoxComponent m_UserName;
	protected SCR_EditBoxComponent m_Password;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);

		// Load credentials
		string name = BohemiaAccountApi.GetEmail();
		m_UserName = SCR_EditBoxComponent.GetEditBoxComponent(USERNAME_WIDGET, m_wRoot);
		if (m_UserName)
		{
			m_UserName.SetValue(name);
			m_UserName.m_OnChanged.Insert(CheckFilledEditBoxes);

			GetGame().GetWorkspace().SetFocusedWidget(m_UserName.GetRootWidget());
		}

		m_Password = SCR_EditBoxComponent.GetEditBoxComponent(PASSWORD_WIDGET, m_wRoot);
		if (m_Password)
			m_Password.m_OnChanged.Insert(CheckFilledEditBoxes);

		CheckFilledEditBoxes();
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{	
		if (m_bIsLoading || !m_UserName || !m_Password)
			return;
		
		string user = m_UserName.GetValue().Trim();
		
		if (!VerifyFormatting(user))
		{
			ShowWarningMessage(true);
			return;
		}
		
		// Verify credentials
		BohemiaAccountApi.Link(m_Callback, user, m_Password.GetValue(), "");
		
		super.OnConfirm();
	}

	//------------------------------------------------------------------------------------------------
	override void OnFailDelayed(BackendCallback callback)
	{
		if (callback.GetHttpCode() == SCR_ELoginFailReason.TWO_FACTOR_AUTHENTICATION)
		{
			SCR_LoginProcessDialogUI.Create2FADialog(m_UserName.GetValue().Trim(), m_Password.GetValue());
			Close();
		}
		else
		{
			super.OnFailDelayed(callback);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void ShowWarningMessage(bool show)
	{
		super.ShowWarningMessage(show);

		if (!show)
			return;
		
		if (m_UserName)
			m_UserName.OnInvalidInput();
		
		if (m_Password)
			m_Password.OnInvalidInput();
	}
	
	//------------------------------------------------------------------------------------------------
	// Username must be name@domain.x format
	override bool VerifyFormatting(string text)
	{
		// check for spaces and @
		if (text.IsEmpty() || text.Contains(" ") || !text.Contains("@") || text.LastIndexOf("@") == text.Length() - 1)
			return false;
		
		array<string> substrings = {};
		text.Split("@", substrings, true);
		
		// make sure there's only one @
		if (substrings.IsEmpty() || substrings.Count() != 2)
			return false;

		// check if the domain contains . and it's not the last symbol
		string domain = substrings[1];
		if (domain.IsEmpty() || !domain.Contains(".") || domain[0] == "." || domain.LastIndexOf(".") == domain.Length() - 1)
			return false;
		
		// make sure there's no symbols except . in the domain string
		for (int i = 0; i < domain.Length(); i++)
		{
			string char = domain[i];
			
			if (char == ".")
				continue;
			
			int ascii = char.ToAscii();
			
			bool number = ascii >= 48 && ascii <= 57;
			bool capLetter = ascii >= 65 && ascii <= 90;
			bool letter = ascii >= 97 && ascii <= 122;
			
			if (!number && !capLetter && !letter)
				return false;
		}

		return super.VerifyFormatting(text);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckFilledEditBoxes()
	{
		m_bForceConfirmButtonDisabled = m_UserName.GetValue().IsEmpty() || m_Password.GetValue().IsEmpty();
		if (m_ConfirmButton)
			m_ConfirmButton.SetEnabled(!m_bForceConfirmButtonDisabled && UpdateButtons());
	}
}

//------------------------------------------------------------------------------------------------
class SCR_LoginDialogConsoleUI : SCR_LoginDialogUI
{
	protected const string OVERLAY_MAIN = "OverlayMain";
	
	protected OverlayWidget m_wOverlayMain; 
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);
		
		m_wOverlayMain = OverlayWidget.Cast(m_wRoot.FindAnyWidget(OVERLAY_MAIN));
	}
	
	//------------------------------------------------------------------------------------------------
	// The dialog has a unique structure so we need to attach the loading overlay to a different Widget than other dialogs
	override OverlayWidget GetDialogBaseOverlay()
	{
		return m_wOverlayMain;
	}
}