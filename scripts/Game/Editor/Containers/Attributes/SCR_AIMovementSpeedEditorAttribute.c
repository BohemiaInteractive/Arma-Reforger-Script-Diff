[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_AiMovementSpeedEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	//---- REFACTOR NOTE START: Not up to date with internal scripting style ----
	// Probably not implemeneted
	// Empty brackets of character check 
	
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
			SCR_AIGroup aiGroup;// = SCR_AIGroup.Cast(GetOwner(editableEntity)); 
			if (!aiGroup) return null;
		}
	
		//Should get character movement speed
		return SCR_BaseEditorAttributeVar.CreateInt(1);
	}	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		//TODO: Implement logics
		if (!var) return;
		
		bool setRunning = (int)m_aValues[var.GetInt()].GetFloatValue();
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		
		if (editableEntity.GetEntityType() == EEditableEntityType.CHARACTER)
		{
			Print("Set Character running: " +  setRunning);
		}
		else if (editableEntity.GetEntityType() == EEditableEntityType.GROUP)
		{
			Print("Set Group running: " +  setRunning);
		}
	}
		
	//---- REFACTOR NOTE END ----
};