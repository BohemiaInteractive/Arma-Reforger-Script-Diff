//------------------------------------------------------------------------------------------------
class SCR_ButtonTextComponent : SCR_ButtonBaseComponent 
{
	[Attribute("Text", desc: "Text widget name within the button")]
	protected LocalizedString m_sTextWidgetName;
	
	[Attribute()]
	protected LocalizedString m_sText;
	
	[Attribute("0", UIWidgets.CheckBox)]
	bool m_bUseTextColorization;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_INFORMATION), UIWidgets.ColorPicker)]
	ref Color m_TextDefault;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.CONTRAST_COLOR), UIWidgets.ColorPicker)]
	ref Color m_TextToggled;
	
	protected TextWidget m_wText;
	
	protected ref ScriptInvokerVoid m_OnMouseEnter;
	protected ref ScriptInvokerVoid m_OnMouseLeave;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wText = TextWidget.Cast(w.FindAnyWidget(m_sTextWidgetName));
		if (m_wText)
			m_wText.SetText(m_sText);
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetToggled(bool toggled, bool animate = true, bool invokeChange = true, bool instant = false)
	{
		if (!m_bCanBeToggled)
			return;
		
		super.SetToggled(toggled, animate, invokeChange, instant);
		ColorizeText(animate);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_OnMouseEnter)
			m_OnMouseEnter.Invoke();
	
		return super.OnMouseEnter(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_OnMouseLeave)
			m_OnMouseLeave.Invoke();

		return super.OnMouseLeave(w, enterW, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ColorizeText(bool animate = true)
	{
		if (!m_bUseTextColorization || !m_wText)
			return;
		
		Color color;
		if (m_bIsToggled && m_bCanBeToggled)
			color = m_TextToggled;
		else
			color = m_TextDefault;
		
		if (animate)
			AnimateWidget.Color(m_wText, color, m_fAnimationRate);
		else
		{
			AnimateWidget.StopAnimation(m_wText, WidgetAnimationColor);
			m_wText.SetColor(color);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		if (m_sText == text)
			return;
		
		m_sText = text;
		if (m_wText)
			m_wText.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTextWithParam(string text, string param1, string param2)
	{
		m_sText = text;
		if (m_wText)
			m_wText.SetTextFormat(text, param1, param2);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetText()
	{
		return m_sText;
	}
	
	//------------------------------------------------------------------------------------------------
	TextWidget GetTextWidget()
	{
		return m_wText;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnMouseEnter()
	{
		if (!m_OnMouseEnter)
			m_OnMouseEnter = new ScriptInvokerVoid();
		
		return m_OnMouseEnter;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnMouseLeave()
	{
		if (!m_OnMouseLeave)
			m_OnMouseLeave = new ScriptInvokerVoid();
		
		return m_OnMouseLeave;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ButtonTextComponent GetButtonText(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_ButtonTextComponent.Cast(
			SCR_WLibComponentBase.GetComponent(SCR_ButtonTextComponent, name, parent, searchAllChildren)
		);
		return comp;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ButtonTextComponent FindButtonTextComponent(notnull Widget w)
	{
		return SCR_ButtonTextComponent.Cast(w.FindHandler(SCR_ButtonTextComponent));
	}
};