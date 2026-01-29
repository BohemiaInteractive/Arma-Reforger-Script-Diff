class SCR_AccountWidgetComponent : SCR_ScriptedWidgetComponent
{
	[Attribute(UIColors.GetColorAttribute(UIColors.CONFIRM), UIWidgets.ColorPicker)]
	protected ref Color m_ColorOnline;

	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_ACTIVE_STANDBY), UIWidgets.ColorPicker)]
	protected ref Color m_ColorOffline;

	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_ACTIVE_STANDBY), UIWidgets.ColorPicker)]
	protected ref Color m_ColorConnecting;

	[Attribute(SCR_ConnectionUICommon.ICON_CONNECTION)]
	protected string m_sIconOnline;

	[Attribute(SCR_ConnectionUICommon.ICON_SERVICES_ISSUES)]
	protected string m_sIconOffline;

	[Attribute(SCR_ConnectionUICommon.ICON_CONNECTION)]
	protected string m_sIconConnecting;
	
	[Attribute("0")]
	protected bool m_bShowOnlineIcon;
	
	[Attribute("Profile")]
	protected string m_sTooltipTag;
	
	[Attribute("#AR-Account_AuthenticationFailed")]
	protected string m_sTooltipMessageOffline;
	
	[Attribute("#AR-Account_Authenticating")]
	protected string m_sTooltipMessageConnecting;
	
	protected SCR_CoreMenuHeaderButtonComponent m_News;
	protected SCR_CoreMenuHeaderButtonComponent m_Career;
	protected SCR_CoreMenuHeaderButtonComponent m_Settings;
	protected SCR_CoreMenuHeaderButtonComponent m_BlockedList;

	protected SCR_ModularButtonComponent m_Profile;
	protected SCR_DynamicIconComponent m_ProfileStatusIcon;

	protected bool m_bLoggedIn;
	protected BackendApi m_BackendApi;
	
	protected SCR_ScriptedWidgetTooltip m_Tooltip;

	static const int AUTH_CHECK_PERIOD = 1000;
	static const ResourceName BLOCKED_USER_DIALOG_CONFIG = "{12C2EC09520BE302}Configs/Blocking/BlockedUsersDialog.conf";

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{	
		super.HandlerAttached(w);

		m_News = SCR_CoreMenuHeaderButtonComponent.Cast(GetComponent(SCR_CoreMenuHeaderButtonComponent, "NewsButton", w));
		if (m_News)
			m_News.GetButton().m_OnClicked.Insert(OnNews);

		m_Career = SCR_CoreMenuHeaderButtonComponent.Cast(GetComponent(SCR_CoreMenuHeaderButtonComponent, "CareerButton", w));
		if (m_Career)
			m_Career.GetButton().m_OnClicked.Insert(OnCareer);
		
		m_Settings = SCR_CoreMenuHeaderButtonComponent.Cast(GetComponent(SCR_CoreMenuHeaderButtonComponent, "SettingsButton", w));
		if (m_Settings)
			m_Settings.GetButton().m_OnClicked.Insert(OnSettings);
		
		// TODO: Hide if no user is on block list (Backend API needed)
		m_BlockedList = SCR_CoreMenuHeaderButtonComponent.Cast(GetComponent(SCR_CoreMenuHeaderButtonComponent, "BlockedUsersButton", w));
		if (m_BlockedList)
		{
			m_BlockedList.GetButton().m_OnClicked.Insert(OnBlocked);
			//if (System.GetPlatform() != EPlatform.PS5 && System.GetPlatform() != EPlatform.PS5_PRO)
				//m_BlockedList.GetButton().SetVisible(false);
		}

		Widget profile = w.FindAnyWidget("Profile");
		if (profile)
		{
			m_Profile = SCR_ModularButtonComponent.FindComponent(profile);
			if (m_Profile)
				m_Profile.m_OnClicked.Insert(OnProfile);
			
			m_ProfileStatusIcon = SCR_DynamicIconComponent.FindComponent("DotOverlay", profile);
			if (m_ProfileStatusIcon)
				m_ProfileStatusIcon.SetIconColor(m_ColorOffline);
		}
		
		m_BackendApi = GetGame().GetBackendApi();
		
		UpdateAuthentication();
		
		// Owner menu events
		SCR_MenuHelper.GetOnMenuFocusGained().Insert(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuOpen().Insert(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuFocusLost().Insert(OnMenuDisabled);
		SCR_MenuHelper.GetOnMenuClose().Insert(OnMenuDisabled);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		// Owner menu events
		SCR_MenuHelper.GetOnMenuFocusGained().Remove(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuOpen().Remove(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuFocusLost().Remove(OnMenuDisabled);
		SCR_MenuHelper.GetOnMenuClose().Remove(OnMenuDisabled);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuEnabled(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(GetRootWidget()))
		{
			GetGame().GetCallqueue().Remove(UpdateAuthentication);
			GetGame().GetCallqueue().CallLater(UpdateAuthentication, AUTH_CHECK_PERIOD, true);
			
			SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);
		}
		
		// Hide news menu button (top right corner) on PS
		if (GetGame().GetPlatformService().GetLocalPlatformKind() == PlatformKind.PSN)
		{
			Widget newsButton = GetRootWidget().FindAnyWidget("NewsButton");
			if (newsButton)
			{
				newsButton.SetVisible(false);
				newsButton.SetEnabled(false);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuDisabled(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(GetRootWidget()))
		{
			GetGame().GetCallqueue().Remove(UpdateAuthentication);
			
			SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		if (tooltip.GetTag() == m_sTooltipTag)
		{
			m_Tooltip = tooltip;
			UpdateAuthentication();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateAuthentication()
	{
		if (!m_BackendApi)
		{
			m_BackendApi = GetGame().GetBackendApi();
			if (!m_BackendApi)
				return;
		}

		if (!m_ProfileStatusIcon)
			return;

		m_ProfileStatusIcon.SetVisible(true);
		
		Color color = m_ColorOffline;
		string image = m_sIconOffline;
		
		string tooltipMessage = m_sTooltipMessageOffline;
		Color tooltipMessageColor = m_ColorOffline;
		
		SCR_ScriptedWidgetTooltipContentBase tooltipContent;
		if (m_Tooltip)
			tooltipContent = m_Tooltip.GetContent();
		
		if (BackendAuthenticatorApi.IsAuthInProgress())
		{
			color = m_ColorConnecting;
			image = m_sIconConnecting;
			
			tooltipMessage = m_sTooltipMessageConnecting;
			tooltipMessageColor = m_ColorConnecting;
		}
		else if (m_BackendApi.IsAuthenticated())
		{
			color = m_ColorOnline;
			image = m_sIconOnline;
			
			m_ProfileStatusIcon.SetVisible(m_bShowOnlineIcon);
			
			if (tooltipContent)
				tooltipMessage = tooltipContent.GetDefaultMessage();
			
			tooltipMessageColor = Color.FromInt(Color.WHITE);
		}

		m_ProfileStatusIcon.SetIconColor(color);
		m_ProfileStatusIcon.SetImage(image);
		
		if (tooltipContent)
		{
			tooltipContent.SetMessage(tooltipMessage);
			tooltipContent.SetMessageColor(tooltipMessageColor);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNews()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ProfileSuperMenu, 0, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCareer()
	{
		//TODO: uncomment once Career Menu is finished
		//GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CareerProfileMenu, 0, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Open the Settings menu
	protected void OnSettings()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.SettingsSuperMenu, 0, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBlocked()
	{
		SCR_BlockedUsersDialogUI dialog = new SCR_BlockedUsersDialogUI();
		SCR_ConfigurableDialogUi.CreateFromPreset(BLOCKED_USER_DIALOG_CONFIG, "blocked_list", dialog);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnProfile()
	{
		// Show profile screen
		if (BohemiaAccountApi.IsLinked())
			SCR_LoginProcessDialogUI.CreateProfileDialog();
		else
			SCR_LoginProcessDialogUI.CreateLoginDialog();
	}
}
