[CinematicTrackAttribute(name:"Enable Track", description:"Track used for enabling/disabling entities")]
class EnableCinematicTrack : CinematicTrackBase
{
	
	[Attribute("true")]
	bool m_bEnable;
	
	private GenericEntity m_GeneralEntity;
	private World globalWorld;
	
	override void OnInit(World world)
	{
		// Find particle entity by using name of track
		findEntity(world);	
		globalWorld = world;
	}
	
	void findEntity(World world)
	{
		// Find particle entity by using name of track
		TStringArray strs = new TStringArray;
		GetTrackName().Split("_", strs, true);
		
		m_GeneralEntity = GenericEntity.Cast(world.FindEntityByName(strs.Get(0)));
	}
	
	override void OnApply(float time)
	{
		
		if (globalWorld)
		{
			findEntity(globalWorld);
		}
		
		if (m_GeneralEntity)
		{			
			
			if(m_bEnable)
			{
				m_GeneralEntity.Show(true);				
				m_GeneralEntity.Update();
			}
			else
			{
				m_GeneralEntity.Show(false);				
				m_GeneralEntity.Update();
			}
		}
	}
}
