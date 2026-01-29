//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Does not inherit from wlib base classes, duplicates code instead.

//! Minimalist progress bar
//------------------------------------------------------------------------------------------------
class SCR_WLibProgressBarComponent : ScriptedWidgetComponent
{
	protected const string WIDGET_CHANGE = "Change";
	
	[Attribute("0")]
	protected float m_fMin;

	[Attribute("1")]
	protected float m_fMax;

	[Attribute("0.5")]
	protected float m_fValue;
	
	[Attribute("false")]
	protected bool m_bReverseDirection;

	[Attribute("true", desc: "Use constant speed of the animation. If false, every change will take the same time.")]
	protected bool m_bConstantAnimationSpeed;
	
	[Attribute("0.0")]
	protected float m_fAnimationTime;
	
	[Attribute("0.760 0.392 0.078 1")]
	ref Color m_SliderColor;

	[Attribute("0 0 0 0.3")]
	ref Color m_BackgroundColor;

	protected Widget m_wRoot;
	protected Widget m_wBar;
	protected Widget m_wChange;
	protected Widget m_wSpacer;
	
	protected float m_fChange;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_wSpacer = w.FindAnyWidget("Spacer");
		m_wChange = w.FindAnyWidget(WIDGET_CHANGE);
		m_wBar = w.FindAnyWidget("Bar");
		
		if (m_wBar)
		{
			m_wBar.SetColor(m_SliderColor);
			if (m_bReverseDirection)
				m_wBar.SetZOrder(1);
		}
		
		Widget background = w.FindAnyWidget("Background");
		if (background)
			background.SetColor(m_BackgroundColor);
		
		UpdateVisuals(false);
	}

	//------------------------------------------------------------------------------------------------
	void SetMin(float value)
	{
		m_fMin = value;
		UpdateVisuals();
	}

	//------------------------------------------------------------------------------------------------
	void SetMax(float value)
	{
		m_fMax = value;
		UpdateVisuals();
	}

	//------------------------------------------------------------------------------------------------
	void SetValue(float value, bool animate = true)
	{
		m_fValue = value;
		UpdateVisuals(animate);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetChange(float value)
	{
		m_fChange = value;
		UpdateVisuals(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateVisuals(bool animate = true)
	{
		if (!m_wSpacer || !m_wBar)
			return;
		
		float progress, currentProgress, change;
		float range = m_fMax - m_fMin;
		if (range <= 0)
			return;
		
		currentProgress = HorizontalLayoutSlot.GetFillWeight(m_wBar);
		progress = m_fValue / range;
		change = m_fChange / range;
		progress = Math.Clamp(progress, 0, 1);

		if (animate && m_fAnimationTime > 0 && currentProgress != progress)
		{
			float speed = 1 / m_fAnimationTime;
			if (m_bConstantAnimationSpeed)
				speed = speed * (1 / Math.AbsFloat(currentProgress - progress));
			
			AnimateWidget.LayoutFill(m_wBar, progress, speed);
			AnimateWidget.LayoutFill(m_wChange, change, speed);
			AnimateWidget.LayoutFill(m_wSpacer, 1 - progress - change, speed);
		}
		else
		{
			HorizontalLayoutSlot.SetFillWeight(m_wBar, progress);
			HorizontalLayoutSlot.SetFillWeight(m_wChange, change);
			HorizontalLayoutSlot.SetFillWeight(m_wSpacer, 1 - progress - change);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void StopProgressAnimation()
	{
		AnimateWidget.StopAnimation(m_wBar, WidgetAnimationLayoutFill);
		AnimateWidget.StopAnimation(m_wSpacer, WidgetAnimationLayoutFill);
		float progress = HorizontalLayoutSlot.GetFillWeight(m_wBar);
		float value = progress * m_fMax - m_fMin;
		SetValue(value, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSliderColor(Color newColor)
	{
		m_SliderColor = newColor;
		m_wBar.SetColor(m_SliderColor);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSliderChangeColor(Color newColor)
	{
		if (m_wChange)
			m_wChange.SetColor(newColor);
	}

	//------------------------------------------------------------------------------------------------
	float GetMin()
	{
		return m_fMin;
	}

	//------------------------------------------------------------------------------------------------
	float GetMax()
	{
		return m_fMax;
	}

	//------------------------------------------------------------------------------------------------
	float GetValue()
	{
		return m_fValue;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetAnimationSpeed()
	{
		return m_fAnimationTime;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsUsingAnimationTime() 
	{
		return m_bConstantAnimationSpeed;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetUsingAnimationTime(bool enable)
	{
		m_bConstantAnimationSpeed = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAnimationTime(float value)
	{
		m_fAnimationTime = value;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_WLibProgressBarComponent GetProgressBar(string name, Widget parent, bool searchAllChildren = true)
	{
		if (!parent || name == string.Empty)
			return null;

		Widget w;
		if (searchAllChildren)
			w = parent.FindAnyWidget(name);
		else
			w = parent.FindWidget(name);

		if (!w)
		{
			Print(string.Format("SCR_WLibProgressBarComponent.GetProgressBar: widget not found: %1", name), LogLevel.WARNING);
			Debug.DumpStack();
			return null;
		}

		return SCR_WLibProgressBarComponent.Cast(w.FindHandler(SCR_WLibProgressBarComponent));
	}
};

//---- REFACTOR NOTE END ----