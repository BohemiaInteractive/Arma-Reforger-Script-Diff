class SCR_CreditsLicenses: ChimeraMenuBase
{
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		protected Widget m_wFooter;
		
		m_wFooter = GetRootWidget().FindAnyWidget("Footer");
		
		super.OnMenuOpen();
		// Subscribe to buttons
		SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, m_wFooter);
		if (back)
			back.m_OnActivated.Insert(EndCredits);
	}
		
	//------------------------------------------------------------------------------------------------
	void EndCredits()
	{
		
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CreditsLicensesMenu)
		
	}
};
