class SCR_MANW_Banner : SCR_ScriptedWidgetComponent
{
	protected const string NAME_BUTTON 	= "m_wBannerButtonMain";
	protected const string NAME_TEXT 	= "BannerText";
	protected const int IMAGES_COUNT 	= 2;
	
	protected int m_iImagesReadyCount;
	
	protected Widget m_wBannerButtonMain;
	
	protected ref BackendCallback m_Callback;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{	
		super.HandlerAttached(w);
		
		m_wRoot.SetVisible(false);
		
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Init()
	{
		if (GetGame().GetBackendApi().GetPopupCount() < 1)
			return;
		
		PopupFeedItem popupItem = GetGame().GetBackendApi().GetPopupItem(0);
		if (!popupItem)
			return;
		
		m_wRoot.SetVisible(true);
		
		TextWidget textTitle = TextWidget.Cast(m_wRoot.FindAnyWidget(NAME_TEXT));
		textTitle.SetText(popupItem.BannerText());
		
		//ImageWidget imgTitle = ImageWidget.Cast(m_wRoot.FindAnyWidget("BG"));
		//imgTitle.LoadImageTexture(0, popupItem.BackgroundImagePath());
		
		m_wBannerButtonMain = m_wRoot.FindAnyWidget(NAME_BUTTON);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{	
		bool result = super.OnClick(w, x, y, button);
		
		if (w != m_wBannerButtonMain)
			return result;
		
		SCR_MANW_Dialogs.CreateBannerDialog();
		
		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPopupReady()
	{
		m_iImagesReadyCount++;
		if (m_iImagesReadyCount < IMAGES_COUNT)
			return;
		
		GetGame().GetCallqueue().Call(Init);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		if (!m_wRoot.IsVisible() && SCR_ServicesStatusHelper.IsBackendConnectionAvailable())
		{
			Init();
			
			//m_Callback = new BackendCallback();
			//GetGame().GetBackendApi().OnPopupReady(m_Callback);
			//m_Callback.SetOnSuccess(OnPopupReady);
		}
	}
}