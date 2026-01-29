[CinematicTrackAttribute(name:"Music Track", description:"Track used for triggering music")]
class SCR_MusicCinematicTrack : CinematicTrackBase
{
	[Attribute("")]
	string m_sMusicEvent;
	
	[CinematicEventAttribute()]
	void Play()
	{
		ChimeraWorld chimeraWorld = GetGame().GetWorld();
		
		if (!chimeraWorld)
		{
			return;
		}	
		
		MusicManager musicManager = chimeraWorld.GetMusicManager();
		
		if (!musicManager)
		{
			return;
		}	

		musicManager.Play(m_sMusicEvent);
	}
	
	[CinematicEventAttribute()]
	void Stop()
	{
		ChimeraWorld chimeraWorld = GetGame().GetWorld();
		
		if (!chimeraWorld)
		{
			return;
		}	
		
		MusicManager musicManager = chimeraWorld.GetMusicManager();
		
		if (!musicManager)
		{
			return;
		}	

		musicManager.Stop(m_sMusicEvent);
	}
}
