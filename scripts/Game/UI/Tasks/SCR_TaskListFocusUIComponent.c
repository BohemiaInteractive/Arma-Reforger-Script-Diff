class SCR_TaskListFocusUIComponent : SCR_ScriptedWidgetComponent
{	
	protected WorkspaceWidget m_Workspace;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_Workspace = GetGame().GetWorkspace();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);

		if (m_Workspace)
			m_Workspace.SetFocusedWidget(m_wRoot);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseEnter(w, enterW, x, y);

		if (m_Workspace)
			m_Workspace.SetFocusedWidget(null);

		return false;
	}
}