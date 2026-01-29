//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_GetINtoSelectedVehicleGroupCommand : SCR_WaypointGroupCommand
{
	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		if (isClient)
		{
			//place to place a logic that would be executed for other players
			return true;
		}
		
		if (GetWaypointPrefab().IsEmpty())
			return false;
		
		SCR_AIGroup slaveGroup = SCR_AIGroup.Cast(target);
		if (!slaveGroup)
			return false;
		
		PlayerCamera camera = GetGame().GetPlayerController().GetPlayerCamera();
		if (!camera)
			return false;
		
		Vehicle vehicle = Vehicle.Cast(cursorTarget);
		if (!vehicle)
			return false;
		
		Resource waypointPrefab = Resource.Load(GetWaypointPrefab());
		if (!waypointPrefab.IsValid())
		{
			//add custom error
			return false;
		}
		
		CheckPreviousWaypoints(slaveGroup);
				
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = vehicle.GetOrigin(); 
		
		AIWaypoint wp = AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(waypointPrefab, null, params));
		if (wp)	
			slaveGroup.AddWaypoint(wp);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown()
	{
		if (!CanBeShownInCurrentLifeState())
			return false;
		
		PlayerCamera camera = GetGame().GetPlayerController().GetPlayerCamera();
		if (!camera)
			return false;
		
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupController)
			return false;
		
		if (!CanRoleShow())
			return false;
		
		IEntity cursorTarget = camera.GetCursorTarget();
		Vehicle vehicle = Vehicle.Cast(cursorTarget);
		return (vehicle != null);
	}
}