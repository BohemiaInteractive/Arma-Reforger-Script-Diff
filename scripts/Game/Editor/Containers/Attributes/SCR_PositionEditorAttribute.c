[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_PositionEditorAttribute : SCR_ValidTypeBaseEditorAttribute
{	
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
		
		vector mat[4];
		editableEntity.GetTransform(mat);
				
		return SCR_BaseEditorAttributeVar.CreateVector(mat[3]);
	}
		
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		if (!var)
			return;
		
		//Set sub labels
		SCR_BaseEditorAttributeVar snapVar;
		
		if (isInit && manager.GetAttributeVariable(SCR_SnapYPositionEditorAttribute, snapVar))
			manager.SetAttributeAsSubAttribute(SCR_SnapYPositionEditorAttribute);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return;
		
		vector mat[4];
		
		editableEntity.GetTransform(mat);
		vector value = var.GetVector();

		mat[3] = value;
		editableEntity.SetTransform(mat, true);
		
		//Print("UPDATE CHILDREN!");
	}
};