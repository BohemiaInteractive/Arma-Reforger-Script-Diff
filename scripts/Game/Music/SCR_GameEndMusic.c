class SCR_GameEndMusic : ScriptedMusic
{
	MusicManager m_MusicManager;
	
	//~ Delay to make sure the OnSpawn music is played correctly after the respawn menu music
	protected void OnGameEnd(string audioString)
	{	
		SetName(audioString);
		m_MusicManager.Play(audioString);	
	}
	
	override void Init() 
	{
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		m_MusicManager = world.GetMusicManager();
		if (!m_MusicManager)
			return;
		
		
		SCR_GameOverScreenManagerComponent.s_OnEndGame.Insert(OnGameEnd);
	}
	
	override void OnDelete()
	{
		SCR_GameOverScreenManagerComponent.s_OnEndGame.Remove(OnGameEnd);
	}
}