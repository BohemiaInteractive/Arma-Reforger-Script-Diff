[EntityEditorProps(category: "GameScripted/DataCollection/", description: "Component used to send data to specific clients.")]
class SCR_DataCollectorCommunicationComponentClass : ScriptComponentClass
{
}

class SCR_DataCollectorCommunicationComponent : ScriptComponent
{
	protected ref ScriptInvoker m_OnDataReceived;

	//------------------------------------------------------------------------------------------------
	notnull ScriptInvoker GetOnDataReceived()
	{
		if (!m_OnDataReceived)
			m_OnDataReceived = new ScriptInvoker();

		return m_OnDataReceived;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerData
	//! \param[in] factionKeys
	//! \param[in] values
	//! \param[in] valuesSize
	void SendData(notnull SCR_PlayerData playerData, notnull array<FactionKey> factionKeys, notnull array<float> values, int valuesSize)
	{
		Rpc(Rpc_DoSendData, playerData.GetStats(), playerData.GetPreviousStats(), factionKeys, values, valuesSize);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] stats
	//! \param[in] previousStats
	//! \param[in] factionKeys
	//! \param[in] values
	//! \param[in] valuesSize
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void Rpc_DoSendData(notnull array<float> stats, notnull array<float> previousStats, notnull array<FactionKey> factionKeys, notnull array<float> values, int valuesSize)
	{
		foreach (float stat : stats)
		{
			Print(stat, LogLevel.DEBUG);
		}

		foreach (float stat : previousStats)
		{
			Print(stat, LogLevel.DEBUG);
		}

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_DataCollectorComponent dataCollector = GetGame().GetDataCollector();
		if (!dataCollector)
			return;

		for (int i = 0, factionsCount = factionKeys.Count(); i < factionsCount; i++)
		{
			array<float> valuesForFaction = {};
			for (int j = 0; j < valuesSize; j++)
			{
				valuesForFaction.Insert(values[i * valuesSize + j]);
			}

			dataCollector.AddStatsToFaction(factionKeys[i], valuesForFaction);
		}

		SCR_PlayerData playerData = dataCollector.GetPlayerData(0, true, false); // Use 0 as ID for local player, as this is always on the client
		playerData.SetStats(stats);
		playerData.SetPreviousStats(previousStats);

		if (m_OnDataReceived)
			m_OnDataReceived.Invoke(playerData);
	}
}
