//[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
//class SCR_TaskActivateContextAction: SCR_SelectedEntitiesContextAction
//{
//	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
//	{
//		SCR_EditableTaskComponent editableTask = SCR_EditableTaskComponent.Cast(selectedEntity);
//		return true; //editableTask && !editableTask.IsTaskActivated();
//	}
//	
//	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
//	{
//		return true;
//	}
//	
//	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
//	{
//		SCR_EditableTaskComponent editableTask = SCR_EditableTaskComponent.Cast(selectedEntity);
//		//editableTask.ActivateTask();
//	}
//};