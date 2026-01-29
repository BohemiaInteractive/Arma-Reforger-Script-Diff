class SCR_MenuMainTileComponent: SCR_MenuTileComponent
{
	protected static string NEWS_FEED_WIDGET = "NewsToast";
	
	protected ref SCR_MainMenuNewsToast m_NewsFeed;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		if (m_wDescription && m_sDescription.IsEmpty())
			m_wDescription.SetVisible(false);
		
		Widget newsFeedWidget = w.FindAnyWidget(NEWS_FEED_WIDGET);
		
		m_NewsFeed = SCR_MainMenuNewsToast.Cast(newsFeedWidget.FindHandler(SCR_MainMenuNewsToast));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_NewsFeed && m_NewsFeed.IsHovered())
			return false;
		
		m_OnClicked.Invoke(this);
		return false;
	}
}