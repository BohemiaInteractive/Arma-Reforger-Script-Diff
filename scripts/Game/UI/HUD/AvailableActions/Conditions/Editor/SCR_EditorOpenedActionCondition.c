//! Returns true when the editor is opened
[BaseContainerProps()]
class SCR_EditorOpenedActionCondition: SCR_AvailableActionCondition
{
	[Attribute()]
	protected bool m_ModeMustNotBeLimited;
	
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		//Editor not open
		if (!SCR_EditorManagerEntity.IsOpenedInstance())
			return GetReturnResult(false);
		
		//Editor is open and mode limted not important
		if (!m_ModeMustNotBeLimited)
			return GetReturnResult(true);
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		
		if (!editorManager)
			return false;
		
		SCR_EditorModeEntity mode = editorManager.GetCurrentModeEntity();
		if (!mode)
			return false;
		
		if (!mode.IsLimited())
			return GetReturnResult(true);
		else 
			return GetReturnResult(false);
	}
};