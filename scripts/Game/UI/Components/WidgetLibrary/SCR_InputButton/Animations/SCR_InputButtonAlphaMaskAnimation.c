class SCR_InputButtonAlphaMaskAnimation : WidgetAnimationAlphaMask
{
	protected SCR_InputButtonDisplay m_InputButtonDisplay;

	protected ImageWidget m_wOutline;
	
	protected float m_fAnimationRate;
	protected bool m_bResetOnComplete;

	override void SetSpeed(float speed)
	{
		m_fSpeed = speed;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAnimationComplete()
	{
		if (m_bResetOnComplete)
		{
			SCR_InputButtonAnimations.ButtonColor(m_InputButtonDisplay.GetBackgroundWidget(), Color.FromInt(Color.WHITE), m_fAnimationRate);
			AnimateWidget.Opacity(m_wOutline, 0, m_fAnimationRate);
			m_InputButtonDisplay.AnimateHoldComplete();
			
			//! Wait for the animation to complete + 1 frame before setting the color back to default
			GetGame().GetCallqueue().CallLater(m_InputButtonDisplay.ResetColor, m_fAnimationRate * 300 + 1, false);
		}
		else
		{
			AnimateWidget.Color(m_InputButtonDisplay.GetBackgroundWidget(), Color.FromInt(Color.WHITE), m_fAnimationRate);
		}
			
	}

	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(float timeSlice)
	{
		m_fCurrentProgress += timeSlice * m_fSpeed;
		if (m_fCurrentProgress > 1)
			m_fCurrentProgress = 1;

		if (!m_InputButtonDisplay)
			return false;

		if (m_fSpeed > 0 && m_fCurrentProgress > 0)
			m_wOutline.SetColor(Color.FromInt(GUIColors.ENABLED.PackToInt()));
		else
			m_wOutline.SetColor(Color.FromInt(UIColors.WARNING.PackToInt()));

		if (m_fSpeed > 0 && m_fCurrentProgress < 0)
			return false;

		m_fValue = GetProgressValue(m_fCurrentProgress);

		// Perform widget animation
		Animate(m_fCurrentProgress == 1);

		if (m_fSpeed < 0 && m_fValue < 0.1)
			Stop();

		if (m_fValue >= m_fValueTarget)
		{
			Stop();
			OnAnimationComplete();
		}

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
	void SCR_InputButtonAlphaMaskAnimation(Widget w, float speed, float targetValue, bool resetOnComplete)
	{
		Widget parent = SCR_WidgetTools.FindWidgetInParents(w, WidgetType.SizeLayoutWidgetTypeID);
		m_InputButtonDisplay = SCR_InputButtonDisplay.Cast(parent.FindHandler(SCR_InputButtonDisplay));
		
		m_wOutline = m_InputButtonDisplay.GetOutlineWidget();
		
		if (!m_wOutline || !m_InputButtonDisplay.GetBackgroundWidget())
			return;

		m_fAnimationRate = m_InputButtonDisplay.m_fAnimationRate;
		m_bResetOnComplete = resetOnComplete;
		m_fValueTarget = targetValue;
		m_fValueDefault = m_wOutline.GetMaskProgress();
		m_wOutline.SetMaskProgress(0);
		m_wOutline.SetOpacity(1);
	}
}