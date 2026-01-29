class SCR_AIDayNightTest : DecoratorScripted
{
	protected TimeAndWeatherManagerEntity m_TimeManager;
	protected float m_fSunriseTime;
	protected float m_fSunsetTime;
	
	//-------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Returns true during daytime, false otherwise.";
	}
	
	//-------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		ChimeraWorld world = owner.GetWorld();
		m_TimeManager = world.GetTimeAndWeatherManager();
		if( !m_TimeManager )
		{
			Print("SCR_AIDayNightTest: Could not find a TimeAndWeatherManagerEntity", LogLevel.WARNING);
			return;
		}
		
		if (!m_TimeManager.GetSunriseHour(m_fSunriseTime) || !m_TimeManager.GetSunsetHour(m_fSunsetTime))
			m_TimeManager = null; // Set it to null to indicate that we failed to initialize this decorator
	}
	
	//-------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_TimeManager)
			return false;

		float currentTime = m_TimeManager.GetTimeOfTheDay();
		
		return currentTime > m_fSunriseTime && currentTime < m_fSunsetTime;
	}
};