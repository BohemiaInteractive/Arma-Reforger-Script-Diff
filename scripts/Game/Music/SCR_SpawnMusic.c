class SCR_SpawnMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
	//~ On local player spawned
	protected void OnPlayerSpawned()
	{
		m_MusicManager.Play(SCR_SoundEvent.SOUND_ONSPAWN);	
	}
	
	override void Init() 
	{
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		m_MusicManager = world.GetMusicManager();
		if (!m_MusicManager)
			return;
		
		SCR_RespawnComponent.SGetOnLocalPlayerSpawned().Insert(OnPlayerSpawned);
	}
	
	override void OnDelete()
	{
		if (!m_MusicManager)
			return;
		
		SCR_RespawnComponent.SGetOnLocalPlayerSpawned().Remove(OnPlayerSpawned);
	}
}