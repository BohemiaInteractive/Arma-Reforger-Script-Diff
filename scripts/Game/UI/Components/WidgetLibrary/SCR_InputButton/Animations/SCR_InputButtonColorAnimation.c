class SCR_InputButtonColorAnimation : WidgetAnimationColor
{
	ImageWidget m_wBackground;

	override bool OnUpdate(float timeSlice)
	{
		m_fCurrentProgress += timeSlice * m_fSpeed;
		if (m_fCurrentProgress > 1)
			m_fCurrentProgress = 1;

		if (m_fCurrentProgress < 0)
			return false;

		m_fValue = GetProgressValue(m_fCurrentProgress);

		//If value of AlphaMask progress is greater than 0.9, stop the animation
		if (m_fValue >= 0.9)
		{
			Stop();
			AnimateWidget.Color(m_wBackground, Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()), 2);
		}

		// Perform widget animation
		Animate(m_fCurrentProgress == 1);

		if (m_fCurrentProgress < 1)
			return false;

		if (m_bRepeat)
		{
			m_fCurrentProgress = 0;
			ReverseDirection();

			if (m_OnCycleCompleted)
				m_OnCycleCompleted.Invoke(this);

			return false;
		}

		// Invoke subscribed changes
		if (m_OnCompleted)
			m_OnCompleted.Invoke(this);

		if (m_OnStopped)
			m_OnStopped.Invoke(this);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_InputButtonColorAnimation(Widget w, float speed, Color targetColor)
	{
		m_wBackground = ImageWidget.Cast(w);
		m_ColorTarget = targetColor;
		m_ColorDefault = w.GetColor();
	}
}