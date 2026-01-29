[Obsolete("Replaced with SCR_TaskMapUIComponent")]
class SCR_TaskAssignButton : ScriptedWidgetComponent
{	
	/*
	protected SCR_MapUITask m_MapUiTask;
	protected ref ScriptInvoker m_OnMapIconClick;

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMapIconClick()
	{
		if (!m_OnMapIconClick)
			m_OnMapIconClick = new ScriptInvoker();

		return m_OnMapIconClick;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			return false;
		
		if (m_OnMapIconClick)
			m_OnMapIconClick.Invoke();

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		Widget frame = m_MapUiTask.GetMapWidget().FindAnyWidget("Border");

		if (frame)
		{
			frame.SetEnabled(true);
			frame.SetOpacity(1);
		}

		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_SELECT, EActionTrigger.DOWN, ButtonPressed);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void ButtonPressed()
	{
		if (m_OnMapIconClick)
			m_OnMapIconClick.Invoke();
	}
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		Widget frame = m_MapUiTask.GetMapWidget().FindAnyWidget("Border");
		if (frame)
		{
			frame.SetEnabled(false);
			frame.SetOpacity(0);
		}

		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_SELECT, EActionTrigger.DOWN, ButtonPressed);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void SetRootWidgetHandler(SCR_MapUITask mapUiTask)
	{
		m_MapUiTask = mapUiTask;
	}
	*/
}
