/*
Component for menu headers, assumes the layout includes a text widget named "Title".
*/

//------------------------------------------------------------------------------------------------
class SCR_CoreMenuHeaderComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("Title", desc: "Title of the menu")]
	protected string m_sTitle;
	
	protected TextWidget m_wTitle;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wTitle = TextWidget.Cast(w.FindAnyWidget("Title"));
		SetTitle(m_sTitle);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTitle(string text)
	{
		if (m_wTitle)
			m_wTitle.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTitleVisible(bool visible)
	{
		if (m_wTitle)
			m_wTitle.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_CoreMenuHeaderComponent FindComponentInHierarchy(notnull Widget root)
	{
		SCR_CoreMenuHeaderComponent comp;

		ScriptedWidgetEventHandler handler = SCR_WidgetTools.FindHandlerInChildren(root, SCR_CoreMenuHeaderComponent);
		if (handler)
			comp = SCR_CoreMenuHeaderComponent.Cast(handler);

		return comp;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_CoreMenuHeaderComponent FindComponent(notnull Widget w)
	{
		return SCR_CoreMenuHeaderComponent.Cast(w.FindHandler(SCR_CoreMenuHeaderComponent));
	}
}