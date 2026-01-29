class SCR_SessionData
{
	//DataEvent to send stats to the database for tracking purposes
	protected ref SCR_SessionDataEvent m_DataEvent = new SCR_SessionDataEvent();

	protected int m_iSessionStartedTick;

	//------------------------------------------------------------------------------------------------
	protected float CalculateSessionDuration()
	{
		return System.GetTickCount(m_iSessionStartedTick) * 0.001;
	}

	//------------------------------------------------------------------------------------------------
	//! \return SCR_SessionDataEvent object
	SCR_SessionDataEvent GetSessionDataEvent()
	{
		return m_DataEvent;
	}

	//------------------------------------------------------------------------------------------------
	void SetSessionTimeAndPlayers()
	{
		m_DataEvent.amt_time_session = CalculateSessionDuration();

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (playerManager)
			m_DataEvent.amt_players = playerManager.GetPlayerCount();
	}

	//------------------------------------------------------------------------------------------------
	//! \return SCR_SessionDataEvent object
	//! Also sets session time and amount of current players to session data
	//! Should be used when getting the session data to send it further
	Managed GetDataEventStats()
	{
		SetSessionTimeAndPlayers();
		return GetSessionDataEvent();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_SessionData()
	{
		m_iSessionStartedTick = System.GetTickCount();

		ServerInfo serverInfo = GetGame().GetServerInfo();
		if (serverInfo)
			m_DataEvent.amt_max_player_slots = serverInfo.GetPlayerLimit();

		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
		{
			array<Faction> factionList = {};
			array<string> factionNames = {};

			factionManager.GetFactionsList(factionList);
			foreach (Faction faction : factionList)
			{
				factionNames.Insert(faction.GetFactionKey());
			}

			m_DataEvent.array_name_faction = SCR_AnalyticsDataCollectionHelper.GetShortDataArray(factionNames.ToString());
		}

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (playerManager)
			m_DataEvent.amt_players = playerManager.GetPlayerCount();
	}
}