[CinematicTrackAttribute(name:"Timecode Track", description:"Track used for showing timecode")]
class TimecodeCinematicTrack : CinematicTrackBase
{
	[Attribute("false")]
	bool m_bPlay;
	
	private World globalWorld;
	private int frame;
	
	override void OnInit(World world)
	{
		
		globalWorld = world;
	}
	
	override void OnApply(float time)
	{
		
		if (m_bPlay)
		{
			frame = Math.Round(time * 60);
			DebugTextScreenSpace.Create(globalWorld, frame.ToString(), DebugTextFlags.ONCE, 20, 20);
		}
	}
}
