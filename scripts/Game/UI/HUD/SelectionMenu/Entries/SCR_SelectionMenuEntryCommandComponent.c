[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomTitleUIInfo("Name")]
class SCR_SelectionMenuEntryCommandComponent : SCR_SelectionMenuEntryIconComponent
{
	[Attribute("CommandText")]
	protected string m_sCommandText;
	
	protected TextWidget m_wCommandText;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wCommandText = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sCommandText));
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCommandText(string text)
	{
		if (m_wCommandText)
			m_wCommandText.SetText(text);
	}
}