//------------------------------------------------------------------------------------------------
class SCR_PlayerData : JsonApiStruct
{
	protected int m_iPlayerID;

	//------------------------------------------------------------------------------------------------
	/*!
	Stats that we keep updating
	*/
	protected ref array<float> m_aStats = {};
	
	//temporal stats single-session based that are only tracked for analytic purposes and not stored in the backend storage
	protected int m_iSecondsAsController = 0, m_iSecondsAsKeyboard = 0;
	protected int m_iLastEvaluationCurrentController = 0;

	//------------------------------------------------------------------------------------------------
	/*!
	Original stats
	*/
	protected ref array<float> m_aPreviousStats = {};

	//------------------------------------------------------------------------------------------------
	/*!
	Possibly criminal stats - We use this to accumulate actions and afterwards decide whether they are criminal or not.
	*/
	protected ref array<float> m_aAccumulatedActions = {};
	protected int m_iAccumulatedActionsTick;
	protected int m_iLatestActionTick;
	protected int m_iLatestCriminalScoreUpdateTick;
	protected float m_fCriminalScore;
	
	//------------------------------------------------------------------------------------------------
	/*!
	TimeOut in case the player is kicked or banned on the current session for notification purposes
	*/
	protected int m_iTimeOut = 0;


	//------------------------------------------------------------------------------------------------
	/*!
	Stats difference - We need this variable because we need a pointer for it to be used on TD from C++ side
	*/
	protected ref array<float> m_aStatsGained = {};

	//------------------------------------------------------------------------------------------------
	/*!
	DataEvent to send stats to the database for tracking purposes
	*/
	ref SCR_PlayerDataEvent dataEvent = new SCR_PlayerDataEvent;

	//------------------------------------------------------------------------------------------------
	/*!
	We store in m_aEarnt the number of points provided by each of the fields to the specializations, and the total sum for each specialization in the corresponding field (sp0, sp1, etc)
	*/
	protected ref array<float> m_aEarnt = {};

	//------------------------------------------------------------------------------------------------
	/*!
	Backend callbacks necessary to make the request to load and store the characterdata from the player's profile
	*/
	protected ref BackendCallback m_CharacterDataCallback = new BackendCallback();
	protected ref BackendCallback m_StoringCallback = new BackendCallback();

	//------------------------------------------------------------------------------------------------
	/*!
	Starting session tick to calculate session duration
	*/
	protected int m_iSessionStartedTickCount;

	//------------------------------------------------------------------------------------------------
	/*!
	Empty Profile
	*/
	protected bool m_bIsEmptyProfile;

	//------------------------------------------------------------------------------------------------
	/*!
	Invoker that notifies UI screens listening to it that the data object is ready
	*/
	protected ref ScriptInvoker m_OnDataReady = new ScriptInvoker();
	
	static ref ScriptInvoker s_OnStatAdded = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	/*!
	Stats configs
	*/
	protected ref SCR_PlayerDataConfigs m_Configs;
	
	/* TODO: Finish rework of player data using StoreDataModule */
	
	//Accumulation of stats in order to look for warcrimes on time slices
	bool accumulationEnabled = false;

