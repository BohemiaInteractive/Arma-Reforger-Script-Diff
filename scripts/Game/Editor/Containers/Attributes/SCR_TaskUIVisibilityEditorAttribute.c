[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_TaskUIVisibilityEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return null;
		
		IEntity owner = editableEntity.GetOwner();
		if (!owner) 
			return null;
		
		SCR_Task task = SCR_Task.Cast(owner);
		if (!task)
			return null;
		
		SCR_ETaskUIVisibility taskUIVisibility = task.GetTaskUIVisibility();
		return SCR_BaseEditorAttributeVar.CreateInt(taskUIVisibility);
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return;
		
		IEntity owner = editableEntity.GetOwner();
		if (!owner) 
			return;
		
		SCR_Task task = SCR_Task.Cast(owner);
		if (!task)
			return;
		
		taskSystem.SetTaskUIVisibility(task, var.GetInt());
	}
}
