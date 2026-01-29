//
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SetTaskCompletionAutomaticEditorAttribute : SCR_BaseEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableTaskComponent task = SCR_EditableTaskComponent.Cast(item);
		if (task && task.GetTaskCompletionType() != EEditorTaskCompletionType.ALWAYS_MANUAL)
			return SCR_BaseEditorAttributeVar.CreateBool(task.GetTaskCompletionType() == EEditorTaskCompletionType.AUTOMATIC);
		else
			return null;
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_EditableTaskComponent task = SCR_EditableTaskComponent.Cast(item);
		
		if (var.GetBool())
			task.SetTaskCompletionType(EEditorTaskCompletionType.AUTOMATIC);
		else 
			task.SetTaskCompletionType(EEditorTaskCompletionType.MANUAL);
	}
};