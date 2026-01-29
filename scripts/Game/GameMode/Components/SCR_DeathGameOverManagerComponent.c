//! Death gameover logic created for single player missions
[ComponentEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_DeathGameOverManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_DeathGameOverManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute("{F94857313E410E60}UI/layouts/HUD/GameOver/EndScreen/EndScreen_Death.layout", params: "layout")]
	protected ResourceName m_sDeathScreenGameOverOverwrite;
	
	[Attribute("1", desc: "If true will pause the game when the player dies and game over is called")]
	protected bool m_bPauseGameOnDeath;
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SINGLEPLAYER_DISABLE_GAMEOVER))
			return;
		
		if (!GetGameMode().IsMaster())
			return;
	
		if (SCR_PlayerController.GetLocalPlayerId() != instigatorContextData.GetVictimPlayerID())
			return;
		
		if (m_bPauseGameOnDeath)
		{
			ChimeraWorld world = GetGame().GetWorld();
			if (world)
				world.PauseGameTime(true);
		}
		
		SCR_GameOverScreenManagerComponent gameOverManager = SCR_GameOverScreenManagerComponent.Cast(GetGameMode().FindComponent(SCR_GameOverScreenManagerComponent)); 
		if (gameOverManager)
			gameOverManager.SetGameOverScreenOverWrite(m_sDeathScreenGameOverOverwrite);
		
		GetGameMode().EndGameMode(SCR_GameModeEndData.CreateSimple(EGameOverTypes.DEATH_SCREEN_DEFAULT));
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_DeathGameOverManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_SINGLEPLAYER, "Singleplayer", "");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SINGLEPLAYER_DISABLE_GAMEOVER, "", "Disable Game Over", "Singleplayer");
	}
}
