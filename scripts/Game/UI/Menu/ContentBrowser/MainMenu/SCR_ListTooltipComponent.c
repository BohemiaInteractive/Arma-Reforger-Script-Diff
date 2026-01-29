/*!
	Component for tooltips that display a list of strings (e.g. hidden filters)
*/
class SCR_ListTooltipComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("{F6609064A9ECD900}UI/layouts/Menus/Tooltips/Tooltip_ListLine_NoBG_Medium.layout")]
	protected ResourceName m_sLineLayout;

	protected VerticalLayoutWidget m_wListWrapper;
	protected Widget m_wSeparator;
	protected ref array<string> m_aMessages = {};

	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		m_wListWrapper = VerticalLayoutWidget.Cast(w.FindAnyWidget("List"));
		if (m_wListWrapper && GetGame().InPlayMode())
			m_wListWrapper.SetVisible(false);		

		m_wSeparator = w.FindAnyWidget("Separator");
		
		super.HandlerAttached(w);
	}

	//------------------------------------------------------------------------------------------------
	void Init(array<string> messages, bool showSeparator = false)
	{
		if (!m_wListWrapper)
			return;
		
		m_wListWrapper.SetVisible(!messages.IsEmpty());
		m_wSeparator.SetVisible(showSeparator);
		
		foreach (string message : messages)
		{
			if (!m_aMessages.IsEmpty() && m_aMessages.Contains(message))
				continue;
			
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sLineLayout, m_wListWrapper);
			if (!w)
				continue;
			
			TextWidget text = TextWidget.Cast(w.FindAnyWidget("Text"));
			if (text)
				text.SetText(message);
			
			m_aMessages.Insert(message);
		}
	}

	//------------------------------------------------------------------------------------------------
	static SCR_ListTooltipComponent FindComponent(notnull Widget w)
	{
		return SCR_ListTooltipComponent.Cast(w.FindHandler(SCR_ListTooltipComponent));
	}
}
