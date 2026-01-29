[BaseContainerProps()]
class SCR_ScenarioFrameworkDayTimeHourCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(defvalue: "0", desc: "Minimal day time hour", params: "0 24 1", category: "Time")]
	int m_iMinHour;

	[Attribute(defvalue: "24", desc: "Maximal day time hour", params: "0 24 1", category: "Time")]
	int m_iMaxHour;

	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		// Here you can debug specific Condition instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Condition Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkDayTimeHourCondition.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		ChimeraWorld world = ChimeraWorld.CastFrom(entity.GetWorld());
		if (!world)
			return true;

		TimeAndWeatherManagerEntity manager = world.GetTimeAndWeatherManager();
		if (!manager)
			return true;

		TimeContainer timeContainer = manager.GetTime();
		int currentHour = timeContainer.m_iHours;

		if (currentHour < m_iMinHour || currentHour > m_iMaxHour)
			return false;

		return true;
	}
}