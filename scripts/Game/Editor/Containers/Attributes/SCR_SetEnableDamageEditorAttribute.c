[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SetEnableDamageEditorAttribute : SCR_ValidTypeBaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//Disabled for now
		return null;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return null;
		
		if (!IsValidEntityType(editableEntity.GetEntityType()))
			return null;
		
		IEntity owner = editableEntity.GetOwner();
		if (!owner) 
			return null;
		
		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));
		if (!damageComponent) 
			return null;
		
		
		if (damageComponent.GetState() == EDamageState.DESTROYED)  
			return null;		
		
		return SCR_BaseEditorAttributeVar.CreateBool(damageComponent.IsDamageHandlingEnabled());
	}	
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		
		IEntity owner =  editableEntity.GetOwner();
		if (!owner) 
			return;
		
		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));
		if (!damageComponent) 
			return;
		
		if (damageComponent.GetState() == EDamageState.DESTROYED)  
			return;	
		
		damageComponent.EnableDamageHandling(var.GetBool());
	}
	
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
	}
};