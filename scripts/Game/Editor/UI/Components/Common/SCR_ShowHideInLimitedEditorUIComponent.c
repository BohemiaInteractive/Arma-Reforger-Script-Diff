class SCR_ShowHideInLimitedEditorUIComponent : ScriptedWidgetComponent
{
	[Attribute("1", desc: "Determines if this element will be visible if the current Editor mode is Limited")]
	protected bool m_bShowInLimitedEditor;
	
	protected Widget m_wRoot;
	protected bool m_bEditorLimited;
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorOpen()
	{
		if (m_bEditorLimited)
			m_wRoot.SetVisible(m_bShowInLimitedEditor);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorClosed()
	{
		if (m_bEditorLimited)
			m_wRoot.SetVisible(!m_bShowInLimitedEditor);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();	
		
		if (!editorManagerEntity)
			return;

		m_bEditorLimited = editorManagerEntity.IsLimited();
		
		if (m_bEditorLimited)
		{
			editorManagerEntity.GetOnOpened().Insert(OnEditorOpen);
			editorManagerEntity.GetOnClosed().Insert(OnEditorClosed);
		
			m_wRoot.SetVisible(m_bShowInLimitedEditor == editorManagerEntity.IsOpened());
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();	
		
		if (!editorManagerEntity)
			return;
		
		if (m_bEditorLimited)
		{
			editorManagerEntity.GetOnOpened().Remove(OnEditorOpen);
			editorManagerEntity.GetOnClosed().Remove(OnEditorClosed);
		}
	}
}
