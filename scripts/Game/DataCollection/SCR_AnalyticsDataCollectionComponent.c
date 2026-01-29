[EntityEditorProps(category: "GameScripted/DataCollection/", description: "Component used for collecting analytics data.")]
class SCR_AnalyticsDataCollectionComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_AnalyticsDataCollectionComponent : SCR_BaseGameModeComponent
{
	[Attribute()]
	protected ref array<ref SCR_AnalyticsDataCollectionModule> m_aModules;

	[Attribute("900", params: "1 inf", desc: "Time in seconds between SessionMeasures events")]
	protected int m_iSessionMeasuresInterval;

	protected IEntity m_Owner;

	protected float m_fSessionMeasuresTimer;

	#ifdef ENABLE_DIAG
	protected int m_iCurrentModuleId;
	protected SCR_AnalyticsDataCollectionModule m_CurrentModule;
	protected ref array<string> m_aModuleNames;
	#endif

	//------------------------------------------------------------------------------------------------
	protected override void OnGameModeStart()
	{
		// Track stats only on server
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		bool isMaster = (rplComponent && rplComponent.IsMaster());
		if (!isMaster)
			return;

		if (!m_Owner)
			Print("AnalyticsDataCollectionComponent: OnGameModeStart: m_Owner is null. Can't add the EntityEvent.FRAME flag thus data collector might not work properly.", LogLevel.ERROR);
		else
			SetEventMask(m_Owner, EntityEvent.FRAME); //Activate the FRAME flag

		foreach (SCR_AnalyticsDataCollectionModule module : m_aModules)
		{
			module.InitModule();
		}

		if (GetGame().GetPlayerManager().GetPlayerCount() == 0)
			m_fSessionMeasuresTimer = float.INFINITY;
		else
			m_fSessionMeasuresTimer = m_iSessionMeasuresInterval;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnGameModeEnd(SCR_GameModeEndData data)
	{
		// Track stats only on server
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		bool isMaster = (rplComponent && rplComponent.IsMaster());
		if (!isMaster)
			return;
		
		foreach (SCR_AnalyticsDataCollectionModule module : m_aModules)
		{
			module.OnGameModeEnd();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnGameEnd()
	{
		foreach (SCR_AnalyticsDataCollectionModule module : m_aModules)
		{
			module.OnGameEnd();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerConnected(int playerId)
	{
		SCR_DataCollectorComponent dataCollector = GetGame().GetDataCollector();
		if (dataCollector)
			dataCollector.RemovePlayer(playerId);

		foreach (SCR_AnalyticsDataCollectionModule module : m_aModules)
		{
			module.OnPlayerConnected(playerId);
		}

		if (GetGame().GetPlayerManager().GetPlayerCount() == 1)
			m_fSessionMeasuresTimer = m_iSessionMeasuresInterval;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		foreach (SCR_AnalyticsDataCollectionModule module : m_aModules)
		{
			module.OnPlayerDisconnected(playerId, cause);
		}

		if (GetGame().GetPlayerManager().GetPlayerCount() == 0)
			m_fSessionMeasuresTimer = float.INFINITY;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		int playerId = requestComponent.GetPlayerId();
		if (playerId == 0)
		{
			Print("AnalyticsDataCollectionComponent: OnPlayerSpawnFinalize_S: playerId is 0.", LogLevel.ERROR);
			return;
		}

		foreach (SCR_AnalyticsDataCollectionModule module : m_aModules)
		{
			module.OnPlayerSpawned(requestComponent, playerId, data, entity);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		foreach (SCR_AnalyticsDataCollectionModule module : m_aModules)
		{
			module.OnControllableDestroyed(instigatorContextData);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] type
	//! \return found module or null if not found
	SCR_AnalyticsDataCollectionModule FindModule(typename type)
	{
		for (int i = m_aModules.Count() - 1; i >= 0; i--)
		{
			if (m_aModules[i].Type() == type)
				return m_aModules[i];
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return all modules
	array<ref SCR_AnalyticsDataCollectionModule> GetAllModules()
	{
		return m_aModules;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] id
	//! \return found module or null if not found
	SCR_AnalyticsDataCollectionModule GetModuleById(int id)
	{
		if (!m_aModules.IsIndexValid(id))
			return null;
		
		return m_aModules.Get(id);
	}

	//------------------------------------------------------------------------------------------------
	//! Disables all analytics data collection modules
	void DisableAllModules()
	{
		foreach (SCR_AnalyticsDataCollectionModule module : m_aModules)
		{
			module.Disable();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_fSessionMeasuresTimer -= timeSlice;

		foreach (SCR_AnalyticsDataCollectionModule module : m_aModules)
		{
			module.Update(timeSlice);

			if (m_fSessionMeasuresTimer <= 0)
				module.SessionMeasures();
		}

		if (m_fSessionMeasuresTimer <= 0)
			SessionMeasures();
	}

	//------------------------------------------------------------------------------------------------
	protected void SessionMeasures()
	{
		if (GetGame().GetPlayerManager().GetPlayerCount() == 0)
		{
			m_fSessionMeasuresTimer = m_iSessionMeasuresInterval;
			return;
		}

		SCR_DataCollectorComponent dataCollector = GetGame().GetDataCollector();
		if (!dataCollector)
			return;

		SCR_SessionData sessionData = dataCollector.GetSessionData();
		if (!sessionData)
			return;

		sessionData.SetSessionTimeAndPlayers();

		SCR_SessionDataEvent sessionEvent = sessionData.GetSessionDataEvent();
		if (!sessionEvent)
			return;

		GetGame().GetStatsApi().SessionMeasures(sessionEvent);

		m_fSessionMeasuresTimer = m_iSessionMeasuresInterval;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		m_Owner = owner;
		
		#ifdef ENABLE_DIAG
		RegisterToDiag();
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);

		#ifdef ENABLE_DIAG
		DisconnectFromDiagSystem(owner);
		#endif
	}

	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	protected void RegisterToDiag()
	{
		ConnectToDiagSystem(m_Owner);
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_ANALYTICS_DATA_COLLECTION, "Analytics Data Collection", "Statistics");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_ANALYTICS_DATA_COLLECTION_ENABLE_DIAG, "", "Enable debug menu", "Analytics Data Collection");
		m_aModuleNames = new array<string>();
		
		foreach (SCR_AnalyticsDataCollectionModule module : m_aModules)
		{
			m_aModuleNames.Insert(SCR_AnalyticsDataCollectionHelper.GetShortModuleName(module.ToString()));
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_ANALYTICS_DATA_COLLECTION_ENABLE_DIAG))
			return;

		DrawDiag();
	}

	//------------------------------------------------------------------------------------------------
	void DrawDiag()
	{
		if (m_aModules.IsEmpty())
		{
			DbgUI.Text("No Analytics Data Collection modules found.");
			return;
		}

		m_CurrentModule = GetModuleById(m_iCurrentModuleId);
		int selectedModuleId = m_iCurrentModuleId;

		DbgUI.Begin("SCR_AnalyticsDataCollectionComponent");
		{
			DbgUI.Combo("Module: ", selectedModuleId, m_aModuleNames);

			if (selectedModuleId != m_iCurrentModuleId)
				OnSetCurrentModule(selectedModuleId);

			if (DbgUI.Button("Enable Module"))
				m_CurrentModule.Enable();

			DbgUI.SameLine();

			if (DbgUI.Button("Disable Module"))
				m_CurrentModule.Disable();

			DbgUI.Spacer(16);

			m_CurrentModule.DrawContent();
		}
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnSetCurrentModule(int id)
	{
		int newModuleId = Math.ClampInt(id, 0, m_aModules.Count() - 1);
		if (m_iCurrentModuleId == newModuleId)
			return;

		m_iCurrentModuleId = newModuleId;
		m_CurrentModule = GetModuleById(m_iCurrentModuleId);
	}
	#endif
}
