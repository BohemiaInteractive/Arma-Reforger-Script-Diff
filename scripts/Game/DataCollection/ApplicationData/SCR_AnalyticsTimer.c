class SCR_AnalyticsTimer
{
	protected WorldTimestamp m_StartSeconds;

	//------------------------------------------------------------------------------------------------
	//! Start tracking time which clears previous start time
	void Start()
	{
		m_StartSeconds = GetGame().GetWorld().GetTimestamp();
	}

	//------------------------------------------------------------------------------------------------
	//! Get time spent from Start()
	//! \return int seconds
	int GetTimeSpent()
	{
		if (!m_StartSeconds)
		{
			Debug.Error("Can't save timer %1 because it was not yet started!");
			return -1;
		}

		return GetGame().GetWorld().GetTimestamp().DiffSeconds(m_StartSeconds);
	}
}