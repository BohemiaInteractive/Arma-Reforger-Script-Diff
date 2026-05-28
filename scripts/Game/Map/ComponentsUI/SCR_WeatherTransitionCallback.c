class SCR_WeatherTransitionCallback : BaseWeatherTransitionCallbacks
{
	protected TimeAndWeatherManagerEntity m_InstanceToUpdate;

	//------------------------------------------------------------------------------------------------
	void SCR_WeatherTransitionCallback(notnull TimeAndWeatherManagerEntity instanceToUpdate)
	{
		m_InstanceToUpdate = instanceToUpdate;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnStateSetImmediately()
	{
		m_InstanceToUpdate.OnWeatherChanged_S(false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnNewTransitionSet(WeatherStateTransitionNode oldTransition, WeatherStateTransitionNode newTransition)
	{
		m_InstanceToUpdate.OnWeatherChanged_S(false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTransitionStarted(WeatherStateTransitionNode transition)
	{
		m_InstanceToUpdate.OnWeatherChanged_S(true);
	}
}
