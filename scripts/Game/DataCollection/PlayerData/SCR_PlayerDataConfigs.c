
// -----------------------------------------------------------------------------
//! Settings item
// -----------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
class SCR_BanSettings : JsonApiStruct
{
	float m_fScoreThreshold;
	float m_fScoreDecreasePerMinute;
	float m_fScoreMultiplier;
	float m_fAccelerationMin;
	float m_fAccelerationMax;
	float m_fBanEvaluationLight;
	float m_fBanEvaluationHeavy;
	float m_fCrimePtFriendKill;
	float m_fCrimePtTeamKill;
	float m_fQualityTimeTemp;
	float m_bVotingSuggestionEnabled;
	
	//------------------------------------------------------------------------------------------------
	void Log(LogLevel lv)
	{
		Print("m_fScoreThreshold: " + m_fScoreThreshold, lv);
		Print("m_fScoreDecreasePerMinute: " + m_fScoreDecreasePerMinute, lv);
		Print("m_fScoreMultiplier: " + m_fScoreMultiplier, lv);
		Print("m_fAccelerationMin: " + m_fAccelerationMin, lv);
		Print("m_fAccelerationMax: " + m_fAccelerationMax, lv);
		Print("m_fBanEvaluationLight: " + m_fBanEvaluationLight, lv);
		Print("m_fBanEvaluationHeavy: " + m_fBanEvaluationHeavy, lv);
		Print("m_fCrimePtFriendKill: " + m_fCrimePtFriendKill, lv);
		Print("m_fCrimePtTeamKill: " + m_fCrimePtTeamKill, lv);
		Print("m_fQualityTimeTemp: " + m_fQualityTimeTemp, lv);
		Print("m_bVotingSuggestionEnabled: " + m_bVotingSuggestionEnabled, lv);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDefault()
	{
		m_fScoreThreshold = 10.0;
		m_fScoreDecreasePerMinute = 0.2;
		m_fScoreMultiplier = 0.2;
		m_fAccelerationMin = 1.0;
		m_fAccelerationMax = 6.0;
		m_fBanEvaluationLight = 0.8;
		m_fBanEvaluationHeavy = 1.0;
		m_fCrimePtFriendKill = 1.0;
		m_fCrimePtTeamKill = 0.7;
		m_fQualityTimeTemp = 1.0;
		m_bVotingSuggestionEnabled = 0.0;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_BanSettings()
	{
		RegAll();
	}
};

//------------------------------------------------------------------------------------------------
class SCR_BanSettingsContainer : JsonApiStruct
{
	protected string m_sDesc;
	protected ref SCR_BanSettings m_BanSettings = new SCR_BanSettings();
	
	//------------------------------------------------------------------------------------------------
	string GetDesc()
	{
		return m_sDesc;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BanSettings GetBanSettings()
	{
		return m_BanSettings;
	}
	
	//------------------------------------------------------------------------------------------------
	void Log(LogLevel lv)
	{
		Print(" -- Settings -- ", lv);
		Print("m_sDesc: " + m_sDesc, lv);
		m_BanSettings.Log(lv);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDefault()
	{
		m_sDesc = "Default params";
		m_BanSettings.SetDefault();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_BanSettingsContainer()
	{
		RegV("m_sDesc");
		RegV("m_BanSettings");
	}
};

//------------------------------------------------------------------------------------------------
class SCR_PlayerDataConfigs : Managed
{

	protected static ref SCR_PlayerDataConfigs instance;

	/****************/
	//RANK AND LEVEL//
	/****************/

	static const float MODIFIER_SPECIALIZATIONS = 0.2;
	static const int XP_NEEDED_FOR_LEVEL = 10000;
	static const int XP_NEEDED_FOR_LEVEL_DIVIDER = 0.0001;
	static const int XP_NEEDED_FOR_RANK = 100000;
	static const int XP_NEEDED_FOR_RANK_DIVIDER = 0.00001;
	static const int MAX_EXP = 1000000;

	/*****************/
	//SPECIALIZATIONS//
	/*****************/

	int SPECIALIZATIONS_COUNT;

	string SPECIALIZATION_0_NAME = "#AR-CareerProfile_Specialization1";
	string SPECIALIZATION_1_NAME = "#AR-CareerProfile_Specialization2";
	string SPECIALIZATION_2_NAME = "#AR-CareerProfile_Specialization3";

	int SPECIALIZATION_0_COUNT;
	int SPECIALIZATION_1_COUNT;
	int SPECIALIZATION_2_COUNT;

	ref array<ref SCR_PlayerDataSpecializationDisplay> m_aSpecialization0 = {};
	ref array<ref SCR_PlayerDataSpecializationDisplay> m_aSpecialization1 = {};
	ref array<ref SCR_PlayerDataSpecializationDisplay> m_aSpecialization2 = {};

	const int SPECIALIZATION_MAX = 1000000;

	//0 - Infantry
	static const float MODIFIER_DISTANCE_WALKED = 1;
	static const float MODIFIER_KILLS = 250;
	static const float MODIFIER_AI_KILLS = 125;
	static const float MODIFIER_PRECISION = 300;

	//1 - Logistics
	static const float MODIFIER_DISTANCE_DRIVEN = 1;
	static const float MODIFIER_DRIVER_OF_PLAYERS = 0.7;
	static const float MODIFIER_TRAVEL_TIME_SUPPLY_VEHICLE = 1;
	static const float MODIFIER_TRAVELED_DISTANCE_SUPPLY_VEHICLE = 5;
	static const int MODIFIER_PLAYERS_DIED_IN_VEHICLE = 500;
	static const int MAX_PLAYERS_DIED_IN_VEHICLE_PENALTY = MODIFIER_PLAYERS_DIED_IN_VEHICLE * 5;

	//2 - Medical Support
	static const float MODIFIER_BANDAGE_SELF = 150;
	static const float MODIFIER_BANDAGE_FRIENDLIES = 300;
	static const float MODIFIER_TOURNIQUET_SELF = 150;
	static const float MODIFIER_TOURNIQUET_FRIENDLIES = 300;
	static const float MODIFIER_SALINE_SELF = 150;
	static const float MODIFIER_SALINE_FRIENDLIES = 300;
	static const float MODIFIER_MORPHINE_SELF = 150;
	static const float MODIFIER_MORPHINE_FRIENDLIES = 300;

	//Config of Ranges
	static const int INTERVALS_COUNT = 5;
	static ref const array<int> INTERVALS_ORIGIN = {0, 160000, 600000, 750000, 900000};
	static ref const array<int> INTERVALS_END = {160000, 600000, 750000, 900000, 1000000};
	static ref const array<float> INTERVALS_C1 = {1.9449, 0.71, 8.1682, 6, 18};
	static ref const array<float> INTERVALS_C2 = {0.9254, 0.9908, 0.9517, 0.9559, 0.9425};

	/************/
	//WAR CRIMES//
	/************/

	static const float WARCRIMES_PUNISHMENT = 0.25;

	static const float MAX_WARCRIMES_VALUE = 250000;
	static const float WARCRIMES_DECREASE_PER_HOUR = 25000;

	static const float STD_POINTS_QUALITY_TIME = 10000;
	
	/**************/
	//BAN SETTINGS//
	/**************/
	
	protected static ref SCR_BanSettingsContainer m_BanSettings = new SCR_BanSettingsContainer();

	/**********/
	//UI STUFF//
	/**********/

	static const float SPPOINTS_CONVERSIONPERCENTAGE =	0.0001;

	/********/
	//BASICS//
	/********/

	static const float MetersToKilometersConversion = 0.001;

	//------------------------------------------------------------------------------------------------
	static SCR_PlayerDataConfigs GetInstance()
	{
		if (!instance)
			instance = new SCR_PlayerDataConfigs();

		return instance;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetScoreThreshold()
	{
		return m_BanSettings.GetBanSettings().m_fScoreThreshold;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetScoreDecreasePerMinute()
	{
		return m_BanSettings.GetBanSettings().m_fScoreDecreasePerMinute;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetScoreDecreasePerSecond()
	{
		return m_BanSettings.GetBanSettings().m_fScoreDecreasePerMinute / 60;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetScoreToAccelerationMultiplier()
	{
		return m_BanSettings.GetBanSettings().m_fScoreMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMinAcceleration()
	{
		return m_BanSettings.GetBanSettings().m_fAccelerationMin;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxAcceleration()
	{
		return m_BanSettings.GetBanSettings().m_fAccelerationMax;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetBanEvaluationLight()
	{
		return m_BanSettings.GetBanSettings().m_fBanEvaluationLight;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetBanEvaluationHeavy()
	{
		return m_BanSettings.GetBanSettings().m_fBanEvaluationHeavy;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetCrimePointsFriendlyKill()
	{
		return m_BanSettings.GetBanSettings().m_fCrimePtFriendKill;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetCrimePointsFriendlyMultiKill()
	{
		return m_BanSettings.GetBanSettings().m_fCrimePtTeamKill;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetQualityTimeTemp()
	{
		return m_BanSettings.GetBanSettings().m_fQualityTimeTemp;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetVotingSuggestionEnabled()
	{
		return m_BanSettings.GetBanSettings().m_bVotingSuggestionEnabled >= 1;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetBanSettingsDescription()
	{
		return m_BanSettings.GetDesc();
	}

	//------------------------------------------------------------------------------------------------
	string GetSpecializationName(int n)
	{
		switch (n)
		{
			case 0: return SPECIALIZATION_0_NAME;
			case 1: return SPECIALIZATION_1_NAME;
			case 2: return SPECIALIZATION_2_NAME;
		}

		return "UNDEFINED_NAME";
	}

	//------------------------------------------------------------------------------------------------
	int GetSpecializationStatsCount(int n)
	{
		switch (n)
		{
			case 0: return SPECIALIZATION_0_COUNT;
			case 1: return SPECIALIZATION_1_COUNT;
			case 2: return SPECIALIZATION_2_COUNT;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	array<ref SCR_PlayerDataSpecializationDisplay> GetSpecializationArray(int n)
	{
		switch (n)
		{
			case 0: return m_aSpecialization0;
			case 1: return m_aSpecialization1;
			case 2: return m_aSpecialization2;
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadJson()
	{
		if (m_BanSettings.GetDesc().IsEmpty())
			GetGame().GetBackendApi().SettingsData("BanSettings", m_BanSettings);
		else
			Print(m_BanSettings.GetDesc(), LogLevel.DEBUG);
		
		if (m_BanSettings.GetDesc().IsEmpty())
		{
			Print("Ban Settings could not be loaded from BE Storage: Relying on default config", LogLevel.WARNING);
			m_BanSettings.SetDefault();
		}
		
		m_BanSettings.Log(LogLevel.DEBUG);
	}

	//------------------------------------------------------------------------------------------------
	private void SCR_PlayerDataConfigs()
	{
		m_aSpecialization0.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization0Display.DISTANCE_WALKED, SCR_EDataStats.DISTANCE_WALKED, "#AR-CareerProfile_DistanceTravelled_ByFoot", "#AR-CareerProfile_KMs"));
		m_aSpecialization0.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization0Display.PLAYER_KILLS, SCR_EDataStats.KILLS, "#AR-CareerProfile_PlayersKilled", "#AR-CareerProfile_Enemies"));
		m_aSpecialization0.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization0Display.AI_KILLS, SCR_EDataStats.AI_KILLS, "#AR-CareerProfile_AIKilled", "#AR-CareerProfile_Enemies"));
		m_aSpecialization0.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization0Display.SHOTS, SCR_EDataStats.SHOTS, "#AR-CareerProfile_BulletsShot", "#AR-CareerProfile_Rounds"));

		m_aSpecialization1.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization1Display.DISTANCE_DRIVEN, SCR_EDataStats.DISTANCE_DRIVEN, "#AR-CareerProfile_DistanceTravelled_AsDriver", "#AR-CareerProfile_KMs"));
		m_aSpecialization1.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization1Display.DISTANCE_AS_OCCUPANT, SCR_EDataStats.DISTANCE_AS_OCCUPANT, "#AR-CareerProfile_DistanceTravelled_AsPassenger", "#AR-CareerProfile_KMs"));
		m_aSpecialization1.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization1Display.POINTS_DRIVING_PEOPLE, SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS, "#AR-CareerProfile_PointsDriverAllies", "#AR-CareerProfile_Points"));

		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.BANDAGE_SELF, SCR_EDataStats.BANDAGE_SELF, "#AR-CareerProfile_BandagesSelf", "#AR-CareerProfile_Times"));
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.BANDAGE_FRIENDLIES, SCR_EDataStats.BANDAGE_FRIENDLIES, "#AR-CareerProfile_BandagesAllies", "#AR-CareerProfile_Times"));
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.TOURNIQUET_SELF, SCR_EDataStats.TOURNIQUET_SELF, "#AR-CareerProfile_TourniquetsSelf", "#AR-CareerProfile_Times"));
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.TOURNIQUET_FRIENDLIES, SCR_EDataStats.TOURNIQUET_FRIENDLIES, "#AR-CareerProfile_TourniquetsAllies", "#AR-CareerProfile_Times"));
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.SALINE_SELF, SCR_EDataStats.SALINE_SELF, "#AR-CareerProfile_SelinesSelf", "#AR-CareerProfile_Times"));
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.SALINE_FRIENDLIES, SCR_EDataStats.SALINE_FRIENDLIES, "#AR-CareerProfile_SelinesAllies", "#AR-CareerProfile_Times"));
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.MORPHINE_SELF, SCR_EDataStats.MORPHINE_SELF, "#AR-CareerProfile_MorphinesSelf", "#AR-CareerProfile_Times"));
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.MORPHINE_FRIENDLIES, SCR_EDataStats.MORPHINE_FRIENDLIES, "#AR-CareerProfile_MorphinesAllies", "#AR-CareerProfile_Times"));

		typename tEnum = SCR_ECareerSp;
		SPECIALIZATIONS_COUNT = tEnum.GetVariableCount();

		tEnum = SCR_ESpecialization0Display;
		SPECIALIZATION_0_COUNT = tEnum.GetVariableCount();

		tEnum = SCR_ESpecialization1Display;
		SPECIALIZATION_1_COUNT = tEnum.GetVariableCount();

		tEnum = SCR_ESpecialization2Display;
		SPECIALIZATION_2_COUNT = tEnum.GetVariableCount();

		if (m_aSpecialization0.Count() != SPECIALIZATION_0_COUNT)
			Print("Error in PlayerDataConfigs: Weird size on specialization0 array", LogLevel.ERROR);

		if (m_aSpecialization1.Count() != SPECIALIZATION_1_COUNT)
			Print("Error in PlayerDataConfigs: Weird size on specialization1 array", LogLevel.ERROR);

		if (m_aSpecialization2.Count() != SPECIALIZATION_2_COUNT)
			Print("Error in PlayerDataConfigs: Weird size on specialization2 array", LogLevel.ERROR);

		LoadJson();
	}
};

//Don't change the orders of any of these enums please, we are using their position as an id
//------------------------------------------------------------------------------------------------
enum SCR_ECareerSp
{
	INFANTRY,
	LOGISTICS,
	MEDICAL
};

//------------------------------------------------------------------------------------------------
enum SCR_ESpecialization0Display
{
	DISTANCE_WALKED,
	PLAYER_KILLS,
	AI_KILLS,
	SHOTS
};

//------------------------------------------------------------------------------------------------
enum SCR_ESpecialization1Display
{
	DISTANCE_DRIVEN,
	DISTANCE_AS_OCCUPANT,
	POINTS_DRIVING_PEOPLE
};

//------------------------------------------------------------------------------------------------
enum SCR_ESpecialization2Display
{
	BANDAGE_SELF,
	BANDAGE_FRIENDLIES,
	TOURNIQUET_SELF,
	TOURNIQUET_FRIENDLIES,
	SALINE_SELF,
	SALINE_FRIENDLIES,
	MORPHINE_SELF,
	MORPHINE_FRIENDLIES
};
