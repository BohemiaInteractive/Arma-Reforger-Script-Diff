[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_RotationEditorAttribute : SCR_ValidTypeBaseValueListEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//disable
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
		vector angles = Math3D.MatrixToAngles(mat);
				
		return SCR_BaseEditorAttributeVar.CreateFloat(angles[0] + 180);
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
		vector angles = Math3D.MatrixToAngles(mat);
		angles[0] = var.GetFloat() - 180;
		Math3D.AnglesToMatrix(angles, mat);
		editableEntity.SetTransform(mat, true);
		
		//Print("UPDATE CHILDREN!");
	}
	
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		outEntries.Insert(new SCR_BaseEditorAttributeDefaultFloatValue(180));
		return super.GetEntries(outEntries);
	}
};