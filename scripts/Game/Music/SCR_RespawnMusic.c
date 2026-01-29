class SCR_RespawnMusic : ScriptedMusic
{
	protected MusicManager m_MusicManager;
	protected bool m_bIsPlaying;
	
	protected void OnRespawnMenuOpen()
	{
		if (!m_bIsPlaying)
			m_MusicManager.Play(SCR_SoundEvent.SOUND_RESPAWNMENU);
	}

	protected void OnRespawnMenuClosed()
	{
		m_MusicManager.Stop(SCR_SoundEvent.SOUND_RESPAWNMENU);
	}
	
	override void Init() 
	{
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		m_MusicManager = world.GetMusicManager();
		if (!m_MusicManager)
			return;
		
		// Register respawn menu screen events
		SCR_DeployMenuBase.SGetOnMenuOpen().Insert(OnRespawnMenuOpen);
		SCR_PlayerDeployMenuHandlerComponent.SGetOnMenuClosed().Insert(OnRespawnMenuClosed);
	}
	
	override void OnDelete()
	{
		SCR_DeployMenuBase.SGetOnMenuOpen().Remove(OnRespawnMenuOpen);
		SCR_PlayerDeployMenuHandlerComponent.SGetOnMenuClosed().Remove(OnRespawnMenuClosed);		
	}
	
	override void OnPlay()
	{
		m_bIsPlaying = true;
	}
	
	override void OnStop()
	{
		m_bIsPlaying = false;
	}
}