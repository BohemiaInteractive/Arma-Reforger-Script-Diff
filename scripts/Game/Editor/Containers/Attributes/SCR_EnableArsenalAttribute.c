[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_EnableArsenalAttribute : SCR_BaseEditorAttribute
{	
	//------------------------------------------------------------------------------------------------
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{		
		//Set sub labels
		if (isInit)
		{
			manager.SetAttributeAsSubAttribute(SCR_ArsenalAmmunitionModeAttribute);
			manager.SetAttributeAsSubAttribute(SCR_ArsenalEquipmentEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_ArsenalOutfitEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_ArsenalWeaponRackEquipmentEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_ArsenalWeaponRackWeaponsEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_ArsenalWeaponsEditorAttribute);
		}
		
		bool enabled = var && var.GetBool();
		
		manager.SetAttributeEnabled(SCR_ArsenalAmmunitionModeAttribute, enabled);
		manager.SetAttributeEnabled(SCR_ArsenalEquipmentEditorAttribute, enabled);
		manager.SetAttributeEnabled(SCR_ArsenalOutfitEditorAttribute, enabled);
		manager.SetAttributeEnabled(SCR_ArsenalWeaponRackEquipmentEditorAttribute, enabled);
		manager.SetAttributeEnabled(SCR_ArsenalWeaponRackWeaponsEditorAttribute, enabled);
		manager.SetAttributeEnabled(SCR_ArsenalWeaponsEditorAttribute, enabled);
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(editableEntity.GetOwner());
		if (!arsenalComponent)
			return null;

		return SCR_BaseEditorAttributeVar.CreateBool(arsenalComponent.IsArsenalEnabled());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(editableEntity.GetOwner());
		if (!arsenalComponent)
			return;
		
		arsenalComponent.SetArsenalEnabled(var.GetBool());
	}
}
