class SCR_CampaignBuildingScreenEffectUIComponent : ScriptedWidgetComponent
{
	protected BlurWidget m_wBlurEffect;
	
	protected const float DURATION_IN_BLUR = 0.75;
	protected const float DURATION_OUT_BLUR = 0.5;
	
	protected const float VALUE_BLUR = 0.8;
	protected const float VALUE_DISABLED = 0.0;

	//------------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		m_wBlurEffect = BlurWidget.Cast(w.FindAnyWidget("ScreenEffectBlur"));
		
		SCR_CampaignBuildingEditorComponent CampaignBuildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!CampaignBuildingEditorComponent)
			return;
		
		CampaignBuildingEditorComponent.GetOnObstructionEventTriggered().Insert(AreaTriggerChange);
		
		if (CampaignBuildingEditorComponent.IsViewObstructed())
			AreaTriggerChange(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] activated
	protected void AreaTriggerChange(bool activated)
	{
		if (activated)
			StartObstructionAnimation();
		else
			FinishObstructionAnimation();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Start screen obstruction effect.
	protected void StartObstructionAnimation()
	{
		AnimateWidget.StopAnimation(m_wBlurEffect, WidgetAnimationBlurIntensity);
		AnimateWidget.BlurIntensity(m_wBlurEffect, VALUE_BLUR, DURATION_IN_BLUR);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Ends screen obstruction effect.
	protected void FinishObstructionAnimation()
	{
		AnimateWidget.StopAnimation(m_wBlurEffect, WidgetAnimationBlurIntensity);
		AnimateWidget.BlurIntensity(m_wBlurEffect, VALUE_DISABLED, DURATION_OUT_BLUR);
	}
};