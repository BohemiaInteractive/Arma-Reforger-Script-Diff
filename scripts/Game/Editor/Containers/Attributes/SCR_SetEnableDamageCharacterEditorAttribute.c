[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SetEnableDamageCharacterEditorAttribute : SCR_SetEnableDamageEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return null;
		
		//Never set players invincible
		if (editableEntity.GetPlayerID() > 0)
			return null;
		
		return super.ReadVariable(item, manager);
	}
	
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{		
		if (isInit)
		{
			manager.SetAttributeAsSubAttribute(SCR_HealthCharacterEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_BloodEditorAttribute);
		}
			
		
		bool enabledDamage =  var && var.GetBool();
		
		manager.SetAttributeEnabled(SCR_HealthCharacterEditorAttribute, enabledDamage);
		manager.SetAttributeEnabled(SCR_BloodEditorAttribute, enabledDamage);
	}
};