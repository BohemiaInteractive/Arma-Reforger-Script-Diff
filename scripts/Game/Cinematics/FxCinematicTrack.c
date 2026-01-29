[CinematicTrackAttribute(name:"FX Track", description:"Track used for animating FX entities")]
class FxCinematicTrack : CinematicTrackBase
{
	[Attribute("0.0", params:"0.0 100.0")]
	float m_fTime;
	
	private ParticleEffectEntity m_ParticleEntity;
	private World globalWorld;
	
	[CinematicEventAttribute()]
	void Play()
	{
		if (globalWorld)
			m_ParticleEntity = ParticleEffectEntity.Cast(globalWorld.FindEntityByName(GetTrackName()));
		
		if (m_ParticleEntity)
			m_ParticleEntity.Stop();
			m_ParticleEntity.Play();
	}
	
	[CinematicEventAttribute()]
	void Stop()
	{
		if (globalWorld)
			m_ParticleEntity = ParticleEffectEntity.Cast(globalWorld.FindEntityByName(GetTrackName()));
		
		if (m_ParticleEntity)
			m_ParticleEntity.Stop();
	}
	
	
	[CinematicEventAttribute()]
	void SetTime()
	{
		if (globalWorld)
			m_ParticleEntity = ParticleEffectEntity.Cast(globalWorld.FindEntityByName(GetTrackName()));
		
		Particles particles;
		
		if(m_ParticleEntity)
			particles = m_ParticleEntity.GetParticles();
		
		if (particles)
			particles.SimulateMultiStep(m_fTime, 30);
	}
	
	override void OnInit(World world)
	{
		// Find particle entity by using name of track
		m_ParticleEntity = ParticleEffectEntity.Cast(world.FindEntityByName(GetTrackName()));
		
		globalWorld = world;
	}
}
