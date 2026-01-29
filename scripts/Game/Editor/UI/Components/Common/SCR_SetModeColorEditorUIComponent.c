class SCR_SetModeColorEditorUIComponent : ScriptedWidgetComponent
{
	[Attribute()]
	protected bool m_bUpdateOnModeChanged;
	protected Widget m_wRoot;
	
	//------------------------------------------------------------------------------------------------
	protected void OnModeChanged(SCR_EditorModeEntity currentModeEntity, SCR_EditorModeEntity prevModeEntity)
	{
		if (!currentModeEntity)
			return;
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;	
		
		SCR_EditorModeUIInfo modeInfo = core.GetDefaultModeInfo(currentModeEntity.GetModeType());
		if (!modeInfo)
			return;
		
		Color color = modeInfo.GetModeColor();
		color.SetA(m_wRoot.GetColor().A());
		
		SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetColor(Color color)
	{
		m_wRoot.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager) 
			return;		
		
		SCR_EditorModeEntity modeEntity = editorManager.GetCurrentModeEntity();
		if (!modeEntity)
			return;
		
		OnModeChanged(modeEntity, null);
		
		if (m_bUpdateOnModeChanged)
			editorManager.GetOnModeChange().Insert(OnModeChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager) 
			return;		
		
		if (m_bUpdateOnModeChanged)
			editorManager.GetOnModeChange().Remove(OnModeChanged);
	}
}
