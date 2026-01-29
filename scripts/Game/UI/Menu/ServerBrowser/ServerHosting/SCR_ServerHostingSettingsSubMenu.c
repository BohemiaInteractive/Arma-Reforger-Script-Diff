/*!
Wrapper sub menu clas for server hosting setting tab
*/
class SCR_ServerHostingSettingsSubMenu : SCR_SubMenuBase
{
	protected SCR_InputButtonComponent m_NavHost;
	protected SCR_InputButtonComponent m_NavSave;
	
	protected bool m_bIsListeningForCommStatus;
	
	protected SCR_ConfigListComponent m_ConfigList;
	
	protected ref ScriptInvokerVoid m_OnHost;
	protected ref ScriptInvokerVoid m_OnSave;

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
		m_bIsListeningForCommStatus = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		m_ConfigList = SCR_ConfigListComponent.Cast(GetRootWidget().FindHandler(SCR_ConfigListComponent));
		
		m_NavHost = CreateNavigationButton(UIConstants.MENU_ACTION_SELECT_HOLD, "#AR-ServerHosting_HostLocally", true);
		if (m_NavHost)
			m_NavHost.m_OnActivated.Insert(OnHost);
		
		UpdateHostButton();
		
		// Tempory disabled saving
		if (!GetGame().IsPlatformGameConsole())
		{
			m_NavSave = CreateNavigationButton("MenuSave", "#AR-PauseMenu_Save", true);
			if (m_NavSave)
				m_NavSave.m_OnActivated.Insert(OnSave);
		}
		
		// Hide 
		ShowNavigationButtons(m_bShown);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();
		
		SCR_ServicesStatusHelper.RefreshPing();
		
		if (!m_bIsListeningForCommStatus)
			SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		m_bIsListeningForCommStatus = true;
		UpdateHostButton();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();
		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
		m_bIsListeningForCommStatus = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateHostButton()
	{
		if (!m_NavHost)
			return;
		
		SCR_ConnectionUICommon.SetConnectionButtonEnabled(m_NavHost, SCR_ServicesStatusHelper.SERVICE_BI_BACKEND_MULTIPLAYER);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateHostButton();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnHost()
	{
		if (m_OnHost)
			m_OnHost.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSave()
	{
		if (m_OnSave)
			m_OnSave.Invoke();
	}
	
	//-------------------------------------------------------------------------------------------
	// API 
	//-------------------------------------------------------------------------------------------
	SCR_ConfigListComponent GetConfigList()
	{
		return m_ConfigList;
	}
	
	//-------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnHost()
	{
		if (!m_OnHost)
			m_OnHost = new ScriptInvokerVoid();
		
		return m_OnHost;
	}
	
	//-------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnSave()
	{
		if (!m_OnSave)
			m_OnSave = new ScriptInvokerVoid();
		
		return m_OnSave;
	}
}