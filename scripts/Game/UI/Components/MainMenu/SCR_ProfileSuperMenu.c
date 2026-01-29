enum SCR_EProfileSuperMenuTabId
{
	NEWS = 0,
	COMMUNITY
}

class SCR_ProfileSuperMenu : SCR_SuperMenuBase
{
	SCR_ModularButtonComponent m_Logo;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		SCR_InputButtonComponent comp = m_DynamicFooter.FindButton(UIConstants.BUTTON_BACK);
		if (comp)
			comp.m_OnActivated.Insert(Close);

		Widget logoWidget = GetRootWidget().FindAnyWidget("LogoButton");
		if (logoWidget)
		{
			m_Logo = SCR_ModularButtonComponent.FindComponent(logoWidget);
			if (m_Logo)
				m_Logo.m_OnClicked.Insert(OnLogoClicked);
		}
		
		// Setup automatic focusing on tab show
		if (!m_SuperMenuComponent)
			return;
		
		SCR_TabViewComponent tabView = m_SuperMenuComponent.GetTabView();
		if (tabView)
			tabView.GetOnContentShow().Insert(OnTabContentShow);
		
		if (GetGame().GetPlatformService().GetLocalPlatformKind() == PlatformKind.PSN)
			tabView.RemoveTabByIdentifier("Community");
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		
		ResetFocus();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLogoClicked()
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.WelcomeDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTabContentShow(SCR_TabViewComponent tabView, Widget widget)
	{
		ResetFocus();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ResetFocus()
	{
		if (m_Logo)
			GetGame().GetWorkspace().SetFocusedWidget(m_Logo.GetRootWidget());
	}
}