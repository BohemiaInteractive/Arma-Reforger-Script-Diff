[BaseContainerProps()]
class SCR_AnalyticsDataCollectionTimeAndWeatherModule : SCR_AnalyticsDataCollectionModule
{
	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerConnected(int playerId)
	{
		SetPlayerTimeAndNight(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerDisconnected(int playerId, KickCauseCode cause, IEntity controlledEntity)
	{
		SetPlayerTimeAndNight(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetPlayerTimeAndNight(int playerId)
	{
		if (!m_bIsEnabled)
			return;

		string timeString;
		bool isNight;
		GetTimeAndNight(timeString, isNight);

		if (timeString.IsEmpty())
			return;

		SCR_PlayerData playerData = SCR_PlayerData.GetPlayerData(playerId);
		if (!playerData)
			return;

		playerData.dataEvent.num_daytime = timeString;
		playerData.dataEvent.is_night = isNight;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the current world time, converts it to military format (12:35 -> 1235) and checks if current time is night time
	//! \param[out] time
	//! \param[out] isNight
	protected void GetTimeAndNight(out string time, out bool isNight)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity manager = world.GetTimeAndWeatherManager();
		if (!manager)
			return;

		float timeOfTheDay = manager.GetTimeOfTheDay();

		int hours, minutes, seconds;
		manager.TimeToHoursMinutesSeconds(timeOfTheDay, hours, minutes, seconds);
		isNight = manager.IsNightHour(timeOfTheDay);
		time = string.Format("%1%2", hours.ToString(2), minutes.ToString(2));
	}

	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void DrawContent()
	{
		if (!m_bIsEnabled)
		{
			DbgUI.Text("Module disabled!");
			return;
		}

		DbgUI.Text("This module does not save any data.");
	}
	#endif

	//------------------------------------------------------------------------------------------------
	protected void ~SCR_AnalyticsDataCollectionTimeAndWeatherModule()
	{
		Disable();
	}
}