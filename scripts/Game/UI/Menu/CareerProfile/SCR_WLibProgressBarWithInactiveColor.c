//------------------------------------------------------------------------------------------------
class SCR_ProgressBarWithInactiveColor : SCR_WLibProgressBarComponent
{
	[Attribute(defvalue: "0.70 0.70 0.70 1", desc: "Inactive Bar Color")]
	protected ref Color m_SliderInactiveColor;
	
	[Attribute(defvalue: "0.70 0.70 0.70 0.3", desc: "Inactive Bar Color")]
	protected ref Color m_OnHoverInactiveColor;
	
	protected bool m_bActive;
	protected int m_iColorChangeTime;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_bActive = false;
		m_iColorChangeTime = 5;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetActive (bool active = true)
	{
		if (!m_wBar)
			return;
		
		if (active)
			m_wBar.SetColor(m_SliderColor);
		else
			m_wBar.SetColor(m_SliderInactiveColor);
		
		m_bActive = active;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOnHover (bool onHover = true)
	{
		if (!m_wBar)
			return;
		
		if (m_bActive)
		{
			AnimateWidget.Color(m_wBar, m_SliderColor, m_iColorChangeTime);
		}
		else
		{
			if (onHover)
			{
				AnimateWidget.Color(m_wBar, m_OnHoverInactiveColor, m_iColorChangeTime);
			}
			else
			{
				AnimateWidget.Color(m_wBar, m_SliderInactiveColor, m_iColorChangeTime);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void Hide()
	{
		if (m_wBar)
			m_wBar.SetOpacity(0);
		
		if (m_wSpacer)
			m_wSpacer.SetOpacity(0);
		
		if (m_wRoot)
			m_wRoot.SetOpacity(0);
	}
	
	//------------------------------------------------------------------------------------------------
	void Unhide()
	{
		if (m_wBar)
			m_wBar.SetOpacity(255);
		
		if (m_wSpacer)
			m_wSpacer.SetOpacity(255);
		
		if (m_wRoot)
			m_wRoot.SetOpacity(255);
	}
};