[CinematicTrackAttribute(name:"Material Animation Track", description:"Track used for animating materials")]
class MaterialCinematicTrack : CinematicTrackBase
{
	[Attribute("0.0")]
	float m_Emissivity;
	
	private GameEntity m_GeneralEntity;
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
		
		m_GeneralEntity = GameEntity.Cast(world.FindEntityByName(strs.Get(0)));
	}
	
	override void OnApply(float time)
	{
		
		if (globalWorld)
		{
			//Finding effect entity each frame for keep working in edit time
			findEntity(globalWorld);
		}
		
		if (m_GeneralEntity)
		{			
				
			ParametricMaterialInstanceComponent paramMat = ParametricMaterialInstanceComponent.Cast(m_GeneralEntity.FindComponent(ParametricMaterialInstanceComponent));
			
			if (paramMat)
				paramMat.SetEmissiveMultiplier(m_Emissivity);
		}
	}
}
