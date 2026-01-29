class SCR_MANW_BannerDialog : SCR_ConfigurableDialogUi
{
	protected SCR_InputButtonComponent m_Confirm;
	
	const string LINK_ITEM = "Link_MakeArma";
	const string LINK_FALLBACK = "https://www.makearmanotwar.com";
	
	protected string m_sButtonLink;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);
		
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		TextWidget title 			= TextWidget.Cast(m_wRoot.FindAnyWidget("Title"));
		TextWidget imgText 			= TextWidget.Cast(m_wRoot.FindAnyWidget("Text2"));
		ImageWidget imgBackground 	= ImageWidget.Cast(m_wRoot.FindAnyWidget("m_wBackground"));
		ImageWidget imgForeground 	= ImageWidget.Cast(m_wRoot.FindAnyWidget("MainObject"));
		TextWidget description 		= TextWidget.Cast(m_wRoot.FindAnyWidget("m_wDescription"));
		Widget descriptionSize 		= m_wRoot.FindAnyWidget("BannerDescription");
		
		PopupFeedItem popupItem = GetGame().GetBackendApi().GetPopupItem(0);
		
		title.SetText(popupItem.HeaderText());
		imgText.SetText(popupItem.ImageText());
		
		//imgBackground.LoadImageTexture(0, popupItem.BackgroundImagePath());
		//imgForeground.LoadImageTexture(0, popupItem.ImagePath());
		
		if (!popupItem.FooterText().IsEmpty() || popupItem.FooterText() != " ")
		{
			string url = GetGame().GetBackendApi().GetLinkItem(LINK_ITEM);
			if (url.IsEmpty())
				url = LINK_FALLBACK;
			
			description.SetTextFormat(popupItem.FooterText(), url);
			descriptionSize.SetVisible(true);
		}
		
		m_Confirm = FindButton(BUTTON_CONFIRM);
		m_Confirm.SetLabel(popupItem.ActionButtonText());
		m_Confirm.SetEnabled(SCR_ServicesStatusHelper.IsBackendConnectionAvailable());
		
		m_sButtonLink = popupItem.ActionButtonURL();
	}
	
	//----------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		super.OnConfirm();
		
		string url = m_sButtonLink;
		if (url.IsEmpty())
		{
			url = GetGame().GetBackendApi().GetLinkItem(LINK_ITEM);
			if (url.IsEmpty())
				url = LINK_FALLBACK;
		}
		
		GetGame().GetPlatformService().OpenBrowser(url);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		m_Confirm.SetEnabled(SCR_ServicesStatusHelper.IsBackendConnectionAvailable());
	}
}