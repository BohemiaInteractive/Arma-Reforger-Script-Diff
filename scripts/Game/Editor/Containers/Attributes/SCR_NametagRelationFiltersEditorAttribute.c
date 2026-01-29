[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_NametagRelationFiltersEditorAttribute : SCR_BaseMultiSelectPresetsEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		if (!IsGameMode(item))
			return null;

		SCR_AdditionalGameModeSettingsComponent additionalSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!additionalSettings)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateInt(additionalSettings.GetNametagRelationFilters());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		if (!IsGameMode(item))
			return;
	
		SCR_AdditionalGameModeSettingsComponent additionalSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!additionalSettings)
			return;
		
		additionalSettings.SetNametagRelationFilters(var.GetInt(), playerID)
	}
}