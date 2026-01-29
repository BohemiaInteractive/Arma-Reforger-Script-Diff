[CinematicTrackAttribute(name:"General Track", description:"Track used for animating general parameters of entities")]
class GeneralCinematicTrack : CinematicTrackBase
{
	
	[Attribute("1.0", params:"0.01 1000.0")]
	float m_bScale;
	
	[Attribute("0 0 0")]
	vector m_YawPitchRoll;
	
	[Attribute("0 0 0")]
	vector m_Position;
	
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
		
		if (strs.Get(0) == "player")
		{
			if(!GetGame().GetPlayerController())
				return;
			
			m_GeneralEntity = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
			
			return;	
		}
		
		m_GeneralEntity = GenericEntity.Cast(world.FindEntityByName(strs.Get(0)));
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
			
			//Set Scale
			m_GeneralEntity.SetScale(m_bScale);
			
			//Set rotation
			m_GeneralEntity.SetYawPitchRoll(m_YawPitchRoll);
			
			vector mat[4];
			m_GeneralEntity.GetTransform(mat);
			mat[3] = m_Position;
			m_GeneralEntity.SetTransform(mat);
			
			m_GeneralEntity.Update();
		}
	}
}
