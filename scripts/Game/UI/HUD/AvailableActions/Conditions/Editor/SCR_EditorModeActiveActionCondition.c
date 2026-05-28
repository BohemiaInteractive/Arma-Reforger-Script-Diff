//! Returns true when the editor is opened
[BaseContainerProps()]
class SCR_EditorModeActiveActionCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: EEditorMode.EDIT.ToString(), desc: "Mode used to evaluate this condition.", uiwidget: UIWidgets.ComboBox, enumType: EEditorMode, category: "Editor Mode")]
	private EEditorMode m_iModeType;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		SCR_EditorManagerEntity editorManager = data.GetEditorManager();
		return GetReturnResult(editorManager && editorManager.GetCurrentMode() == m_iModeType);
	}
}
