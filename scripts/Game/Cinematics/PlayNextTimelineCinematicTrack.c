[CinematicTrackAttribute(name:"Play Next Cinematic Track", description:"Track used for playing another cinematic track")]
class PlayNextTimelineCinematicTrack : CinematicTrackBase
{
	[Attribute("")]
	string m_sCinematicScene;

	private ChimeraWorld globalWorld;	
	private CinematicEntity cineToPlay;
	
	[CinematicEventAttribute()]
	void PlayNext()
	{
		cineToPlay = CinematicEntity.Cast(globalWorld.FindEntityByName(m_sCinematicScene));
		if (cineToPlay)
			cineToPlay.Play();
	}
	
	override void OnFinish()
	{
		PlayNext();
	}
	
	override void OnInit(World world)
	{
		globalWorld = world;
	}
}
