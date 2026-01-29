[CinematicTrackAttribute(name:"GetWorld Track", description:"Track used for getting world")]
class GetWorldCinematicTrack : CinematicTrackBase
{
	static World globalWorld;
	
	override void OnInit(World world)
	{
		globalWorld = world;
	}
	
	static World GetWorld()
	{
		return globalWorld;
	}
}
