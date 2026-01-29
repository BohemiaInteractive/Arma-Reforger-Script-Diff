class SCR_PulsateUIComponent : ScriptedWidgetComponent
{
	[Attribute("Pulse")]
	protected string m_sPulseWidgetName;
	
	[Attribute("2")]
	protected float m_fPulsateAnimationSpeed;
	
	[Attribute("", UIWidgets.ComboBox, "Animation curve", "", ParamEnumArray.FromEnum(EAnimationCurve))]
	protected EAnimationCurve m_eCurve;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		Widget pulse = w.FindAnyWidget(m_sPulseWidgetName);
		WidgetAnimationBase anim = AnimateWidget.Opacity(pulse, 0, m_fPulsateAnimationSpeed);
		if (!anim)
			return;
		
		anim.SetRepeat(true);
		anim.SetCurve(m_eCurve);
	}
}
