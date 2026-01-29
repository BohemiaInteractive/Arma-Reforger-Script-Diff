class SCR_StatsPanel_PacketLoss : SCR_StatsPanelBase
{
	//------------------------------------------------------------------------------------------------
	override protected float GetValue()
	{
		RplConnectionStats stats = Replication.GetConnectionStats(m_RplIdentity);
		
		float value = stats.GetPacketLoss();
		
		#ifdef DEBUG_STATS_PANELS
		if (value == 0)
			value = Math.RandomFloat(0.01,1) - 0.85;
			value = Math.Clamp(value, 0, 1);
		#endif			
		
		//PrintFormat("PacketLoss: %1", value);
		
		value = Math.Ceil(value * 100);

		return value;
	}
}