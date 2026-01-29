class SCR_DeathMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
		//~ When the player died
	protected void OnPlayerDied(notnull SCR_InstigatorContextData instigatorContextData)
	{	
		if (instigatorContextData.GetVictimPlayerID() != SCR_PlayerController.GetLocalPlayerId())
			return;
		
		m_MusicManager.Play(SCR_SoundEvent.SOUND_ONDEATH);
	}
	
	override void Init() 
	{
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		m_MusicManager = world.GetMusicManager();
		if (!m_MusicManager)
			return;
		
			//~ On player spawned and died register
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerKilled().Insert(OnPlayerDied);
		}
		
	}
	
	override void OnDelete()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerKilled().Remove(OnPlayerDied);
		}
	}
}