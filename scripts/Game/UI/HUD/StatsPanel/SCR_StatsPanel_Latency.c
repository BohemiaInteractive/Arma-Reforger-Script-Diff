class SCR_StatsPanel_Latency : SCR_StatsPanelBase
{
	//------------------------------------------------------------------------------------------------
	override protected float GetValue()
	{
		RplConnectionStats stats = Replication.GetConnectionStats(m_RplIdentity);

		float value = stats.GetRoundTripTimeInMs();
		
		#ifdef DEBUG_STATS_PANELS
		if (value == 0)
			value = Math.RandomFloat(0.01,0.1);
		
		value = value * 1000;
		#endif		
		
		//PrintFormat("TripTime: %1", value);
		
		return value;
	}
}