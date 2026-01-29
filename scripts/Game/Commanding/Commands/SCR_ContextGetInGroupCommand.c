[BaseContainerProps(), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_ContextGetIngroupCommand : SCR_WaypointGroupCommand
{
	[Attribute(desc: "Prefab of the AI waypoint used in this command to get into specific vehicle", params: "et class=AIWaypoint")]
	protected ResourceName m_sSpecificEntityWP;
	
	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		if (isClient && playerID == SCR_PlayerController.GetLocalPlayerId())
		{
			//SpawnWPVisualization(targetPosition, playerID);
			return true;
		}

		//SpawnWPVisualization(targetPosition, playerID);
		if (!m_sWaypointPrefab || !m_sSpecificEntityWP || !target || !targetPosition)
			return false;
		
		return DecideAndSetWP(cursorTarget, target, targetPosition, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	bool DecideAndSetWP(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID)
	{				
		IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!playerEntity)
			return false;
		
		ChimeraCharacter playerCharacter = ChimeraCharacter.Cast(playerEntity);		
		if (!playerCharacter)
			return false;
		
		if (playerCharacter.IsInVehicle())
		{
			CompartmentAccessComponent compartmentComp = playerCharacter.GetCompartmentAccessComponent();
			if (!compartmentComp)
				return false;
			
			IEntity vehicleIn = compartmentComp.GetVehicleIn(playerCharacter);
			if (!vehicleIn)
				return false;
			
			return CreateBoardingEntityWP(target, vehicleIn, vehicleIn.GetOrigin());
		}
		
		if (cursorTarget)
		{
			Vehicle cursorVehicle = Vehicle.Cast(cursorTarget.GetRootParent());
			if (cursorVehicle)
				return CreateBoardingEntityWP(target, cursorVehicle, cursorVehicle.GetOrigin());
		}
		
		//handle default logic
		return SetWaypointForAIGroup(target, targetPosition, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	bool CreateBoardingEntityWP(IEntity targetGroup, IEntity entity, vector position)
	{
		Resource waypointPrefab = Resource.Load(m_sSpecificEntityWP);
		if (!waypointPrefab.IsValid())
		{
			Print("SCR_ContextGetInGroupCommand:: Invalid prefab path", LogLevel.ERROR);
			return false;
		}
		
		SCR_AIGroup slaveGroup = SCR_AIGroup.Cast(targetGroup);
		if (!slaveGroup)
			return false;	
					
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position; 
	
		CheckPreviousWaypoints(slaveGroup);
	
		SCR_BoardingEntityWaypoint wp = SCR_BoardingEntityWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(waypointPrefab, null, params));
		if (!wp)
			return false;
		
		if (m_bForceCommand)
			wp.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_PLAYER);
	
		wp.SetCompletionRadius(m_fCompletionRadius);
		wp.SetEntity(entity);
		slaveGroup.AddWaypoint(wp);
		
		return true;
	}
}