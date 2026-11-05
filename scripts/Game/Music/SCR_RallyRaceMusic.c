class SCR_RallyRaceMusic : ScriptedMusic
{
	protected MusicManager m_MusicManager;
	protected bool m_bIsPlaying;
	protected bool m_bShouldPlay;

	//------------------------------------------------------------------------------------------------
	protected void OnMenuOpened()
	{
		m_MusicManager.Play(SCR_SoundEvent.SOUND_MUSIC_RALLY_MENU);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuClosed()
	{
		m_MusicManager.Stop(SCR_SoundEvent.SOUND_MUSIC_RALLY_MENU);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRaceStart(int playerID)
	{
		// menu music fades out naturally as race music takes over
		if (!m_bIsPlaying)
			m_MusicManager.Play(SCR_SoundEvent.SOUND_MUSIC_RALLY);

		m_bShouldPlay = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRaceFinished()
	{
		m_MusicManager.Stop(SCR_SoundEvent.SOUND_MUSIC_RALLY);
		m_bShouldPlay = false;
	}

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		m_MusicManager = world.GetMusicManager();
		if (!m_MusicManager)
			return;

		SCR_RaceEntity.s_OnRaceStarted.Insert(OnRaceStart);
		SCR_RaceEntity.s_OnRaceFinished.Insert(OnRaceFinished);
		SCR_RallyCustomizationMenu.s_OnMenuOpened.Insert(OnMenuOpened);
		SCR_RallyCustomizationMenu.s_OnMenuClosed.Insert(OnMenuClosed);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete()
	{
		SCR_RaceEntity.s_OnRaceStarted.Remove(OnRaceStart);
		SCR_RaceEntity.s_OnRaceFinished.Remove(OnRaceFinished);
		SCR_RallyCustomizationMenu.s_OnMenuOpened.Remove(OnMenuOpened);
		SCR_RallyCustomizationMenu.s_OnMenuClosed.Remove(OnMenuClosed);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPlay()
	{
		m_bIsPlaying = true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnStop()
	{
		m_bIsPlaying = false;
	}

	//------------------------------------------------------------------------------------------------
	override bool ShouldPlay()
	{
		return m_bShouldPlay;
	}
}
