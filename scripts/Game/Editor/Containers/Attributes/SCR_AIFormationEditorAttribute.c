// Script File
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_AIFormationEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		return null;
		
		//WIP
		#ifndef WORKBENCH
		return null;
		#endif
		
		/*
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity || (editableEntity.GetEntityType() != EEditableEntityType.GROUP)) return null;
		if (editableEntity.HasEntityState(EEditableEntityState.PLAYER)) return null;
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(editableEntity.GetOwner()); 
		if (!aiGroup) return null;
		AIFormationComponent AIFormation = AIFormationComponent.Cast(editableEntity.GetOwner().FindComponent(AIFormationComponent));
		if (!AIFormation) return null;
		AIFormationDefinition formation = AIFormation.GetFormation();
		if (!formation) return null;
		*/

		
		//Print(formation.GetName());
		

		
		return SCR_BaseEditorAttributeVar.CreateInt(0);
	}	
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) return;
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		
	}
};