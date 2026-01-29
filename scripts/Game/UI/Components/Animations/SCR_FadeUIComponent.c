class SCR_FadeUIComponent : ScriptedWidgetComponent
{
	[Attribute("", desc: "Leave empty if root is faded")]
	protected string m_sFadeWidgetName;
	
	[Attribute("1", desc: "The Opacitity target of the widget on fade in", params: "0 1")]
	protected float m_fFadeInOpacitytarget;
		
	[Attribute("2", params: "0 50")]
	protected float m_fFadeInAnimationSpeed;
	
	[Attribute("2", params: "0 50")]
	protected float m_fFadeOutAnimationSpeed;
	
	[Attribute("0", desc: "Fades the widget in on handler attached")]
	protected bool m_bAutoFadeIn;
	
	[Attribute("0.1", desc: "In seconds, Delay between each call que to check if animation is done", params: "0 10")]
	protected float m_bFadeDoneUpdateSpeed;

	protected Widget m_wFadeWidget;
	
	[Attribute("", "UI Sfx is played when on fade in starts. Leave empty to play no SFX")]
	protected string m_sOnFadeInSfx;
	
	[Attribute("0", desc: "Will terminate any other Sfx if true and played")]
	protected bool m_bFadeInSfxHasPriority;
	
	[Attribute("", "UI Sfx is played when on fade out starts. Leave empty to play no SFX")]
	protected string m_sOnFadeOutSfx;
	
	[Attribute("0", desc: "Will terminate any other Sfx if true and played")]
	protected bool m_bFadeOutSfxHasPriority;
	
	//state
	protected bool m_bIsWaitingForDelayedFadeIn;
	protected bool m_bIsWaitingForDelayedFadeOut;
	protected bool m_bIsFading;
	protected bool m_bIsFadingIn;
	protected bool m_bCheckingForFadeOutDestroy = false;
	
	protected ref ScriptInvoker Event_OnFadeDone; //Var SCR_FadeUIComponent, Bool isFadeIn
	
	//------------------------------------------------------------------------------------------------
	//! Fade in widget
	//! \param[in] setOpacityZero will set opacity to 0 if this is true. Else keeps current opacity
	void FadeIn(bool setOpacityZero = true)
	{	
		m_bIsWaitingForDelayedFadeIn = false;
		
		if (!m_wFadeWidget)
			return;
		
		if (!setOpacityZero && m_bIsFading && m_bIsFadingIn)
			return;

		m_bCheckingForFadeOutDestroy = false;
		
		if (setOpacityZero)
			m_wFadeWidget.SetOpacity(0);
		
		AnimateWidget.Opacity(m_wFadeWidget, m_fFadeInOpacitytarget, m_fFadeInAnimationSpeed, true);
		
		if (!m_sOnFadeInSfx.IsEmpty())
			SCR_UISoundEntity.SoundEvent(m_sOnFadeInSfx, m_bFadeInSfxHasPriority);
		
		StartListenToFadeDone(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fade in with delay (in miliseconds)
	//! \param[in] delay how long will the delay be fore fading
	//! \param[in]setOpacityZero will set opacity to 0 if this is true. Else keeps current opacity
	void DelayedFadeIn(int delay, bool setOpacityZero = true)
	{
		if (!m_wFadeWidget)
			return;
		
		m_bCheckingForFadeOutDestroy = false;

		if (setOpacityZero)
			m_wFadeWidget.SetOpacity(0);
		
		m_bIsWaitingForDelayedFadeIn = true;
		GetGame().GetCallqueue().CallLater(FadeIn, delay, false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fade out widget
	//! \param[in] destroyOnFadeOut will destroy widget if fade out animation is done
	void FadeOut(bool destroyOnFadeOut)
	{
		m_bIsWaitingForDelayedFadeOut = false;
		
		if (!m_wFadeWidget)
			return;
		
		m_bCheckingForFadeOutDestroy = destroyOnFadeOut;
		
		if (m_bIsFading && !m_bIsFadingIn)
			return;
		
		AnimateWidget.StopAllAnimations(m_wFadeWidget);
		AnimateWidget.Opacity(m_wFadeWidget, 0, m_fFadeOutAnimationSpeed, true);
		
		if (!m_sOnFadeOutSfx.IsEmpty())
			SCR_UISoundEntity.SoundEvent(m_sOnFadeOutSfx, m_bFadeOutSfxHasPriority);
		
		StartListenToFadeDone(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fade out with delay (in milliseconds)
	//! \param[in] delay how long will fading take
	//! \param[in] destroyOnFadeOut will destroy widget if fade out animation is done
	//! \param[in] setOpacityTofadeInTarget will set opacity to the given fade in amount if this is true. Else keeps current opacity
	void DelayedFadeOut(int delay, bool destroyOnFadeOut, bool setOpacityTofadeInTarget = true)
	{
		if (!m_wFadeWidget)
			return;

		if (setOpacityTofadeInTarget)
			m_wFadeWidget.SetOpacity(m_fFadeInOpacitytarget);
		
		m_bIsWaitingForDelayedFadeOut = true;
		GetGame().GetCallqueue().CallLater(FadeOut, delay, false, destroyOnFadeOut);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cancel current fade
	//! \param[in] callFadeDone if true it will still call the fade done event
	void CancelFade(bool callFadeDone)
	{
		if (m_bIsWaitingForDelayedFadeIn)
			GetGame().GetCallqueue().Remove(FadeIn);
		
		if (m_bIsWaitingForDelayedFadeOut)
			GetGame().GetCallqueue().Remove(FadeOut);
		
		if (!m_bIsFading)
			return;
		
		if (!callFadeDone)
		{
			m_bIsFading = false;
			GetGame().GetCallqueue().Remove(FadeDoneCheck);
		}
		
		if (m_bIsFadingIn)
			m_wFadeWidget.SetOpacity(m_fFadeInOpacitytarget);
		else if (!m_bIsFadingIn)
			m_wFadeWidget.SetOpacity(0);
		
		AnimateWidget.StopAllAnimations(m_wFadeWidget);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the fade in speed
	//! \param[in] fadeInSpeed new fade in speed
	void SetFadeInSpeed(float fadeInSpeed)
	{
		m_fFadeInAnimationSpeed = fadeInSpeed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Fade widget
	//! \return the widget to be faded
	Widget GetFadeWidget()
	{
		return m_wFadeWidget;
	}

	//------------------------------------------------------------------------------------------------
	//! \return if the widget is being faded by the this specific component
	bool IsFading()
	{
		return m_bIsFading;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if the currently fading and if the fade is a fade in, false otherwise
	bool IsFadingIn()
	{
		return m_bIsFadingIn && IsFading();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if the currently fading and if the fade is a fade out, false otherwise
	bool IsFadingOut()
	{
		return !m_bIsFadingIn && IsFading();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return fade in opacity target
	float GetTargetFadeInOpacity()
	{
		return m_fFadeInOpacitytarget;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Fade done Script Invoker
	//! \return script invoker triggering when fade is done. Vars: SCR_FadeUIComponent bool isFadeIn
	ScriptInvoker GetOnFadeDone()
	{
		if (!Event_OnFadeDone)
			Event_OnFadeDone = new ScriptInvoker();

		return Event_OnFadeDone;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void StartListenToFadeDone(bool isFadeIn)
	{
		m_bIsFadingIn = isFadeIn;
		
		if (!m_bIsFading)
		{
			m_bIsFading = true;
			GetGame().GetCallqueue().CallLater(FadeDoneCheck, m_bFadeDoneUpdateSpeed * 1000, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FadeDoneCheck()
	{
		if (AnimateWidget.IsAnimating(m_wFadeWidget))
			return;
		
		if (m_bIsFadingIn && m_wFadeWidget.GetOpacity() != m_fFadeInOpacitytarget)
			return;
		else if (!m_bIsFadingIn && m_wFadeWidget.GetOpacity() != 0)
			return;
		
		if (Event_OnFadeDone)
			Event_OnFadeDone.Invoke(this, m_bIsFadingIn);
		
		m_bIsFading = false;
		GetGame().GetCallqueue().Remove(FadeDoneCheck);
		
		if (!m_bIsFadingIn)
		{
			if (m_bCheckingForFadeOutDestroy)
				m_wFadeWidget.RemoveFromHierarchy();
			else 
				m_wFadeWidget.SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode()) 
			return;
		
		if (m_sFadeWidgetName.IsEmpty())
			m_wFadeWidget = w;
		else 
			m_wFadeWidget = w.FindAnyWidget(m_sFadeWidgetName);
		
		if (!m_wFadeWidget)
		{
			Print(string.Format("'SCR_FadeUIComponent' could not find widget '%1'", m_sFadeWidgetName), LogLevel.ERROR);
			return;
		}
		
		if (m_bAutoFadeIn)
			FadeIn();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_bIsFading && GetGame())
			GetGame().GetCallqueue().Remove(FadeDoneCheck);
	}
}
