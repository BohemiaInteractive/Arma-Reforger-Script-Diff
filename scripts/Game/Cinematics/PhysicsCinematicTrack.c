[CinematicTrackAttribute(name:"Physics Track", description:"Track used for physics")]
class PhysicsCinematicTrack : CinematicTrackBase
{
	[Attribute("0 0 0")]
	vector m_velocity;
	
	[Attribute("0 0 0")]
	vector m_angularVelocity;
	
	[CinematicEventAttribute()]
	void playImpulse()
	{
		if(m_GeneralEntity)
		{
			vector velOrig = m_GeneralEntity.GetPhysics().GetVelocity();
			vector vel = {velOrig[0] + m_velocity[0], velOrig[1] + m_velocity[1], velOrig[2] + m_velocity[2]};
			m_GeneralEntity.GetPhysics().SetVelocity(vel);
			m_GeneralEntity.GetPhysics().SetAngularVelocity(m_angularVelocity);
		}
	}
	
	void findEntity(World world)
	{
		// Find particle entity by using name of track
		TStringArray strs = new TStringArray;
		GetTrackName().Split("_", strs, true);
		
		m_GeneralEntity = GenericEntity.Cast(world.FindEntityByName(strs.Get(0)));
	}
	
	private GenericEntity m_GeneralEntity;
	private World globalWorld;
	
	override void OnInit(World world)
	{
		// Find particle entity by using name of track
		findEntity(world);		
		globalWorld = world;
	}
	
	override void OnApply(float time)
	{
	}
}
