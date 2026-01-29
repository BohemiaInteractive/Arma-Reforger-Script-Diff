[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_EditorContextMenuHintCondition : SCR_BaseEditorHintCondition
{
	//------------------------------------------------------------------------------------------------
	protected void OnMenuOpen(notnull array<SCR_BaseEditorAction> actions, vector cursorWorldPosition, out notnull array<ref SCR_EditorActionData> filteredActions, out int flags = 0)
	{
		if (!filteredActions.IsEmpty())
			Activate();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_ContextActionsEditorComponent contextMenuManager = SCR_ContextActionsEditorComponent.Cast(SCR_ContextActionsEditorComponent.GetInstance(SCR_ContextActionsEditorComponent));
		if (contextMenuManager)
			contextMenuManager.GetOnMenuOpen().Insert(OnMenuOpen);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_ContextActionsEditorComponent contextMenuManager = SCR_ContextActionsEditorComponent.Cast(SCR_ContextActionsEditorComponent.GetInstance(SCR_ContextActionsEditorComponent));
		if (contextMenuManager)
			contextMenuManager.GetOnMenuOpen().Remove(OnMenuOpen);
	}
}
