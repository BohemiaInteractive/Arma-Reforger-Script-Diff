class SCR_TimeAndWeatherHandlerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_TimeAndWeatherHandlerComponent : SCR_BaseGameModeComponent
{
	[Attribute("8", UIWidgets.Slider, "Starting time of day (hours)", "0 23 1", category:"Time")]
	protected int m_iStartingHours;

	[Attribute("0", UIWidgets.Slider, "Starting time of day (minutes)", "0 59 1", category:"Time")]
	protected int m_iStartingMinutes;

	[Attribute("0", category:"Time")]
	protected bool m_bRandomStartingDaytime;

	[Attribute("0", desc: "Random time range is superior to random time. When both are set to true, random range will be used.", category:"Time")]
	protected bool m_bRandomStartingDaytimeRange;

	[Attribute("8", UIWidgets.Slider, "Starting range of starting time of day \n (hours)", "0 23 1", category:"Time")]
	protected int m_iRangeHoursStart;

	[Attribute("0", UIWidgets.Slider, "Starting range of starting time of day \n (minutes)", "0 59 1", category:"Time")]
	protected int m_iRangeMinutesStart;

	[Attribute("8", UIWidgets.Slider, "Ending range of starting time of day \n (hours)", "0 23 1", category:"Time")]
	protected int m_iRangeHoursEnd;

	[Attribute("0", UIWidgets.Slider, "Ending range of starting time of day \n (minutes)", "0 59 1", category:"Time")]
	protected int m_iRangeMinutesEnd;

	[Attribute("1", UIWidgets.Slider, "Time acceleration during the day (1 = 100%, 2 = 200% etc)", "0.1 12 0.1", category:"Time")]
	protected float m_fDayTimeAcceleration;

	[Attribute("1", UIWidgets.Slider, "Time acceleration during the night (1 = 100%, 2 = 200% etc)", "0.1 12 0.1", category:"Time")]
	protected float m_fNightTimeAcceleration;

	[Attribute("0", category:"Weather")]
	protected bool m_bRandomStartingWeather;

	[Attribute("0", desc: "Use predefine sets of weather and time.", category:"Weather")]
	protected bool m_bUsePredefineStartingTimeAndWeather;

	[Attribute(desc: "List of predefine time and weather settings.", category:"Weather")]
	protected ref array<ref SCR_TimeAndWeatherState> m_aStartingWeatherAndTime;

	[Attribute("0", desc: "Weather can change during gameplay", category:"Weather")]
	protected bool m_bRandomWeatherChanges;

	[Attribute("0", desc: "Scenario header setting will overwrite these values.", category:"Weather")]
	protected bool m_bAllowHeaderSettings;

	[Attribute(defvalue: "0", desc: "Wind Override", category:"Wind")]
	protected bool m_bWindOverride;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Wind Speed", params: "0 20", category:"Wind")]
	protected float m_fWindSpeed;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Wind Direction", params: "0 1", category:"Wind")]
	protected float m_fWindDirection;

	[Attribute(defvalue: "0", desc: "Fog Override", category:"Fog")]
	protected bool m_bFogOverride;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Fog Amount", params: "0 1", category:"Fog")]
	protected float m_fFogAmount;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Fog Height Density", params: "0 1", category:"Fog")]
	protected float m_fFogHeightDensity;

	[Attribute(defvalue: "0", desc: "Override Date", category:"Date")]
	protected bool m_bDateOverride;

	[Attribute(defvalue: "1", UIWidgets.Slider, desc: "Day", params: "1 12", category:"Date")]
	protected int m_iDay;

	[Attribute(defvalue: "1", UIWidgets.Slider, desc: "Month", params: "1 12", category:"Date")]
	protected int m_iMonth;

	[Attribute(defvalue: "1989", UIWidgets.Slider, desc: "Year", params: "1899 2050", category:"Date")]
	protected int m_iYear;

	const int DAYTIME_CHECK_PERIOD = 60000;	//ms
	const float DEFAULT_DAYTIME_START = 5.0;
	const float DEFAULT_DAYTIME_END = 19.0;
	const int DAY_DURATION = 24 * 60 * 60;

	protected bool m_bDaytimeAcceleration = true;
	protected bool m_bSavedSettingApplied = false;

	protected static SCR_TimeAndWeatherHandlerComponent s_Instance;

	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_TimeAndWeatherHandlerComponent GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	void SetupWind(float windSpeed, float windDirection)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetOwner().GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity manager = world.GetTimeAndWeatherManager();
		if (!manager)
			return;

		manager.SetWindSpeedOverride(true, windSpeed);
		manager.SetWindDirectionOverride(true, windDirection);
	}

	//------------------------------------------------------------------------------------------------
	void SetupFog(float fogAmount, float fogHeightDensity)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetOwner().GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity manager = world.GetTimeAndWeatherManager();
		if (!manager)
			return;

		manager.SetFogAmountOverride(true, fogAmount);
		manager.SetFogHeightDensityOverride(true, fogHeightDensity);
	}

	//------------------------------------------------------------------------------------------------
	void SetupDate(int day, int month, int year)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetOwner().GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity manager = world.GetTimeAndWeatherManager();
		if (!manager)
			return;

		if (!manager.SetDate(year, month, day))
			Print("SCR_TimeAndWeatherHandlerComponent: Invalid date parameter", LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] hours
	//! \param[in] minutes
	//! \param[in] seconds
	//! \param[in] loadedWeatherState
	//! \param[in] loadDone
	void SetupDaytimeAndWeather(int hours, int minutes, int seconds = 0, string loadedWeatherState = "", bool loadDone = false)
	{
		if (m_bSavedSettingApplied)
			return;

		m_bSavedSettingApplied = loadDone;

		ChimeraWorld world = ChimeraWorld.CastFrom(GetOwner().GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity manager = world.GetTimeAndWeatherManager();
		if (!manager)
			return;

		float sunrise, morning, evening, sunset;

		// Use world-calculated sunrise and sunset values if possible, otherwise use defaults
		if (manager.GetSunriseHour(sunrise))
		{
			manager.GetSunsetHour(sunset);
		}
		else
		{
			sunrise = DEFAULT_DAYTIME_START;
			sunset = DEFAULT_DAYTIME_END;
		}

		if (m_bRandomStartingDaytime && !loadDone && !m_bRandomStartingDaytimeRange)
		{
			// Compile a list of presets based on the sunrise and sunset times of current world if we're randomizing
			morning = sunrise + 0.25;	// Just so it's not still completely dark at the start
			float noon = (sunrise + sunset) * 0.5;
			float afternoon = (noon + sunset) * 0.5;
			evening = sunset - 0.5;
			float night = noon + 12;

			if (night >= 24)
				night -= 24;

			array<float> startingTimes = {morning, noon, afternoon, evening, night};

			// Add weights so evening / night is a bit more rare
			int index = SCR_ArrayHelper.GetWeightedIndex({25, 25, 25, 15, 10}, Math.RandomFloat01());
			float startingTime;

			if (startingTimes.IsIndexValid(index))
				startingTime = startingTimes[index];
			else
				startingTime = startingTimes.GetRandomElement();

			manager.TimeToHoursMinutesSeconds(startingTime, hours, minutes, seconds);
		}

		if (m_bRandomStartingDaytimeRange && !loadDone)
		{
			if (m_iRangeHoursStart <= m_iRangeHoursEnd)
			{
				hours = Math.RandomInt(m_iRangeHoursStart, m_iRangeHoursEnd);
			}
			else
			{
				hours = Math.RandomInt(m_iRangeHoursStart, m_iRangeHoursEnd + 24) - 24;
			}

			if (m_iRangeMinutesStart <= m_iRangeMinutesEnd)
			{
				minutes = Math.RandomInt(m_iRangeMinutesStart, m_iRangeMinutesEnd);
			}
			else
			{
				minutes = Math.RandomInt(m_iRangeHoursStart, m_iRangeHoursEnd + 60) - 60;
			}
		}

		if (m_bRandomStartingWeather && !loadDone)
		{
			array<ref WeatherState> weatherStates = {};
			manager.GetWeatherStatesList(weatherStates);

			if (!weatherStates.IsEmpty())
				manager.ForceWeatherTo(!m_bRandomWeatherChanges, weatherStates.GetRandomElement().GetStateName());
		}

		if (m_bUsePredefineStartingTimeAndWeather && !m_aStartingWeatherAndTime.IsEmpty())
		{
			SCR_TimeAndWeatherState timeAndWEatherState = m_aStartingWeatherAndTime.GetRandomElement();
			manager.ForceWeatherTo(!m_bRandomWeatherChanges, timeAndWEatherState.GetWeatherPresetName());
			hours = timeAndWEatherState.GetStartingHour();
			minutes = timeAndWEatherState.GetStartingMinutes();
		}

		if (!loadedWeatherState.IsEmpty())
			manager.ForceWeatherTo(!m_bRandomWeatherChanges, loadedWeatherState);

		manager.SetHoursMinutesSeconds(hours, minutes, seconds);

		// Periodically check if the acceleration is correct, based on time of day
		// SetTimeEvent is not usable since it requires changing Periodicity attribute directly in the manager entity in the world layer
		if (m_fDayTimeAcceleration != 1 || m_fNightTimeAcceleration != 1)
		{
			HandleDaytimeAcceleration(true);
			GetGame().GetCallqueue().Remove(HandleDaytimeAcceleration);
			GetGame().GetCallqueue().CallLater(HandleDaytimeAcceleration, DAYTIME_CHECK_PERIOD, true, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleDaytimeAcceleration(bool setup = false)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetOwner().GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity manager = world.GetTimeAndWeatherManager();
		if (!manager)
			return;

		if (manager.IsNightHour(manager.GetTimeOfTheDay()))
		{
			if (m_bDaytimeAcceleration || setup)
			{
				m_bDaytimeAcceleration = false;
				manager.SetDayDuration(DAY_DURATION / m_fNightTimeAcceleration);
			}
		}
		else
		{
			if (!m_bDaytimeAcceleration || setup)
			{
				m_bDaytimeAcceleration = true;
				manager.SetDayDuration(DAY_DURATION / m_fDayTimeAcceleration);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnWorldPostProcess(World world)
	{
		super.OnWorldPostProcess(world);

		if (!Replication.IsServer() || !GetGame().InPlayMode())
			return;

		if (s_Instance != this)
		{
			Print("Multiple instances of SCR_TimeAndWeatherHandlerComponent detected.", LogLevel.WARNING);
			return;
		}

		SetupDaytimeAndWeather(m_iStartingHours, m_iStartingMinutes);

		if (m_bDateOverride)
			SetupDate(m_iDay, m_iMonth, m_iYear);

		if (m_bFogOverride)
			SetupFog(m_fFogAmount, m_fFogHeightDensity);

		if (m_bWindOverride)
			SetupWind(m_fWindSpeed, m_fWindDirection);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		// Allow only one instance
		if (s_Instance || !GetGame().InPlayMode())
			return;

		// Allow permanent startign daytime & weather for debugging purposes
#ifdef TDM_CLI_SELECTION
		return;
#endif

		s_Instance = this;

		if (!Replication.IsServer())
			return;

		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());

		if (m_bAllowHeaderSettings && header && header.m_bOverrideScenarioTimeAndWeather)
		{
			m_iStartingHours = header.m_iStartingHours;
			m_iStartingMinutes = header.m_iStartingMinutes;
			m_bRandomStartingDaytime = header.m_bRandomStartingDaytime;
			m_fDayTimeAcceleration = header.m_fDayTimeAcceleration;
			m_fNightTimeAcceleration = header.m_fNightTimeAcceleration;
			m_bRandomStartingWeather = header.m_bRandomStartingWeather;
			m_bRandomWeatherChanges = header.m_bRandomWeatherChanges;
		}
	}
}
