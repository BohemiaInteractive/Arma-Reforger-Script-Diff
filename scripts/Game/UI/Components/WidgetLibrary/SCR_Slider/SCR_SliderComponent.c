//------------------------------------------------------------------------------------------------
class SCR_SliderComponent : SCR_ChangeableComponentBase
{
	[Attribute("0.5")]
	protected float m_fValue;

	[Attribute("0")]
	protected float m_fMinValue;

	[Attribute("1")]
	protected float m_fMaxValue;

	[Attribute("0.05")]
	protected float m_fStep;

	[Attribute("%1", UIWidgets.LocaleEditBox, "Localization friendly string: %1 shows primary value, %2 shows secondary value. It can be overwritten by SetValueString() function")]
	protected string m_sFormatText;

	[Attribute("1", UIWidgets.EditBox, "Multiplies the value in displayed text.\nWith 100 and percentage format, value 1 is visualized as 100%")]
	protected float m_fShownValueMultiplier;
	
	[Attribute("1", UIWidgets.EditBox, "Multiplies the value in displayed text.\nWith 100 and percentage format, value 1 is visualized as 100%")]
	protected float m_fShownSecondaryMultiplier;

	[Attribute("1", UIWidgets.EditBox, "Multiplies the internal value for primary value display before offset is added.")]
	protected float m_fValueMultiplier;

	[Attribute("1", UIWidgets.EditBox, "Multiplies the internal value for secondary value display before offset is added.")]
	protected float m_fSecondaryMultiplier;

	[Attribute("0", UIWidgets.EditBox, "Offsets the internal value for primary value display.\nWith 1, value 1 is read as 2")]
	protected float m_fValueOffset;

	[Attribute("0", UIWidgets.EditBox, "Offsets the internal value for secondary value display.\nWith 1, value 1 is read as 2")]
	protected float m_fSecondaryOffset;

	[Attribute("0", UIWidgets.CheckBox, "Should the value text be clamped to Min/Max value after offset is added?")]
	protected bool m_bClampValue;

	[Attribute("0", UIWidgets.CheckBox, "Should the value text be rounded?")]
	protected bool m_bRoundValue;	
	
	[Attribute("0")]
	protected int m_iDecimalPrecision;

	[Attribute(SCR_SoundEvent.SOUND_FE_ITEM_CHANGE)]
	protected string m_sChangeSound;

	protected TextWidget m_wText;
	protected SliderWidget m_wSlider;
	protected float m_fOldValue;

	protected ref SCR_EventHandlerComponent m_Handler;
	protected ref ScriptInvoker m_OnChangedFinal;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wSlider = SliderWidget.Cast(w.FindAnyWidget("Slider"));
		m_wText = TextWidget.Cast(w.FindAnyWidget("SliderText"));

		if (!m_wSlider)
			return;

		// Get slider handling
		m_Handler = SCR_EventHandlerComponent.Cast(m_wSlider.FindHandler(SCR_EventHandlerComponent));
		if (!m_Handler)
			return;

		SetSliderSettings(m_fMinValue, m_fMaxValue, m_fStep, m_sFormatText);

		m_Handler.GetOnFocus().Insert(OnSliderFocus);
		m_Handler.GetOnFocusLost().Insert(OnSliderFocusLost);
		
		m_Handler.GetOnChange().Insert(OnValueChanged);
		m_Handler.GetOnChangeFinal().Insert(OnValueFinal);

		UpdateValue();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		m_Handler.GetOnChange().Remove(OnValueChanged);
		m_Handler.GetOnChangeFinal().Remove(OnValueFinal);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		// Do not focus parent, call the super function on slider focus

		// Set focus to handler
		GetGame().GetWorkspace().SetFocusedWidget(m_wSlider);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnValueChanged(Widget w)
	{
		float value = UpdateValue();

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Can lead to severe sound spamming based on speed of user interaction
		
		if (m_sChangeSound != string.Empty && !float.AlmostEqual(m_fOldValue, value))
			PlaySound(m_sChangeSound);
		
//---- REFACTOR NOTE END ----
		
		m_fOldValue = value;
		
		m_OnChanged.Invoke(this, value);
	}