	//------------------------------------------------------------------------------------------------
	void SCR_PlayerData (int id, bool hasPlayerBeenAuditted, bool requestFromBackend = true)
	{
		if (!GetGame().GetBackendApi())
			return;

		RegV("m_aStats");

		m_Configs = SCR_PlayerDataConfigs.GetInstance();
		m_iPlayerID = id;

		m_iSessionStartedTickCount = System.GetTickCount();

		typename tEnum = SCR_EDataStats;
		for (int i = tEnum.GetVariableCount(); i > 0; i--)
		{
			m_aAccumulatedActions.Insert(0);
		}

		if (requestFromBackend)
			RequestLoadData(hasPlayerBeenAuditted);
		else if (!hasPlayerBeenAuditted)
			LoadEmptyProfile();
		
		if (!GetGame().GetDataCollector())
			return;
		
		accumulationEnabled = GetGame().GetDataCollector().FindModule(SCR_DataCollectorCrimesModule) != null;
		
		m_iLastEvaluationCurrentController = m_iSessionStartedTickCount;
		EInputDeviceType device = GetGame().GetInputManager().GetLastUsedInputDevice();
		UpdateTrackingController(device, device);
		GetGame().OnInputDeviceUserChangedInvoker().Insert(UpdateTrackingController);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_PlayerData GetPlayerData(int playerID)
	{
		return GetGame().GetDataCollector().GetPlayerData(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Called manually from the SCR_DataCollectorCrimesModule
	void SetTimeOut(int time)
	{
		m_iTimeOut = time;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTimeOut()
	{
		return m_iTimeOut;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateTrackingController(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		int currentTickCount = System.GetTickCount();
		
		int newTime = (currentTickCount - m_iLastEvaluationCurrentController) * 0.001;
		switch (oldDevice)
		{
			case EInputDeviceType.GAMEPAD: case EInputDeviceType.JOYSTICK:
				m_iSecondsAsController += newTime;
				break;
			case EInputDeviceType.MOUSE: case EInputDeviceType.KEYBOARD: 
				m_iSecondsAsKeyboard += newTime;
				break;
		}
		
		m_iLastEvaluationCurrentController = currentTickCount;
	}

	//------------------------------------------------------------------------------------------------
	//! Overrides previous stats array with the provided one
	//! To be used on clients only!
	void SetPreviousStats(array<float> previousStats)
	{
		m_aPreviousStats = previousStats;
	}

	//------------------------------------------------------------------------------------------------
	array<float> GetPreviousStats()
	{
		return m_aPreviousStats;
	}

	//------------------------------------------------------------------------------------------------
	//! Overrides stats array with the provided one
	//! To be used on clients only!
	void SetStats(array<float> stats)
	{
		m_aStats = stats;
	}

	//------------------------------------------------------------------------------------------------
	array<float> GetStats()
	{
		return m_aStats;
	}

	//------------------------------------------------------------------------------------------------
	void RequestLoadData(bool HasPlayerBeenAuditted = false)
	{
		BackendApi ba = GetGame().GetBackendApi();

		if (!ba)
		{
			Print("SCR_PlayerData:RequestLoadData: Backend api is null!", LogLevel.WARNING);
			m_bIsEmptyProfile = true;
			return;
		}

		if (HasPlayerBeenAuditted)
		{
			ba.PlayerData(this, m_iPlayerID);
			BackendDataReady();
			return;
		}

		if (!m_CharacterDataCallback)
		{
			Print("SCR_PlayerData:RequestLoadData: The character data callback is null", LogLevel.WARNING);
			m_bIsEmptyProfile = true;
			return;
		}

		if (ba.IsAuthenticated())
		{
			Print("Making menuCallback request for PlayerData", LogLevel.VERBOSE);
			m_CharacterDataCallback.SetOnSuccess(BackendDataReady);
			m_CharacterDataCallback.SetOnError(LoadEmptyProfile);
			ba.PlayerRequest(EBackendRequest.EBREQ_GAME_CharacterGet, m_CharacterDataCallback, this, m_iPlayerID); //ID 0 refers to the local player
			//The callback has a reference to this instance so it will automatically call the BackendDataReady or the LoadEmptyProfile methods
		}
		else
		{
			Print("SCR_PlayerData:RequestLoadData: Requesting data before player has been authenticated!", LogLevel.WARNING);
			m_bIsEmptyProfile = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetDataReadyInvoker()
	{
		return m_OnDataReady;
	}

	//------------------------------------------------------------------------------------------------
	void BackendDataReady()
	{
		typename characterDataEnum = SCR_EDataStats;

		if (m_aStats.Count() != characterDataEnum.GetVariableCount())
		{
			Print("SCR:PlayerDataStats:FillWithProfile: The CharacterData from this player seems incomplete or empty. A full profile should have "+characterDataEnum.GetVariableCount()+" stats, but this one has "+m_aStats.Count()+". Is it a new player?", LogLevel.VERBOSE);
		
			if (!m_aStats.IsEmpty())
			{
				Print("SCR_PlayerData of this player is not empty, but the size is not correct either. We will override their profile with a new one, which will erase their old profile.", LogLevel.DEBUG);
			}
			
			LoadEmptyProfile();
			return;
		}
		
		m_bIsEmptyProfile = false;
		m_aPreviousStats.InsertAll(m_aStats);
		m_OnDataReady.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	void LoadEmptyProfile()
	{
		typename characterDataEnum = SCR_EDataStats;
		int count = characterDataEnum.GetVariableCount();
		
		m_aStats.Clear();

		for (int i = 0; i < count; i++)
		{
			if (i == SCR_EDataStats.LEVEL_EXPERIENCE)
				m_aStats.Insert(m_Configs.XP_NEEDED_FOR_LEVEL);
			else if (i == SCR_EDataStats.CRIME_ACCELERATION)
				m_aStats.Insert(m_Configs.GetMinAcceleration());
			else
				m_aStats.Insert(0);
				
		}

		m_bIsEmptyProfile = true;

		m_aPreviousStats.InsertAll(m_aStats);
		m_OnDataReady.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	sealed void StoreProfile()
	{
		if (!m_CharacterDataCallback || !m_aStats || m_aStats.IsEmpty())
			return;

		BackendApi ba = GetGame().GetBackendApi();
		if (!ba)
			return;

		if (!IsDataProgressionReady())
			CalculateStatsChange();

		#ifndef WORKBENCH
			ba.PlayerRequest(EBackendRequest.EBREQ_GAME_CharacterUpdateS2S, m_StoringCallback, this, m_iPlayerID);
		#else
			ba.PlayerRequest(EBackendRequest.EBREQ_GAME_DevCharacterUpdate, m_StoringCallback, this, m_iPlayerID);
		#endif

		//SCR_PlayerDataEvent dataEvent = new SCR_PlayerDataEvent();
		//dataEvent.MetersWalked = 99999;
		//GetGame().GetStatsApi().PlayerEvent(m_iPlayerID, dataEvent);
		//Print("Data event sent from Store Profile!!", LogLevel.DEBUG);
	}

	//------------------------------------------------------------------------------------------------
	Managed GetDataEventStats()
	{
		if (!IsDataProgressionReady() || m_aStatsGained.IsEmpty())
		{
			Print("Maybe a problem: DataProgression is not ready but the analytics event was requested", LogLevel.WARNING);
			CalculateStatsChange();
		}
		
		EInputDeviceType device = GetGame().GetInputManager().GetLastUsedInputDevice();
		UpdateTrackingController(device, device);
		
		Print("Time with gamepad was " + m_iSecondsAsController, LogLevel.DEBUG);
		Print("Time with keyboard was " + m_iSecondsAsKeyboard, LogLevel.DEBUG);

		dataEvent.amt_time_mission = m_aStatsGained[SCR_EDataStats.SESSION_DURATION];
		dataEvent.amt_distance_on_foot = m_aStatsGained[SCR_EDataStats.DISTANCE_WALKED];
		dataEvent.amt_distance_vehicle = m_aStatsGained[SCR_EDataStats.DISTANCE_DRIVEN];
		dataEvent.kills = m_aStatsGained[SCR_EDataStats.KILLS];
		//dataEvent.kills_from_over_25m;
		//dataEvent.kills_from_over_50m;
		//dataEvent.kills_from_over_100m;
		dataEvent.friendly_kills = m_aStatsGained[SCR_EDataStats.FRIENDLY_KILLS];
		//dataEvent.friendly_kills_from_over_25m;
		//dataEvent.friendly_kills_from_over_50m;
		//dataEvent.friendly_kills_from_over_100m;
		dataEvent.vehicle_kills = m_aStatsGained[SCR_EDataStats.ROADKILLS] + m_aStatsGained[SCR_EDataStats.AI_ROADKILLS];
		dataEvent.friendly_vehicle_kills = m_aStatsGained[SCR_EDataStats.FRIENDLY_ROADKILLS] + m_aStatsGained[SCR_EDataStats.FRIENDLY_AI_ROADKILLS];
		dataEvent.deaths = m_aStatsGained[SCR_EDataStats.DEATHS];
		//dataEvent.deaths_by_bleeding_out;
		//dataEvent.deaths_by_falling;
		dataEvent.shots = m_aStatsGained[SCR_EDataStats.SHOTS];
		//dataEvent.hits;
		//dataEvent.hits_from_over_25m;
		//dataEvent.hits_from_over_50m;
		//dataEvent.hits_from_over_100m;
		dataEvent.healing_allies = m_aStatsGained[SCR_EDataStats.BANDAGE_FRIENDLIES]; //Careful
		dataEvent.healing_self = m_aStatsGained[SCR_EDataStats.BANDAGE_SELF]; //Careful
		//dataEvent.weapon1_id;
		//dataEvent.weapon1_seconds;
		//dataEvent.weapon1_shots;
		//dataEvent.weapon1_hits;
		//dataEvent.weapon2_id;
		//dataEvent.weapon2_seconds;
		//dataEvent.weapon2_shots;
		//dataEvent.weapon2_hits;
		//dataEvent.weapon3_id;
		//dataEvent.weapon3_seconds;
		//dataEvent.weapon3_shots;
		//dataEvent.weapon3_hits;
		//dataEvent.setbase_coordinates;
		dataEvent.rank_points = m_aStatsGained[SCR_EDataStats.LEVEL_EXPERIENCE];
		dataEvent.rank_total_points = m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE];
		dataEvent.warcrimes_points = m_aStatsGained[SCR_EDataStats.WARCRIMES];
		dataEvent.warcrimes_total_points = m_aStats[SCR_EDataStats.WARCRIMES];
		dataEvent.rank_level = m_aStatsGained[SCR_EDataStats.RANK];
		dataEvent.gt1_points = m_aStatsGained[SCR_EDataStats.SPPOINTS0];
		dataEvent.gt1_total_points = m_aStats[SCR_EDataStats.SPPOINTS0];
		dataEvent.gt2_points = m_aStatsGained[SCR_EDataStats.SPPOINTS1];
		dataEvent.gt2_total_points = m_aStats[SCR_EDataStats.SPPOINTS1];
		dataEvent.gt3_points = m_aStatsGained[SCR_EDataStats.SPPOINTS2];
		dataEvent.gt3_total_points = m_aStats[SCR_EDataStats.SPPOINTS2];
		dataEvent.gt_global_points = (m_aStatsGained[SCR_EDataStats.SPPOINTS0] + m_aStatsGained[SCR_EDataStats.SPPOINTS1] + m_aStatsGained[SCR_EDataStats.SPPOINTS2]);
		dataEvent.seconds_as_controller = m_iSecondsAsController;
		dataEvent.seconds_as_keyboard = m_iSecondsAsKeyboard;

		return dataEvent;
	}

	//------------------------------------------------------------------------------------------------
	sealed void DebugCalculateStats()
	{
		Print("SCR_PlayerData:DebugCalculateStats: This method calls calculateStatsChange providing fake stats for debugging purposes.", LogLevel.DEBUG);

		AddStat(SCR_EDataStats.SESSION_DURATION, 7200);
		AddStat(SCR_EDataStats.DISTANCE_WALKED, 20000);
		AddStat(SCR_EDataStats.KILLS, 20);
		AddStat(SCR_EDataStats.AI_KILLS, 20);
		AddStat(SCR_EDataStats.SHOTS, 800);
		AddStat(SCR_EDataStats.GRENADES_THROWN, 10);
		AddStat(SCR_EDataStats.DISTANCE_DRIVEN, 50000);
		AddStat(SCR_EDataStats.FRIENDLY_KILLS, 30);
		AddStat(SCR_EDataStats.BANDAGE_SELF, 30);
		AddStat(SCR_EDataStats.BANDAGE_FRIENDLIES, 10);
		AddStat(SCR_EDataStats.TOURNIQUET_SELF, 30);
		AddStat(SCR_EDataStats.TOURNIQUET_FRIENDLIES, 10);

		if (!IsDataProgressionReady())
			CalculateStatsChange();
	}

	//------------------------------------------------------------------------------------------------
	sealed void CalculateStatsChange()
	{
		//We find the differences provided by this session
		//In order to calculate Rank, Specializations, and War Crime points
		m_aStatsGained = CalculateStatsDifference();

		for (int i = m_aStatsGained.Count()-1; i >= 0; i--)
		{
			m_aEarnt.Insert(0);
		}

		if (m_aStatsGained[SCR_EDataStats.SESSION_DURATION] < 1)
			return;

		//Little assert
		if (m_aStatsGained[SCR_EDataStats.SPPOINTS0] != 0 || m_aStatsGained[SCR_EDataStats.SPPOINTS1] != 0 || m_aStatsGained[SCR_EDataStats.SPPOINTS2] != 0|| m_aStatsGained[SCR_EDataStats.RANK] != 0 || m_aStatsGained[SCR_EDataStats.LEVEL_EXPERIENCE] != 0)
			Print("SCR_PlayerData:CalculateStatsChange: Specialization stats, experience or rank stats were modified prematurely. The career data for this session might be compromised", LogLevel.WARNING);

		//CALCULATE STATS ADDITIONS TO THE SPECIALIZATIONS, THEN WAR CRIMES, THEN SPECIALIZATION POINTS, AND THEN LEVEL OF EXPERIENCE AND RANK

		//Specializations: There are three: Infantry, Logistics and Medical
		int totalSpecializationPointsGain = 0;
		//
		//
		//Infantry
		if (m_aStats[SCR_EDataStats.SPPOINTS0] < m_Configs.SPECIALIZATION_MAX)
		{
			m_aEarnt[SCR_EDataStats.DISTANCE_WALKED] = m_aStatsGained[SCR_EDataStats.DISTANCE_WALKED] * m_Configs.MODIFIER_DISTANCE_WALKED;
			m_aEarnt[SCR_EDataStats.KILLS] = m_aStatsGained[SCR_EDataStats.KILLS] * m_Configs.MODIFIER_KILLS;
			m_aEarnt[SCR_EDataStats.AI_KILLS] = m_aStatsGained[SCR_EDataStats.AI_KILLS] * m_Configs.MODIFIER_AI_KILLS;

			if (m_aStatsGained[SCR_EDataStats.SHOTS] <= 0)
				m_aEarnt[SCR_EDataStats.SHOTS] = 0;
			else
				//This title is misleading: We are treating shots as "precision in killing" of sorts
				m_aEarnt[SCR_EDataStats.SHOTS] = ((m_aStatsGained[SCR_EDataStats.KILLS] - m_aStatsGained[SCR_EDataStats.ROADKILLS] + m_aStatsGained[SCR_EDataStats.AI_KILLS] - m_aStatsGained[SCR_EDataStats.AI_ROADKILLS]) / m_aStatsGained[SCR_EDataStats.SHOTS]) * m_Configs.MODIFIER_PRECISION;

			/* We should not use GRENADESTHROWN yet
			if (m_aStatsGained[SCR_EDataStats.GRENADESTHROWN] <= 0)
				m_aEarnt[SCR_EDataStats.GRENADESTHROWN] = 0;
			else
				m_aEarnt[SCR_EDataStats.GRENADESTHROWN] = (m_aStatsGained[SCR_EDataStats.KILLS] / m_aStatsGained[SCR_EDataStats.GRENADESTHROWN]) * m_Configs.MODIFIERPRECISION;
			*/

			m_aEarnt[SCR_EDataStats.SPPOINTS0] = m_aEarnt[SCR_EDataStats.DISTANCE_WALKED] + m_aEarnt[SCR_EDataStats.KILLS] + m_aEarnt[SCR_EDataStats.AI_KILLS] + m_aEarnt[SCR_EDataStats.SHOTS]; /*+ m_aEarnt[SCR_EDataStats.GRENADES_THROWN]; */
		}
		else
			m_aEarnt[SCR_EDataStats.SPPOINTS0] = 0;

		m_aStatsGained[SCR_EDataStats.SPPOINTS0] = m_aEarnt[SCR_EDataStats.SPPOINTS0];
		totalSpecializationPointsGain += m_aStatsGained[SCR_EDataStats.SPPOINTS0];

		//
		//
		//Logistics
		if (m_aStats[SCR_EDataStats.SPPOINTS1] < m_Configs.SPECIALIZATION_MAX)
		{
			m_aEarnt[SCR_EDataStats.DISTANCE_DRIVEN] = m_aStatsGained[SCR_EDataStats.DISTANCE_DRIVEN] * m_Configs.MODIFIER_DISTANCE_DRIVEN;
			m_aEarnt[SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS] = m_aStatsGained[SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS];

			m_aEarnt[SCR_EDataStats.PLAYERS_DIED_IN_VEHICLE] = -1 * Math.Min(m_aStatsGained[SCR_EDataStats.PLAYERS_DIED_IN_VEHICLE] * m_Configs.MODIFIER_PLAYERS_DIED_IN_VEHICLE, m_Configs.MAX_PLAYERS_DIED_IN_VEHICLE_PENALTY);

			m_aEarnt[SCR_EDataStats.SPPOINTS1] = m_aEarnt[SCR_EDataStats.DISTANCE_DRIVEN] + m_aEarnt[SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS] + m_aEarnt[SCR_EDataStats.PLAYERS_DIED_IN_VEHICLE];

			if (m_aEarnt[SCR_EDataStats.SPPOINTS1] <= 0)
				m_aEarnt[SCR_EDataStats.SPPOINTS1] = 0;
		}
		else
			m_aEarnt[SCR_EDataStats.SPPOINTS1] = 0;

		m_aStatsGained[SCR_EDataStats.SPPOINTS1] = m_aEarnt[SCR_EDataStats.SPPOINTS1];
		totalSpecializationPointsGain += m_aStatsGained[SCR_EDataStats.SPPOINTS1];

		//
		//
		//Medical
		if (m_aStats[SCR_EDataStats.SPPOINTS2] < m_Configs.SPECIALIZATION_MAX)
		{

			//TODO: transform these into m_aEarnt[i] = m_aStatsGained[i] * m_Configs[i] if possible
			
			m_aEarnt[SCR_EDataStats.BANDAGE_SELF] = m_aStatsGained[SCR_EDataStats.BANDAGE_SELF] * m_Configs.MODIFIER_BANDAGE_SELF;
			m_aEarnt[SCR_EDataStats.BANDAGE_FRIENDLIES] = m_aStatsGained[SCR_EDataStats.BANDAGE_FRIENDLIES] * m_Configs.MODIFIER_BANDAGE_FRIENDLIES;
			m_aEarnt[SCR_EDataStats.TOURNIQUET_SELF] = m_aStatsGained[SCR_EDataStats.TOURNIQUET_SELF] * m_Configs.MODIFIER_TOURNIQUET_SELF;
			m_aEarnt[SCR_EDataStats.TOURNIQUET_FRIENDLIES] = m_aStatsGained[SCR_EDataStats.TOURNIQUET_FRIENDLIES] * m_Configs.MODIFIER_TOURNIQUET_FRIENDLIES;
			m_aEarnt[SCR_EDataStats.SALINE_SELF] = m_aStatsGained[SCR_EDataStats.SALINE_SELF] * m_Configs.MODIFIER_SALINE_SELF;
			m_aEarnt[SCR_EDataStats.SALINE_FRIENDLIES] = m_aStatsGained[SCR_EDataStats.SALINE_FRIENDLIES] * m_Configs.MODIFIER_SALINE_FRIENDLIES;
			m_aEarnt[SCR_EDataStats.MORPHINE_SELF] = m_aStatsGained[SCR_EDataStats.MORPHINE_SELF] * m_Configs.MODIFIER_MORPHINE_SELF;
			m_aEarnt[SCR_EDataStats.MORPHINE_FRIENDLIES] = m_aStatsGained[SCR_EDataStats.MORPHINE_FRIENDLIES] * m_Configs.MODIFIER_MORPHINE_FRIENDLIES;

			m_aEarnt[SCR_EDataStats.SPPOINTS2] = m_aEarnt[SCR_EDataStats.BANDAGE_SELF] + m_aEarnt[SCR_EDataStats.BANDAGE_FRIENDLIES] + m_aEarnt[SCR_EDataStats.TOURNIQUET_SELF] + m_aEarnt[SCR_EDataStats.TOURNIQUET_FRIENDLIES]+ m_aEarnt[SCR_EDataStats.SALINE_SELF] + m_aEarnt[SCR_EDataStats.SALINE_FRIENDLIES] + m_aEarnt[SCR_EDataStats.MORPHINE_SELF] + m_aEarnt[SCR_EDataStats.MORPHINE_FRIENDLIES];
		}
		else
			m_aEarnt[SCR_EDataStats.SPPOINTS2] = 0;

		m_aStatsGained[SCR_EDataStats.SPPOINTS2] = m_aEarnt[SCR_EDataStats.SPPOINTS2];
		totalSpecializationPointsGain += m_aStatsGained[SCR_EDataStats.SPPOINTS2];

		//CALCULATE QUALITYTIME RATIO
		//qualityTimeRatio is calculated as the ratio between (ExpectedPointsPerHour * HoursPlayed) and ObtainedPoints.
		//For expected points, I use m_fSTDPointsQualityTime as the expected growth per specialization and 2.5 as the number of specializations with it as an accurate simplification
		float qualityTimeRatio = totalSpecializationPointsGain / (1 +(m_Configs.STD_POINTS_QUALITY_TIME * 2.5 * (m_aStatsGained[SCR_EDataStats.SESSION_DURATION] / 3600)));

		//Little assert on the qualityTimeRatio to check player didn't perform abnormaly well
		if (qualityTimeRatio > 1.5)
			Print("SCR_PlayerData:CalculateStatsChange: totalSpecializationPointsGain is suspiciously high. That means player performed better than we expected which could point an an oversight from our side, or even a player cheating. Player with session id " + m_iPlayerID + " gained " + totalSpecializationPointsGain + " points on a session of " + (m_aStatsGained[SCR_EDataStats.SESSION_DURATION] / 60) + " minutes, ending with a qualityTimeRatio of " + qualityTimeRatio, LogLevel.WARNING);
		else
			Print("SCR_PlayerData:CalculateStatsChange: QualityTimeRatio (aka how well player performed) is: " + qualityTimeRatio, LogLevel.DEBUG);

		//LOWER WAR CRIMES IF THERE'S ANY
		if (m_aStats[SCR_EDataStats.WARCRIMES] > 0)
		{
			// WarCrimes go down by m_fWarCrimesDecreaseRatePerHour * qualityTimeRatio * time
			int warCrimesDown = qualityTimeRatio * (m_aStatsGained[SCR_EDataStats.SESSION_DURATION]/ 3600) * m_Configs.WARCRIMES_DECREASE_PER_HOUR;

			//All war crimes go down equitatively by amountToDecrease / numberOfNonZeroWarCrimes amount
			int amountToDecrease = (m_aStats[SCR_EDataStats.WARCRIME_HARMING_FRIENDLIES]) * warCrimesDown / m_aStats[SCR_EDataStats.WARCRIMES];

			//Divide equitatively all the war crimes. In this case: only 1
			m_aStats[SCR_EDataStats.WARCRIME_HARMING_FRIENDLIES] = m_aStats[SCR_EDataStats.WARCRIME_HARMING_FRIENDLIES] - amountToDecrease / 1;
			m_aStats[SCR_EDataStats.WARCRIMES] = m_aStats[SCR_EDataStats.WARCRIMES] - warCrimesDown;

			if (m_aStats[SCR_EDataStats.WARCRIMES] > m_Configs.MAX_WARCRIMES_VALUE)
				m_aStats[SCR_EDataStats.WARCRIMES] = m_Configs.MAX_WARCRIMES_VALUE;
			else if (m_aStats[SCR_EDataStats.WARCRIMES] < 0)
				m_aStats[SCR_EDataStats.WARCRIMES] = 0;
		}

		//UPDATE THE SPECIALIZATION POINTS
		//HERE we update the specializationPoints
		//------------------------------------------------------------------------------------------------

		int i, j;
		int currentSpPoints, gainedRawPoints;
		for (i = 0; i < m_Configs.SPECIALIZATIONS_COUNT; i++)
		{
			j = 0;
			switch (i)
			{
				case SCR_ECareerSp.INFANTRY:
					gainedRawPoints = m_aStatsGained[SCR_EDataStats.SPPOINTS0];
					currentSpPoints = m_aStats[SCR_EDataStats.SPPOINTS0];
				break;
				case SCR_ECareerSp.LOGISTICS:
					gainedRawPoints = m_aStatsGained[SCR_EDataStats.SPPOINTS1];
					currentSpPoints = m_aStats[SCR_EDataStats.SPPOINTS1];
				break;
				case SCR_ECareerSp.MEDICAL:
					gainedRawPoints = m_aStatsGained[SCR_EDataStats.SPPOINTS2];
					currentSpPoints = m_aStats[SCR_EDataStats.SPPOINTS2];
				break;
				default: continue;
			}

			if (currentSpPoints >= m_Configs.SPECIALIZATION_MAX)
				continue;

			//Punish the specialization growth if there are war crimes
			if (m_aStats[SCR_EDataStats.WARCRIMES] > 0)
				gainedRawPoints *= (1 - m_Configs.WARCRIMES_PUNISHMENT);

			while (j < m_Configs.INTERVALS_COUNT && m_Configs.INTERVALS_END[j] < currentSpPoints)
			{
				j++;
			}
			
			if (j >= m_Configs.INTERVALS_COUNT)
			{
				Print ("SCR_PlayerData:CalculateStatsChange: currentSpPoints outside of the accepted ranges.", LogLevel.WARNING);
				continue;
			}

			//Formula: NewSpecializationPoints = CurrentSpecializationPoints + GainedPoints * c1 * (c2 ^ CurrentSpecializationPoints)
			//m_aStats.m_aSpPoints[i] = m_aStats.m_aSpPoints[i] + rawSpPoints[i] * m_Configs.INTERVALS_C1[j] * (Math.Pow(m_Configs.INTERVALS_C2[j], m_aStats.m_aSpPoints[i]));
			AddPointsToSpecialization(i, gainedRawPoints * m_Configs.INTERVALS_C1[j] * (Math.Pow(m_Configs.INTERVALS_C2[j], currentSpPoints * m_Configs.XP_NEEDED_FOR_LEVEL_DIVIDER)));
		}

		//LAST BUT NOT LEAST,  INCREASE THE LEVEL OF EXPERIENCE
		//IF player has max level, skip this step
		if (m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE] >= m_Configs.MAX_EXP)
		{
			m_aEarnt[SCR_EDataStats.SESSION_DURATION] = 0;
			m_aEarnt[SCR_EDataStats.LEVEL_EXPERIENCE] = 0;
			return;
		}

		//Calculate gains in level of experience. Currently, we are only using sessionduration and specializationgains
		m_aEarnt[SCR_EDataStats.SESSION_DURATION] = m_aStatsGained[SCR_EDataStats.SESSION_DURATION] * qualityTimeRatio;
		m_aEarnt[SCR_EDataStats.LEVEL_EXPERIENCE] = m_aEarnt[SCR_EDataStats.SESSION_DURATION] + totalSpecializationPointsGain * m_Configs.MODIFIER_SPECIALIZATIONS;

		if (m_aStats[SCR_EDataStats.WARCRIMES] > 0)
		{
			m_aEarnt[SCR_EDataStats.LEVEL_EXPERIENCE] = m_aEarnt[SCR_EDataStats.LEVEL_EXPERIENCE] * (1 - m_Configs.WARCRIMES_PUNISHMENT);
		}
		m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE] = m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE] + m_aEarnt[SCR_EDataStats.LEVEL_EXPERIENCE];
		if (m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE] > m_Configs.MAX_EXP)
			m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE] = m_Configs.MAX_EXP;

		m_aStats[SCR_EDataStats.RANK] = m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE] * m_Configs.XP_NEEDED_FOR_RANK_DIVIDER;

		m_aStatsGained[SCR_EDataStats.RANK] = m_aStats[SCR_EDataStats.RANK] - m_aPreviousStats[SCR_EDataStats.RANK];
		m_aStatsGained[SCR_EDataStats.LEVEL_EXPERIENCE] = m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE] - m_aPreviousStats[SCR_EDataStats.LEVEL_EXPERIENCE];
	}

	//------------------------------------------------------------------------------------------------
	array<float> GetArrayEarntPoints()
	{
		return m_aEarnt;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDataReady()
	{
		return !m_aStats.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	bool IsDataProgressionReady()
	{
		return m_aEarnt && !m_aEarnt.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	void FillArrayWithSpecializationPoints(inout array<float> SpPoints, bool newStats = true)
	{
		for (int i = 0; i < m_Configs.SPECIALIZATIONS_COUNT; i++)
		{
			SpPoints.Insert(GetSpecializationPoints(i, newStats));
		}
	}

	//------------------------------------------------------------------------------------------------
	sealed array<float> CalculateStatsDifference()
	{
		array<float> StatsToReturn = {};

		int count = m_aStats.Count();
		CalculateSessionDuration();

		for (int i = 0; i < count; i++)
		{
			StatsToReturn.Insert(m_aStats[i] - m_aPreviousStats[i]);
		}

		return StatsToReturn;
	}

	//------------------------------------------------------------------------------------------------
	float GetSpecializationPoints(int n, bool newStats = true)
	{
		if (newStats)
		{
			if (!m_aStats || m_aStats.IsEmpty())
				return 0;

			switch (n)
			{
				case SCR_ECareerSp.INFANTRY:
					return m_aStats[SCR_EDataStats.SPPOINTS0];
				case SCR_ECareerSp.LOGISTICS:
					return m_aStats[SCR_EDataStats.SPPOINTS1];
				case SCR_ECareerSp.MEDICAL:
					return m_aStats[SCR_EDataStats.SPPOINTS2];
			}

			Print ("SCR_PlayerData:GetSpecializationPoints: WRONG SPECIALIZATION ID.", LogLevel.WARNING);
			return 0;
		}

		if (!m_aPreviousStats || m_aPreviousStats.IsEmpty())
			return 0;

		switch (n)
		{
			case SCR_ECareerSp.INFANTRY:
				return m_aPreviousStats[SCR_EDataStats.SPPOINTS0];
			case SCR_ECareerSp.LOGISTICS:
				return m_aPreviousStats[SCR_EDataStats.SPPOINTS1];
			case SCR_ECareerSp.MEDICAL:
				return m_aPreviousStats[SCR_EDataStats.SPPOINTS2];
		}

		Print ("SCR_PlayerData:GetSpecializationPoints: WRONG SPECIALIZATION ID.", LogLevel.WARNING);
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddPointsToSpecialization(int n, float value)
	{
		if (!m_aStats || m_aStats.IsEmpty())
			return;

		switch (n)
		{
			case SCR_ECareerSp.INFANTRY:
				m_aStats[SCR_EDataStats.SPPOINTS0] = Math.Min(m_aStats[SCR_EDataStats.SPPOINTS0] + value, m_Configs.SPECIALIZATION_MAX);
				return;
			case SCR_ECareerSp.LOGISTICS:
				m_aStats[SCR_EDataStats.SPPOINTS1] = Math.Min(m_aStats[SCR_EDataStats.SPPOINTS1] + value, m_Configs.SPECIALIZATION_MAX);
				return;
			case SCR_ECareerSp.MEDICAL:
				m_aStats[SCR_EDataStats.SPPOINTS2] = Math.Min(m_aStats[SCR_EDataStats.SPPOINTS2] + value, m_Configs.SPECIALIZATION_MAX);
				return;
		}
		Print ("SCR_PlayerData:AddPointsToSpecialization: WRONG SPECIALIZATION ID.", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	//! This method sets the SCR_PlayerDataSpecializationDisplay arrays from the PlayerDataConfigs. Typically the Career Menu will use this
	void PrepareSpecializationStatsDisplay()
	{
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.DISTANCE_WALKED].SetValue(GetStat(SCR_EDataStats.DISTANCE_WALKED) * m_Configs.MetersToKilometersConversion);
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.PLAYER_KILLS].SetValue(GetStat(SCR_EDataStats.KILLS));
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.AI_KILLS].SetValue(GetStat(SCR_EDataStats.AI_KILLS));
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.SHOTS].SetValue(GetStat(SCR_EDataStats.SHOTS));

		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.DISTANCE_DRIVEN].SetValue(GetStat(SCR_EDataStats.DISTANCE_DRIVEN) * m_Configs.MetersToKilometersConversion);
		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.DISTANCE_AS_OCCUPANT].SetValue(GetStat(SCR_EDataStats.DISTANCE_AS_OCCUPANT) * m_Configs.MetersToKilometersConversion);
		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.POINTS_DRIVING_PEOPLE].SetValue(GetStat(SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS));

		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.BANDAGE_SELF].SetValue(GetStat(SCR_EDataStats.BANDAGE_SELF));
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.BANDAGE_FRIENDLIES].SetValue(GetStat(SCR_EDataStats.BANDAGE_FRIENDLIES));
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.TOURNIQUET_SELF].SetValue(GetStat(SCR_EDataStats.TOURNIQUET_SELF));
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.TOURNIQUET_FRIENDLIES].SetValue(GetStat(SCR_EDataStats.TOURNIQUET_FRIENDLIES));
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.SALINE_SELF].SetValue(GetStat(SCR_EDataStats.SALINE_SELF));
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.SALINE_FRIENDLIES].SetValue(GetStat(SCR_EDataStats.SALINE_FRIENDLIES));
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.MORPHINE_SELF].SetValue(GetStat(SCR_EDataStats.MORPHINE_SELF));
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.MORPHINE_FRIENDLIES].SetValue(GetStat(SCR_EDataStats.MORPHINE_FRIENDLIES));
	}

	//------------------------------------------------------------------------------------------------
	//! This method sets the SCR_PlayerDataSpecializationDisplay arrays from the PlayerDataConfigs. Typically the Debrief screen will use this
	void PrepareSpecializationProgressionStatsDisplay()
	{
		if (m_aStatsGained.IsEmpty())
		{
			Print("Requesting statsDifference in PrepareSpecializationProgressionStatsDisplay but array is empty. Recalculating it!", LogLevel.DEBUG);
			m_aStatsGained = CalculateStatsDifference();
		}

		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.DISTANCE_WALKED].SetValue(m_aStatsGained[SCR_EDataStats.DISTANCE_WALKED] * m_Configs.MetersToKilometersConversion);
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.PLAYER_KILLS].SetValue(m_aStatsGained[SCR_EDataStats.KILLS]);
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.AI_KILLS].SetValue(m_aStatsGained[SCR_EDataStats.AI_KILLS]);
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.SHOTS].SetValue(m_aStatsGained[SCR_EDataStats.SHOTS]);

		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.DISTANCE_DRIVEN].SetValue(m_aStatsGained[SCR_EDataStats.DISTANCE_DRIVEN] * m_Configs.MetersToKilometersConversion);
		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.DISTANCE_AS_OCCUPANT].SetValue(m_aStatsGained[SCR_EDataStats.DISTANCE_AS_OCCUPANT] * m_Configs.MetersToKilometersConversion);
		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.POINTS_DRIVING_PEOPLE].SetValue(m_aStatsGained[SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS]);

		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.BANDAGE_SELF].SetValue(m_aStatsGained[SCR_EDataStats.BANDAGE_SELF]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.BANDAGE_FRIENDLIES].SetValue(m_aStatsGained[SCR_EDataStats.BANDAGE_FRIENDLIES]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.TOURNIQUET_SELF].SetValue(m_aStatsGained[SCR_EDataStats.TOURNIQUET_SELF]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.TOURNIQUET_FRIENDLIES].SetValue(m_aStatsGained[SCR_EDataStats.TOURNIQUET_FRIENDLIES]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.SALINE_SELF].SetValue(m_aStatsGained[SCR_EDataStats.SALINE_SELF]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.SALINE_FRIENDLIES].SetValue(m_aStatsGained[SCR_EDataStats.SALINE_FRIENDLIES]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.MORPHINE_SELF].SetValue(m_aStatsGained[SCR_EDataStats.MORPHINE_SELF]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.MORPHINE_FRIENDLIES].SetValue(m_aStatsGained[SCR_EDataStats.MORPHINE_FRIENDLIES]);
	}

	//------------------------------------------------------------------------------------------------
	bool IsEmptyProfile()
	{
		return m_bIsEmptyProfile;
	}

	//m_aStats and m_aPreviousStats must not be accessed from outside of this class.
	//That is why there are getters and setters for their attributes, but not for their instances
	//
	//
	//
	//SETTERS:

	//------------------------------------------------------------------------------------------------
	//! temp:
	//! True: The stat should be added to temporary actions that might be evaluated further by a module
	//! False: The stat is final. Add it to this player session's stats
	void AddStat(SCR_EDataStats stat, float amount = 1, bool temp = true)
	{
		if (!accumulationEnabled)
			temp = false;
		
		if (temp)
		{
			m_iLatestActionTick = System.GetTickCount();
			if (m_iAccumulatedActionsTick == 0)
				m_iAccumulatedActionsTick = m_iLatestActionTick;
			
			m_aAccumulatedActions[stat] = m_aAccumulatedActions[stat] + amount;
		}
		else
		{
			m_aStats[stat] = m_aStats[stat] + amount;
		}
		
		s_OnStatAdded.Invoke(m_iPlayerID, stat, amount, temp);
	}
	
	//------------------------------------------------------------------------------------------------
	//USE WITH CARE AND ONLY IN SPECIAL CIRCUMSTANCES!
	void OverrideStat(SCR_EDataStats stat, float amount = 1)
	{
		m_aStats[stat] = amount;
	}

	//GETTERS:

	//------------------------------------------------------------------------------------------------
	float GetStat(SCR_EDataStats stat, bool newStat = true)
	{
		if (newStat)
			return m_aStats[stat];

		return m_aPreviousStats[stat];
	}

	///////////
	//Generic//
	///////////

	//------------------------------------------------------------------------------------------------
	int GetAccumulatedActionsTick()
	{
		return m_iAccumulatedActionsTick;
	}

	//------------------------------------------------------------------------------------------------
	int GetLatestActionTick()
	{
		return m_iLatestActionTick;
	}
	
	int GetLatestCriminalScoreUpdateTick()
	{
		return m_iLatestCriminalScoreUpdateTick;
	}

	//------------------------------------------------------------------------------------------------
	SCR_PlayerDataConfigs GetConfigs()
	{
		return m_Configs;
	}

	//------------------------------------------------------------------------------------------------
	array<float> GetAccumulatedActions()
	{
		array<float> copy = {};
		copy.Copy(m_aAccumulatedActions);
		return copy;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetCriminalScore()
	{
		return m_fCriminalScore;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCriminalScore(float score)
	{
		m_iLatestCriminalScoreUpdateTick = System.GetTickCount();
		m_fCriminalScore = score;
	}

	//------------------------------------------------------------------------------------------------
	void ResetAccumulatedActions()
	{
		for (int i = 0, count = m_aAccumulatedActions.Count(); i < count; i++)
		{
			m_aAccumulatedActions[i] = 0;
		}
		
		m_iAccumulatedActionsTick = 0;
		
	}

	//------------------------------------------------------------------------------------------------
	void CalculateSessionDuration()
	{
		// HOTFIX: m_aStats is empty on reconnect to end screen. This shouldn't be probably called
		// at all, but it is to someone else to review this system. Lets just check, if the index
		// is valid to prevent VM exception
		if (!m_aStats.IsIndexValid(SCR_EDataStats.SESSION_DURATION))
		{
			return;
		}

		m_aStats[SCR_EDataStats.SESSION_DURATION] = m_aStats[SCR_EDataStats.SESSION_DURATION] + (System.GetTickCount() - m_iSessionStartedTickCount) * 0.001;
		m_iSessionStartedTickCount = System.GetTickCount();
	}

	/*****************/
	//Specializations//
	/*****************/

	//------------------------------------------------------------------------------------------------
	float GetSpecializationCount(int n)
	{
		switch (n)
		{
			case 0: return m_Configs.SPECIALIZATION_0_COUNT;
			case 1: return m_Configs.SPECIALIZATION_1_COUNT;
			case 2: return m_Configs.SPECIALIZATION_2_COUNT;
		}
		return -1;
	}
};

//DO NOT CHANGE THE ORDER OF THE ITEMS OF THE ENUM. (!!!) WE USE THE INDEX AS AN ID IN THE PLAYERPROFILE'S CHARACTERDATA
//------------------------------------------------------------------------------------------------
enum SCR_EDataStats
{
	RANK,//!< Rank of the player
	LEVEL_EXPERIENCE, //!< XP points
	SESSION_DURATION, //!< Total duration of sessions
	SPPOINTS0, //!< INFANTRY
	SPPOINTS1, //!< LOGISTICS
	SPPOINTS2, //!< MEDICAL
	WARCRIMES, //!< War criminal points
	DISTANCE_WALKED, //!< Distance walked on foot
	KILLS, //!< Enemies killed
	AI_KILLS, //!< AI enemies killed
	SHOTS, //!< Bullets shot
	GRENADES_THROWN, //!< Grenades thrown
	FRIENDLY_KILLS, //!< Friendly human kills
	FRIENDLY_AI_KILLS, //!< Friendly AI kills
	DEATHS, //!< Deaths
	DISTANCE_DRIVEN, //!< Distance driven with a vehicle
	POINTS_AS_DRIVER_OF_PLAYERS, //!< Points obtained for driving players around
	PLAYERS_DIED_IN_VEHICLE, //!< Players that died inside a vehicle while the player drives it
	ROADKILLS, //!< Human enemies killed with a vehicle
	FRIENDLY_ROADKILLS, //!< Human friendlies killed with a vehicle
	AI_ROADKILLS, //!< AI enemies killed with a vehicle
	FRIENDLY_AI_ROADKILLS, //!< AI friendlies killed with a vehicle
	DISTANCE_AS_OCCUPANT, //!< Distance the player was driven around with a vehicle
	BANDAGE_SELF, //!< Number of times player bandaged themself
	BANDAGE_FRIENDLIES, //!< Number of times player bandaged friendlies (not including themself)
	TOURNIQUET_SELF, //!< Number of times player tourniquetted themself
	TOURNIQUET_FRIENDLIES, //!< Number of times player tourniquetted friendlies (not including themself)
	SALINE_SELF, //!< Number of times player salined themself
	SALINE_FRIENDLIES, //!< Number of times player salined friendlies (not including themself)
	MORPHINE_SELF, //!< Number of times player morphined themself
	MORPHINE_FRIENDLIES, //!< Number of times player morphined friendlies (not including themself)
	WARCRIME_HARMING_FRIENDLIES, //!< Harming friendlies
	CRIME_ACCELERATION, //!< Kick & Ban acceleration
	KICK_SESSION_DURATION, //!< Session duration at the time player was kicked the last time
	KICK_STREAK, //!< How many times was the player kicked in a row after last kick in a short succession
	LIGHTBAN_SESSION_DURATION, //!< Session duration at the time player was lightbanned the last time
	LIGHTBAN_STREAK, //!< How many times was the player lightbanned in a row after the last lightban in a short succession
	HEAVYBAN_SESSION_DURATION, //!< Session duration at the time player was heavybanned the last time
	HEAVYBAN_STREAK //!< How many times was the player heavybanned in a row after the last heavyban in a short succession
};
