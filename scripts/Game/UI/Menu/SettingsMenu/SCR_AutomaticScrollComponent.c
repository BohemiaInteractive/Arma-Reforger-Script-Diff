class SCR_AutomaticScrollComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("Content", UIWidgets.EditBox, "Name of widget which will be animated")]
	protected string m_sWidgetContentName;
	
	[Attribute("HorizontalContent", UIWidgets.EditBox, "Name of widget which will be animated")]
	protected string m_sWidgetHorizontalName;
	
	[Attribute("false", UIWidgets.CheckBox, "When true, content will animate only when focused. When false, animate all the time")]
	protected bool m_bAnimateOnFocus;
	
	[Attribute("false", UIWidgets.CheckBox, "When true, content will animate only when focused. When false, animate all the time")]
	protected bool m_bAnimateVertically;
	
	protected bool m_bIsInit;
	
	protected Widget m_ContentFrame;
	protected Widget m_ContentHorizontal;
	protected SCR_HorizontalScrollAnimationComponent m_ScrollComponent;
	
	//------------------------------------------------------------------------------------------------
	protected void ResetScrolling()
	{		
		m_ScrollComponent.AnimationStop();
		m_ScrollComponent.ResetPosition();
	}
	
	//------------------------------------------------------------------------------------------------
	// Need to wait two frames for everything to init correctly
	protected void UpdateScrollFrameSkip()
	{
		GetGame().GetCallqueue().Call(StartScrolling);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void StartScrolling()
	{		
		ResetScrolling();
		
		float frameX, frameY, textX, textY;
		m_ContentHorizontal.GetScreenSize(frameX, frameY);
		m_ContentFrame.GetScreenSize(textX, textY);
		
		bool move;
		if (m_bAnimateVertically)
			move = textY > frameY;
		else
			move = textX > frameX;
		
		// Animation
		if (move)
			m_ScrollComponent.AnimationStart(m_bAnimateVertically);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if (m_bAnimateOnFocus && m_bIsInit)
			StartScrolling();
		
		return super.OnFocus(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (m_bAnimateOnFocus && m_bIsInit)
			ResetScrolling();
		
		return super.OnFocusLost(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_ContentHorizontal = w.FindAnyWidget(m_sWidgetHorizontalName);
		if (!m_ContentHorizontal)
			return;
		
		m_ContentFrame = w.FindAnyWidget(m_sWidgetContentName);
		m_ScrollComponent = SCR_HorizontalScrollAnimationComponent.Cast(m_ContentHorizontal.FindHandler(SCR_HorizontalScrollAnimationComponent));
		
		if (!m_ScrollComponent || !m_ContentFrame)
			return;
		
		m_bIsInit = true;
		
		ResetScrolling();
		
		if (!m_bAnimateOnFocus)
			GetGame().GetCallqueue().Call(UpdateScrollFrameSkip);
	}
}