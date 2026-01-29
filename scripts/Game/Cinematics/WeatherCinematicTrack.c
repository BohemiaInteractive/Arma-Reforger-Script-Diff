[CinematicTrackAttribute(name:"Weather Track", description:"Allows you to change weather options")]
class WeatherCinematicTrack : CinematicTrackBase
{
	[Attribute("0.0", params:"0.0 23.9")]
	float m_fTimeOfDay;
	
	[Attribute("0", params:"0 3")]
	int m_iWeatherType;
	
	[Attribute("false")]
	bool setSpecifics;
	
	[Attribute("0.0", params:"0.0 20.0")]
	float m_fWindSpeed;
	
	[Attribute("0.0", params:"0.0 360.0")]
	float m_fWindDirection;
	
	[Attribute("0.0", params:"0.0 1.0")]
	float m_fWetness;
	
	[Attribute("0.0", params:"0.0 1.0")]
	float m_fPuddles;
	
	[Attribute("0.0", params:"0.0 1.0")]
	float m_fFog;
	
	[Attribute("0.0", params:"0.0 1.0")]
	float m_fFogHeight;
	
	
	[Attribute("false")]
	bool setDate;
	
	[Attribute("1989")]
	int m_iYear;
	
	[Attribute("4")]
	int m_iMonth;
	
	[Attribute("1")]
	int m_iDay;
	
	float m_fActualTimeOfDay;
	float m_fActualWindSpeed;
	float m_fActualWindDirection;
	int m_iActualWeatherType;
	
	ChimeraWorld chimWorld;
	
	TimeAndWeatherManagerEntity weatherManager;
	
	override void OnInit(World world)
	{
		chimWorld = world;
		
		m_iActualWeatherType = -1;
		
		if(chimWorld)
			weatherManager = chimWorld.GetTimeAndWeatherManager();
			
		
		m_fActualTimeOfDay = -1;
		m_fActualWindSpeed = -1;
		m_fActualWindDirection = -1;
	}
	
	override void OnApply(float time)
	{	
		
		if (weatherManager)
		{
			
			WeatherState weatherState = weatherManager.GetCurrentWeatherState();
			
			if (setSpecifics)
			{
				weatherManager.SetFogAmountOverride(true, m_fFog);
				weatherManager.SetFogHeightDensityOverride(true, m_fFogHeight);
				weatherManager.SetCurrentWetness(m_fWetness);
				weatherManager.SetCurrentWaterAccumulation(0,m_fPuddles);
				
				//Wind speed and direction
				if (m_fWindSpeed != m_fActualWindSpeed)
				{
					weatherManager.SetWindSpeedOverride(true, m_fWindSpeed);
					m_fActualWindSpeed = m_fWindSpeed;
				}
				
				if (m_fWindDirection != m_fActualWindDirection)
				{
					weatherManager.SetWindDirectionOverride(true, m_fWindDirection);
					 m_fActualWindDirection = m_fWindDirection;
				}
			}
			
			//Time of day
			if (m_fTimeOfDay != m_fActualTimeOfDay)
			{
				weatherManager.SetTimeOfTheDay(m_fTimeOfDay, true);
				
				if (setDate)
				{
					weatherManager.SetDate(m_iYear, m_iMonth, m_iDay);
				}
				m_fActualTimeOfDay = m_fTimeOfDay;
			}
			
			
			if ( m_iWeatherType == 0 && weatherState.GetStateName() != "Clear")
			{
				weatherManager.ForceWeatherTo(false, "Clear");
				
			} else if ( m_iWeatherType == 1 && weatherState.GetStateName() != "Cloudy") {
			
			  weatherManager.ForceWeatherTo(false, "Cloudy");
				
			} else if ( m_iWeatherType == 2 && weatherState.GetStateName() != "Overcast") {
			
			  weatherManager.ForceWeatherTo(false, "Overcast");
				
			} else if (m_iWeatherType == 3 && weatherState.GetStateName() != "Rainy") {
			
			  weatherManager.ForceWeatherTo(false, "Rainy");
			}
			
			weatherManager.UpdateWeather(0.01);
		}
	}
}