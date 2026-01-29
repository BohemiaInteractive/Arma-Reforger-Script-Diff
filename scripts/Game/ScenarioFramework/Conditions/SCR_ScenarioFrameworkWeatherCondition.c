[BaseContainerProps()]
class SCR_ScenarioFrameworkWeatherCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(defvalue: "0", desc: "Minimal wind speed in meters per second", params: "0 100 0.001", precision: 3, category: "Wind")]
	float m_fMinWindSpeed;

	[Attribute(defvalue: "20", desc: "Maximal wind speed in meters per second", params: "0 100 0.001", precision: 3, category: "Wind")]
	float m_fMaxWindSpeed;

	[Attribute(defvalue: "0", desc: "Minimal rain intensity", params: "0 1 0.001", precision: 3, category: "Rain")]
	float m_fMinRainIntensity;

	[Attribute(defvalue: "1", desc: "Maximal rain intensity", params: "0 1 0.001", precision: 3, category: "Rain")]
	float m_fMaxRainIntensity;

	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		// Here you can debug specific Condition instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Condition Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkWeatherCondition.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		ChimeraWorld world = ChimeraWorld.CastFrom(entity.GetWorld());
		if (!world)
			return true;

		TimeAndWeatherManagerEntity manager = world.GetTimeAndWeatherManager();
		if (!manager)
			return true;

		float currentWindSpeed = manager.GetWindSpeed();
		if (currentWindSpeed < m_fMinWindSpeed || currentWindSpeed > m_fMaxWindSpeed)
			return false;

		float currentRainIntensity = manager.GetRainIntensity();
		if (currentRainIntensity < m_fMinRainIntensity || currentRainIntensity > m_fMaxRainIntensity)
			return false;

		return true;
	}
}