[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeTime : SCR_ScenarioFrameworkActionBase
{
	[Attribute("8", UIWidgets.Slider, "Time of day (hours)", "0 23 1")]
	int m_iHours;

	[Attribute("0", UIWidgets.Slider, "Time of day (minutes)", "0 59 1")]
	int m_iMinutes;
	
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

		manager.SetHoursMinutesSeconds(m_iHours, m_iMinutes, 0);
	}
}