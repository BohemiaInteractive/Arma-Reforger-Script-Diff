/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes
*/
class SCR_VectorSliderEditorAttributeUIComponent: SCR_SliderEditorAttributeUIComponent
{	
	//~ Gets the max slider setting from attributes rather then Slider Data
	protected override void SetSliderSettings(SCR_BaseEditorAttributeVar var, SCR_BaseEditorAttributeEntrySlider sliderData)
	{
		if (!var)
		{
			super.SetSliderSettings(var, sliderData);
			return;
		}
		
		m_SliderData = sliderData;
		
		float min, step;
		m_SliderData.GetSliderMinMaxStep(min, m_fMaxSliderValue, step);
		m_fMaxSliderValue = var.GetVector()[1];
		m_SliderWidgetComponent.SetSliderSettings(min, m_fMaxSliderValue, step, m_SliderData.GetSliderValueFormating());
		
		if (m_fDefaultValue == float.MAX)
			m_fDefaultValue = m_fMaxSliderValue;
	}
}