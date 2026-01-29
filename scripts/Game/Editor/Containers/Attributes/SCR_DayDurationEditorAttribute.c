/**
Day duration Attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_DayDurationEditorAttribute: SCR_BaseValueListEditorAttribute
{
	//---- REFACTOR NOTE START: Hardcoded values that are not clear 
	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return null;
		
		GenericEntity ent = GenericEntity.Cast(item);
		ChimeraWorld world = ent.GetWorld();
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager) return null;
		
		return SCR_BaseEditorAttributeVar.CreateFloat(86400 / timeManager.GetDayDuration());
	}
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) return;
		
		GenericEntity ent = GenericEntity.Cast(item);
		ChimeraWorld world = ent.GetWorld();
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager) return;

		timeManager.SetDayDuration(86400 / var.GetFloat());

		//Notification
		if (item)
		{
			int notificationInt = var.GetFloat() * 10;
			SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(ENotification.EDITOR_ATTRIBUTES_DAY_DURATION_CHANGED, playerID, notificationInt * 100);
		}
	}
	
	//---- REFACTOR NOTE END ----
};
