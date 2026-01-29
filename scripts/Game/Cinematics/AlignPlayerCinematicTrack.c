[CinematicTrackAttribute(name:"Align player", description:"Track used for aligning player to entity")]
class AlignPlayerCinematicTrack : CinematicTrackBase
{
	
	[Attribute(defvalue:"", desc:"Bone we want be attached to")]
	string entityToAttachTo;
	
	[Attribute("1.0", params: "0 1")]
	float m_fLerpFactor;
	
	private IEntity player;
	private IEntity entityToAttach;
	private vector transform[4];
	private vector playerTransform[4];
	private World globalWorld;
	
	override void OnInit(World world)
	{		
		globalWorld = world;
		
		array<int> playerIDs = {};
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
		foreach (int playerID : playerIDs)
		{
			player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			if (!player)
				continue;
		}
	}
	
	override void OnApply(float time)
	{			
		entityToAttach =  globalWorld.FindEntityByName(entityToAttachTo);
		
		if (entityToAttach && player)
		{
			entityToAttach.GetTransform(transform);
			player.GetTransform(playerTransform);
			
			playerTransform[0] = vector.Lerp(playerTransform[0], transform[0], m_fLerpFactor);
			playerTransform[1] = vector.Lerp(playerTransform[1], transform[1], m_fLerpFactor);
			playerTransform[2] = vector.Lerp(playerTransform[2], transform[2], m_fLerpFactor);
			playerTransform[3] = vector.Lerp(playerTransform[3], transform[3], m_fLerpFactor);
			
			player.SetTransform(playerTransform);
		}	
	}
}
