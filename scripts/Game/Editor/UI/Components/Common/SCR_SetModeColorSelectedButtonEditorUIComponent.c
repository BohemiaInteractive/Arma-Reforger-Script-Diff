class SCR_SetModeColorSelectedButtonEditorUIComponent : SCR_SetModeColorEditorUIComponent
{
	protected SCR_ButtonBaseComponent m_wButton;
	
	//------------------------------------------------------------------------------------------------
	protected override void SetColor(Color color)
	{
		if (!m_wButton)
		{	
			m_wButton = SCR_ButtonBaseComponent.Cast(m_wRoot.FindHandler(SCR_ButtonBaseComponent));
			
			if (!m_wButton)
				return;
		}

		m_wButton.m_BackgroundSelected = color;
		m_wButton.m_BackgroundSelectedHovered = color;
		m_wButton.m_BackgroundClicked = color;
		
		if (m_wButton.IsToggled())
			m_wButton.ColorizeBackground(false);
	}
}
