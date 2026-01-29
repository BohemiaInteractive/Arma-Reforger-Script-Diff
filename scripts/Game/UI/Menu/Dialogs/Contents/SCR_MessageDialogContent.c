/*!
Store message dialog content
*/
class SCR_MessageDialogContent : SCR_ScriptedWidgetComponent
{
	[Attribute("Message")]
	protected string m_sMessage;
	
	protected TextWidget m_wMessage;
	
	//------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wMessage = TextWidget.Cast(w.FindAnyWidget(m_sMessage));
	}
	
	//------------------------------------------------------------------------------------------
	void SetMessage(string message)
	{
		if (m_wMessage)
			m_wMessage.SetText(message);
	}
	
	//------------------------------------------------------------------------------------------
	string GetMessage()
	{
		if (!m_wMessage)
			return "";
		
		return m_wMessage.GetText();
	}
}