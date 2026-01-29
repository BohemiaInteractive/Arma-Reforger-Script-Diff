//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_FollowGroupCommand : SCR_WaypointGroupCommand
{
	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		if (!m_sWaypointPrefab || !target || !targetPosition)
			return false;
		
		if (isClient)
		{
			//place to place a logic that would be executed for other players
			return true;
		}
		
		SCR_AIGroup slaveGroup = SCR_AIGroup.Cast(target);
		if (!slaveGroup)
			return false;
		
		Resource waypointPrefab = Resource.Load(GetWaypointPrefab());
		if (!waypointPrefab.IsValid())
			return false;
		
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController)
			return false;
		IEntity controlledEntity = playerController.GetControlledEntity();
		if (!controlledEntity)
			return false;
		
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = controlledEntity.GetOrigin(); 
	
		CheckPreviousWaypoints(slaveGroup);
		
		// Hotfix: teleport group to leader, otherwise if group is very split up, bots will try to regroup somewhere in the middle, far away
		vector targetTransform[4];
		controlledEntity.GetTransform(targetTransform);
		slaveGroup.SetTransform(targetTransform);
		slaveGroup.Update();

		// Crucial part about follow waypoint: we must set entity so it knows who to follow
		// Also it is of SCR_EntityWaypoint class, not SCR_AIWaypoint.
		SCR_EntityWaypoint wp = SCR_EntityWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(waypointPrefab, null, params));
		if (!wp)
			return false;
		wp.SetEntity(controlledEntity);
		
		if (m_bForceCommand)
			wp.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_PLAYER);
		slaveGroup.AddWaypoint(wp);
		
		AIGroupMovementComponent slaveGroupMovementComp = AIGroupMovementComponent.Cast(slaveGroup.FindComponent(AIGroupMovementComponent));
		if (slaveGroupMovementComp)
			slaveGroupMovementComp.SetFormationDisplacement(1);
		
		return true;
	}
}