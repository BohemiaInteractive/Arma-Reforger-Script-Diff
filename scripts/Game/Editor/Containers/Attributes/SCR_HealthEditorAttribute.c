/**
Entity Health Attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_HealthEditorAttribute : SCR_ValidTypeBaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//~ With the new health system health can no longer be set with just a slider.
		//~ Todo: Create health system for individual parts that can be damaged
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
		
		return SCR_BaseEditorAttributeVar.CreateFloat(Math.Round(damageComponent.GetHealthScaled() * 100));
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
		
		if (!damageComponent.IsDamageHandlingEnabled())
			return;
		
		float health = (var.GetFloat() / 100);
		
		HitZone defaultHitzone = damageComponent.GetDefaultHitZone();
		
		defaultHitzone.SetHealthScaled(health);
	}
};