class SCR_MultiTextTooltipUIComponent : ScriptedWidgetComponent
{
	[Attribute(defvalue: "1 1 1 1", desc: "Color of Text.")]
	protected ref Color m_cTextColor;
	
	[Attribute("")]
	protected ResourceName m_TextPrefab;
	
	protected Widget m_wRoot;
	
	//------------------------------------------------------------------------------------------------
	//!
	void ClearAllText()
	{
		if (!m_wRoot)
			return; 
		
		Widget child = m_wRoot.GetChildren();
		Widget childtemp;
		while (child)
		{
			childtemp = child;
			child = child.GetSibling();
			childtemp.RemoveFromHierarchy();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] color
	void SetInitTextColor(Color color)
	{
		m_cTextColor = color;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] text
	//! \param[in] param1
	//! \param[in] param2
	//! \param[in] param3
	//! \param[in] param4
	//! \return
	TextWidget AddText(string text, string param1 = string.Empty, string param2 = string.Empty, string param3 = string.Empty, string param4 = string.Empty)
	{
		if (!m_wRoot)
			return null;
		
		Widget newWidget = GetGame().GetWorkspace().CreateWidgets(m_TextPrefab, m_wRoot);
		if (!newWidget)
			return null;
		
		TextWidget textWidget = TextWidget.Cast(newWidget);
		if (!textWidget)
			return null;
		
		textWidget.SetColor(m_cTextColor);
		textWidget.SetTextFormat(text, param1, param2, param3, param4);
		return textWidget;
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
	}
}
