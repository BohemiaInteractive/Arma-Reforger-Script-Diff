class SCR_ConnectionUICommon
{
	static const string ICON_SERVICES_ISSUES = 	"connection-issues";
	static const string ICON_CONNECTION = 		"connection";
	static const string ICON_DISCONNECTION = 	"disconnection";
	
	static const string MESSAGE_SERVICES_ISSUES = 	"#AR-CoreMenus_Tooltips_UnavailableServices";
	static const string MESSAGE_CONNECTING = 		"#AR-Workshop_Connecting";
	static const string MESSAGE_DISCONNECTION = 	"#AR-CoreMenus_Tooltips_NoConnection";
	
	static const string MESSAGE_VERBOSE_DISCONNECTION =	"#AR-Workshop_WarningNoConnection";
	static const string MESSAGE_VERBOSE_TIMEOUT =		"#AR-Workshop_Dialog_Error_ConnectionTimeout2";
	
	// Connection state related methods
	//------------------------------------------------------------------------------------------------
	//! Updates the button based on the state of services
	static bool SetConnectionButtonEnabled(SCR_InputButtonComponent button, string serviceName, bool forceDisabled = false, bool animate = true)
	{
		if (!button)
			return false;

		bool serviceActive = SCR_ServicesStatusHelper.IsServiceActive(serviceName);
		bool enabled = serviceActive && !forceDisabled;
		button.SetEnabled(enabled, animate);

		if (forceDisabled && serviceActive)
		{
			button.ResetTexture();
			return true;
		}

		SetConnectionButtonTexture(button, enabled);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Forces the button into the desired state irrelevant of services
	static bool ForceConnectionButtonEnabled(SCR_InputButtonComponent button, bool enabled, bool animate = true)
	{
		if (!button)
			return false;

		button.SetEnabled(enabled, animate);
		SetConnectionButtonTexture(button, enabled);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void SetConnectionButtonTexture(SCR_InputButtonComponent button, bool enabled)
	{
		if (!button)
			return;

		if (enabled)
		{
			button.ResetTexture();
			return;
		}

		button.SetTexture(UIConstants.ICONS_IMAGE_SET, GetConnectionIssuesIcon(), UIColors.CopyColor(UIColors.WARNING_DISABLED));
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetConnectionIssuesIcon()
	{
		switch (SCR_ServicesStatusHelper.GetLastReceivedCommStatus())
		{
			case SCR_ECommStatus.FAILED: 		return ICON_SERVICES_ISSUES;
			case SCR_ECommStatus.NOT_EXECUTED:	return ICON_SERVICES_ISSUES;
			case SCR_ECommStatus.RUNNING: 		return ICON_CONNECTION;
		}
		
		if (!SCR_ServicesStatusHelper.AreMultiplayerServicesAvailable())
			return ICON_SERVICES_ISSUES;
		
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetConnectionIssuesMessage()
	{
		switch (SCR_ServicesStatusHelper.GetLastReceivedCommStatus())
		{
			case SCR_ECommStatus.FAILED: 		return MESSAGE_SERVICES_ISSUES;
			case SCR_ECommStatus.NOT_EXECUTED:	return MESSAGE_SERVICES_ISSUES;
			case SCR_ECommStatus.RUNNING: 		return MESSAGE_CONNECTING;
		}
		
		if (!SCR_ServicesStatusHelper.AreMultiplayerServicesAvailable())
			return MESSAGE_SERVICES_ISSUES;
		
		return string.Empty;
	}
}