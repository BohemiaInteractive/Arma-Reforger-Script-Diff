/*!
Component for visual behavior of radil menu number hint
Can display current slot number and change color
*/

class SCR_RadialMenuNumberHintComponent : SCR_SimpleEntryComponent
{
	// Widgets 
	[Attribute("Background")]
	protected string m_sBackround;
	
	protected Widget m_wBackround;
	
	// Colors 
	[Attribute("255 255 255 255")]
	protected ref Color m_cTextDefault;
	
	[Attribute("255 255 255 255")]
	protected ref Color m_cBackgroundDefault;
	
	[Attribute("255 255 255 255")]
	protected ref Color m_cTextHighlight;
	
	[Attribute("255 255 255 255")]
	protected ref Color m_cBackgroundHighlight;
	
	//------------------------------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wBackround = w.FindAnyWidget(m_sBackround);
		
		Highlight(false);
	}
	
	//------------------------------------------------------------------------------------------------------------------------
	void Highlight(bool highlight)
	{
		// Highligh 
		if (highlight)
		{
			if (m_wBackround)
				m_wBackround.SetColor(m_cBackgroundHighlight);
			
			if (m_wLabel)
				m_wLabel.SetColor(m_cTextHighlight);
			
			if (m_wMessage)
				m_wMessage.SetColor(m_cTextHighlight);
				
			return;
		}
		
		// Default 
		if (m_wBackround)
			m_wBackround.SetColor(m_cBackgroundDefault);
		
		if (m_wLabel)
			m_wLabel.SetColor(m_cTextDefault);
		
		if (m_wMessage)
			m_wMessage.SetColor(m_cTextDefault);
	}
}