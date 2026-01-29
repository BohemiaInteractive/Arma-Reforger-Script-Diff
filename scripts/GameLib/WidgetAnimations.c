//------------------------------------------------------------------------------------------------
class WidgetAnimationBlurIntensity : WidgetAnimationBase
{
	protected float m_fValueDefault;
	protected float m_fValueTarget;
	protected float m_fValueCurrent;
	protected ImageWidget m_wImage;
		
	//------------------------------------------------------------------------------------------------
	protected override void Animate(bool finished)
	{
		if (!m_wWidget)
			return;
		
		BlurWidget blurWidget = BlurWidget.Cast(m_wWidget);
		if (!blurWidget)
			return;
		
		m_fValueCurrent = Math.Lerp(m_fValueDefault, m_fValueTarget, m_fValue);
		blurWidget.SetIntensity(m_fValueCurrent);
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] w
	//! \param[in] speed
	//! \param[in] targetValue
	void WidgetAnimationBlurIntensity(Widget w, float speed, float targetValue)
	{
		m_fValueTarget = targetValue;
		m_fValueDefault = 0;
	}
};
//------------------------------------------------------------------------------------------------
class WidgetAnimationOpacity : WidgetAnimationBase
{
	protected float m_fValueDefault;
	protected float m_fValueTarget;
	protected float m_fValueCurrent;
	protected bool m_bChangeVisibleFlag;

	//------------------------------------------------------------------------------------------------
	// Expose target opacity value so it can be adjusted while the animation is running
	void SetTargetValue(float targetValue)
	{
		m_fValueTarget = targetValue;
	}	

	//------------------------------------------------------------------------------------------------
	// Get target opacity value from running animation
	float GetTargetValue()
	{
		return m_fValueTarget;
	}		
			
	//------------------------------------------------------------------------------------------------
	protected override void ReverseDirection()
	{
		float f = m_fValueDefault;
		m_fValueDefault = m_fValueTarget;
		m_fValueTarget = f;
		m_fValue = 0;
	}

