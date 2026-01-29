[CinematicTrackAttribute(name:"Light Track", description:"Track used for animating light entities")]
class LightCinematicTrack : CinematicTrackBase
{
	[Attribute("true")]
	bool m_bEnabled;
	
	[Attribute("5.0", params:"0.2 200")]
	float m_fRadius;
	
	[Attribute("1 1 1 1", params:"srgb")]
	ref Color m_Color;
	
	[Attribute("5", params:"-10 20")]
	float m_fLV;
	
	private LightEntity m_LightEntity;
	private World globalWorld;
	
	override void OnInit(World world)
	{
		// Find light entity by using name of track
		m_LightEntity = LightEntity.Cast(world.FindEntityByName(GetTrackName()));
		
		globalWorld = world;
	}
	
	override void OnApply(float time)
	{
		
		if (globalWorld)
		{
			//Finding effect entity each frame for keep working in edit time
			m_LightEntity = LightEntity.Cast(globalWorld.FindEntityByName(GetTrackName()));
		}
			
		if (m_LightEntity)
		{
		
			//Set Radius
			m_LightEntity.SetRadius(m_fRadius);
			
			//Set if enabled
			m_LightEntity.SetEnabled(m_bEnabled);
			
			//Set color and LV intensity
			m_LightEntity.SetColor(m_Color, m_fLV);
		}
	}
}
