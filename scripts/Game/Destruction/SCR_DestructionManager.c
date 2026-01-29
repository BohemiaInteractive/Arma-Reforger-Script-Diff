class SCR_DestructionManagerClass : DestructionManagerClass
{
}

class SCR_DestructionManager : DestructionManager
{
	//map of rpl id of regional destruction manager
	//and array containing the position + center of the interior bounding boxes of buildings that have been processed in this client
	ref map <RplId, ref array<ref vector>> m_RegionalManagerHandledBoxes = new map<RplId, ref array<ref vector>>();
	
	static SCR_DestructionManager GetDestructionManagerInstance()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		
		return SCR_DestructionManager.Cast(world.GetDestructionManager());
	}	
}