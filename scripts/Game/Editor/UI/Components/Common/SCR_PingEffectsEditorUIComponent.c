class SCR_PingEffectsEditorUIComponent : ScriptedWidgetComponent
{
	[Attribute("PingBorder")]
	protected string m_sPingBorderName;
	
	[Attribute("PulseWidget")]
	protected string m_sPulseWidgetName;
	
	[Attribute("2")]
	protected float m_fPingBorderAnimationSpeed;
	
	[Attribute("1")]
	protected float m_fPulsateAnimationSpeed;
	
	[Attribute("5", desc: "The deletion is done with a seperate timer in SCR_PingEditorComponent")]
	protected int m_iFadeOutDelaySeconds;
	
	protected Widget m_Root;

	//State
	protected bool m_bWaitingForFade = true;
	
	//---- REFACTOR NOTE START: Hardcoded values
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_Root = w;
		Widget pingBorder = w.FindAnyWidget(m_sPingBorderName);
		Widget pulseWidget = w.FindAnyWidget(m_sPulseWidgetName);
		
		if (!pulseWidget || !pingBorder)
			return;
		
		float sizeBorder[2] = {150, 150};
		float sizePulse[2] = {70, 70};
		AnimateWidget.Opacity(pingBorder, 0, m_fPingBorderAnimationSpeed);
		AnimateWidget.Size(pingBorder, sizeBorder, m_fPingBorderAnimationSpeed);
		
		WidgetAnimationBase animationOpacity = AnimateWidget.Opacity(pulseWidget, 0, m_fPulsateAnimationSpeed);
		if (animationOpacity)
			animationOpacity.SetRepeat(true);
		
		WidgetAnimationBase animationSize = AnimateWidget.Size(pulseWidget, sizePulse, m_fPulsateAnimationSpeed);
		if (animationSize)
			animationSize.SetRepeat(true);
		
		GetGame().GetCallqueue().CallLater(FadeOut, m_iFadeOutDelaySeconds * 1000);
	}
	
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	//~Todo: Should delete the widget once fading is done rather then having a seperate delete timer
	protected void FadeOut()
	{
		m_bWaitingForFade = false;
		AnimateWidget.Opacity(m_Root, 0, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_bWaitingForFade)
			GetGame().GetCallqueue().Remove(FadeOut);
	}
}
