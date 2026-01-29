/**
Dynamic description for SCR_LoadoutSavingBlackListEditorAttribute to show when player saved loadouts are cleared or not cleared
*/
[BaseContainerProps(), BaseContainerCustomStringTitleField("Loadout blacklist description (CUSTOM)")]
class SCR_LoadoutSavingBlackListAttributeDynamicDescription : SCR_BaseAttributeDynamicDescription
{	
	//------------------------------------------------------------------------------------------------
	override void InitDynamicDescription(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		super.InitDynamicDescription(attribute);
		
		if (!attribute.IsInherited(SCR_LoadoutSavingBlackListEditorAttribute))
			Print("'SCR_LoadoutSavingBlackListAttributeDynamicDescription' is not attached to the 'SCR_LoadoutSavingBlackListEditorAttribute'!", LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{		
		if (!super.IsValid(attribute, attributeUi) || !attribute.IsInherited(SCR_LoadoutSavingBlackListEditorAttribute))
			return false;
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return false;
		
		return var.GetVector()[2] != 0;
	}
};
