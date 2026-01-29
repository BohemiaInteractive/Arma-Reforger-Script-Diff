//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_PatrolGroupCommand : SCR_WaypointGroupCommand
{
	static const ResourceName cycleWPResource = "{35BD6541CBB8AC08}Prefabs/AI/Waypoints/AIWaypoint_Cycle.et";
	
	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		if (isClient && playerID == SCR_PlayerController.GetLocalPlayerId())
			return true;

		if (!target || !targetPosition)
			return false;
		
		return Patrol(target, targetPosition, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	bool Patrol(IEntity target, vector targetPosition, int playerID)
	{
		SCR_AIGroup slaveGroup = SCR_AIGroup.Cast(target);
		if (!slaveGroup)
			return false;
		
		Resource waypointPrefab = Resource.Load(GetWaypointPrefab());
		if (!waypointPrefab.IsValid())
			return false;
		
		Resource cycleWPPrefab = Resource.Load(cycleWPResource);
		if (!cycleWPPrefab.IsValid())
			return false;
		
		SCR_PlayerController orderingPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
		if (!orderingPlayerController)
			return false;
		
		IEntity playerEntity = orderingPlayerController.GetControlledEntity();
		if (!playerEntity)
			return false;
		
		CheckPreviousWaypoints(slaveGroup);	
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = playerEntity.GetOrigin(); 

		array<AIWaypoint> waypoints = {};
		
		PlayerController controller = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!controller)
			return false;
		
		SCR_PlayerControllerCommandingComponent commandingComp = SCR_PlayerControllerCommandingComponent.Cast(controller.FindComponent(SCR_PlayerControllerCommandingComponent));
		if (!commandingComp)
			return false;
		
		waypoints.Insert(SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(waypointPrefab, null, params)));
		waypoints[0].SetCompletionRadius(m_fCompletionRadius);
		commandingComp.DrawWaypointVisualization(playerEntity.GetOrigin(), m_fCompletionRadius);
		
		params.Transform[3] = targetPosition; 
		waypoints.Insert(SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(waypointPrefab, null, params)));
		waypoints[1].SetCompletionRadius(m_fCompletionRadius);
		commandingComp.DrawWaypointVisualization(targetPosition, m_fCompletionRadius, false);
		
		AIWaypointCycle waypointCycle = AIWaypointCycle.Cast(GetGame().SpawnEntityPrefabLocal(cycleWPPrefab, null, params)); 
		
		if (!waypointCycle)
			return false;	
		
		waypointCycle.SetWaypoints(waypoints);
		
		slaveGroup.AddWaypoint(waypointCycle);
		
		return true;
	}
}