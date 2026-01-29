class SCR_SliderBloodAttributeComponent : SCR_SliderEditorAttributeUIComponent
{
	[Attribute("BloodUnconsciousBar", desc: "Image that shows when Character becomes conscious")]
	protected string m_sUnconsciousBarName;
	
	[Attribute("BloodUnconsciousBarTop", desc: "Set at the end of the bar")]
	protected string m_sUnconsciousBarTopName;
	
	[Attribute("BloodUnconsciousFillRight", desc: "Filler make sure the bar is set the correct size")]
	protected string m_sUnconsciousFillerRightName;
	
	[Attribute("1", desc: "Size of the BarTop")]
	protected float m_iUnconsciousBarTopSize;
	
	protected float m_fUnconsciousLevel;
	
	protected Widget m_UnconsciousBar;
	protected Widget m_UnconsciousBarTop;
	protected Widget m_UnconsciousFillerRight;
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{	
		m_UnconsciousBar = w.FindAnyWidget(m_sUnconsciousBarName);
		m_UnconsciousBarTop = w.FindAnyWidget(m_sUnconsciousBarTopName);
		m_UnconsciousFillerRight = w.FindAnyWidget(m_sUnconsciousFillerRightName);
		
		LayoutSlot.SetFillWeight(m_UnconsciousBarTop, m_iUnconsciousBarTopSize);
		
		if (!m_UnconsciousBar || !m_UnconsciousFillerRight || !m_UnconsciousBarTop)
			return;
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return;
		
		vector bloodData = var.GetVector();
		
		m_fUnconsciousLevel = bloodData[1] * 100;
		
		super.Init(w, attribute);
	}
	
	/*!
	Get what percentage the blood must be for the entity to go unconscious
	\return Percentage of blood for the entity to go unconscious
	*/
	float GetUnconsciousLevel()
	{
		return m_fUnconsciousLevel;
	}
	
	override void SetSliderSettings(SCR_BaseEditorAttributeVar var, SCR_BaseEditorAttributeEntrySlider sliderData)
	{
		super.SetSliderSettings(var, sliderData);
		UpdateBloodSlider();
	}
	
	//Sets a default state for the UI and var value if conflicting attribute
	override void SetVariableToDefaultValue(SCR_BaseEditorAttributeVar var)
	{	
		super.SetVariableToDefaultValue(var);
		UpdateBloodSlider();
	}
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{						
		super.SetFromVar(var);
		UpdateBloodSlider();
	}
	
	override bool OnChangeInternal(Widget w, int x, int y, bool finished)
	{		
		super.OnChangeInternal(w, x, y, finished);
		UpdateBloodSlider();
		return false;
	}
	
	protected void UpdateBloodSlider()
	{
		float value = m_SliderWidgetComponent.GetValue();
		
		if (value >= m_fUnconsciousLevel)
		{
			m_UnconsciousBarTop.SetVisible(true);
			LayoutSlot.SetFillWeight(m_UnconsciousBar, Math.Clamp(m_fUnconsciousLevel - m_iUnconsciousBarTopSize, 0, 100));
			LayoutSlot.SetFillWeight(m_UnconsciousFillerRight, Math.Clamp(100 - m_fUnconsciousLevel, 0, 100));
		}
		else 
		{
			if (value >= m_iUnconsciousBarTopSize)
			{
				m_UnconsciousBarTop.SetVisible(true);
				LayoutSlot.SetFillWeight(m_UnconsciousBar, Math.Clamp(value - m_iUnconsciousBarTopSize, 0 , 100));
				LayoutSlot.SetFillWeight(m_UnconsciousFillerRight, Math.Clamp(100 - value, 0, 100));
			}
			else 
			{
				m_UnconsciousBarTop.SetVisible(false);
				LayoutSlot.SetFillWeight(m_UnconsciousBar, Math.Clamp(value, 0 , 100));
				LayoutSlot.SetFillWeight(m_UnconsciousFillerRight, Math.Clamp(100 - value, 0, 100));
			}
		}
	}
};
