//! Returns true when the editor is opened
[BaseContainerProps()]
class SCR_EditorModeActiveActionCondition : SCR_AvailableActionCondition
{
	[Attribute(SCR_Enum.GetDefault(EEditorMode.EDIT), desc: "Mode used to evaluate this condition.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorMode), category: "Editor Mode")]
	private EEditorMode m_iModeType;

	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager)
			return false;

		return GetReturnResult(editorManager.GetCurrentMode() == m_iModeType);
	}
}
