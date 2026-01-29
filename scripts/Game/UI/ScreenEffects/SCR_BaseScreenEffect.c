class SCR_BaseScreenEffect : SCR_InfoDisplayExtended
{
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Called when any settings are changed in the settings menu, upon DisplayStartDraw()
	//! Called from SCR_ScreenEffectsManager. So not every effect has to listen for this individually
	//! Relevant when your effect needs to adapt when particular settings are changed, such as switching from normal DoF to Bokeh
	void SettingsChanged()
	{
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayUpdate(IEntity owner, float timeSlice)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Called every frame from SCR_ScreenEffectsManager
	void UpdateEffect(float timeSlice)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Automatically called upon DisplayStartDraw() and DisplayStopDraw() from SCR_ScreenEffectsManager
	//! Due to many screeneffects listening to invokers, ClearEffects() is often called manually OnControlledEntityChanged()
	void ClearEffects()
	{
	}

	//------------------------------------------------------------------------------------------------
	//! The widget will be made visible or hidden based on its opacity.
	protected void UpdateEffectVisibility(notnull ImageWidget effectWidget)
	{
		effectWidget.SetVisible(effectWidget.GetOpacity() != 0);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] effectWidget to animate its opacity and alpha mask.
	//! \param[in] opacity desired target value.
	//! \param[in] alpha desired target value.
	//! \param[in] opacitySpeed desired speed value (default is 0).
	//! \param[in] alphaSpeed desired speed value (default is 0).
	//! \param[in] updateVisibility Whether the visibility should be updated based on the final opacity (default is true).
	//! \param[in] updateVisibility Whether the visibility should be updated based on the final opacity (default is true).
	protected void AnimateEffectVisibility(notnull ImageWidget effectWidget, float opacity, float alpha, float opacitySpeed = 1.0, float alphaSpeed = 1.0, bool updateVisibility = true)
	{
		AnimateWidget.Opacity(effectWidget, opacity, opacitySpeed, updateVisibility);
		AnimateWidget.AlphaMask(effectWidget, alpha, alphaSpeed);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowSingleEffect(notnull ImageWidget effectWidget)
	{
		effectWidget.SetOpacity(1);
		effectWidget.SetMaskProgress(1);
		effectWidget.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void HideSingleEffect(notnull ImageWidget effectWidget)
	{
		AnimateWidget.StopAllAnimations(effectWidget);
		effectWidget.SetOpacity(0);
		effectWidget.SetMaskProgress(0);
		effectWidget.SetVisible(false);
	}
}
