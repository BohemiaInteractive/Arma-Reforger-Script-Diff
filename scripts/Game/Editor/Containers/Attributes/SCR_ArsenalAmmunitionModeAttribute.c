[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ArsenalAmmunitionModeAttribute : SCR_BaseFloatValueHolderEditorAttribute
{	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(editableEntity.GetOwner());
		if (!arsenalComponent)
			return null;
		
		SCR_EArsenalAttributeGroup arsenalGroups = arsenalComponent.GetEditableAttributeGroups();
		if (!(arsenalGroups & SCR_EArsenalAttributeGroup.AMMUNITION_MODE) || !(arsenalGroups & SCR_EArsenalAttributeGroup.WEAPONS))
			return null;
		
		SCR_EArsenalItemMode itemMode =  arsenalComponent.GetSupportedArsenalItemModes();
		
		int weaponsAmmunitionIndex;
		
		if ((itemMode & SCR_EArsenalItemMode.WEAPON) && (itemMode & SCR_EArsenalItemMode.AMMUNITION))
			weaponsAmmunitionIndex = 0;
		else if (itemMode & SCR_EArsenalItemMode.WEAPON)
			weaponsAmmunitionIndex = 1;
		else 
			weaponsAmmunitionIndex = 2;
		
		return SCR_BaseEditorAttributeVar.CreateInt(weaponsAmmunitionIndex);
	}
	
	//---- REFACTOR NOTE START: weaponsAmmunitionIndex seem oddly specific but hard to understand ----
	
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
		
		SCR_EArsenalItemMode itemMode =  arsenalComponent.GetSupportedArsenalItemModes();
		
		int weaponsAmmunitionIndex = var.GetInt();
		
		if (weaponsAmmunitionIndex == 0)
		{
			itemMode |= SCR_EArsenalItemMode.WEAPON;
			itemMode |= SCR_EArsenalItemMode.WEAPON_VARIANTS;
			itemMode |= SCR_EArsenalItemMode.AMMUNITION;
		}
		else if (weaponsAmmunitionIndex == 1)
		{
			itemMode |= SCR_EArsenalItemMode.WEAPON;
			itemMode |= SCR_EArsenalItemMode.WEAPON_VARIANTS;
			itemMode &= ~SCR_EArsenalItemMode.AMMUNITION;
		}
		else 
		{
			itemMode &= ~SCR_EArsenalItemMode.WEAPON;
			itemMode &= ~SCR_EArsenalItemMode.WEAPON_VARIANTS;
			itemMode |= SCR_EArsenalItemMode.AMMUNITION;
		}
		
		arsenalComponent.SetSupportedArsenalItemModes(itemMode);
	}
	
	//---- REFACTOR NOTE END ----
}
