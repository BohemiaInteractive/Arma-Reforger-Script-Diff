[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_EditorTransformHintCondition : SCR_BaseEditorHintCondition
{
	//------------------------------------------------------------------------------------------------
	override protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_TransformingEditorComponent transformManager = SCR_TransformingEditorComponent.Cast(SCR_TransformingEditorComponent.GetInstance(SCR_TransformingEditorComponent));
		if (transformManager)
			transformManager.GetOnTransformationStart().Insert(Activate);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_TransformingEditorComponent transformManager = SCR_TransformingEditorComponent.Cast(SCR_TransformingEditorComponent.GetInstance(SCR_TransformingEditorComponent));
		if (transformManager)
			transformManager.GetOnTransformationStart().Remove(Activate);
	}
}
