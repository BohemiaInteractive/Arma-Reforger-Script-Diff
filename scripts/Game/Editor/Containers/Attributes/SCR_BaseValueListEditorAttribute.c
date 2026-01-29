/**
Attribute base for slider values for other attributes to inherent from
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_BaseValueListEditorAttribute: SCR_BaseEditorAttribute
{		
	[Attribute()]
	protected ref SCR_EditorAttributeBaseValues m_baseValues;
	
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		outEntries.Insert(new SCR_BaseEditorAttributeEntrySlider(m_baseValues));
		return outEntries.Count();
	}
};


[BaseContainerProps()]
class SCR_EditorAttributeBaseValues
{
	[Attribute(uiwidget: UIWidgets.LocaleEditBox, defvalue: "%1")]
	protected LocalizedString m_sSliderValueFormating;
	
	[Attribute()]
	protected float m_fMin;
	
	[Attribute(defvalue: "100")]
	protected float m_fMax;
	
	[Attribute(defvalue: "1")]
	protected float m_fStep;
	
	[Attribute(defvalue: "2")]
	protected int m_iDecimals;
	
	void GetSliderValues(out string sliderValueFormating, out float min, out float max, out float step, out int decimals)
	{
		sliderValueFormating = m_sSliderValueFormating;
		min = m_fMin;
		max = m_fMax;
		step = m_fStep;
		decimals = m_iDecimals;
	}
	
	float GetMaxValue()
	{
		return m_fMax;
	}
	
	void SetMaxValue(float maxValue)
	{
		m_fMax = maxValue;
	}
};