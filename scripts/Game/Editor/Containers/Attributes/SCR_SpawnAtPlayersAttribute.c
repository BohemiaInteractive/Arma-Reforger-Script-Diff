/**
Respawn attribute to enable/disable respawn and respawn time, for getting and setting varriables in Editor Attribute window
*/
// Script File 
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SpawnAtPlayersEditorAttribute : SCR_BaseEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return null;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;
		
		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
		if (!playerSpawnPointManager)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(playerSpawnPointManager.IsPlayerSpawnPointsEnabled());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
		if (!playerSpawnPointManager)
			return;
			
		//Notification
		playerSpawnPointManager.EnablePlayerSpawnPoints(var.GetBool(), playerID);
	}
};