	//------------------------------------------------------------------------------------------------
	protected float UpdateValue()
	{
		float value;
		if (m_wSlider)
			value = m_wSlider.GetCurrent();

		// Update displayed text, optional secondary value display
		if (m_wText && m_wText.IsVisible())
		{
			float value1 = value * m_fValueMultiplier + m_fValueOffset;
			float value2 = value * m_fSecondaryMultiplier + m_fSecondaryOffset;

			if (m_bClampValue)
			{
				value1 = Math.Clamp(value1, m_fMinValue, m_fMaxValue);
				value2 = Math.Clamp(value2, m_fMinValue, m_fMaxValue);
			}

			if (m_bRoundValue)
			{
				value1 = RoundValue(value1 * m_fShownValueMultiplier, m_iDecimalPrecision);
				value2 = RoundValue(value2 * m_fShownSecondaryMultiplier, m_iDecimalPrecision);
			}

			m_wText.SetTextFormat(m_sFormatText, value1, value2);
		}

		return value;
	}

	//------------------------------------------------------------------------------------------------
	protected float RoundValue(float value, int precision)
	{
		if (!m_bRoundValue)
			return value;
		
		float coef = Math.Pow(10, precision);
		value = Math.Round(value * coef) / coef;

		return value;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void OnValueFinal(Widget w)
	{
		float value;
		if (m_wSlider)
			value = m_wSlider.GetCurrent();

		if (m_OnChangedFinal)
			m_OnChangedFinal.Invoke(this, value);
	}

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Solutions such as these feel weird given the current widget event system. There's probably a better setup
	
	//------------------------------------------------------------------------------------------------
	protected void OnSliderFocus()
	{
		m_wRoot.SetFlags(WidgetFlags.NOFOCUS);

		super.OnFocus(m_wRoot, 0, 0); // Emulate focus on a parent class
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSliderFocusLost()
	{
		m_wRoot.ClearFlags(WidgetFlags.NOFOCUS);

		super.OnFocusLost(m_wRoot, 0, 0); // Emulate focus on a parent class
	}


//---- REFACTOR NOTE END ----
	
	// User API
	//------------------------------------------------------------------------------------------------
	void SetValue(float value)
	{
		if (!m_wSlider)
			return;

		m_fValue = value;
		m_wSlider.SetCurrent(value);
		OnValueChanged(m_wSlider);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnChangedFinal()
	{
		if (!m_OnChangedFinal)
			m_OnChangedFinal = new ScriptInvoker();
		return m_OnChangedFinal;
	}

	//------------------------------------------------------------------------------------------------
	float GetValue()
	{
		return m_wSlider.GetCurrent();
	}

	//------------------------------------------------------------------------------------------------
	void SetFormatText(string text)
	{
		m_sFormatText = text;
	}

	//------------------------------------------------------------------------------------------------
	string GetFormatText()
	{
		return m_sFormatText;
	}

	//------------------------------------------------------------------------------------------------
	void ShowCustomValue(string value)
	{
		if (!m_wText || !m_wText.IsVisible())
			return;

		m_wText.SetTextFormat(m_sFormatText, value);
	}

	//------------------------------------------------------------------------------------------------
	void SetSliderSettings(float min, float max, float step, string formatText = string.Empty)
	{
		m_wSlider.SetMin(min);
		m_wSlider.SetMax(max);
		m_wSlider.SetStep(step);
		if (formatText != string.Empty)
			m_sFormatText = formatText;
	}

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// m_wSlider could be null
	
	//------------------------------------------------------------------------------------------------
	void SetMin(float min)
	{
		m_wSlider.SetMin(min);
	}

	//------------------------------------------------------------------------------------------------
	void SetMax(float max)
	{
		m_wSlider.SetMax(max);
	}

	//------------------------------------------------------------------------------------------------
	void SetStep(float step)
	{
		m_wSlider.SetStep(step);
	}

//---- REFACTOR NOTE END ----
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// the error case is a valid value: why not return -1 instead?
	
	//------------------------------------------------------------------------------------------------
	float GetMin()
	{
		if (!m_wSlider)
			return 0;

		return m_wSlider.GetMin();
	}

	//------------------------------------------------------------------------------------------------
	float GetMax()
	{
		if (!m_wSlider)
			return 0;

		return m_wSlider.GetMax();
	}

	//------------------------------------------------------------------------------------------------
	float GetStep()
	{
		if (!m_wSlider)
			return 0;

		return m_wSlider.GetStep();
	}

//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	void SetShownValueMultiplier(float multiplier)
	{
		m_fShownValueMultiplier = multiplier;
	}

	//------------------------------------------------------------------------------------------------
	float GetShownValueMultiplier()
	{
		return m_fShownValueMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_SliderComponent GetSliderComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		return SCR_SliderComponent.Cast(SCR_ScriptedWidgetComponent.GetComponent(SCR_SliderComponent, name, parent, searchAllChildren));
	}
};
