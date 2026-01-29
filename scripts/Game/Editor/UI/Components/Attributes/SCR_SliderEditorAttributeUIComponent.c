/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes
*/
class SCR_SliderEditorAttributeUIComponent: SCR_BaseEditorAttributeUIComponent
{		
	protected SCR_SliderComponent m_SliderWidgetComponent;
	protected ref SCR_BaseEditorAttributeEntrySlider m_SliderData;
	protected float m_fMaxSliderValue;
	protected float m_fDefaultValue = float.MAX;
	
	protected bool m_bIsTimeSlider;
	//~ If value is 0 should it hide it or show it as 00. Eg if normally 23:05:19 (h:m:s) and s becomes 0 and is ticked here than it will be come 23:05
	protected ETimeFormatParam m_eHideIfZero;
	//~ If true than it will never show seconds even if seconds are not 0
	protected bool m_bAlwaysHideSeconds;
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{		
		//Set slider label
		Widget sliderWidgetLibary = w.FindAnyWidget(m_sUiComponentName);
		if (!sliderWidgetLibary) return;
		
		m_SliderWidgetComponent = SCR_SliderComponent.Cast(sliderWidgetLibary.FindHandler(SCR_SliderComponent));
		if (!m_SliderWidgetComponent)
			return;

		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (var)
		{
			array<ref SCR_BaseEditorAttributeEntry> entries = new array<ref SCR_BaseEditorAttributeEntry>;
			attribute.GetEntries(entries);
			
			foreach (SCR_BaseEditorAttributeEntry entry: entries)
			{
				SCR_BaseEditorAttributeEntryTimeSlider timeSlider = SCR_BaseEditorAttributeEntryTimeSlider.Cast(entry);
				if (timeSlider)
				{
					m_bIsTimeSlider = true;
					timeSlider.GetTimeSliderValues(m_eHideIfZero, m_bAlwaysHideSeconds);
					continue;
				}
				
				SCR_BaseEditorAttributeEntrySlider data = SCR_BaseEditorAttributeEntrySlider.Cast(entry);
				if (data)
				{
					m_SliderData = data;
					SetSliderSettings(var, m_SliderData);
					continue;
				}
				
				SCR_BaseEditorAttributeDefaultFloatValue defaultValue = SCR_BaseEditorAttributeDefaultFloatValue.Cast(entry);
				if (defaultValue)
				{
					m_fDefaultValue = defaultValue.GetDefaultFloatValue();
				}
			}
		}
				
		super.Init(w, attribute);
	}
	
	//~ Is overridden in SCR_VectorSliderEditorAttributeUIComponent
	protected void SetSliderSettings(SCR_BaseEditorAttributeVar var, SCR_BaseEditorAttributeEntrySlider sliderData)
	{
		m_SliderData = sliderData;
		
		float min, step;
		m_SliderData.GetSliderMinMaxStep(min, m_fMaxSliderValue, step);
		m_SliderWidgetComponent.SetSliderSettings(min, m_fMaxSliderValue, step, m_SliderData.GetSliderValueFormating());
		
		if (m_fDefaultValue == float.MAX)
			m_fDefaultValue = m_fMaxSliderValue;
	}
	
	//Sets a default state for the UI and var value if conflicting attribute
	override void SetVariableToDefaultValue(SCR_BaseEditorAttributeVar var)
	{	
		m_SliderWidgetComponent.SetValue(m_fDefaultValue);
		m_SliderWidgetComponent.ShowCustomValue(GetSliderValueText(m_fDefaultValue));
		
		if (var)
			var.SetFloat(m_fDefaultValue);
	}
	
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{						
		super.SetFromVar(var);
		
		if (!var)
			return;
		
		float value = var.GetFloat();
		
		m_SliderWidgetComponent.SetValue(value);
		m_SliderWidgetComponent.ShowCustomValue(GetSliderValueText(value));
		OnChangeInternal(null, 0, 0, false);
		
		if (value > m_fMaxSliderValue)
			PrintFormat("%1 attribute slider is set to '%2' but can only support up to '%3'", GetAttribute().GetUIInfo().GetName(), value.ToString(), m_fMaxSliderValue.ToString());
	}

	
	override bool OnChangeInternal(Widget w, int x, int y, bool finished)
	{		
		SCR_BaseEditorAttribute attribute = GetAttribute();
		if (!attribute) 
			return false;
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariable(true);
		if (!var) 
			return false;
		
		float value = m_SliderWidgetComponent.GetValue();
		m_SliderWidgetComponent.ShowCustomValue(GetSliderValueText(value));
		
		if (var.GetFloat() == value)
			return false;
		
		var.SetFloat(value);
		super.OnChangeInternal(w, x, y, finished);
		
		return false;
	}
	
	protected string GetSliderValueText(float value)
	{
		if (m_SliderData)
		{
			if (!m_bIsTimeSlider)
			{
				return m_SliderData.GetText(value);
			}
			//~ Time slider
			else 
			{
				if (!m_bAlwaysHideSeconds)
					return SCR_FormatHelper.GetTimeFormatting(value, m_eHideIfZero);
				else 
					return SCR_FormatHelper.GetTimeFormattingHideSeconds(value, m_eHideIfZero);
			}
		}
			
		return "MISSING m_SliderData!";
	}
};