class SCR_RacerData
{
	protected Vehicle m_Vehicle;
	protected int m_iPlayerID;
	protected float m_fRaceStartTime;
	protected float m_fRaceEndTime;
	protected int m_iCheckpointsPassed;
	protected int m_iLapsDone;
	protected float m_fMaxSpeedAchieved = int.MIN;
	protected float m_fSpeedSampleSum;
	protected int m_iSpeedSampleCount;

	protected WorldTimestamp m_FlippedVehicleWorldTimestamp;

	protected ref SCR_RaceSave m_RaceSave;
	protected ref SCR_GhostSave m_GhostSave;

	protected ref SCR_RallyCustomizationData m_CustomizationData;
	protected ResourceName m_sVehiclePrefab;

	protected TimeAndWeatherManagerEntity m_TimeWeatherManager;

	//------------------------------------------------------------------------------------------------
	Vehicle GetVehicle()
	{
		return m_Vehicle;
	}

	//------------------------------------------------------------------------------------------------
	void SetVehicle(Vehicle vehicle)
	{
		m_Vehicle = vehicle;
	}

	//------------------------------------------------------------------------------------------------
	void SetSpawnInfo(SCR_RallyCustomizationData customizationData, ResourceName vehiclePrefab)
	{
		m_CustomizationData = customizationData;
		m_sVehiclePrefab = vehiclePrefab;

		if (m_GhostSave)
			m_GhostSave.m_sVehiclePrefab = vehiclePrefab;
	}

	//------------------------------------------------------------------------------------------------
	SCR_RallyCustomizationData GetCustomizationData()
	{
		return m_CustomizationData;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetVehiclePrefab()
	{
		return m_sVehiclePrefab;
	}

	//------------------------------------------------------------------------------------------------
	void ResetForRestart()
	{
		m_fRaceStartTime = 0;
		m_fRaceEndTime = 0;
		m_iCheckpointsPassed = 0;
		m_iLapsDone = 0;
		m_fMaxSpeedAchieved = int.MIN;
		m_fSpeedSampleSum = 0;
		m_iSpeedSampleCount = 0;

		m_FlippedVehicleWorldTimestamp = null;

		m_RaceSave = new SCR_RaceSave();
		m_GhostSave = new SCR_GhostSave();
		m_GhostSave.m_aPositions = {};
		m_GhostSave.m_sVehiclePrefab = m_sVehiclePrefab;
		m_RaceSave.m_aGhost.Insert(m_GhostSave);
	}

	//------------------------------------------------------------------------------------------------
	int GetPlayerID()
	{
		return m_iPlayerID;
	}

	//------------------------------------------------------------------------------------------------
	float GetStartTime()
	{
		return m_fRaceStartTime;
	}

	//------------------------------------------------------------------------------------------------
	void SetStartTime(float time)
	{
		m_fRaceStartTime = time;
	}

	//------------------------------------------------------------------------------------------------
	int GetCheckpointsPassed()
	{
		return m_iCheckpointsPassed;
	}

	//------------------------------------------------------------------------------------------------
	void IncreaseCheckpointsPassed()
	{
		m_iCheckpointsPassed++;
	}

	//------------------------------------------------------------------------------------------------
	float GetEndTime()
	{
		return m_fRaceEndTime;
	}

	//------------------------------------------------------------------------------------------------
	void SetEndTime(float time)
	{
		m_fRaceEndTime = time;
	}

	//------------------------------------------------------------------------------------------------
	int GetLapsDone()
	{
		return m_iLapsDone;
	}

	//------------------------------------------------------------------------------------------------
	void IncreaseLapsDone()
	{
		m_iLapsDone++;
	}

	//------------------------------------------------------------------------------------------------
	void RecordMaximumSpeed()
	{
		float speed = GetVehicle().GetPhysics().GetVelocity().Length();
		if (speed > m_fMaxSpeedAchieved)
			m_fMaxSpeedAchieved = speed;

		m_fSpeedSampleSum += speed;
		m_iSpeedSampleCount++;
	}

	//------------------------------------------------------------------------------------------------
	float GetMaxAchievedSpeed()
	{
		return m_fMaxSpeedAchieved;
	}

	//------------------------------------------------------------------------------------------------
	float GetAverageSpeed()
	{
		if (m_iSpeedSampleCount <= 0)
			return 0;

		return m_fSpeedSampleSum / m_iSpeedSampleCount;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the duration since the vehicle is flipped, in milliseconds
	//! - time since vehicle is flipped (only positive)
	//! - 0 if not flipped
	//! - -1 if no vehicle
	int GetTimeSinceFlippedVehicle()
	{
		if (!m_Vehicle)
			return -1;

		if (!m_FlippedVehicleWorldTimestamp)
			return 0;

		return m_Vehicle.GetWorld().GetTimestamp().DiffMilliseconds(m_FlippedVehicleWorldTimestamp);
	}

	//------------------------------------------------------------------------------------------------
	bool IsVehicleFlipped()
	{
		return m_FlippedVehicleWorldTimestamp != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Set the racer vehicle's flipped state
	//! \param[in] flippedState true for flipped, false for not flipped
	//! \return true if the state was set, false otherwise (e.g vehicle not found)
	bool SetVehicleFlipped(bool flippedState)
	{
		if (!m_Vehicle)
			return false;

		if (flippedState)
			m_FlippedVehicleWorldTimestamp = m_Vehicle.GetWorld().GetTimestamp();
		else
			m_FlippedVehicleWorldTimestamp = null;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void WriteFrameGhostData()
	{
		vector vehicleTransform[4];
		m_Vehicle.GetTransform(vehicleTransform);

		m_GhostSave.m_aPositions.Insert(SCR_TimedPositionSave.Create(m_TimeWeatherManager.GetEngineTime() - GetStartTime(), vehicleTransform));
	}

	//------------------------------------------------------------------------------------------------
	void FinishWritingGhostData()
	{
		m_RaceSave.m_fGoalReachedTime = m_TimeWeatherManager.GetEngineTime() - GetStartTime();

		SCR_PersistentRaceData raceData = SCR_PersistentRaceData.Cast(PersistenceSystem.GetInstance().GetPersistentState(SCR_PersistentRaceData));

		array<SCR_RaceSave> races = raceData.GetRaces();
		if (!races)
			return;

		//always save the ghost if there is no previous run
		if (races.IsEmpty())
		{
			SCR_PersistentRaceData.OnRaceFinishedCode(m_RaceSave);
			return;
		}

		//we store just one ghost per race so we can always get first
		SCR_RaceSave previousGhost = races.Get(0);

		//override the previous record if we beat it
		if (previousGhost.m_fGoalReachedTime > m_RaceSave.m_fGoalReachedTime)
			SCR_PersistentRaceData.OnRaceFinishedCode(m_RaceSave);

		//in case of previous record not being beaten, nothing happens
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] playerID
	void SCR_RacerData(int playerID)
	{
		m_iPlayerID = playerID;
		m_RaceSave = new SCR_RaceSave();
		m_GhostSave = new SCR_GhostSave();
		m_GhostSave.m_aPositions = {};
		m_RaceSave.m_aGhost.Insert(m_GhostSave);

		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return;

		m_TimeWeatherManager = world.GetTimeAndWeatherManager();
//		if (!m_TimeWeatherManager)
//			return;
	}
}
