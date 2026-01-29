[Obsolete("Replaced with SCR_TaskMapUIComponent")]
class SCR_TaskOverlayButton : ScriptedWidgetComponent
{
	/*
	protected SCR_MapUITask m_MapUiTask;
	
	
	//------------------------------------------------------------------------------------------------
	override event bool OnMouseEnter(Widget w, int x, int y)
	{				
		if (m_MapUiTask && m_MapUiTask.m_wHorizLayout && !m_MapUiTask.m_wHorizLayout.IsEnabled())	 // buttons are disabled
			m_MapUiTask.m_wHorizLayout.SetEnabled(true);

		return false;
	}
	
	//------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (!m_MapUiTask)
			return false;

		ButtonWidget assignButton = ButtonWidget.Cast(m_MapUiTask.GetMapWidget().FindAnyWidget("TaskTitleButton"));
		Widget overlayWidget = m_MapUiTask.GetMapWidget().FindAnyWidget("OverlayWidget");
		Widget assignee = m_MapUiTask.GetMapWidget().FindAnyWidget("Assignee");
		Widget iconHover = m_MapUiTask.GetMapWidget().FindAnyWidget("TaskIconHover");

		if (!assignButton || !overlayWidget || !assignee || !iconHover)
			return false;

		if (enterW != overlayWidget && enterW != assignButton)
		{
			assignButton.SetVisible(false);

			assignee.SetEnabled(false);
			assignee.SetOpacity(0);

			iconHover.SetEnabled(false);
			iconHover.SetOpacity(0);
			
			m_MapUiTask.m_wHorizLayout.SetEnabled(false);
		}

		return false;
	}

	//------------------------------------------------------------------------------
	void SetRootWidgetHandler(SCR_MapUITask mapUiTask)
	{
		m_MapUiTask = mapUiTask;
	}
	*/
}
