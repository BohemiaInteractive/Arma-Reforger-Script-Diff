[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_TaskStateEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
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
		
		SCR_ETaskState taskState = task.GetTaskState();
		int index = ConvertValueToIndex(taskState);
		
		return SCR_BaseEditorAttributeVar.CreateInt(index);
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
		
		float floatVal;
		ConvertIndexToValue(var.GetInt(), floatVal);
		
		SCR_ScenarioFrameworkTask frameworkTask = SCR_ScenarioFrameworkTask.Cast(task);
		if (frameworkTask)
		{
			SCR_ScenarioFrameworkLayerTask layerTask = frameworkTask.GetLayerTask();
			if (layerTask)
				layerTask.ProcessLayerTaskState(floatVal, true);
		}
		else
		{
			taskSystem.SetTaskState(task, floatVal);
		}
	}
}
