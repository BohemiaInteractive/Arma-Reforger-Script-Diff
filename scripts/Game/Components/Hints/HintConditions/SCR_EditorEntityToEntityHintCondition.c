[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_EditorEntityToEntityHintCondition : SCR_BaseEditorHintCondition
{
	//[Attribute("-1", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	//protected EEditableEntityType m_EditedType;
	
	[Attribute("-1", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	protected EEditableEntityType m_TargetType;
	
	//------------------------------------------------------------------------------------------------
	protected void OnTargetChange(SCR_EditableEntityComponent target)
	{
		if (target)
		{
			if (target.GetEntityType() == m_TargetType)
				Activate();
		}
		else
		{
			Deactivate();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_PreviewEntityEditorComponent previewManager = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent));
		if (previewManager)
			previewManager.GetOnTargetChange().Insert(OnTargetChange);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_PreviewEntityEditorComponent previewManager = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent));
		if (previewManager)
			previewManager.GetOnTargetChange().Remove(OnTargetChange);
	}
}
