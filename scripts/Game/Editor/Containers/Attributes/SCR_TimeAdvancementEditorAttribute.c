// Script File 
/**
Weather attribute if it should enable/disable day Advancement
*/
// Script File 
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_TimeAdvancementEditorAttribute : SCR_BaseEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) return null;
		
		GenericEntity ent = GenericEntity.Cast(item);
		ChimeraWorld world = ent.GetWorld();
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager) return null;
		 
		return SCR_BaseEditorAttributeVar.CreateBool(weatherManager.GetIsDayAutoAdvanced());
	}
	
	//Disable respawn time if respawning is disabled
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{	
		if (isInit)
			manager.SetAttributeAsSubAttribute(SCR_DayDurationEditorAttribute);
		
		manager.SetAttributeEnabled(SCR_DayDurationEditorAttribute, var && var.GetBool());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		GenericEntity ent = GenericEntity.Cast(item);
		ChimeraWorld world = ent.GetWorld();
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager) 
			return;
		 
		bool value = var.GetBool();
		weatherManager.SetIsDayAutoAdvanced(value);
		
		if (item)
		{
			if (value)
				SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(ENotification.EDITOR_ATTRIBUTES_DAY_ADVANCE_ENABLED, playerID);
			else 
				SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(ENotification.EDITOR_ATTRIBUTES_DAY_ADVANCE_DISABLED, playerID);
		}
	}
};