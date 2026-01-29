[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeWeather : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "The name of the weather preference as it can be found in weatherStates.conf.")]
	string m_sWeatherPresetName;
	
	[Attribute("0", desc: "Weather it can change during gameplay")]
	bool m_bRandomWeatherChanges;
	
	[Attribute(defvalue: "0", desc: "Transition Duration", params: "0 inf 0.01")]
	float m_fTransitionDuration;
	
	[Attribute(defvalue: "1", desc: "Automatic Wind behaviour. Untick to further tune it with other attributes.")]
	bool m_bAutomaticWind;
	
	[Attribute(defvalue: "0", desc: "Wind Speed", params: "0 inf 0.01")]
	float m_fWindSpeed;
	
	[Attribute(defvalue: "0", desc: "Wind Direction", params: "0 inf 0.01")]
	float m_fWindDirection;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;
		
		TimeAndWeatherManagerEntity manager = world.GetTimeAndWeatherManager();
		if (!manager)
			return;

		manager.ForceWeatherTo(!m_bRandomWeatherChanges, m_sWeatherPresetName, m_fTransitionDuration);
		manager.DelayedSetWindOverride(!m_bAutomaticWind);
		
		if (m_bAutomaticWind)
			return;
		
		manager.DelayedOverrideWindSpeed(m_fWindSpeed);
		manager.DelayedOverrideWindDirection(m_fWindDirection);
	}
}