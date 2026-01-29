//! This class helps with managing fade in and fade out animations properly:
//! You can call FadeIn() once and it will fade in
//! Then it will stay for a fixed amount of time in this state
//! Later it will start the fade out animation and stay in this state
//! Or you can call ForceVisible(true/false) periodically and it will handle the transitions itself
//! ! Remember to call Update() yourself ! It won't get called automatically.

class SCR_FadeInOutAnimator
{
	// Member variables
	protected Widget m_Widget;
	protected int m_iState;
	protected int m_iFormerState
	protected float m_fFadeOutWaitTime;
	protected float m_fFadeInSpeed;
	protected float m_fFadeOutSpeed;
	protected bool m_bForceVisible;
	protected float m_fTimer;
	protected bool m_bFadeOutSetVisibleFalse; // When fade out is done, widget will be SetVisible(false)
	protected bool m_bAutoFadeOut;	// Should it fade out after the 'fadeOutWaitTime' or not (default: true)
	
	// States
	const int STATE_FADE_IN = 0;
	const int STATE_FADE_OUT_WAIT = 1;
	const int STATE_FADE_OUT = 2;
	const int STATE_FADING_DONE = 3;
	
	//invokers
	ref ScriptInvoker<int, int> m_OnStateChanged;
	
	//-----------------------------------------------------
	void SCR_FadeInOutAnimator(Widget w, float fadeInSpeed, float fadeOutSpeed, float fadeOutWaitTime, bool fadeOutSetVisibleFalse = false)
	{
		m_Widget = w;
		m_fFadeOutSpeed = fadeOutSpeed;
		m_fFadeInSpeed = fadeInSpeed;
		m_fFadeOutWaitTime = fadeOutWaitTime;
		m_bFadeOutSetVisibleFalse = fadeOutSetVisibleFalse;
		m_bAutoFadeOut = true;
		
		m_iState = STATE_FADE_OUT_WAIT;
		m_bForceVisible = false;
	}

	//-----------------------------------------------------
	void Update(float timeSlice)
	{		
		switch (m_iState)
		{
			case STATE_FADING_DONE:
				
				if (m_bForceVisible && m_Widget.GetOpacity() < 1)
				{
					m_iState = STATE_FADE_IN;
					if (m_OnStateChanged)
						m_OnStateChanged.Invoke(STATE_FADING_DONE, m_iState);
				}
			
				break;
			
			case STATE_FADE_IN:
			
				float opacity = Math.Clamp(m_Widget.GetOpacity() + timeSlice * m_fFadeInSpeed, 0 ,1);
				m_Widget.SetOpacity(opacity);
				m_Widget.SetVisible(true);
			
				if (opacity >= 1)
				{
					if (m_bAutoFadeOut)
					{
						m_fTimer = m_fFadeOutWaitTime;	
						m_iState = STATE_FADE_OUT_WAIT;
						if (m_OnStateChanged)
							m_OnStateChanged.Invoke(STATE_FADE_IN, m_iState);
					}
					else
					{
						m_iState = STATE_FADING_DONE;
						if (m_OnStateChanged)
							m_OnStateChanged.Invoke(STATE_FADE_IN, m_iState);
					}
				}
				
				break;
			
			case STATE_FADE_OUT_WAIT:
 
				if (m_bForceVisible)
				{
					m_fTimer = m_fFadeOutWaitTime;
				}
				else
				{
					m_fTimer -= timeSlice;
				
					if (m_fTimer <= 0)
					{
						m_iState = STATE_FADE_OUT;
						if (m_OnStateChanged)
							m_OnStateChanged.Invoke(STATE_FADE_OUT_WAIT, m_iState);
					}
				}
				break;
			
			case STATE_FADE_OUT:
				
				if (m_bForceVisible)
				{
					m_iState = STATE_FADE_IN;
					if (m_OnStateChanged)
						m_OnStateChanged.Invoke(STATE_FADE_OUT, m_iState);
				}
				else
				{
					float opacity = Math.Clamp(m_Widget.GetOpacity() - m_fFadeOutSpeed*timeSlice, 0, 1);
					m_Widget.SetOpacity(opacity);
				
					if (opacity <= 0)
					{
						if (m_bFadeOutSetVisibleFalse)
							m_Widget.SetVisible(false);
					
						m_iState = STATE_FADING_DONE;
						if (m_OnStateChanged)
							m_OnStateChanged.Invoke(STATE_FADE_OUT, m_iState);
					}
				}
				break;
		}
	}
	
	//-----------------------------------------------------
	void FadeIn(bool fadeOut = true)
	{
		m_bAutoFadeOut = fadeOut;
		m_iFormerState = m_iState;
		m_iState = STATE_FADE_IN;
		if (m_OnStateChanged)		
			m_OnStateChanged.Invoke(m_iFormerState, m_iState);
	}

	//-----------------------------------------------------
	void FadeOut()
	{		
		m_iFormerState = m_iState;
		m_iState = STATE_FADE_OUT;
		if (m_OnStateChanged)
			m_OnStateChanged.Invoke(m_iFormerState, m_iState);
	}
	
	//-----------------------------------------------------
	void FadeOutInstantly()
	{
		m_Widget.SetOpacity(0);
		m_iFormerState = m_iState;
		m_iState = STATE_FADE_OUT;
		if (m_OnStateChanged)
			m_OnStateChanged.Invoke(m_iFormerState, m_iState);
		Update(0);
	}
	
	//-----------------------------------------------------
	void ForceVisible(bool force)
	{
		m_bForceVisible = force;
	}
	
	//-----------------------------------------------------
	//! Returns state. See STATE_ constants.
	int GetState()
	{
		return m_iState;
	}
	
	//-----------------------------------------------------
	//! Returns TRUE if animation is running.
	bool IsRunning()
	{
		return m_iState != STATE_FADING_DONE;
	}	
	
	ScriptInvoker GetOnStateChanged()
	{
		if (!m_OnStateChanged)
			m_OnStateChanged = ScriptInvoker();
		return m_OnStateChanged;
	}
};