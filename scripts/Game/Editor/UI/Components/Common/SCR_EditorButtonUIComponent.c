class SCR_EditorButtonUIComponent : ScriptedWidgetComponent
{
	protected Widget m_Widget;
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorLimitedChange(bool isLimited)
	{
		m_Widget.SetVisible(!isLimited);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		SCR_EditorManagerEntity.OpenInstance();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager)
			return;
		
		m_Widget = w;
		
		editorManager.GetOnLimitedChange().Insert(OnEditorLimitedChange);
		OnEditorLimitedChange(editorManager.IsLimited());
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager)
			return;
		
		editorManager.GetOnLimitedChange().Remove(OnEditorLimitedChange);
	}
}
