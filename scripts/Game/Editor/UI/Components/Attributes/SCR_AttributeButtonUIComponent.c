class SCR_AttributeButtonUIComponent: ScriptedWidgetComponent
{
	protected SCR_ButtonBoxAttributeUIComponent m_AttributeUIComponent;
	
	protected string m_sButtonDescription;
	
	protected bool m_bIsFocused;
	protected bool m_bListeningToMouseLeft;
	protected bool m_FocusUpdateBlocked;
	
	protected const int MOUSE_LEFT_REMOVE_DESCRIPTION_DELAY = 50;
	
	/*!
	Init the button setting references, description and events
	\param attributeUIComponent ref to attribute UI it is part of
	\param button button component on it
	\param buttonDescription button description to display
	\param buttonName %1 in button description
	*/
	void ButtonDescriptionInit(SCR_ButtonBoxAttributeUIComponent attributeUIComponent, SCR_ButtonBaseComponent button, string buttonDescription, string buttonName = string.Empty)
	{
		m_AttributeUIComponent = attributeUIComponent;
		
		if (!buttonDescription.IsEmpty())
			m_sButtonDescription = buttonDescription;
		else 
			m_sButtonDescription = buttonName;
		
		button.m_OnShowBorder.Insert(OnButtonFocus);
	}
	
	protected void OnButtonFocus(SCR_ButtonBaseComponent button, bool value)
	{
		if (!value)
			return;
	}
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_bIsFocused = true;
		if (!m_AttributeUIComponent)
			return false;
		
		if (!m_AttributeUIComponent.GetIsFocused())
			return false;
		
		m_AttributeUIComponent.ShowButtonDescription(this, true, m_sButtonDescription);
		return false;
	}
	
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_bIsFocused = false;

		if (!m_bListeningToMouseLeft)
		{
			m_bListeningToMouseLeft = true;
			GetGame().GetCallqueue().CallLater(OnMouseLeaveDelay, MOUSE_LEFT_REMOVE_DESCRIPTION_DELAY);
		}
		
		return false;
	}
	
	protected void OnMouseLeaveDelay()
	{
		m_bListeningToMouseLeft = false;		
		
		if (m_bIsFocused)
			return;
		
		if (!m_AttributeUIComponent)
			return;
		
		m_AttributeUIComponent.ShowButtonDescription(this, false);
	}
};
