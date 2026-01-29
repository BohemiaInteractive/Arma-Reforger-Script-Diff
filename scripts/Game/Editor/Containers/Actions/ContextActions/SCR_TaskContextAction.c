[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_TaskContextAction : SCR_SelectedEntitiesContextAction
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum("Complete", "0", ""), ParamEnum("Fail", "1", "") }, desc: "State to which the task will be set.")]
	protected int m_iTaskState;
	
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return selectedEntity && selectedEntity.GetEntityType() == EEditableEntityType.TASK && SCR_TaskSystem.GetInstance();
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		if (!SCR_TaskSystem.GetInstance())
			return;
		
		SCR_Task task = SCR_Task.Cast(selectedEntity.GetOwner());
		if (!task)
			return;
		
		switch (m_iTaskState)
		{
			//--- Complete
			case 0:
			{
				task.SetTaskState(SCR_ETaskState.COMPLETED);
				break;
			}
			//--- Fail
			case 1:
			{
				task.SetTaskState(SCR_ETaskState.FAILED);
				break;
			}
		}
	}
};