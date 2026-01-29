class SCR_TaskListFocusComponent : ScriptedWidgetComponent
{
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			return;

		SetMapContextAllowed(false);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		SetMapContextAllowed(false);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (enterW)
		{
			// enterW is null in map, but in deploy menu mouse enters a root widget of deploy screen so to filter it out we check if the enterW has a deployHandler.
			SCR_DeployMenuHandler deployMenu = SCR_DeployMenuHandler.Cast(enterW.FindHandler(SCR_DeployMenuHandler));
			if (!deployMenu)
				return false;
		}

		SetMapContextAllowed(true);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	private void SetMapContextAllowed(bool val)
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;

		SCR_MapTaskListUI taskListUI = SCR_MapTaskListUI.Cast(mapEntity.GetMapUIComponent(SCR_MapTaskListUI));
		if (!taskListUI)
			return;

		taskListUI.SetMapContextAllowed(val);
	}
}
