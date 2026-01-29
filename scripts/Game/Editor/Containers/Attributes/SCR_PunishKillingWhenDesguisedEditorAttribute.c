[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_PunishKillingWhenDesguisedEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		if (!IsGameMode(item))
			return null;
		
		SCR_PerceivedFactionManagerComponent perceivedManager = SCR_PerceivedFactionManagerComponent.GetInstance();
		if (!perceivedManager)
			return null;
		
		int index = ConvertValueToIndex(perceivedManager.GetPunishmentKillingWhileDisguisedFlags());
		if (index <= -1)
		{
			Print("'SCR_PunishKillingWhenDesguisedEditorAttribute' trying to get the flag values but the combination of flags does not exist in the attribute data. Make sure to add the value to the attribute", LogLevel.WARNING);
			index = 0;
		}
		
		return SCR_BaseEditorAttributeVar.CreateInt(index);
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_PerceivedFactionManagerComponent perceivedManager = SCR_PerceivedFactionManagerComponent.GetInstance();
		if (!perceivedManager)
			return;
		
		float value;
		if (!ConvertIndexToValue(var.GetInt(), value))
			return;
		
		perceivedManager.SetPunishmentKillingWhileDisguisedFlags_S(value, playerID);
	}
}
