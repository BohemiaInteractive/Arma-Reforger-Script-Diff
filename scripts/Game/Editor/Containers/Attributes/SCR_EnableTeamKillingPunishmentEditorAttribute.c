[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_EnableTeamKillingPunishmentEditorAttribute : SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		if (!IsGameMode(item))
			return null;
		
		SCR_AdditionalGameModeSettingsComponent additionalGameSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!additionalGameSettings)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(additionalGameSettings.IsTeamKillingPunished());
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		SCR_BaseEditorAttributeVar perceivedFactionVar;
		manager.GetAttributeVariable(SCR_SetPerceivedCharacterFactionEditorAttribute, perceivedFactionVar);
		
		bool perceivedFactionEnabled;
		
		if (perceivedFactionVar)
			perceivedFactionEnabled = perceivedFactionVar.GetInt() != 0;
		
		manager.SetAttributeEnabled(SCR_PunishKillingWhenDesguisedEditorAttribute, perceivedFactionEnabled && var.GetBool());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_AdditionalGameModeSettingsComponent additionalGameSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!additionalGameSettings)
			return;
		
		additionalGameSettings.SetEnableTeamKillPunishment_S(var.GetBool(), playerID);
	}
}
