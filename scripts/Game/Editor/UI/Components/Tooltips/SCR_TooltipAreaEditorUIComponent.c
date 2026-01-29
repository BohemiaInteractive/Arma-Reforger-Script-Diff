//! @ingroup Editor_UI Editor_UI_Components

class SCR_TooltipAreaEditorUIComponent : SCR_BaseEditorUIComponent
{
	[Attribute()]
	protected string m_sMouseAndKeyboardWidgetName;
	
	[Attribute()]
	protected string m_sControllerWidgetName;
	
	[Attribute()]
	protected int m_iOffsetTop;
	
	[Attribute()]
	protected int m_iOffsetRight;
	
	[Attribute()]
	protected int m_iOffsetBottom;
	
	[Attribute()]
	protected int m_iOffsetLeft;
	
	protected InputManager m_InputManager;
	protected Widget m_MouseAndKeyboardWidget;
	protected Widget m_ControllerWidget;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Widget GetAreaWidget()
	{
		if (m_InputManager.IsUsingMouseAndKeyboard())
			return m_MouseAndKeyboardWidget;
		else
			return m_ControllerWidget;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] top
	//! \param[out] right
	//! \param[out] bottom
	//! \param[out] left
	void GetOffsets(out int top, out int right, out int bottom, out int left)
	{
		top = m_iOffsetTop;
		right = m_iOffsetRight;
		bottom = m_iOffsetBottom;
		left = m_iOffsetLeft;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ClearTooltips()
	{
		ClearAreaWidget(m_MouseAndKeyboardWidget);
		ClearAreaWidget(m_ControllerWidget);
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearAreaWidget(Widget w)
	{
		Widget child = w.GetChildren();
		while (child)
		{
			child.RemoveFromHierarchy();
			child = child.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		m_InputManager = GetGame().GetInputManager();
		
		m_MouseAndKeyboardWidget = w.FindAnyWidget(m_sMouseAndKeyboardWidgetName);
		if (!m_MouseAndKeyboardWidget)
			Print(string.Format("Widget '%1' not found!", m_sMouseAndKeyboardWidgetName), LogLevel.NORMAL);
		
		m_ControllerWidget = w.FindAnyWidget(m_sControllerWidgetName);
		if (!m_ControllerWidget)
			Print(string.Format("Widget '%1' not found!", m_sControllerWidgetName), LogLevel.NORMAL);
	}
}
