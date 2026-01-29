[BaseContainerProps()]
class SCR_AnalyticsDataCollectionModule
{
	[Attribute("1", UIWidgets.Slider, desc: "Seconds between updates of the module")]
	float m_fUpdatePeriod;

	protected float m_fTimeSinceUpdate = 0;

	protected bool m_bIsEnabled = true;

	//------------------------------------------------------------------------------------------------
	void InitModule()
	{
		Enable();
	}

	//------------------------------------------------------------------------------------------------
	void Update(float timeTick);

	//------------------------------------------------------------------------------------------------
	protected void AddInvokers(IEntity player);

	//------------------------------------------------------------------------------------------------
	protected void RemoveInvokers(IEntity player);

	//------------------------------------------------------------------------------------------------
	void OnPlayerSpawned(SCR_SpawnRequestComponent requestComponent, int playerId, SCR_SpawnData data, IEntity controlledEntity)
	{
		AddInvokers(controlledEntity);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconnected(int playerId, KickCauseCode cause, IEntity controlledEntity = null)
	{
		if (!controlledEntity)
			controlledEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);

		if (controlledEntity)
			RemoveInvokers(controlledEntity);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerConnected(int playerId);

	//------------------------------------------------------------------------------------------------
	void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData);

	//------------------------------------------------------------------------------------------------
	void OnGameModeEnd();

	//------------------------------------------------------------------------------------------------
	void OnGameEnd();

	//------------------------------------------------------------------------------------------------
	void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		//If the previous entity exists, remove the invokers from it
		if (from)
			RemoveInvokers(from);

		//If there's no entity yet we can assume the player disconnected or it hasn't spawned.
		//In this case we do nothing and keep listening to the spawn invoker.
		//Otherwise, we add the invokers to the 'to' entity
		if (to)
			AddInvokers(to);
	}

	//------------------------------------------------------------------------------------------------
	//! Periodically called when server has players
	//! To be overriden in modules to add session data before it is sent
	void SessionMeasures();

	//------------------------------------------------------------------------------------------------
	protected SCR_SessionDataEvent GetSessionDataEvent()
	{
		SCR_DataCollectorComponent dataCollector = GetGame().GetDataCollector();
		if (!dataCollector)
			return null;

		SCR_SessionData sessionData = dataCollector.GetSessionData();
		if (!sessionData)
			return null;

		sessionData.SetSessionTimeAndPlayers();

		SCR_SessionDataEvent sessionEvent = sessionData.GetSessionDataEvent();
		if (!sessionEvent)
			return null;

		return sessionEvent;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		return m_bIsEnabled;
	}

	//------------------------------------------------------------------------------------------------
	void Enable()
	{
		m_bIsEnabled = true;
	}

	//------------------------------------------------------------------------------------------------
	void Disable()
	{
		m_bIsEnabled = false;
	}

	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	void DrawContent()
	{
		if (!m_bIsEnabled)
			DbgUI.Text("Module disabled!");
	}
	#endif
}
