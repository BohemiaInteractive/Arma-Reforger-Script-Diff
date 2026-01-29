[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SetEnableDamageOtherEditorAttribute : SCR_SetEnableDamageEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		return super.ReadVariable(item, manager);
	}
	
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		if (isInit)
			manager.SetAttributeAsSubAttribute(SCR_HealthOtherEditorAttribute);
		
		bool enabledDamage =  var && var.GetBool();
		
		manager.SetAttributeEnabled(SCR_HealthOtherEditorAttribute, enabledDamage);
	}
};