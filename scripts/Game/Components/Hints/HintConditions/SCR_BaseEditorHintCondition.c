[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_BaseEditorHintCondition: SCR_BaseHintCondition
{
	protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager);
	protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager);
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorManagerInitOwner()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		core.Event_OnEditorManagerInitOwner.Remove(OnEditorManagerInitOwner);
		
		OnInitConditionEditor(core.GetEditorManager());
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnInitCondition(Managed owner)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			OnInitConditionEditor(editorManager);
		}
		else
		{
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			core.Event_OnEditorManagerInitOwner.Insert(OnEditorManagerInitOwner);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitCondition(Managed owner)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			OnExitConditionEditor(editorManager);
	}
}
