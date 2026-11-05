[EntityEditorProps(category: "GameScripted/Rally", description: "Crowd Manager for Race, manages crowds and sends signals from race", dynamicBox: true, visible: false)]
class SCR_CrowdManagerClass : GenericEntityClass
{
}

class SCR_CrowdManager : GenericEntity
{
	protected SCR_CrowdManagerGroup m_CrowdGroupHere;
	protected SCR_CrowdManagerGroup m_CrowdGroupNext;
	protected SCR_RaceEntity m_RaceEntity;
	protected vector m_vRaceStartPosition;
	protected SCR_RaceCheckpoint m_EndCheckpoint;
	protected ref array<SCR_EntityWaypoint> m_aWPVehiclesToUnflip = {};

	const protected float CROWD_RADIUS 				= 40.0; // radius around each checkpoint where AIs are being searched for
	const protected float VEHICLE_RECOVERY_RADIUS	= 40.0; // radius around flipped vehicle where AIs are being searched for
	const protected int INDIFERENT_RACE_MOOD 		= 50;
	const protected int POSITIVE_RACE_MOOD			= 100;	// maximum mood

	const ResourceName CROWD_GROUP_PATH = "{61E76842821840FF}Prefabs/AI/Groups/CrowdManagerGroup.et";
	const ResourceName UNFLIP_VEHICLE_WP = "{158B09C431AA2D91}Prefabs/AI/Waypoints/AIWaypoint_UnflipVehicle.et";

