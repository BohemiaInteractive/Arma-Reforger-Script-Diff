// Generic component that limits TextWidget or RichTextWidget content length
class SCR_LimitTextLength : ScriptedWidgetComponent
{
	[Attribute("1000")]
	protected int m_iMaxTextLenght;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		LimitText(w);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(Widget w)
	{
		GetGame().GetCallqueue().CallLater(LimitText, 0, false, w);
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------
	protected void LimitText(Widget w)
	{
		TextWidget textWidget = TextWidget.Cast(w);
		
		if (!textWidget)
			return;
		
		string text = textWidget.GetText();
		
		// Edit text if it's too long
		if (text.Length() > m_iMaxTextLenght)
		{
			text = text.Substring(0, m_iMaxTextLenght);
			text += "...";
		}
		
		textWidget.SetText(text);
	}
};