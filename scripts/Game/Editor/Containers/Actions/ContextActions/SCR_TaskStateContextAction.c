[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_TaskStateContextAction : SCR_SelectedEntitiesContextAction
{
	[Attribute(defvalue: SCR_ETaskState.CREATED.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETaskState))]
	protected SCR_ETaskState m_eTaskState;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return selectedEntity && selectedEntity.GetEntityType() == EEditableEntityType.TASK;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;
		
		SCR_Task task = SCR_Task.Cast(selectedEntity.GetOwner());
		if (!task)
			return;
		
		taskSystem.SetTaskState(task, m_eTaskState);
	}
};