class SCR_BrowserLinkComponent : ScriptedWidgetComponent
{
	[Attribute("Link_Bohemia")]
	protected string m_sAddress;
	
	[Attribute("false")]
	protected bool m_bIsURL;
	
	protected SCR_ButtonBaseComponent m_Button;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_Button = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		if (m_Button)
		{
			SCR_ServicesStatusHelper.RefreshPing();
			SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
			
			m_Button.SetEnabled(SCR_ServicesStatusHelper.IsBackendConnectionAvailable());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		return OpenBrowser();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		if (m_Button)
			m_Button.SetEnabled(SCR_ServicesStatusHelper.IsBackendConnectionAvailable());
	}
	
	//------------------------------------------------------------------------------------------------
	bool OpenBrowser()
	{
		PlatformService ps = GetGame().GetPlatformService();
		
		if (!ps)
			return false;

		BackendApi backendAPI = GetGame().GetBackendApi();
		string url = backendAPI.GetLinkItem(m_sAddress);

		if (m_bIsURL)
			ps.OpenBrowser(m_sAddress);
		else
			ps.OpenBrowser(url);
		
		return true;
	}
};