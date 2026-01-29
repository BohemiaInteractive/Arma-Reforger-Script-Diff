[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_FactionLoadoutBaseEditorAttribute : SCR_LoadoutBaseEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//~Todo: Disabled for now
		return null;
		
		SCR_EditableFactionComponent editableFaction = SCR_EditableFactionComponent.Cast(item);
		if (!editableFaction)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateInt(editableFaction.GetAllowedArsenalItemTypes());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_EditableFactionComponent editableFaction = SCR_EditableFactionComponent.Cast(item);
		if (!editableFaction)
			return;
		
		//Get the current and add/remove flags as all arsenal attributes edit the same enum and would otherwise override itself
		SCR_EArsenalItemType loadOutFlags = editableFaction.GetAllowedArsenalItemTypes();
		
		int newArsenalflags = var.GetInt();
		
		foreach (SCR_EditorAttributeFloatStringValueHolder value: m_aValues)
		{
			//Check if flag is true or not
			if (newArsenalflags & (int)value.GetFloatValue())
				loadOutFlags |= (int)value.GetFloatValue();
			else 
				loadOutFlags &= ~(int)value.GetFloatValue();
		}
		
		//Update the arsenal
		editableFaction.SetAllowedArsenalItemTypes(loadOutFlags);
	}
	
};