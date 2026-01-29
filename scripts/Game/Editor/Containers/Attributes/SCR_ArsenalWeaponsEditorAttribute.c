[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ArsenalWeaponsEditorAttribute : SCR_ArsenalBaseEditorAttribute
{
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		//Set sub labels
		if (isInit)
			manager.SetAttributeAsSubAttribute(SCR_ArsenalAmmunitionModeAttribute);
		
		bool arsenalsEnabled = true;
		
		//~ Make sure the ammunition is only enabled if the arsenal is enabled
		SCR_BaseEditorAttributeVar arsenalEnabledVar;
		if (manager.GetAttributeVariable(SCR_EnableArsenalAttribute, arsenalEnabledVar))
			arsenalsEnabled = arsenalEnabledVar.GetBool();

		//~ If Weapons null or only Attachment set SCR_ArsenalAmmunitionModeAttribute disabled
		manager.SetAttributeEnabled(SCR_ArsenalAmmunitionModeAttribute, var && arsenalsEnabled && (var.GetInt() != 0 && var.GetInt() != SCR_EArsenalItemType.WEAPON_ATTACHMENT));
		
		super.UpdateInterlinkedVariables(var, manager, isInit);
	}
}
