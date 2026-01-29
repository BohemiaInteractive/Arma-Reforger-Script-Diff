class SCR_BloodTooltipDetailBlockEditorUIComponent : ScriptedWidgetComponent
{
	[Attribute()]
	protected ref array<ref SCR_BloodBoxColors> m_aBloodBoxColors;
	
	[Attribute()]
	protected ref Color m_cBorderColorDead;
	
	[Attribute("Background")]
	protected string m_sBloodBoxFillName;

	[Attribute("Border")]
	protected string m_sBloodBoxBorderName;
	
	protected Widget m_BloodBoxFill;
	protected Widget m_BloodBoxBorder;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] bloodStateFill
	//! \param[in] bloodStateBoder
	void SetBloodStateVisuals(int bloodStateFill, int bloodStateBoder)
	{
		if (!m_BloodBoxFill || !m_BloodBoxBorder)
			return;
		
		if (bloodStateFill < 0 || bloodStateFill >= m_aBloodBoxColors.Count() || bloodStateBoder >= m_aBloodBoxColors.Count())
			return;
		
		m_BloodBoxFill.SetColor(m_aBloodBoxColors[bloodStateFill].m_cFillColor);
		
		if (bloodStateBoder < 0)
			m_BloodBoxBorder.SetColor(m_cBorderColorDead);
		else 
			m_BloodBoxBorder.SetColor(m_aBloodBoxColors[bloodStateBoder].m_cBorderColor);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_BloodBoxFill = w.FindAnyWidget(m_sBloodBoxFillName);
		m_BloodBoxBorder = w.FindAnyWidget(m_sBloodBoxBorderName);
	}
}

[BaseContainerProps()]
class SCR_BloodBoxColors
{
	[Attribute()]
	ref Color m_cFillColor;

	[Attribute()]
	ref Color m_cBorderColor;
}
