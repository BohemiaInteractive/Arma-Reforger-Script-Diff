class TimeAndWeatherManagerEntitySerializer : ScriptedEntitySerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return TimeAndWeatherManagerEntity;
	}

	//------------------------------------------------------------------------------------------------
	override static EDeserializeFailHandling GetDeserializeFailHandling()
	{
		return EDeserializeFailHandling.ERROR;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity entity, notnull BaseSerializationSaveContext context)
	{
		const TimeAndWeatherManagerEntity timeAndWeatherManager = TimeAndWeatherManagerEntity.Cast(entity);

		int year, month, day, hour, minute, second;
		timeAndWeatherManager.GetDate(year, month, day);
		timeAndWeatherManager.GetHoursMinutesSeconds(hour, minute, second);

		const WeatherState currentWeather = timeAndWeatherManager.GetCurrentWeatherState();
		const string state = currentWeather.GetStateName();
		const bool looping = timeAndWeatherManager.IsWeatherLooping();

		const float dayDuration = timeAndWeatherManager.GetDayDuration();
		const bool isDayAutoAdvanced = timeAndWeatherManager.GetIsDayAutoAdvanced();

		const float latitude = timeAndWeatherManager.GetCurrentLatitude();
		const float longitude = timeAndWeatherManager.GetCurrentLongitude();

		const float timeZoneOffset = timeAndWeatherManager.GetTimeZoneOffset();

		const bool dstEnabled = timeAndWeatherManager.IsDSTEnabled();
		const float dstOffset = timeAndWeatherManager.GetDSTOffset();

		const bool windSpeedOverridden = timeAndWeatherManager.IsWindSpeedOverridden();
		const float windSpeed = timeAndWeatherManager.GetWindSpeed();

		const bool windDirectionOverridden = timeAndWeatherManager.IsWindDirectionOverridden();
		const float windDirection = timeAndWeatherManager.GetWindDirection();

		const bool isRainIntensityOverridden = timeAndWeatherManager.IsRainIntensityOverridden();
		const float rainIntensity = timeAndWeatherManager.GetRainIntensity();

		const bool fogAmountOverridden = timeAndWeatherManager.IsFogAmountOverridden();
		const float fogAmount = timeAndWeatherManager.GetFogAmount();

		const bool fogHeightDensityOverridden = timeAndWeatherManager.IsFogHeightDensityOverridden();
		const float fogHeightDensity = timeAndWeatherManager.GetFogHeightDensity();

		const float wetness = timeAndWeatherManager.GetCurrentWetness();
		const float waterAccumulationCracks = timeAndWeatherManager.GetCurrentWaterAccumulationCracks();
		const float waterAccumulationPuddles = timeAndWeatherManager.GetCurrentWaterAccumulationPuddles();

		const bool overrideSunIrradiance = timeAndWeatherManager.GetOverrideSunIrradiance();
		const float sunIrradianceMinOverride = timeAndWeatherManager.GetSunIrradianceMinOverride();
		const float sunIrradianceMaxOverride = timeAndWeatherManager.GetSunIrradianceMaxOverride();

		const bool overrideTemperature = timeAndWeatherManager.GetOverrideTemperature();
		const float temperatureAirMinOverride = timeAndWeatherManager.GetTemperatureAirMinOverride();
		const float temperatureAirMaxOverride = timeAndWeatherManager.GetTemperatureAirMaxOverride();
		const float temperatureWaterMinOverride = timeAndWeatherManager.GetTemperatureWaterMinOverride();
		const float temperatureWaterMaxOverride = timeAndWeatherManager.GetTemperatureWaterMaxOverride();
		const float temperatureSkyMinOverride = timeAndWeatherManager.GetTemperatureSkyMinOverride();
		const float temperatureSkyMaxOverride = timeAndWeatherManager.GetTemperatureSkyMaxOverride();

		context.WriteValue("version", 1);

		context.Write(year);
		context.Write(month);
		context.Write(day);
		context.Write(hour);
		context.Write(minute);
		context.Write(second);

		context.WriteDefault(dayDuration, 86400.0);
		context.WriteDefault(isDayAutoAdvanced, true);
		context.WriteDefault(latitude, 50.073);
		context.WriteDefault(longitude, 14.437);
		context.WriteDefault(timeZoneOffset, 0);
		context.WriteDefault(dstEnabled, true);
		if (dstEnabled)
			context.WriteDefault(dstOffset, 1.0);

		context.Write(state);
		context.WriteDefault(looping, false);

		context.WriteDefault(wetness, 0.0);
		context.WriteDefault(waterAccumulationCracks, 0.0);
		context.WriteDefault(waterAccumulationPuddles, 0.0);

		context.WriteDefault(windSpeedOverridden, false);
		if (windSpeedOverridden)
			context.Write(windSpeed);

		context.WriteDefault(windDirectionOverridden, false);
		if (windDirectionOverridden)
			context.Write(windDirection);

		context.WriteDefault(isRainIntensityOverridden, false);
		if (isRainIntensityOverridden)
			context.Write(rainIntensity);

		context.WriteDefault(fogAmountOverridden, false);
		if (fogAmountOverridden)
			context.Write(fogAmount);

		context.WriteDefault(fogHeightDensityOverridden, false);
		if (fogHeightDensityOverridden)
			context.Write(fogHeightDensity);

		context.WriteDefault(overrideSunIrradiance, false);
		if (overrideSunIrradiance)
		{
			context.Write(sunIrradianceMinOverride);
			context.Write(sunIrradianceMaxOverride);
		}

		context.WriteDefault(overrideTemperature, false);
		if (overrideTemperature)
		{
			context.Write(temperatureAirMinOverride);
			context.Write(temperatureAirMaxOverride);
			context.Write(temperatureWaterMinOverride);
			context.Write(temperatureWaterMaxOverride);
			context.Write(temperatureSkyMinOverride);
			context.Write(temperatureSkyMaxOverride);
		}

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity entity, notnull BaseSerializationLoadContext context)
	{
		auto timeAndWeatherManager = TimeAndWeatherManagerEntity.Cast(entity);

		int version;
		context.Read(version);

		int year, month, day, hour, minute, second;
		context.Read(year);
		context.Read(month);
		context.Read(day);
		context.Read(hour);
		context.Read(minute);
		context.Read(second);
		timeAndWeatherManager.SetDate(year, month, day);
		timeAndWeatherManager.SetHoursMinutesSeconds(hour, minute, second);

		float dayDuration;
		if (context.Read(dayDuration))
			timeAndWeatherManager.SetDayDuration(dayDuration);

		bool isDayAutoAdvanced;
		if (context.Read(isDayAutoAdvanced))
			timeAndWeatherManager.SetIsDayAutoAdvanced(isDayAutoAdvanced);

		float latitude;
		if (context.Read(latitude))
			timeAndWeatherManager.SetCurrentLatitude(latitude);

		float longitude;
		if (context.Read(longitude))
			timeAndWeatherManager.SetCurrentLongitude(longitude);

		float timeZoneOffset;
		if (context.Read(timeZoneOffset))
			timeAndWeatherManager.SetTimeZoneOffset(timeZoneOffset);

		bool dstEnabled;
		if (context.Read(dstEnabled))
			timeAndWeatherManager.SetDSTEnabled(dstEnabled);
		
		if (dstEnabled)
		{
			float dstOffset;
			if (context.Read(dstOffset))
				timeAndWeatherManager.SetDSTOffset(dstOffset);
		}

		string state;
		bool looping;
		context.Read(state);
		context.ReadDefault(looping, false);
		timeAndWeatherManager.ForceWeatherTo(looping, state);

		float wetness;
		if (context.Read(wetness))
			timeAndWeatherManager.SetCurrentWetness(wetness);

		float waterAccumulationCracks, waterAccumulationPuddles;
		context.ReadDefault(waterAccumulationCracks, 0.0);
		context.ReadDefault(waterAccumulationPuddles, 0.0);
		timeAndWeatherManager.SetCurrentWaterAccumulation(waterAccumulationCracks, waterAccumulationPuddles);
		
		bool windSpeedOverridden;
		context.ReadDefault(windSpeedOverridden, false);
		if (windSpeedOverridden)
		{
			float windSpeed;
			context.Read(windSpeed);
			timeAndWeatherManager.SetWindSpeedOverride(true, windSpeed);
		}

		bool windDirectionOverridden;
		context.ReadDefault(windDirectionOverridden, false);
		if (windDirectionOverridden)
		{
			float windDirection;
			context.Read(windDirection);
			timeAndWeatherManager.SetWindDirectionOverride(true, windDirection);
		}

		bool isRainIntensityOverridden;
		context.ReadDefault(isRainIntensityOverridden, false);
		if (isRainIntensityOverridden)
		{
			float rainIntensity;
			context.Read(rainIntensity);
			timeAndWeatherManager.SetRainIntensityOverride(true, rainIntensity);
		}

		bool fogAmountOverridden;
		context.ReadDefault(fogAmountOverridden, false);
		if (fogAmountOverridden)
		{
			float fogAmount;
			context.Read(fogAmount);
			timeAndWeatherManager.SetFogAmountOverride(true, fogAmount);
		}

		bool fogHeightDensityOverridden;
		context.ReadDefault(fogHeightDensityOverridden, false);
		if (fogHeightDensityOverridden)
		{
			float fogHeightDensity;
			context.Read(fogHeightDensity);
			timeAndWeatherManager.SetFogHeightDensityOverride(true, fogHeightDensity);
		}

		bool overrideSunIrradiance;
		context.ReadDefault(overrideSunIrradiance, false);
		if (overrideSunIrradiance)
		{
			float sunIrradianceMinOverride, sunIrradianceMaxOverride;
			context.Read(sunIrradianceMinOverride);
			context.Read(sunIrradianceMaxOverride);
			timeAndWeatherManager.SetOverrideSunIrradiance(true);
			timeAndWeatherManager.SetSunIrradianceOverrideValues(sunIrradianceMinOverride, sunIrradianceMaxOverride);
		}

		bool overrideTemperature;
		context.ReadDefault(overrideTemperature, false);
		if (overrideTemperature)
		{
			float temperatureAirMinOverride, temperatureAirMaxOverride;
			float temperatureWaterMinOverride, temperatureWaterMaxOverride;
			float temperatureSkyMinOverride, temperatureSkyMaxOverride;
			context.Read(temperatureAirMinOverride);
			context.Read(temperatureAirMaxOverride);
			context.Read(temperatureWaterMinOverride);
			context.Read(temperatureWaterMaxOverride);
			context.Read(temperatureSkyMinOverride);
			context.Read(temperatureSkyMaxOverride);
			timeAndWeatherManager.SetOverrideTemperature(true);
			timeAndWeatherManager.SetTemperatureOverrideValues(temperatureAirMinOverride, temperatureAirMaxOverride, temperatureWaterMinOverride, temperatureWaterMaxOverride, temperatureSkyMinOverride, temperatureSkyMaxOverride);
		}

		return true;
	}
}
