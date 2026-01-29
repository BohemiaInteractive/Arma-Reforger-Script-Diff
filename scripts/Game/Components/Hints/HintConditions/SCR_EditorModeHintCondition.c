[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_EditorModeHintCondition : SCR_BaseEditorHintCondition
{
	[Attribute()]
	private bool m_bIsLimited;
	
	//------------------------------------------------------------------------------------------------
	protected void OnModePostActivate()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager && editorManager.IsLimited() == m_bIsLimited)
			Activate();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_EditorModeEntity mode = editorManager.GetCurrentModeEntity();
		if (mode)
			mode.GetOnPostActivate().Insert(OnModePostActivate);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		Deactivate();
		
		SCR_EditorModeEntity mode = editorManager.GetCurrentModeEntity();
		if (mode)
			mode.GetOnPostActivate().Remove(OnModePostActivate);
	}
}
