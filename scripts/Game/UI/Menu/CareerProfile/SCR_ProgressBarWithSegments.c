//------------------------------------------------------------------------------------------------
class SCR_ProgressBarWithSegments : SCR_WLibProgressBarComponent
{	
	[Attribute(defvalue: "0.761 0.386 0.08 0.059", desc: "Progressed Bar Color")]
	protected ref Color m_iProgressedBarColor;
	
	[Attribute(defvalue: "0.761 0.484 0.199 1", desc: "Minus Bar Color")]
	protected ref Color m_iMinusBarColor;
	
	[Attribute("0.0")]
	protected float m_fProgressed;
	
	[Attribute("0.0")]
	protected float m_fMinus;
	
	
	protected Widget m_wProgressedBar;
	protected Widget m_wMinusBar;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wRoot.SetVisible(false);
		
		m_wProgressedBar = w.FindAnyWidget("ProgressedBar");
		m_wMinusBar = w.FindAnyWidget("MinusBar");
		
		if (m_wProgressedBar)
		{
			m_wProgressedBar.SetColor(m_iProgressedBarColor);
			if (m_bReverseDirection)
				m_wBar.SetZOrder(1);
		}
		
		if (m_wMinusBar)
		{
			m_wMinusBar.SetColor(m_iMinusBarColor);
			if (m_bReverseDirection)
				m_wBar.SetZOrder(1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void UpdateVisuals(bool animate = true)
	{
		if (!m_wSpacer || !m_wBar || !m_wProgressedBar || !m_wMinusBar)
			return;
		
		float original,  progressed, minus;
		
		float range = m_fMax - m_fMin;
		if (range <= 0)
			return;
		
		original = m_fValue / range;
		progressed = m_fProgressed / range;
		minus = m_fMinus / range;
		
		original = Math.Clamp(original, 0, 1);
		progressed = Math.Clamp(progressed, 0, 1);
		minus = Math.Clamp(minus, 0, 1);
		
		HorizontalLayoutSlot.SetFillWeight(m_wBar, original);
		HorizontalLayoutSlot.SetFillWeight(m_wSpacer, 1 - original - progressed - minus);
		
		if (progressed < original/1000)
		{
			m_wProgressedBar.SetVisible(false);
			m_wProgressedBar.SetEnabled(false);
		}
		else
		{
			m_wProgressedBar.SetVisible(true);
			m_wProgressedBar.SetEnabled(true);
			HorizontalLayoutSlot.SetFillWeight(m_wProgressedBar, progressed);
		}
		
		if (minus < original/1000)
		{
			m_wMinusBar.SetVisible(false);
			m_wMinusBar.SetEnabled(false);
		}
		else
		{
			m_wMinusBar.SetVisible(true);
			m_wMinusBar.SetEnabled(true);
			HorizontalLayoutSlot.SetFillWeight(m_wMinusBar, minus);
		}
		
		m_wRoot.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetProgressBarValues (float value, float progressed, float minus)
	{
		m_fValue = value;
		m_fProgressed = progressed;
		m_fMinus = minus;
		UpdateVisuals();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetValue(float value, bool animate = true)
	{
		super.SetValue(value, animate);
		
		if (m_wProgressedBar){
			m_wProgressedBar.SetVisible(false);
			m_wProgressedBar.SetEnabled(false);
		}
		
		if (m_wMinusBar){
			m_wMinusBar.SetVisible(false);
			m_wMinusBar.SetEnabled(false);
		}
	}
};