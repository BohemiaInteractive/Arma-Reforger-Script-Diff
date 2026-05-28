//! Returns true when the editor is opened
[BaseContainerProps()]
class SCR_EditorOpenedActionCondition: SCR_AvailableActionCondition
{
	[Attribute()]
	protected bool m_ModeMustNotBeLimited;
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		//Editor not open
		SCR_EditorManagerEntity editorMgr = data.GetEditorManager();
		if (!editorMgr || !editorMgr.IsOpened())
			return GetReturnResult(false);
		
		//Editor is open and mode limted not important
		if (!m_ModeMustNotBeLimited)
			return GetReturnResult(true);

		SCR_EditorModeEntity mode = data.GetEditorModeEntity();
		return GetReturnResult(mode && !mode.IsLimited());
	}
}