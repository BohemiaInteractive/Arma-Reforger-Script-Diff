[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_EditorOpenedHintCondition : SCR_BaseEditorHintCondition
{
	//------------------------------------------------------------------------------------------------
	override protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		editorManager.GetOnOpened().Insert(Activate);
		editorManager.GetOnClosed().Insert(Deactivate);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		editorManager.GetOnOpened().Remove(Activate);
		editorManager.GetOnClosed().Remove(Deactivate);
	}
}
