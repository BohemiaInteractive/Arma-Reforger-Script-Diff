// Script File SCR_ToggleDamageEditorAttribute
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ToggleDamageEditorAttribute : SCR_BaseEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//WIP
		#ifndef WORKBENCH
		return null;
		#endif
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) return null;
		
		IEntity owner =  editableEntity.GetOwner();
		if (!owner) return null;
		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));
		
		if (!damageComponent) return null;
				
		return SCR_BaseEditorAttributeVar.CreateInt(1);
	}
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) return;
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		
		IEntity owner =  editableEntity.GetOwner();
		if (!owner) return;
		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));
		if (!damageComponent) return;
		
		Print("Set Damage enabled: " + var.GetInt());
	}
};