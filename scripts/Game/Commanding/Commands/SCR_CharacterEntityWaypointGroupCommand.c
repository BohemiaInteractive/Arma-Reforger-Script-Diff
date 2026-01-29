//command that takes an EntityWaypoint, and executes the waypoint with entity under cursor, or player controller entity if Character entity not found
[BaseContainerProps(), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_CharacterEntityWaypointGroupCommand : SCR_WaypointGroupCommand
{
	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		if (isClient && playerID == SCR_PlayerController.GetLocalPlayerId())
		{
			//SpawnWPVisualization(targetPosition, playerID);
			return true;
		}

		//SpawnWPVisualization(targetPosition, playerID);
		if (m_sWaypointPrefab.Empty || !target || !targetPosition)
			return false;
		
		IEntity targetEntity;
		if (cursorTarget)
			targetEntity = cursorTarget.GetRootParent();
		
		return CreateCharacterEntityWP(targetEntity, target, targetPosition, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	bool CreateCharacterEntityWP(IEntity cursorTarget, IEntity targetGroup, vector targetPosition, int playerID)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(cursorTarget);
		if (!character)
			character = ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));
		
		if (!character)
			return false;

		Resource waypointPrefab = Resource.Load(GetWaypointPrefab());
		if (!waypointPrefab.IsValid())
		{
			Print("SCR_CharacterEntityWaypointGroupCommand:: Invalid prefab path", LogLevel.ERROR);
			return false;
		}
		
		SCR_AIGroup slaveGroup = SCR_AIGroup.Cast(targetGroup);
		if (!slaveGroup)
			return false;	
					
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = character.GetOrigin(); 
	
		CheckPreviousWaypoints(slaveGroup);
	
		SCR_EntityWaypoint wp = SCR_EntityWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(waypointPrefab, null, params));
		if (!wp)
			return false;
		
		if (m_bForceCommand)
			wp.SetPriorityLevel(SCR_AIActionBase.PRIORITY_LEVEL_PLAYER);
	
		wp.SetCompletionRadius(m_fCompletionRadius);
		wp.SetEntity(character);
		slaveGroup.AddWaypoint(wp);

		return true;
	}
}