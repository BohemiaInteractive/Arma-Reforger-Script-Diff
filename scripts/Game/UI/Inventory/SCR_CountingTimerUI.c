class SCR_CountingTimerUI : SCR_ScriptedWidgetComponent
{
	
	[Attribute("", desc: "name of widget of icon attached to the timer")]
	protected string m_sIconName;
	
	protected ImageWidget m_wImageWidget;
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateIngameIcon()
	{
		if (!m_wImageWidget)
			return;

		if (AnimateWidget.IsAnimating(m_wImageWidget))
			return;
		
		if (m_wImageWidget.GetOpacity() == 1)
			AnimateWidget.Opacity(m_wImageWidget, 0, 1);
		else
			AnimateWidget.Opacity(m_wImageWidget, 1, 1);	
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_wImageWidget = ImageWidget.Cast(w.FindAnyWidget(m_sIconName));
		
		if (!m_wImageWidget)
			return;

		// Update the icon at 30 fps. It needs to update when the animation has finished to make re-animating seamless
		GetGame().GetCallqueue().CallLater(UpdateIngameIcon, 30, true);
	}
}