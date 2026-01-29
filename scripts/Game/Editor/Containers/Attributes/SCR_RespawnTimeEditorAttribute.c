/**
Respawn attribute to enable/disable respawn and respawn time, for getting and setting varriables in Editor Attribute window
*/
// Script File 
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_RespawnTimeEditorAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) return null;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode) return null;
		SCR_RespawnTimerComponent respawnTimeComponent = SCR_RespawnTimerComponent.Cast(gameMode.FindComponent(SCR_RespawnTimerComponent));
		if (!respawnTimeComponent) return null;
		
		float value = respawnTimeComponent.GetRespawnTime();
		return SCR_BaseEditorAttributeVar.CreateFloat(value);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) return;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode) return;
		SCR_RespawnTimerComponent respawnTimeComponent = SCR_RespawnTimerComponent.Cast(gameMode.FindComponent(SCR_RespawnTimerComponent));
		if (!respawnTimeComponent) return;
		float value = var.GetFloat();

		
		if (value != respawnTimeComponent.GetRespawnTime())
		{
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_RESPAWN_TIME_CHANGED, playerID, value * 1000);
		}
		
		respawnTimeComponent.SetRespawnTime(value);
	}	
};