	//------------------------------------------------------------------------------------------------
	override event protected void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		// EOnInit order between entities isn't guaranteed, so SCR_RaceCustomizationPointEntity may not
		// have registered itself with SCR_AdditionalGameModeSettingsComponent yet. Defer by one frame so
		// every entity's EOnInit has already run by the time this executes.
		GetGame().GetCallqueue().CallLater(InitCrowdManager, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void InitCrowdManager()
	{
		// finding race entity
		BaseWorld world = GetWorld();
		if (!world)
		{
			Print("Cannot find World!", LogLevel.ERROR);
			return;
		}

		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return;

		SCR_RaceCustomizationPointEntity customizationPointEntity = settings.GetCustomizationEntity();
		if (!customizationPointEntity)
		{
			Print("Cannot find Race Customization entity!", LogLevel.ERROR);
			return;
		}

		m_RaceEntity = SCR_RaceEntity.Cast(world.FindEntityByName(customizationPointEntity.GetAssignedRace()));
		if (!m_RaceEntity)
		{
			Print("Cannot find Race Entity!", LogLevel.ERROR);
			return;
		}

		// spawning dynamic group to control AIs

		PrepareRaceGroups(world);

		if (!m_CrowdGroupHere || !m_CrowdGroupNext)
		{
			Print("Cannot spawn CrowdGroups", LogLevel.ERROR);
			return;
		}

		m_RaceEntity.GetOnCheckpointsUpdated().Insert(OnCheckpointsUpdated);
		m_RaceEntity.GetOnRaceStarted().Insert(OnRaceStarted);
		m_RaceEntity.GetOnRacePrepared().Insert(OnRacePrepared);
		if (m_RaceEntity.IsRacePrepared())
			OnRacePrepared();
		
		m_RaceEntity.GetOnVehicleFlipped().Insert(OnVehicleFlipped);
		m_RaceEntity.GetOnVehicleRecovery().Insert(OnVehicleRecovery);
	}

	//------------------------------------------------------------------------------------------------
	protected void PrepareRaceGroups(BaseWorld world)
	{
		EntitySpawnParams params = new EntitySpawnParams();
		Resource groupPrefab = Resource.Load(CROWD_GROUP_PATH);

		m_CrowdGroupHere = SCR_CrowdManagerGroup.Cast(GetGame().SpawnEntityPrefab(groupPrefab, world, params));
		m_CrowdGroupNext = SCR_CrowdManagerGroup.Cast(GetGame().SpawnEntityPrefab(groupPrefab, world, params));

		// trying to restrain from dormancy
		m_CrowdGroupHere.SetImportance(SCR_EAISpawnImportance.CRITICAL);
		m_CrowdGroupNext.SetImportance(SCR_EAISpawnImportance.CRITICAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRacePrepared()
	{
		m_vRaceStartPosition = m_RaceEntity.GetStartCheckpoint().GetOrigin();
		CreateCrowd(m_vRaceStartPosition, CROWD_RADIUS, mood : INDIFERENT_RACE_MOOD);
		m_EndCheckpoint = m_RaceEntity.GetEndCheckpoint();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCheckpointsUpdated(IEntity checkpointEnt)
	{
		SCR_RaceCheckpoint thisCheckpoint = SCR_RaceCheckpoint.Cast(checkpointEnt);
		if (!thisCheckpoint)
			return;

		int mood;

		SCR_RaceCheckpoint nextCheckpoint = m_RaceEntity.FindNextCheckpoint(thisCheckpoint);
		if (nextCheckpoint)
		{
			vector crowdPosition = nextCheckpoint.GetOrigin();
			if (m_EndCheckpoint == nextCheckpoint)
				mood = POSITIVE_RACE_MOOD;
			else
				mood = Math.RandomInt(0,2) * POSITIVE_RACE_MOOD;	// either 0 or 100
			
			CreateCrowd(crowdPosition, mood: mood);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVehicleFlipped(IEntity flippedVehicle)
	{
		if (!flippedVehicle)
			return;
		
		vector vehiclePosition = flippedVehicle.GetOrigin();
		
		m_CrowdGroupHere.DissolveCrowd();
		
		if (m_CrowdGroupHere.CreateCrowd(vehiclePosition, radius : VEHICLE_RECOVERY_RADIUS, aiLimit : 6) == 0)
		{
#ifdef AI_DEBUG
			PrintFormat("CrowdManager: flipped vehicle detected at %1 but no AIs found to help", vehiclePosition);		
#endif
			return;
		}
		
		EntitySpawnParams params = new EntitySpawnParams();
		Resource wpPrefab = Resource.Load(UNFLIP_VEHICLE_WP);
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = vehiclePosition;
		
		SCR_EntityWaypoint wp = SCR_EntityWaypoint.Cast(GetGame().SpawnEntityPrefab(wpPrefab, GetGame().GetWorld(), params));
		if (!wp)
			return;
		
		wp.SetEntity(flippedVehicle);
		if (m_aWPVehiclesToUnflip.Find(wp) < 0)
			m_aWPVehiclesToUnflip.Insert(wp);
		m_CrowdGroupHere.AddWaypoint(wp);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVehicleRecovery(IEntity recoveredVehicle)
	{
		foreach (SCR_EntityWaypoint wp : m_aWPVehiclesToUnflip)
		{
			if (!wp || wp.GetEntity() != recoveredVehicle)
				continue;

			m_aWPVehiclesToUnflip.RemoveItem(wp);
			SCR_EntityHelper.DeleteEntityAndChildren(wp);
			break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRaceStarted (int playerID)
	{
		SCR_RaceCheckpoint startCheckpoint = m_RaceEntity.GetStartCheckpoint();
		m_CrowdGroupHere.UpdateMood(POSITIVE_RACE_MOOD);
	}

	//------------------------------------------------------------------------------------------------
	// \param[in] position	Position where the crowd should be created
	// \param[in] radius	Radius of the crowd for the TagManager to search for audiance
	// \param[in] aiLimit	max number of AIs in the crowd
	// \param[in] mood		mood of the crowd - cheering vs booing
	protected bool CreateCrowd(vector position, float radius = CROWD_RADIUS, int aiLimit = -1, int mood = 100)
	{
		m_CrowdGroupHere.GetCrowd(m_CrowdGroupNext);

		int crowdSize = m_CrowdGroupNext.CreateCrowd(position, radius, aiLimit);
		m_CrowdGroupNext.UpdateMood(mood);

#ifdef AI_DEBUG
		PrintFormat("Crowd manager allocated %1 spectators around position %2", crowdSize, Debug.GetPositionLinkString(position, 5), level: LogLevel.NORMAL);
		PrintFormat("Crowd manager updated mood to %1 at checkpoint %2", mood, Debug.GetPositionLinkString(position, 5), level: LogLevel.NORMAL);
#endif // AI_DEBUG

		return m_CrowdGroupNext.GetAgentsCount() > 0;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_CrowdManager(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		Activate();
	}
}
