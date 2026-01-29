class SCR_LabelComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("Label's text")]
	protected string m_sLabel;
	
	[Attribute("30")]
	protected float m_fPaddingTop;

	protected TextWidget m_wLabelWidget;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (!w)
			return;

		m_wLabelWidget = TextWidget.Cast(w.FindAnyWidget("Label"));
		ResetTopPadding();
		SetText(m_sLabel);
	}

	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		m_wLabelWidget.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTopPadding(float padding)
	{
		if (!m_wLabelWidget)
			return;
		
		float left, top, right, bottom;
		AlignableSlot.GetPadding(m_wLabelWidget, left, top, right, bottom);
		AlignableSlot.SetPadding(m_wLabelWidget, left, padding, right, bottom);
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetTopPadding()
	{
		SetTopPadding(m_fPaddingTop);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVisible(bool visible)
	{
		if (!m_wLabelWidget)
			return;
		
		m_wLabelWidget.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_LabelComponent FindLabelComponent(notnull Widget w)
	{
		return SCR_LabelComponent.Cast(w.FindHandler(SCR_LabelComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_LabelComponent GetComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		return SCR_LabelComponent.Cast(SCR_ScriptedWidgetComponent.GetComponent(SCR_LabelComponent, name, parent, searchAllChildren));
	}
};
