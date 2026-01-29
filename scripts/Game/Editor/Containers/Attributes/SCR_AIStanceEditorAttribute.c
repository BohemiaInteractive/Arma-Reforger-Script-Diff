[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_AiStanceEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	//---- REFACTOR NOTE START: Not up to date with internal scripting style ----
	// Probably not implemeneted
	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity || (editableEntity.GetEntityType() != EEditableEntityType.CHARACTER && editableEntity.GetEntityType() != EEditableEntityType.GROUP)) return null;
		if (editableEntity.HasEntityState(EEditableEntityState.PLAYER)) return null;
		
		//WIP
		#ifndef WORKBENCH
		return null;
		#endif
		
		//If character
		if (editableEntity.GetEntityType() == EEditableEntityType.CHARACTER)
		{
			
		}
		//If group
		else
		{
			SCR_AIGroup aiGroup = SCR_AIGroup.Cast(editableEntity.GetOwner()); 
			if (!aiGroup) return null;
		}
	
		//Should get character Stance
		return SCR_BaseEditorAttributeVar.CreateInt(0);
	}	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		
		//TODO: Implement logics
		if (!var) return;
		
		if (editableEntity.GetEntityType() == EEditableEntityType.CHARACTER)
		{
			Print("Set Character stance to: " +  m_aValues[var.GetInt()].GetName());
		}
		else if (editableEntity.GetEntityType() == EEditableEntityType.GROUP)
		{
			Print("Set Group stance to: " +  m_aValues[var.GetInt()].GetName());
		}
	}
	
	//---- REFACTOR NOTE END ----
};