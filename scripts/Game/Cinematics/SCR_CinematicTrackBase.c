[CinematicTrackAttribute(name:"", description:"")]
class SCR_CinematicTrackBase : CinematicTrackBase
{	
	protected World m_World;
		
	override void OnInit(World world)
	{
		super.OnInit(world);
		m_World = world;
	}
	
	IEntity FindOwner()
	{
		if (!m_World)
		{
			return null;
		}
		
		// Get entity name from track name
		TStringArray strs = new TStringArray;
		GetTrackName().Split("_", strs, true);
		
		return m_World.FindEntityByName(strs.Get(0));
	}
	
	IEntity FindOwner(string ownerName)
	{
		if (!m_World)
		{
			return null;
		}
				
		return m_World.FindEntityByName(ownerName);
	}
}