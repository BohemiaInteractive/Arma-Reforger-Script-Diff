class SCR_MainMenuNewsToast : SCR_ScriptedWidgetComponent
{
	[Attribute(uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EPlatform))]
	private SCR_EPlatform m_eHideOnPlatform;
	
	protected const string LINK_FALLBACK = "https://reforger.armaplatform.com/news";

	protected const string WIDGET_NAME_TITLE = "m_wTitle";
	protected const string WIDGET_NAME_DESC = "m_wDescription";
	protected const string WIDGET_NAME_BUTTON = "m_wButton";

	protected TextWidget m_wTitle;
	protected TextWidget m_wDescription;
	protected SCR_InputButtonComponent m_MoreInfoButton;

	protected string m_sMoreInfoLink;
	
	protected bool m_bIsHovered;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		if (SCR_Global.IsEditMode())
			return;
		
		EPlatform currentPlatform = System.GetPlatform();
		w.SetVisible(false);
		
		if ((m_eHideOnPlatform & (1 << currentPlatform)) == (1 << currentPlatform))
			return;

		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
	}

	//------------------------------------------------------------------------------------------------
	protected void Init()
	{
		if (GetGame().GetBackendApi().GetNewsCount() < 1)
			return;

		NewsFeedItem newsItem = GetGame().GetBackendApi().GetNewsItem(0);
		if (!newsItem)
			return;
		
		m_wRoot.SetVisible(true);

		m_wTitle = TextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_NAME_TITLE));
		m_wDescription = TextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_NAME_DESC));

		m_wTitle.SetText(newsItem.Title());
		m_wDescription.SetText(newsItem.Date());

		Widget buttonWidget = m_wRoot.FindAnyWidget(WIDGET_NAME_BUTTON);
		if (!buttonWidget)
			return;

		m_sMoreInfoLink = newsItem.URL();
		if (m_sMoreInfoLink.IsEmpty())
			m_sMoreInfoLink = LINK_FALLBACK;

		m_MoreInfoButton = SCR_InputButtonComponent.Cast(buttonWidget.FindHandler(SCR_InputButtonComponent));
		if (m_MoreInfoButton)
			m_MoreInfoButton.m_OnActivated.Insert(OnMoreInfo);
		
		if (GetGame().GetGameInstallStatus() != 1.0)
			m_wRoot.SetVisible(0);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsHovered()
	{
		return m_bIsHovered;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_bIsHovered = true;
		return super.OnMouseEnter(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_bIsHovered = false;
		return super.OnMouseLeave(w, enterW, x, y);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMoreInfo()
	{
		float xPos, yPos, width, height;
		m_wRoot.GetScreenPos(xPos, yPos);
		m_wRoot.GetWorkspace().GetScreenSize(width, height);
		
		GetGame().GetWorkspace().DPIUnscale(xPos);
		GetGame().GetWorkspace().DPIUnscale(yPos);
		
		if (xPos < 0 || yPos < 0 || xPos > width || yPos > height)
			return;
		
		GetGame().GetPlatformService().OpenBrowser(m_sMoreInfoLink);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		if (!m_wRoot.IsVisible() && SCR_ServicesStatusHelper.IsBackendConnectionAvailable())
			Init();
	}
}
