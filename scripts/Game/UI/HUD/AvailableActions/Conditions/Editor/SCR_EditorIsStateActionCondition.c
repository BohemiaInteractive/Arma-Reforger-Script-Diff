//! Returns true when editor state is the given state
[BaseContainerProps()]
class SCR_EditorIsStateActionCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: EEditorState.NONE.ToString(), desc: "Editor State", uiwidget: UIWidgets.ComboBox, enumType: EEditorState)]
	protected EEditorState m_EditorState;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		SCR_StatesEditorComponent statesMgr = data.GetEditorStatesManagerComponent();
		return GetReturnResult(statesMgr && statesMgr.GetState() == m_EditorState);
	}
}
