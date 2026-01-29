/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class BaseWeatherManagerEntityClass: GenericEntityClass
{
}

class BaseWeatherManagerEntity: GenericEntity
{
	proto external ref WeatherState GetCurrentWeatherState();

	/*!
	Retrieves in-game day duration as realtime seconds.
	\return Returns the duration of one in-game day as realtime seconds.
	*/
	proto external float GetDayDuration();
	/*!
	Sets in-game day duration as realtime seconds.
	Only issuable by the authority.
	Automatically broadcast to all clients.
	\param realtimeSeconds Seconds in real time one ingame day will take.
	\return Returns true when command is issued successfully, false otherwise.
	*/
	proto external bool SetDayDuration(float realtimeSeconds);
	/*!
	Enables or disabled automatic daytime advancement.
	Only issuable by the authority.
	Automatically broadcast to all clients.
	\param autoAdvanced True to enable, false to disable.
	\return Returns true when command is issued successfully, false otherwise.
	*/
	proto external bool SetIsDayAutoAdvanced(bool autoAdvanced);
	/*!
	Retrieves whether automatic time advancement is enabled or not.
	\return Returns true if daytime is advaned automatically, false otherwise.
	*/
	proto external bool GetIsDayAutoAdvanced();
	/*!
	Sets longitude as <-180, +180>.
	Can only be set by the authority (server, singleplayer...)
	\return Returns true when command is authorized, false otherwise.
	*/
	proto external bool SetCurrentLongitude(float longitude);
	/*!
	Returns current longitude.
	*/
	proto external float GetCurrentLongitude();
	/*!
	Returns current latitude.
	*/
	proto external float GetCurrentLatitude();
	/*!
	Sets latitude as <-90, +90>.
	Can only be set by the authority (server, singleplayer...)
	\return Returns true when command is authorized, false otherwise.
	*/
	proto external bool SetCurrentLatitude(float latitude);
	/*!
	Returns time zone offset (in hours)
	*/
	proto external float GetTimeZoneOffset();
	/*!
	Sets time zone offset (in hours)
	in range <-12.0, +14.0>
	*/
	proto external bool SetTimeZoneOffset(float utcTimeZone);
	/*!
	Gets DST offset (in hours)
	*/
	proto external float GetDSTOffset();
	/*!
	Sets DST Offset (in hours).
	Can only be set by authority.
	*/
	proto external bool SetDSTOffset(float dstOffsetHours);
	/*!
	Retrieves whether DST (daylight savings time) is enabled.
	*/
	proto external bool IsDSTEnabled();
	/*!
	Sets whether DST (daylight savings time) is enabled or disabled.
	*/
	proto external bool SetDSTEnabled(bool enabled);
	/*!
	Sets in-game wind speed in meters per second.
	Only issuable by the authority.
	Automatically broadcast to all clients.
	\param windSpeed Wind speed in m/s.
	\return Returns true when command is issued successfully, false otherwise.
	*/
	proto external bool SetWindSpeedOverride(bool doOverride, float windSpeed = 0);
	/*!
	Retrieves in-game wind speed in meters per second.
	\return Returns the in-game wind speed in meters per second.
	*/
	proto external float GetWindSpeed();
	/*!
	Sets in-game wind direction angle in degrees.
	Only issuable by the authority.
	Automatically broadcast to all clients.
	\param windDirection Wind direction angle in degrees <0, 360.0>
	\return Returns true when command is issued successfully, false otherwise.
	*/
	proto external bool SetWindDirectionOverride(bool doOverride, float windDirection = 0);
	/*!
	Retrieves in-game wind direction angle in degrees.
	\return Returns the in-game wind direction angle in degrees <0, 360.0>
	*/
	proto external float GetWindDirection();
	proto external bool IsWindSpeedOverridden();
	proto external bool IsWindDirectionOverridden();
	/*!
	Sets current rain intensity override (in range <0.0, 1.0>)
	Can only be set by the authority (server, singleplayer...)
	\return Returns true when command is authorized, false otherwise.
	*/
	proto external bool SetRainIntensityOverride(bool doOverride, float rainIntensity = 0);
	proto external float GetRainIntensity();
	proto external bool IsRainIntensityOverridden();
	/*!
	Sets current fog override (in range <0.0, 1.0>)
	Can only be set by the authority (server, singleplayer...)
	\return Returns true when command is authorized, false otherwise.
	*/
	proto external bool SetFogAmountOverride(bool doOverride, float fog = 0);
	proto external bool SetFogHeightDensityOverride(bool doOverride, float heightDensity = 0);
	proto external float GetFogAmount();
	proto external bool IsFogAmountOverridden();
	proto external float GetFogHeightDensity();
	proto external bool IsFogHeightDensityOverridden();
	/*
	Sets current wetness (in <0,1>)
	Can only be set by the authority (server, singleplayer...)
	\return Returns true when command is authorized, false otherwise.
	*/
	proto external bool SetCurrentWetness(float wetness);
	/*
	Gets current wetness (in <0,1>)
	*/
	proto external float GetCurrentWetness();
	/*
	Sets current water accumulation(in <0,1>)
	Can only be set by the authority (server, singleplayer...)
	\param waterAccumulationCracks water accumulation value in cracks
	\param waterAccumulationPuddles water accumulation value in puddles
	\return Returns true when command is authorized, false otherwise.
	*/
	proto external bool SetCurrentWaterAccumulation(float waterAccumulationCracks, float waterAccumulationPuddles);
	/*
	Gets current water accumulation in cracks (in <0,1>)
	*/
	proto external float GetCurrentWaterAccumulationCracks();
	/*
	Gets current water accumulation in puddles (in <0,1>)
	*/
	proto external float GetCurrentWaterAccumulationPuddles();
	proto external void AddLightning(WeatherLightning lightning);
	/*!
	Get weather state transition manager
	\return Transition manager, null if client is not master.
	*/
	proto BaseWeatherStateTransitionManager GetTransitionManager();
	/*!
	Gets moon phase for supplied date, geo location is not needed but timezone yes in order to calculate UTC correctly.

	\param year Year.
	\param month Month in range <1, 12>
	\param day Day in range <1, 31>
	\param timeOfTheDay24 Time of the day in 24 hour floating point format.
	\param timezone TimeZone Offset in hours ranging <-12, +14>
	\param dstOffset DST (daylight savings time) offset, must be 0.0 or positive value.

	\return phase in <0...1> range. 0 = new moon, 1 = full moon
	*/
	proto float GetMoonPhaseForDate(int year, int month, int day, float timeOfTheDay24, float timezone, float dstOffset);
	/*!
	Retrieve current sun direction, moon direction and moon phase.

	\param outSunDir Current sun direction
	\param outMoonDir Current moon direction
	\param outMoonPhase01 Moon phase (0 = new moon, 1 = full moon)
	*/
	proto void GetCurrentSunMoonDirAndPhase(out vector outSunDir, out vector outMoonDir, out float outMoonPhase01);
	/*!
	Gets sunrise hour in 24 hour format e.g. 12.50 is 12:30 A.M
	\return True if there is a valid sunrise with current latitude/longitude/date configuration, false otherwise
	*/
	proto bool GetSunriseHour(out float hour24);
	/*!
	Gets sunset hour in 24 hour format e.g. 12.50 is 12:30 A.M
	\return True if there is a valid sunset with current latitude/longitude/date configuration, false otherwise
	*/
	proto bool GetSunsetHour(out float hour24);
	/*!
	Gets sunrise hour in 24 hour format e.g. 12.50 is 12:30 A.M.
	For specific date, geolocation, timezone and DST.

	\param year Year.
	\param month Month in range <1, 12>
	\param day Day in range <1, 31>
	\param latitude Latitude in range <-90, 90> where +90 equals north pole, 0 equator and -90.0 south pole
	\param longitude Longitude in range <-180.0, 180.0> where 0 equals (almost) greenwich meridian.
	\param timezone TimeZone Offset in hours ranging <-12, +14>
	\param dstOffset DST (daylight savings time) offset, must be 0.0 or positive value.

	\return True if there is a valid sunrise with current latitude/longitude/date configuration, false otherwise
	*/
	proto bool GetSunriseHourForDate(int year, int month, int day, float latitude, float longitude, float timezone, float dstOffset, out float hour24);
	/*!
	Gets sunset hour in 24 hour format e.g. 12.50 is 12:30 A.M for specific date.
	\return True if there is a valid sunset with current latitude/longitude/date configuration, false otherwise
	*/
	proto bool GetSunsetHourForDate(int year, int month, int day, float latitude, float longitude, float timezone, float dstOffset, out float hour24);
	/*!
	Retrieves the current time of the day.
	\return Time fraction in hours <0.0, 24.0>
	Example:
	@code
	SetTimeOfTheDay(16.5);
	float totd = GetTimeOfTheDay();
	Print(totd);

	>> float totd = 16.5
	@endcode
	*/
	proto external float GetTimeOfTheDay();
	/*!
	Sets the current time of the day.
	Only issuable by the authority.
	Automatically broadcast to all clients.
	\param hours24 Time fraction in hours <0.0, 24.0>
	\param immediateChange Whether change should be applied immediately, forcing recomputation. This should be true only in case of editor and similar items.
	\return Returns true when command is issued successfully, false otherwise.
	Example:
	@code
		SetTimeOfTheDay(16.5);
		int h, m, s;
		GetHoursMinutesSeconds(h, m, s);
		Print(h);
		Print(m);
		Print(s);

		>> int h = 16
		>> int m = 30
		>> int s = 00
	@endcode
	*/
	proto external bool SetTimeOfTheDay(float hours24, bool immediateChange = false);
	/*!
	Sets the current time of the day.
	Only issuable by the authority.
	Automatically broadcast to all clients.
	\param hours Hours <0,24>
	\param minutes Minutes <0,60>
	\param seconds Seconds <0,60>
	\param immediateChange Whether change should be applied immediately, forcing recomputation. This should be true only in case of editor and similar items.
	\return Returns true when command is issued successfully, false otherwise.
	Example:
	@code
		SetHoursMinutesSeconds(16, 30, 00);
		int h, m, s;
		GetHoursMinutesSeconds(h, m, s);
		Print(h);
		Print(m);
		Print(s);

		>> int h = 16
		>> int m = 30
		>> int s = 00
	@endcode
	*/
	proto external bool SetHoursMinutesSeconds(int hours, int minutes, int seconds, bool immediateChange = false);
	/*!
	Returns current in-game year
	*/
	proto external int GetYear();
	/*!
	Returns current in-game month of the year (Range 1-12)
	*/
	proto external int GetMonth();
	/*!
	Returns current in-game day of the month (Range 1-31)
	*/
	proto external int GetDay();
	/*!
	Returns the in-game day
	*/
	proto external int GetDayInYear();
	/*!
	Sets the current in-game date (year/month/day).
	Only issuable by the authority.
	Automatically broadcast to all clients.
	You may check date validity prior to this call with CheckValidDate method.

	\param year Year
	\param month Month (in 1-12 range)
	\param day Day (in 1-31 range)
	\param immediateChange Whether change should be applied immediately, forcing recomputation. This should be true only in case of editor and similar items.
	\return Returns true when command is issued successfully and date is VALID, false otherwise.
	*/
	proto external bool SetDate(int year, int month, int day, bool immediateChange = false);
	/*!
	Retrieves the current date (Year, Month in 1-12 range, Day in 1-31 range).
	*/
	proto void GetDate(out int year, out int month, out int day);
	/*!
	Retrieves the current time of the day.
	Example:
	@code
	SetTimeOfTheDay(16.5);
	int h, m, s;
	GetHoursMinutesSeconds(h, m, s);
	Print(h);
	Print(m);
	Print(s);

	>> int h = 16
	>> int m = 30
	>> int s = 00
	@endcode
	*/
	proto void GetHoursMinutesSeconds(out int hours, out int minutes, out int seconds);
	/*
	Converts in-game time of the day to hours, minutes and seconds.
	\param hours24 In-game time as a fraction of day <0.0, 24.0>
	*/
	static proto void TimeToHoursMinutesSeconds(float hours24, out int hours, out int minutes, out int seconds);
	/*
	Converts hours, minutes and seconds to in-game time of the day.
	\return Returns time of the day as a fraction of day <0.0, 24.0>
	*/
	static proto float HoursMinutesSecondsToTime(int hours, int minutes, int seconds);
	/*!
	Returns current in-game week day in range [0-6], starts at Monday=0.
	\return positive integer 0-6 representing each day of the week, starting at Monday.  -1 if current date is invalid.
	*/
	proto external int GetWeekDay();
	/*!
	Returns week day in range [0-6], starts at Monday=0, for the supplied date.
	\return positive integer 0-6 representing each day of the week, starting at Monday.  -1 if supplied date is invalid.

	\param year Year.
	\param month Month in range <1, 12>
	\param day Day in range <1, 31>
	*/
	proto external int GetWeekDayForDate(int year, int month, int day);
	/*!
	Retrieves the current engine time (synchronized between clients),
	this value is not affected by time multiplier nor automatic time advancement attributes.
	*/
	proto external float GetEngineTime();
	/*!
	Gets all Weather States.
	*/
	proto external void GetWeatherStatesList(out notnull array<ref WeatherState> outStates);
	/*!
	Gets moon phase for supplied time of the day.
	\return phase in <0...1> range. 0 = new moon, 1 = full moon
	*/
	proto external float GetMoonPhase(float timeOfTheDay24);
	/*!
	Sets if Sun irradiance override is enabled
	\return true if command was succesful
	*/
	proto external bool SetOverrideSunIrradiance(bool doOverride);
	/*! Gets if Sun irradience override is enabled */
	proto external bool GetOverrideSunIrradiance();
	/*!
	Sets values for Sun Irradiance override (values are for min/max during the day)
	\return true if command was succesful
	*/
	proto external bool SetSunIrradianceOverrideValues(float sunIrradianceMin, float sunIrradianceMax);
	/*! Gets value for Sun Irradiance Min override. Value represents minimal Sun irradiance during the day */
	proto external float GetSunIrradianceMinOverride();
	/*! Gets value for Sun Irradiance Max override. Value represents maximal Sun irradiance during the day */
	proto external float GetSunIrradianceMaxOverride();
	/*!
	Sets if temperature override is enabled
	\return true if command was succesful
	*/
	proto external bool SetOverrideTemperature(bool doOverride);
	/*! Gets if temperature override is enabled */
	proto external bool GetOverrideTemperature();
	/*!
	Sets values for temperature override (in degrees Celsius). Values are for min/max during the day.
	\return true if command was succesful
	*/
	proto external bool SetTemperatureOverrideValues(float temperatureAirMin, float temperatureAirMax,	float temperatureSkyMin, float temperatureSkyMax, float temperatureWaterMin, float temperatureWaterMax);
	/*! Gets value for Temperature Air Min override (in degrees Celsius). Value represents minimal air temperature during the day */
	proto external float GetTemperatureAirMinOverride();
	/*! Gets value for Temperature Air Max override (in degrees Celsius). Value represents maximal air temperature during the day */
	proto external float GetTemperatureAirMaxOverride();
	/*! Gets value for Temperature Water Min override (in degrees Celsius). Value represents minimal water temperature during the day */
	proto external float GetTemperatureWaterMinOverride();
	/*! Gets value for Temperature Water Max override (in degrees Celsius). Value represents maximal water temperature during the day */
	proto external float GetTemperatureWaterMaxOverride();
	/*! Gets value for Temperature Sky Min override (in degrees Celsius). Value represents minimal sky temperature during the day */
	proto external float GetTemperatureSkyMinOverride();
	/*! Gets value for Temperature Sky Max override (in degrees Celsius). Value represents maximal sky temperature during the day */
	proto external float GetTemperatureSkyMaxOverride();
	/*!
	* Forces complete recomputation of temperature table based on current min/max temperatures and sun irradiance.
	* This needs to be called if you change some min/max temperature settings and want to immediately change
	* temperatures of all objects based on these settings.
	* Beware! This is performance heavy so use only when really needed (for example when changing temperature settings and
	* you want to see immediate effect of the changes).
	*/
	proto external bool ForceTemperatureTableRecompute();
	/*!
	Checks if date is valid (checks for leap years and correct day of the month)

	\param year Year
	\param month Month (in 1-12 range)
	\param day Day (in 1-31 range)
	\return Returns true when date is valid, false otherwise.
	*/
	proto external bool CheckValidDate(int year, int month, int day);
	/*!
	update weather by selected timeSlice

	\param timeSlice					timeslice
	\param advanceSimulation	advance simulation time by the timeslice value, if false, only weather is updated
	*/
	proto external void UpdateWeather(float timeSlice, bool advanceSimulation = true);
}

/*!
\}
*/
