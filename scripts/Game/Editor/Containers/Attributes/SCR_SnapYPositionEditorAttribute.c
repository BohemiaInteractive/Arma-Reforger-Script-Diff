
/*! An local attribute that dictates if position that is set is snapped to ground. If the attribute is missing then snap to ground is always true
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SnapYPositionEditorAttribute : SCR_ValidTypeBaseEditorAttribute
{	
	protected ref ScriptInvoker Event_OnValueChanged = new ScriptInvoker;
	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		//Disable
		return null;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return null;
		
		if (!IsValidEntityType(editableEntity.GetEntityType()))
			return null;
		
		//~Todo: Make sure chilren positions are set correctly
		if (editableEntity.GetChildrenCount() > 0)
			return null;
		
		//Resets everytime editor is opened
		return SCR_BaseEditorAttributeVar.CreateBool(true);
	}
	
	ScriptInvoker GetOnChanged()
	{
		return Event_OnValueChanged;
	}
	
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		Event_OnValueChanged.Invoke(GetVariableOrCopy().GetBool());
	}
};