	//------------------------------------------------------------------------------------------------
	protected override void Animate(bool finished)
	{
		if (!m_wWidget)
			return;
		
		m_fValueCurrent = Math.Lerp(m_fValueDefault, m_fValueTarget, m_fValue);
		m_wWidget.SetOpacity(m_fValueCurrent);
		
		if (finished && m_bChangeVisibleFlag)
			m_wWidget.SetVisible(m_fValueCurrent != 0);
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationOpacity(Widget w, float speed, float targetValue, bool toggleVisibility = false)
	{
		m_fValueTarget = targetValue;
		m_fValueDefault = w.GetOpacity();
		m_bChangeVisibleFlag = toggleVisibility;
		
		if (toggleVisibility)
			w.SetVisible(true);
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationPadding : WidgetAnimationBase
{
	protected float m_fValueDefault[4];
	protected float m_fValueTarget[4];
	protected bool m_bIsGrid;
	
	//------------------------------------------------------------------------------------------------
	protected override void ReverseDirection()
	{
		float f[4] = {m_fValueDefault[0], m_fValueDefault[1], m_fValueDefault[2], m_fValueDefault[3]};
		m_fValueDefault = m_fValueTarget;
		m_fValueTarget = f;
	}

	//------------------------------------------------------------------------------------------------
	protected override void Animate(bool finished)
	{
		if (!m_wWidget)
			return;
		
		float l = Math.Lerp(m_fValueDefault[0], m_fValueTarget[0], m_fValue);
		float t = Math.Lerp(m_fValueDefault[1], m_fValueTarget[1], m_fValue);
		float r = Math.Lerp(m_fValueDefault[2], m_fValueTarget[2], m_fValue);
		float b = Math.Lerp(m_fValueDefault[3], m_fValueTarget[3], m_fValue);
		
		if (m_bIsGrid)
			GridSlot.SetPadding(m_wWidget, l, t, r, b);
		else
			AlignableSlot.SetPadding(m_wWidget, l, t, r, b);
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationPadding(Widget w, float speed, float padding[4])
	{
		m_fValueTarget = padding;

		float l,t,r,b;
		m_bIsGrid = GridLayoutWidget.Cast(w);
		if (m_bIsGrid)
			GridSlot.GetPadding(w, l,t,r,b);
		else
			AlignableSlot.GetPadding(w, l,t,r,b);

		m_fValueDefault = {l,t,r,b};
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationLayoutFill : WidgetAnimationBase
{
	protected float m_fValueDefault;
	protected float m_fValueTarget;
	protected float m_fValueCurrent;
	
	//------------------------------------------------------------------------------------------------
	protected override void ReverseDirection()
	{
		float f = m_fValueDefault;
		m_fValueDefault = m_fValueTarget;
		m_fValueTarget = f;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void Animate(bool finished)
	{
		if (!m_wWidget)
			return;
		
		m_fValueCurrent = Math.Lerp(m_fValueDefault, m_fValueTarget, m_fValue);
		LayoutSlot.SetFillWeight(m_wWidget, m_fValueCurrent);
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationLayoutFill(Widget w, float speed, float targetValue)
	{
		m_fValueTarget = targetValue;
		m_fValueDefault = LayoutSlot.GetFillWeight(w);
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationAlphaMask : WidgetAnimationBase
{
	protected float m_fValueDefault;
	protected float m_fValueTarget;
	protected float m_fValueCurrent;
	protected ImageWidget m_wImage;
	
	//------------------------------------------------------------------------------------------------
	protected override void ReverseDirection()
	{
		float f = m_fValueDefault;
		m_fValueDefault = m_fValueTarget;
		m_fValueTarget = f;
	}

	//------------------------------------------------------------------------------------------------
	protected override void Animate(bool finished)
	{
		if (!m_wImage)
			return;
		
		m_fValueCurrent = Math.Lerp(m_fValueDefault, m_fValueTarget, m_fValue);
		m_wImage.SetMaskProgress(m_fValueCurrent);
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationAlphaMask(Widget w, float speed, float targetValue)
	{
		m_wImage = ImageWidget.Cast(w);
		if (!m_wImage)
			return;

		m_fValueTarget = targetValue;
		m_fValueDefault = m_wImage.GetMaskProgress();
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationImageRotation : WidgetAnimationBase
{
	protected float m_fValueDefault;
	protected float m_fValueTarget;
	protected float m_fValueCurrent;
	protected ImageWidget m_wImage;
	
	//------------------------------------------------------------------------------------------------
	protected override void ReverseDirection()
	{
		float f = m_fValueDefault;
		m_fValueDefault = m_fValueTarget;
		m_fValueTarget = f;
	}

	//------------------------------------------------------------------------------------------------
	protected override void Animate(bool finished)
	{
		if (!m_wImage)
			return;
		
		m_fValueCurrent = Math.Lerp(m_fValueDefault, m_fValueTarget, m_fValue);
		m_wImage.SetRotation(m_fValueCurrent);
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationImageRotation(Widget w, float speed, float targetValue)
	{
		m_wImage = ImageWidget.Cast(w);
		if (!m_wImage)
			return;
		
		m_fValueTarget = targetValue;
		m_fValueDefault = m_wImage.GetRotation();
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationColor : WidgetAnimationBase
{
	protected ref Color m_ColorDefault;
	protected ref Color m_ColorTarget;
	protected ref Color m_ColorCurrent;
	
	//------------------------------------------------------------------------------------------------
	protected override void ReverseDirection()
	{
		Color color = new Color(m_ColorDefault.R(), m_ColorDefault.G(), m_ColorDefault.B(), m_ColorDefault.A());
		m_ColorDefault = m_ColorTarget;
		m_ColorTarget = color;
		m_fValue = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void Animate(bool finished)
	{
		if (!m_wWidget)
			return;
		
		m_ColorCurrent = m_ColorDefault.LerpNew(m_ColorTarget, m_fValue);
		m_wWidget.SetColor(m_ColorCurrent);
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationColor(Widget w, float speed, Color targetColor)
	{
		m_ColorTarget = targetColor;
		m_ColorDefault = w.GetColor();
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationFrameSize : WidgetAnimationBase
{
	protected float m_fValueDefault[2];
	protected float m_fValueTarget[2];
	protected float m_fValueCurrent[2];
	
	//------------------------------------------------------------------------------------------------
	protected override void ReverseDirection()
	{
		float f[2] = {m_fValueDefault[0], m_fValueDefault[1]};
		m_fValueDefault = m_fValueTarget;
		m_fValueTarget = f;
		m_fValue = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void Animate(bool finished)
	{
		if (!m_wWidget)
			return;
		
		m_fValueCurrent[0] = Math.Lerp(m_fValueDefault[0], m_fValueTarget[0], m_fValue);
		m_fValueCurrent[1] = Math.Lerp(m_fValueDefault[1], m_fValueTarget[1], m_fValue);
		FrameSlot.SetSize(m_wWidget, m_fValueCurrent[0], m_fValueCurrent[1]);
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationFrameSize(Widget w, float speed, float size[2])
	{
		m_fValueTarget = size;
		m_fValueDefault[0] = FrameSlot.GetSizeX(w);
		m_fValueDefault[1] = FrameSlot.GetSizeY(w);
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationPosition : WidgetAnimationBase
{
	protected float m_fValueDefault[2];
	protected float m_fValueTarget[2];
	protected float m_fValueCurrent[2];
	
	//------------------------------------------------------------------------------------------------
	protected override void ReverseDirection()
	{
		float f[2] = {m_fValueDefault[0], m_fValueDefault[1]};
		m_fValueDefault = m_fValueTarget;
		m_fValueTarget = f;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void Animate(bool finished)
	{
		if (!m_wWidget)
			return;
		
		m_fValueCurrent[0] = Math.Lerp(m_fValueDefault[0], m_fValueTarget[0], m_fValue);
		m_fValueCurrent[1] = Math.Lerp(m_fValueDefault[1], m_fValueTarget[1], m_fValue);
		FrameSlot.SetPos(m_wWidget, m_fValueCurrent[0], m_fValueCurrent[1]);
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationPosition(Widget w, float speed, float position[2])
	{
		m_fValueTarget = position;
		
		m_fValueDefault[0] = FrameSlot.GetPosX(w);
		m_fValueDefault[1] = FrameSlot.GetPosY(w);
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationImageSaturation : WidgetAnimationBase
{
	protected float m_fValueDefault;
	protected float m_fValueTarget;
	protected float m_fValueCurrent;
	protected ImageWidget m_wImage;
	
	//------------------------------------------------------------------------------------------------
	protected override void ReverseDirection()
	{
		float f = m_fValueDefault;
		m_fValueDefault = m_fValueTarget;
		m_fValueTarget = f;
	}

	//------------------------------------------------------------------------------------------------
	protected override void Animate(bool finished)
	{
		if (!m_wImage)
			return;
		
		m_fValueCurrent = Math.Lerp(m_fValueDefault, m_fValueTarget, m_fValue);
		m_wImage.SetSaturation(m_fValueCurrent);
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationImageSaturation(Widget w, float speed, float targetValue)
	{
		m_wImage = ImageWidget.Cast(w);
		if (!m_wImage)
			return;
		
		m_fValueTarget = targetValue;
		m_fValueDefault = m_wImage.GetSaturation();
	}
};

//------------------------------------------------------------------------------------------------
class WidgetAnimationValue : WidgetAnimationBase
{
	protected float m_fValueDefault;
	protected float m_fValueTarget;
	protected float m_fValueCurrent;
	
	//------------------------------------------------------------------------------------------------
	protected override void ReverseDirection()
	{
		float f = m_fValueDefault;
		m_fValueDefault = m_fValueTarget;
		m_fValueTarget = f;
	}

	//------------------------------------------------------------------------------------------------
	protected override void Animate(bool finished)
	{
		m_fValueCurrent = Math.Lerp(m_fValueDefault, m_fValueTarget, m_fValue);
	}
	
	//------------------------------------------------------------------------------------------------
	void WidgetAnimationValue(Widget w, float speed, float targetValue, float defaultValue)
	{
		m_fValueTarget = targetValue;
		m_fValueDefault = defaultValue;
	}
};