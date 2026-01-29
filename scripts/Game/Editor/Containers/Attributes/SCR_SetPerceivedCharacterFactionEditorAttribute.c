[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SetPerceivedCharacterFactionEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		if (!IsGameMode(item))
			return null;
		
		SCR_PerceivedFactionManagerComponent perceivedManager = SCR_PerceivedFactionManagerComponent.GetInstance();
		if (!perceivedManager)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateInt(ConvertValueToIndex(perceivedManager.GetCharacterPerceivedFactionOutfitType()));
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{		
		//~ Set sub labels
		if (isInit)
			manager.SetAttributeAsSubAttribute(SCR_PunishKillingWhenDesguisedEditorAttribute);
		
		int value;
		
		if (!ConvertIndexToValue(var.GetInt(), value))
			return;
		
		bool perceivedFactionEnabled = value != SCR_EPerceivedFactionOutfitType.DISABLED;
		
		SCR_BaseEditorAttributeVar punishTeamKillingVar;
		manager.GetAttributeVariable(SCR_EnableTeamKillingPunishmentEditorAttribute, punishTeamKillingVar);
		
		bool punishTeamKilling = true;
		
		if (punishTeamKillingVar)
			punishTeamKilling = punishTeamKillingVar.GetBool();
		
		manager.SetAttributeEnabled(SCR_PunishKillingWhenDesguisedEditorAttribute, perceivedFactionEnabled && punishTeamKilling);
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_PerceivedFactionManagerComponent perceivedManager = SCR_PerceivedFactionManagerComponent.GetInstance();
		if (!perceivedManager)
			return;
		
		int value;
		
		if (!ConvertIndexToValue(var.GetInt(), value))
			return;
		
		perceivedManager.SetCharacterPerceivedFactionOutfitType_S(value, playerID);
	}
}