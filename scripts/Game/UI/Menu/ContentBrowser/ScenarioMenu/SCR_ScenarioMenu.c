/*
Menu with a list view of scenarios.
*/
class SCR_ScenarioMenu : SCR_SuperMenuBase
{
	protected SCR_InputButtonComponent m_NavBack;

	//-----------------------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		// Find nav buttons
		m_NavBack = m_DynamicFooter.FindButton(UIConstants.BUTTON_BACK);
		
		if (m_NavBack)
			m_NavBack.m_OnActivated.Insert(Close);
		
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
	
	//-----------------------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		
		SCR_AnalyticsApplication.GetInstance().OpenScenarios();	
	}
	
	//-----------------------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();	
		
		SCR_AnalyticsApplication.GetInstance().CloseScenarios();
	}
};