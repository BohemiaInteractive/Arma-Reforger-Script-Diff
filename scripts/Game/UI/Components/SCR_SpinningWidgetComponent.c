/*!
Spins the widget at constant speed when enabled.
It can only be attached to a text or image widget.
*/

class SCR_SpinningWidgetComponent : ScriptedWidgetComponent
{
	[Attribute("1", UIWidgets.Auto, "Speed, revolutions per second. Positive values mean clockwise rotation.")]
	float m_fSpeedRps;
	
	protected Widget m_wRoot;
	
	protected ref SCR_SpinningWidgetAnimationAbstract m_Anim;
	
	
	//-----------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{	
		if (!GetGame().InPlayMode())
			return;
		
		m_wRoot = w;
		
		if (TextWidget.Cast(w))
			m_Anim = new SCR_SpinningWidgetAnimation<TextWidget>(w);
		else if (ImageWidget.Cast(w))
			m_Anim = new SCR_SpinningWidgetAnimation<ImageWidget>(w);
		
		if (!m_Anim)
			return;
		
		ScriptCallQueue queue = GetGame().GetCallqueue();
		queue.CallLater(OnEachFrame, 0, true);
	}
	
	//-----------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (!GetGame() || !GetGame().InPlayMode())
			return;
		
		ScriptCallQueue queue = GetGame().GetCallqueue();
		queue.Remove(OnEachFrame);
	}
	
	//-----------------------------------------------------------------------------
	protected void OnEachFrame()
	{
		if (!m_wRoot.IsEnabled() || !m_wRoot.IsVisible() || !m_Anim)
			return;
		
		float tDelta = ftime / 1000.0;
		m_Anim.Animate(tDelta, m_fSpeedRps);
	}
};

//-----------------------------------------------------------------------------
//! Abstract class of a spinning animation
class SCR_SpinningWidgetAnimationAbstract
{
	void Animate(float tDelta, float speedRps);
};

//-----------------------------------------------------------------------------
//! Templated animation which can be applied to different widget types. Not all widgets support SetRotation.
class SCR_SpinningWidgetAnimation<Widget TWidget> : SCR_SpinningWidgetAnimationAbstract
{
	TWidget m_wTarget;
	
	
	//-----------------------------------------------------------------------------
	void SCR_SpinningWidgetAnimation(Widget w)
	{
		m_wTarget = TWidget.Cast(w);
	}
	
	//-----------------------------------------------------------------------------
	override void Animate(float tDelta, float speedRps)
	{
		float angle = m_wTarget.GetRotation();
		angle += tDelta*speedRps*360.0;
		if (angle > 360.0)
			angle -= 360.0;
		if (angle < 0)
			angle += 360.0;
		m_wTarget.SetRotation(angle);
	}
};