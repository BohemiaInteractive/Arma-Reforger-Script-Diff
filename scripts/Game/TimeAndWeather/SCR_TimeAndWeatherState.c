[BaseContainerProps()]
class SCR_TimeAndWeatherState
{
	[Attribute(desc: "The name of the weather preference as it can be found in weatherStates.conf.")]
	protected string m_sWeatherPresetName;
	
	[Attribute("8", UIWidgets.Slider, desc: "Starting time of day (hour)", "0 23 1")]
	protected int m_iStartingHour;

	[Attribute("0", UIWidgets.Slider, "Starting time of day (minutes)", "0 59 1")]
	protected int m_iStartingMinutes;
	
	//------------------------------------------------------------------------------------------------
	int GetStartingHour()
	{
		return m_iStartingHour;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetStartingMinutes()
	{
		return m_iStartingMinutes;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetWeatherPresetName()
	{
		return m_sWeatherPresetName;
	}
}

