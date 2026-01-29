class SCR_FadeInOutEffect : SCR_BaseScreenEffect
{
	//Widget
	private ImageWidget 							m_wFadeInOut;

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_wFadeInOut = ImageWidget.Cast(m_wRoot.FindAnyWidget("FadeOut"));
	}

	//------------------------------------------------------------------------------------------------
	//! Fade the screen in/out
	//! \param fadeDirection fades out if true, in if false
	void FadeOutEffect(bool fadeDirection, float seconds = 0.35)
	{
		if (seconds < 0.1)
			return;
		
		AnimateWidget.StopAllAnimations(m_wFadeInOut);
		AnimateEffectVisibility(m_wFadeInOut, (int)fadeDirection, (int)fadeDirection, seconds, seconds);
	}

	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		if (m_wFadeInOut)
			HideSingleEffect(m_wFadeInOut);
	}
};
