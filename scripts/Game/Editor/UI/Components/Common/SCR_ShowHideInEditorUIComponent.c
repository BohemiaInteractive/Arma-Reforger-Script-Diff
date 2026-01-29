class SCR_ShowHideInEditorUIComponent : ScriptedWidgetComponent
{
	protected Widget m_Root;
	
	[Attribute("1")]
	protected bool m_bShowInEditor;
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorOpen()
	{
		m_Root.SetVisible(m_bShowInEditor);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorClosed()
	{
		m_Root.SetVisible(!m_bShowInEditor);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_Root = w;
		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();	
		
		if (!editorManagerEntity)
			return;
		
		editorManagerEntity.GetOnOpened().Insert(OnEditorOpen);
		editorManagerEntity.GetOnClosed().Insert(OnEditorClosed);
		
		m_Root.SetVisible(m_bShowInEditor == editorManagerEntity.IsOpened());
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();	
		
		if (!editorManagerEntity)
			return;
		
		editorManagerEntity.GetOnOpened().Remove(OnEditorOpen);
		editorManagerEntity.GetOnClosed().Remove(OnEditorClosed);
	}
}
