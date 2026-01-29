/*!
Simple entry for display nointeractive items with left and right text
*/

//------------------------------------------------------------------------------------------------------------------------
class SCR_SimpleEntryComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("Label")]
	protected string m_sLabel;
	
	[Attribute("Message")]
	protected string m_sMessage;
	
	[Attribute("Label")]
	protected string m_sLabelText;
	
	[Attribute("Message")]
	protected string m_sMessageText;
	
	protected TextWidget m_wLabel;
	protected TextWidget m_wMessage;
	
	//------------------------------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wLabel = TextWidget.Cast(w.FindAnyWidget(m_sLabel));
		m_wMessage = TextWidget.Cast(w.FindAnyWidget(m_sMessage));
		
		if (m_wLabel)
			m_wLabel.SetText(m_sLabelText);
		
		if (m_wMessage)
			m_wMessage.SetText(m_sMessageText);
	}
	
	//------------------------------------------------------------------------------------------------------------------------
	void SetMessages(string left, string right)
	{
		m_sLabelText = left;
		m_sMessageText = right;
		
		if (m_wLabel)
			m_wLabel.SetText(left);
		
		if (m_wMessage)
			m_wMessage.SetText(right);
	}
	
	//------------------------------------------------------------------------------------------------------------------------
	TextWidget GetLabel()
	{
		return m_wLabel;
	}
	
	//------------------------------------------------------------------------------------------------------------------------
	string GetLeftText()
	{
		if (!m_wLabel)
			return string.Empty;
		
		return m_wLabel.GetText();
	}
	
	//------------------------------------------------------------------------------------------------------------------------
	TextWidget GetMessage()
	{
		return m_wMessage;
	}
	
	//------------------------------------------------------------------------------------------------------------------------
	string GetRightText()
	{
		if (!m_wMessage)
			return string.Empty;
		
		return m_wMessage.GetText();
	}
	
	//------------------------------------------------------------------------------------------------------------------------
	Widget GetRoot()
	{
		return m_wRoot;
	}
}