//------------------------------------------------------------------------------------------------
class SCR_GameModeCampaignClass : SCR_BaseGameModeClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_GameModeCampaign : SCR_BaseGameMode
{
	[Attribute("1", desc: "Terminate the scenario when Conflict is finished.", category: "Campaign")]
	protected bool m_bTerminateScenario;

	[Attribute("2", desc: "How many control points does a faction need to win", params: "1 inf 1", category: "Campaign")]
	protected int m_iControlPointsThreshold;

	[Attribute("300", desc: "How long does a faction need to hold the control points to win (seconds).", params: "0 inf 1", category: "Campaign")]
	protected float m_fVictoryTimer;

	[Attribute("1000", desc: "Supplies will be autoreplenished in bases until this limit is reached.", params: "0 inf 1", category: "Campaign")]
	protected int m_iSuppliesReplenishThreshold;

	[Attribute("200", desc: "Supplies will be autoreplenished in bases quickly until this limit is reached (HQs are not affected).", params: "0 inf 1", category: "Campaign")]
	protected int m_iQuickSuppliesReplenishThreshold;

	[Attribute("2", desc: "Supplies income will be multiplied by this number unless the quick replenish threshold has been reached.", params: "1 inf 0.05", category: "Campaign")]
	protected float m_fQuickSuppliesReplenishMultiplier;

	[Attribute("5", desc: "How often are supplies replenished in bases (seconds).", params: "1 inf 1", category: "Campaign")]
	protected int m_iSuppliesArrivalInterval;

	[Attribute("40", desc: "How many supplies are periodically replenished in HQs.", params: "0 inf 1", category: "Campaign")]
	protected int m_iRegularSuppliesIncome;

	[Attribute("4", desc: "How many supplies are periodically replenished in non-HQ bases.", params: "0 inf 1", category: "Campaign")]
	protected int m_iRegularSuppliesIncomeBase;

	[Attribute("4", desc: "How many extra supplies are periodically replenished in non-HQ bases per base connected via radio.", params: "0 inf 1", category: "Campaign")]
	protected int m_iRegularSuppliesIncomeExtra;

	[Attribute("600", desc: "The amount of supplies in HQs.", params: "0 inf 1", category: "Campaign")]
	protected int m_iHQStartingSupplies;

	[Attribute("100", desc: "When randomized, the least supplies a base can hold at the start.", params: "0 inf 1", category: "Campaign")]
	protected int m_iMinStartingSupplies;

	[Attribute("500", desc: "When randomized, the most supplies a base can hold at the start.", params: "0 inf 1", category: "Campaign")]
	protected int m_iMaxStartingSupplies;

	[Attribute("25", desc: "The step by which randomized supplies will be added in randomization. Min and Max limits should be divisible by this.", params: "1 inf 1", category: "Campaign")]
	protected int m_iStartingSuppliesInterval;

	[Attribute("0.5", desc: "Fraction of XP awarded to players unloading supplies which they have not loaded themselves.", params: "0 inf", category: "Campaign")]
	protected float m_fSupplyOffloadAssistanceReward;

	[Attribute("US", category: "Campaign")]
	protected FactionKey m_sBLUFORFactionKey;

	[Attribute("USSR", category: "Campaign")]
	protected FactionKey m_sOPFORFactionKey;

	[Attribute("FIA", category: "Campaign")]
	protected FactionKey m_sINDFORFactionKey;

	[Attribute("1", UIWidgets.CheckBox, "Randomized starting supplies in small bases", category: "Campaign")]
	protected bool m_bRandomizeSupplies;

	[Attribute("1200", UIWidgets.EditBox, "The furthest an independent supply depot can be from the nearest base to still be visible in the map.", params: "0 inf 1", category: "Campaign")]
	protected int m_iSupplyDepotIconThreshold;

	[Attribute("{B3E7B8DC2BAB8ACC}Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et", category: "Campaign")]
	protected ResourceName m_sSeekDestroyWaypointPrefab;

	[Attribute("1800", UIWidgets.EditBox, "If suicide is committed more than once in this time (seconds), a respawn penalty is issued.", params: "0 inf 1", category: "Campaign")]
	protected int m_iSuicidePenaltyCooldown;

	[Attribute("30", UIWidgets.EditBox, "Stacking extra deploy cooldown after suicide (seconds). Gets deducted over time.", params: "0 inf 1", category: "Campaign")]
	protected int m_iSuicideRespawnDelay;

	[Attribute("600", UIWidgets.EditBox, "How often is the post-suicide deploy cooldown penalty deducted (seconds).", params: "0 inf 1", category: "Campaign")]
	protected int m_iSuicideForgiveCooldown;

	[Attribute("0", UIWidgets.CheckBox, "Player can volunteer for Commander role", category: "Campaign")]
	protected bool m_bCommanderRoleEnabled;

	[Attribute("2000", desc: "HQ candidates with greater distance than this are considered for selection", params: "0 inf 1", category: "Campaign")]
	protected float m_fAcceptableDistanceBetweenFactionHQs;

	[Attribute("3000", desc: "HQ candidates with greater distance than this are preferred for selection", params: "0 inf 1", category: "Campaign")]
	protected float m_fPreferredDistanceBetweenFactionHQs;

	[Attribute("0", UIWidgets.CheckBox, "Players can establish bases. If disabled, the game starts with existing FOBs", category: "Campaign")]
	protected bool m_bEstablishingBasesEnabled;

	[Attribute("0", UIWidgets.CheckBox, "When enabled, FOBs automatically regenerate supplies", category: "Campaign")]
	protected bool m_bSuppliesAutoRegenerationEnabled;

	[Attribute("300", desc: "How often are supplies replenished in source bases (seconds).", params: "1 inf 1", category: "Campaign")]
	protected int m_iSuppliesArrivalIntervalSource;

	[Attribute("2500", desc: "How many supplies are periodically replenished in source bases.", params: "0 inf 1", category: "Campaign")]
	protected int m_iRegularSuppliesIncomeSource;

	[Attribute("1", desc: "Priority of HQ spawn point in spawn point list. Spawn points are listed from highest priority to lowest.", params: "0 inf 1", category: "Campaign")]
	protected int m_iHQSpawnPointPriority;

	[Attribute(SCR_ECharacterRank.PRIVATE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "All players will start the match at this rank.", enums: ParamEnumArray.FromEnum(SCR_ECharacterRank), category: "Campaign")]
	protected SCR_ECharacterRank m_eStartingRank;

	[Attribute("-1", desc: "Maximum amount of bases that can be established per faction. The amount of Established bases is also limited by the amount of callsigns. If value of -1 is used, the amount of established bases is only limited by callsign amount.", params: "-1 inf 1", category: "Campaign")]
	protected int m_iFactionEstablishBaseLimit;
	
	[Attribute("{11A29F36F362D318}Prefabs/MP/Campaign/SCR_HQRadioSoundEntity.et", desc:"Entity with a sound component that is used to load and play HQ radio sounds.", params: "et", category: "Campaign")]
	protected ResourceName m_sHQRadioSoundEntityPrefab;

	static const int MINIMUM_DELAY = 100;
	static const int UI_UPDATE_DELAY = 250;
	static const int MEDIUM_DELAY = 1000;
	static const int DEFAULT_DELAY = 2000;
	static const int BACKEND_DELAY = 25000;

	protected ref ScriptInvoker m_OnFactionAssignedLocalPlayer;
	protected ref ScriptInvoker m_OnStarted;
	protected ref ScriptInvoker m_OnMatchSituationChanged;
	protected ref ScriptInvoker m_OnCallsignOffsetChanged;
	protected ref ScriptInvokerEntity2 s_OnEntityRequested;

	protected ref array<SCR_PlayerRadioSpawnPointCampaign> m_aRadioSpawnPoints = {};
	protected ref array<ref SCR_CampaignClientData> m_aRegisteredClients = {};

	protected bool m_bIgnoreMinimumVehicleRank;
	protected bool m_bIsTutorial;
	protected bool m_bMatchOver;

	protected ref SCR_CampaignMilitaryBaseManager m_BaseManager = new SCR_CampaignMilitaryBaseManager(this);

	[RplProp(onRplName: "OnStarted")]
	protected bool m_bStarted;

	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected WorldTimestamp m_fVictoryTimestamp;

	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected WorldTimestamp m_fVictoryPauseTimestamp;

	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected int m_iWinningFactionId = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;

	[RplProp(onRplName: "OnCallsignOffsetChanged")]
	protected int m_iCallsignOffset = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;

	//------------------------------------------------------------------------------------------------
	bool GetEstablishingBasesEnabled()
	{
		return m_bEstablishingBasesEnabled;
	}

	//------------------------------------------------------------------------------------------------
	bool GetSuppliesAutoRegenerationEnabled()
	{
		return m_bSuppliesAutoRegenerationEnabled;
	}

	//------------------------------------------------------------------------------------------------
	int GetSuppliesArrivalIntervalSource()
	{
		return m_iSuppliesArrivalIntervalSource;
	}

	//------------------------------------------------------------------------------------------------
	int GetRegularSuppliesIncomeSource()
	{
		return m_iRegularSuppliesIncomeSource;
	}

	//------------------------------------------------------------------------------------------------
	float GetAcceptableDistanceBetweenFactionHQs()
	{
		return m_fAcceptableDistanceBetweenFactionHQs;
	}

	//------------------------------------------------------------------------------------------------
	float GetPreferredDistanceBetweenFactionHQs()
	{
		return m_fPreferredDistanceBetweenFactionHQs;
	}

	//------------------------------------------------------------------------------------------------
	int GetHQSpawnPointPriority()
	{
		return m_iHQSpawnPointPriority;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetStartingRank()
	{
		return m_eStartingRank;
	}

	//------------------------------------------------------------------------------------------------
	int GetFactionEstablishBaseLimit()
	{
		return m_iFactionEstablishBaseLimit;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetHQRadioSoundEntityPrefab()
	{
		return m_sHQRadioSoundEntityPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when the local player picks a faction
	ScriptInvoker GetOnFactionAssignedLocalPlayer()
	{
		if (!m_OnFactionAssignedLocalPlayer)
			m_OnFactionAssignedLocalPlayer = new ScriptInvoker();

		return m_OnFactionAssignedLocalPlayer;
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when Conflict gamemode has started
	ScriptInvoker GetOnStarted()
	{
		if (!m_OnStarted)
			m_OnStarted = new ScriptInvoker();

		return m_OnStarted;
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when an event happened which should be communicated to players (i.e. amount of control points held etc.)
	ScriptInvoker GetOnMatchSituationChanged()
	{
		if (!m_OnMatchSituationChanged)
			m_OnMatchSituationChanged = new ScriptInvoker();

		return m_OnMatchSituationChanged;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCallsignOffsetChanged()
	{
		if (!m_OnCallsignOffsetChanged)
			m_OnCallsignOffsetChanged = new ScriptInvoker();

		return m_OnCallsignOffsetChanged;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerEntity2 GetOnEntityRequested()
	{
		if (!s_OnEntityRequested)
			s_OnEntityRequested = new ScriptInvokerEntity2();

		return s_OnEntityRequested;
	}

	//------------------------------------------------------------------------------------------------
	int GetSuppliesReplenishThreshold()
	{
		return m_iSuppliesReplenishThreshold;
	}

	//------------------------------------------------------------------------------------------------
	int GetQuickSuppliesReplenishThreshold()
	{
		return m_iQuickSuppliesReplenishThreshold;
	}

	//------------------------------------------------------------------------------------------------
	float GetQuickSuppliesReplenishMultiplier()
	{
		return m_fQuickSuppliesReplenishMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	int GetSuppliesArrivalInterval()
	{
		return m_iSuppliesArrivalInterval;
	}

	//------------------------------------------------------------------------------------------------
	int GetRegularSuppliesIncome()
	{
		return m_iRegularSuppliesIncome;
	}

	//------------------------------------------------------------------------------------------------
	int GetRegularSuppliesIncomeBase()
	{
		return m_iRegularSuppliesIncomeBase;
	}

	//------------------------------------------------------------------------------------------------
	int GetRegularSuppliesIncomeExtra()
	{
		return m_iRegularSuppliesIncomeExtra;
	}

	//------------------------------------------------------------------------------------------------
	int GetSupplyDepotIconThreshold()
	{
		return m_iSupplyDepotIconThreshold;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseManager GetBaseManager()
	{
		return m_BaseManager;
	}

	//------------------------------------------------------------------------------------------------
	float GetVictoryTimer()
	{
		return m_fVictoryTimer;
	}

	//------------------------------------------------------------------------------------------------
	int GetControlPointTreshold()
	{
		return m_iControlPointsThreshold;
	}

	//------------------------------------------------------------------------------------------------
	int GetMinStartingSupplies()
	{
		return m_iMinStartingSupplies;
	}

	//------------------------------------------------------------------------------------------------
	int GetMaxStartingSupplies()
	{
		return m_iMaxStartingSupplies;
	}

	//------------------------------------------------------------------------------------------------
	int GetStartingSuppliesInterval()
	{
		return m_iStartingSuppliesInterval;
	}

	//------------------------------------------------------------------------------------------------
	float GetSupplyOffloadAssistanceReward()
	{
		return m_fSupplyOffloadAssistanceReward;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetSeekDestroyWaypointPrefab()
	{
		return m_sSeekDestroyWaypointPrefab;
	}

	//------------------------------------------------------------------------------------------------
	bool GetCommanderRoleEnabled()
	{
		return m_bCommanderRoleEnabled;
	}

	//------------------------------------------------------------------------------------------------
	int GetWinningFactionId()
	{
		return m_iWinningFactionId;
	}

	//------------------------------------------------------------------------------------------------
	WorldTimestamp GetVictoryTimestamp()
	{
		return m_fVictoryTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	WorldTimestamp GetVictoryPauseTimestamp()
	{
		return m_fVictoryPauseTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	bool IsTutorial()
	{
		return m_bIsTutorial;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsMatchOver()
	{
		return m_bMatchOver;
	}

	//------------------------------------------------------------------------------------------------
	bool HasStarted()
	{
		return m_bStarted;
	}

	//------------------------------------------------------------------------------------------------
	int GetCallsignOffset()
	{
		return m_iCallsignOffset;
	}

	//------------------------------------------------------------------------------------------------
	[Friend(SCR_GameModeCampaignSerializer)]
	protected void SetCallsignOffset(int offset)
	{
		if (m_iCallsignOffset == offset)
			return;

		m_iCallsignOffset = offset;

		Replication.BumpMe();
		OnCallsignOffsetChanged();
	}

	//------------------------------------------------------------------------------------------------
	void OnMatchSituationChanged()
	{
		if (m_OnMatchSituationChanged)
			m_OnMatchSituationChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		// Sync respawn radios & control points amount
		writer.WriteInt(m_BaseManager.GetTargetActiveBasesCount());

		int controlPointsHeldBLUFOR;
		int controlPointsHeldOPFOR;

		SCR_CampaignFaction factionBLUFOR = GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		SCR_CampaignFaction factionOPFOR = GetFactionByEnum(SCR_ECampaignFaction.OPFOR);

		if (factionBLUFOR)
			controlPointsHeldBLUFOR = factionBLUFOR.GetControlPointsHeld();

		if (factionOPFOR)
			controlPointsHeldOPFOR = factionOPFOR.GetControlPointsHeld();

		writer.WriteInt(controlPointsHeldBLUFOR);
		writer.WriteInt(controlPointsHeldOPFOR);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		// Sync respawn radios & control points amount
		int activeBasesTotal;

		reader.ReadInt(activeBasesTotal);

		m_BaseManager.SetTargetActiveBasesCount(activeBasesTotal);

		if (m_BaseManager.GetActiveBasesCount() == activeBasesTotal)
			m_BaseManager.OnAllBasesInitialized();

		int controlPointsHeldBLUFOR, controlPointsHeldOPFOR

		reader.ReadInt(controlPointsHeldBLUFOR);
		reader.ReadInt(controlPointsHeldOPFOR);

		GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).SetControlPointsHeld(controlPointsHeldBLUFOR);
		GetFactionByEnum(SCR_ECampaignFaction.OPFOR).SetControlPointsHeld(controlPointsHeldOPFOR);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_GameModeCampaign GetInstance()
	{
		return SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
	}

	//------------------------------------------------------------------------------------------------
	//! Get corresponding client data. Optionally: create new object if not found
	//! \param[in] playerId
	//! \param[in] create
	SCR_CampaignClientData GetClientData(int playerId, bool create = false)
	{
		if (playerId == 0)
			return null;

		const UUID playerIdentity = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		if (playerIdentity.IsNull())
			return null;

		SCR_CampaignClientData clientData;

		for (int i = 0, clientsCount = m_aRegisteredClients.Count(); i < clientsCount; i++)
		{
			if (m_aRegisteredClients[i].GetID() == playerIdentity)
			{
				clientData = m_aRegisteredClients[i];
				break;
			}
		}

		if (!clientData && create)
		{
			clientData = new SCR_CampaignClientData();
			clientData.SetID(playerIdentity);
			m_aRegisteredClients.Insert(clientData);
		}

		return clientData;
	}

	//------------------------------------------------------------------------------------------------
	void SetControlPointsHeld(SCR_CampaignFaction faction, int newCount)
	{
		if (faction.GetControlPointsHeld() == newCount)
			return;

		int index = GetGame().GetFactionManager().GetFactionIndex(faction);

		Rpc(RPC_DoSetControlPointsHeld, index, newCount);
		RPC_DoSetControlPointsHeld(index, newCount)
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoSetControlPointsHeld(int factionIndex, int count)
	{
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		faction.SetControlPointsHeld(count);
		OnMatchSituationChanged();
	}

	//------------------------------------------------------------------------------------------------
	void BroadcastMHQFeedback(SCR_EMobileAssemblyStatus msgID, int playerID, int factionID)
	{
		Rpc(RpcDo_BroadcastMHQFeedback, msgID, playerID, factionID);

		if (RplSession.Mode() != RplMode.Dedicated)
			RpcDo_BroadcastMHQFeedback(msgID, playerID, factionID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_BroadcastMHQFeedback(SCR_EMobileAssemblyStatus msgID, int playerID, int factionID)
	{
		SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.GetInstance();

		if (!comp)
			return;

		comp.MobileAssemblyFeedback(msgID, playerID, factionID)
	}

	//------------------------------------------------------------------------------------------------
	protected void Start()
	{
		if (IsProxy() || m_bStarted)
			return;

		if (!SCR_PersistenceSystem.IsLoadInProgress())
			InitalBaseSetup();

		m_BaseManager.UpdateBases(true);

		m_bStarted = true;
		Replication.BumpMe();
		m_BaseManager.OnAllBasesInitialized();
		OnStarted();
	}

	//------------------------------------------------------------------------------------------------
	protected void InitalBaseSetup()
	{
		// Compose custom bases array from header
		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();
		if (!baseManager)
			return;

		float hQSupplies = m_iHQStartingSupplies;
		bool whitelist = false;
		array<string> customBaseList = {};

		SCR_MissionHeaderCampaign header = SCR_MissionHeaderCampaign.Cast(GetGame().GetMissionHeader());
		if (header)
		{
			if (header.m_iStartingHQSupplies != -1)
				hQSupplies = header.m_iStartingHQSupplies;

			whitelist = header.m_bCustomBaseWhitelist;
			foreach (SCR_CampaignCustomBase customBase : header.m_aCampaignCustomBaseList)
			{
				customBaseList.Insert(customBase.GetBaseName());
			}
		}

		array<SCR_CampaignMilitaryBaseComponent> candidatesForHQ = {};
		array<SCR_CampaignMilitaryBaseComponent> controlPoints = {};
		array<SCR_MilitaryBaseComponent> bases = {};
		baseManager.GetBases(bases);

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);
			if (!campaignBase)
				continue;

			// Ignore the base if it's disabled in mission header
			if (header)
			{
				const string baseName = campaignBase.GetOwner().GetName();
				const int listIndex = customBaseList.Find(baseName);

				if (listIndex != -1)
				{
					if (!whitelist)
						continue;
				}
				else if (whitelist)
				{
					continue;
				}

				if (whitelist && listIndex != -1)
					campaignBase.ApplyHeaderSettings(header.m_aCampaignCustomBaseList[listIndex]);
			}

			if (!campaignBase.CanBeHQ())
				campaignBase.Initialize();

			if (campaignBase.CanBeHQ())
				candidatesForHQ.Insert(campaignBase);

			if (campaignBase.IsControlPoint())
				controlPoints.Insert(campaignBase);
		}

		if (candidatesForHQ.Count() < 2)
		{
			Print("Not enough suitable starting locations found in current setup. Check 'Can Be HQ' attributes in SCR_CampaignMilitaryBaseComponent!", LogLevel.ERROR);
			return;
		}

		// Process HQ selection
		array<SCR_CampaignMilitaryBaseComponent> selectedHQs = {};
		m_BaseManager.SelectHQs(candidatesForHQ, controlPoints, selectedHQs);
		m_BaseManager.SetHQFactions(selectedHQs);

		foreach (SCR_CampaignMilitaryBaseComponent hq : selectedHQs)
		{
			hq.SetAsHQ(true);
			hq.SetStartingSupplies(hQSupplies);
			hq.Initialize();
		}

		foreach (SCR_CampaignMilitaryBaseComponent candidate : candidatesForHQ)
		{
			if (candidate.IsHQ())
				continue;

			if (!candidate.DisableWhenUnusedAsHQ())
			{
				candidate.Initialize();
				continue;
			}

			RplComponent baseRplComponent = RplComponent.Cast(candidate.GetOwner().FindComponent(RplComponent));
			if (!baseRplComponent)
				continue;

			baseManager.UnregisterBase(candidate);
			baseRplComponent.DeleteRplEntity(candidate.GetOwner(), false);
		}

		m_BaseManager.UpdateBases();

		m_BaseManager.InitializeBases(selectedHQs, m_bRandomizeSupplies);

		if (m_iCallsignOffset == SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN)
		{
			const int basesCount = baseManager.GetBasesCount();
			const int basesHalf = Math.Ceil(basesCount * 0.5);
			const int offset = Math.RandomIntInclusive(1, basesHalf);
			SetCallsignOffset(offset);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnStarted()
	{
		SCR_RadioCoverageSystem coverageSystem = SCR_RadioCoverageSystem.GetInstance();
		if (coverageSystem)
			coverageSystem.TogglePeriodicUpdates(false);

		if (IsMaster())
		{
			// Start periodical checks for winning faction
			GetGame().GetCallqueue().CallLater(CheckForWinner, DEFAULT_DELAY, true);

			SCR_CharacterRankComponent.s_OnRankChanged.Insert(OnRankChanged);

			SCR_AmbientVehicleSystem vehiclesManager = SCR_AmbientVehicleSystem.GetInstance();
			if (vehiclesManager)
				vehiclesManager.GetOnVehicleSpawned().Insert(OnAmbientVehicleSpawned);

			array<SCR_SpawnPoint> spawnpoints = SCR_SpawnPoint.GetSpawnPoints();
			foreach (SCR_SpawnPoint spawnpoint : spawnpoints)
			{
				if (!spawnpoint)
					continue;

				DisableExtraSpawnpoint(spawnpoint);
				SetPrioritySpawnpoint(spawnpoint);
			}

			SCR_SpawnPoint.Event_SpawnPointAdded.Insert(DisableExtraSpawnpoint);
		}

		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(OnSpawnPointFactionAssigned);

		if (m_OnStarted)
			m_OnStarted.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);

		const SCR_PlayerController localController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!localController)
			return;

		if (playerId != localController.GetPlayerId())
			return; // Not about own local controller

		const SCR_PlayerFactionAffiliationComponent playerFactionAff = SCR_PlayerFactionAffiliationComponent.Cast(localController.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!playerFactionAff)
			return;

		// Listen for future changes
		playerFactionAff.GetOnPlayerFactionChangedInvoker().Insert(OnLocalPlayerFactionAssigned);

		const SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(playerFactionAff.GetAffiliatedFaction());
		if (faction) // Faction known already so trigger event
			OnLocalPlayerFactionAssigned(playerFactionAff, null, faction);
	}

	//------------------------------------------------------------------------------------------------
	void OnCallsignOffsetChanged()
	{
		if (m_OnCallsignOffsetChanged)
			m_OnCallsignOffsetChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void DisableExtraSpawnpoint(SCR_SpawnPoint spawnpoint)
	{
		if (spawnpoint.Type() != SCR_SpawnPoint)
			return;

		spawnpoint.SetFaction(null);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetPrioritySpawnpoint(SCR_SpawnPoint spawnpoint)
	{
		IEntity spawnpointParent = spawnpoint.GetRootParent();
		if (!spawnpointParent)
			return;

		SCR_CampaignMilitaryBaseComponent baseComponent = SCR_CampaignMilitaryBaseComponent.Cast(spawnpointParent.FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (!baseComponent || !baseComponent.IsHQ())
			return;

		spawnpoint.SetPriority(m_iHQSpawnPointPriority);
	}

	//------------------------------------------------------------------------------------------------
	//! Find out if any faction has won and it's time to end the match
	protected void CheckForWinner()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);
		ChimeraWorld world = GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();
		WorldTimestamp lowestVictoryTimestamp;
		WorldTimestamp blockPauseTimestamp;
		WorldTimestamp actualVictoryTimestamp;
		SCR_CampaignFaction winner;

		foreach (Faction faction : factions)
		{
			SCR_CampaignFaction fCast = SCR_CampaignFaction.Cast(faction);

			if (!fCast || !fCast.IsPlayable() || fCast == GetFactionByEnum(SCR_ECampaignFaction.INDFOR))
				continue;

			blockPauseTimestamp = fCast.GetPauseByBlockTimestamp();

			if (blockPauseTimestamp == 0)
				actualVictoryTimestamp = fCast.GetVictoryTimestamp();
			else
				actualVictoryTimestamp = curTime.PlusMilliseconds(
					fCast.GetVictoryTimestamp().DiffMilliseconds(fCast.GetPauseByBlockTimestamp())
				);

			if (actualVictoryTimestamp != 0)
			{
				if (!winner || actualVictoryTimestamp.Less(lowestVictoryTimestamp))
				{
					lowestVictoryTimestamp = actualVictoryTimestamp;
					winner = fCast;
				}
			}
		}

		if (winner)
		{
			if (lowestVictoryTimestamp.LessEqual(curTime))
			{
				GetGame().GetCallqueue().Remove(CheckForWinner);
				int winnerId = factionManager.GetFactionIndex(winner);
				RPC_DoEndMatch(winnerId);
				Rpc(RPC_DoEndMatch, winnerId);
				OnMatchSituationChanged();
			}
			else if (factionManager.GetFactionIndex(winner) != m_iWinningFactionId || winner.GetVictoryTimestamp() != m_fVictoryTimestamp || winner.GetPauseByBlockTimestamp() != m_fVictoryPauseTimestamp)
			{
				m_iWinningFactionId = factionManager.GetFactionIndex(winner);
				m_fVictoryTimestamp = winner.GetVictoryTimestamp();
				m_fVictoryPauseTimestamp = winner.GetPauseByBlockTimestamp();
				OnMatchSituationChanged();
				Replication.BumpMe();
			}
		}
		else if (m_iWinningFactionId != -1 || m_fVictoryTimestamp != 0)
		{
			m_iWinningFactionId = -1;
			m_fVictoryTimestamp = null;
			m_fVictoryPauseTimestamp = null;
			OnMatchSituationChanged();
			Replication.BumpMe();
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoEndMatch(int winningFactionId)
	{
		m_bMatchOver = true;

		if (IsProxy())
			return;

		FactionManager fManager = GetGame().GetFactionManager();
		array<Faction> factions = {};
		fManager.GetFactionsList(factions);
		Faction winningFaction = fManager.GetFactionByIndex(winningFactionId);

		if (winningFaction)
		{
			foreach (Faction faction : factions)
			{
				SCR_CampaignFaction f = SCR_CampaignFaction.Cast(faction);

				if (!f)
					continue;

				if (winningFaction == f)
					f.SendHQMessage(SCR_ERadioMsg.VICTORY, param: winningFactionId);
				else
					f.SendHQMessage(SCR_ERadioMsg.DEFEAT, param: winningFactionId);
			}
		}

		// Match is over, save it so if "Continue" is selected following this the game is not loaded at an end screen
		//GetGame().GetSaveManager().Save(ESaveType.AUTO);

		// For the server end the game, replicate to all clients.
		// listening components can react to this by e.g. showing end screen
		if (m_bTerminateScenario)
		{
			SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(EGameOverTypes.ENDREASON_SCORELIMIT, winnerFactionId: winningFactionId);
			EndGameMode(endData);
		}
	}
	//------------------------------------------------------------------------------------------------
	override void OnGameStart()
	{
		super.OnGameStart();

		if (GetGame().InPlayMode() && IsMaster())
			Start();
	}

	//------------------------------------------------------------------------------------------------
	FactionKey GetFactionKeyByEnum(SCR_ECampaignFaction faction)
	{
		switch (faction)
		{
			case SCR_ECampaignFaction.INDFOR:
			{
				return m_sINDFORFactionKey;
			};

			case SCR_ECampaignFaction.BLUFOR:
			{
				return m_sBLUFORFactionKey;
			};

			case SCR_ECampaignFaction.OPFOR:
			{
				return m_sOPFORFactionKey;
			};
		}

		return FactionKey.Empty;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetFactionByEnum(SCR_ECampaignFaction faction)
	{
		FactionManager fManager = GetGame().GetFactionManager();

		if (!fManager)
			return null;

		return SCR_CampaignFaction.Cast(fManager.GetFactionByKey(GetFactionKeyByEnum(faction)));
	}

	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	//! Getter for "Rank required" parameter for spawning vehicles.
	// TRUE, if rank requirement is disabled
	bool CanRequestVehicleWithoutRank()
	{
		return m_bIgnoreMinimumVehicleRank;
	}

	//------------------------------------------------------------------------------------------------
	void SetIsTutorial(bool isTutorial)
	{
		m_bIsTutorial = isTutorial;

		if (m_bIsTutorial)
			SCR_PopUpNotification.SetFilter(SCR_EPopupMsgFilter.TUTORIAL);
		else
			SCR_PopUpNotification.SetFilter(SCR_EPopupMsgFilter.ALL);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAmbientVehicleSpawned(SCR_AmbientVehicleSpawnPointComponent spawnpoint, Vehicle vehicle)
	{
		SCR_HelicopterDamageManagerComponent helicopterDamageManager = SCR_HelicopterDamageManagerComponent.Cast(vehicle.FindComponent(SCR_HelicopterDamageManagerComponent));

		// Ignore non-helicopter vehicles
		if (!helicopterDamageManager)
			return;

		array<HitZone> hitZones = {};
		helicopterDamageManager.GetAllHitZonesInHierarchy(hitZones);
		vector transform[3];
		transform[0] = vehicle.GetOrigin();
		transform[1] = vector.Forward;
		transform[2] = vector.Up;

		DamageManagerComponent damageManager;

		// Damage the engine and hull
		foreach (HitZone hitZone : hitZones)
		{
			if (!hitZone.IsInherited(SCR_EngineHitZone) && !hitZone.IsInherited(SCR_FlammableHitZone))
				continue;

			damageManager = DamageManagerComponent.Cast(hitZone.GetHitZoneContainer());
			if (!damageManager)
				continue;

			SCR_DamageContext damageContext = new SCR_DamageContext(EDamageType.TRUE, hitZone.GetMaxHealth() * 0.75, transform, damageManager.GetOwner(), hitZone, Instigator.CreateInstigator(null), null, -1, -1);
			helicopterDamageManager.HandleDamage(damageContext);
		}

		array<SCR_FuelManagerComponent> fuelManagers = {};
		array<BaseFuelNode> fuelNodes = {};
		SCR_FuelManagerComponent.GetAllFuelManagers(vehicle, fuelManagers);

		// Remove all fuel
		foreach (SCR_FuelManagerComponent fuelManager : fuelManagers)
		{
			fuelNodes.Clear();
			fuelManager.GetFuelNodesList(fuelNodes);

			foreach (BaseFuelNode fuelNode : fuelNodes)
			{
				fuelNode.SetFuel(0.0);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRankChanged(SCR_ECharacterRank oldRank, SCR_ECharacterRank newRank, notnull IEntity owner, bool silent)
	{
		if (silent)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(owner);
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetPlayerFaction(playerId));
		if (!faction)
			return;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;

		SCR_RankIDCampaign rank = SCR_RankIDCampaign.Cast(factionManager.GetRankByID(newRank));
		if (!rank)
			return;

		SCR_ERadioMsg radio;
		if (newRank < oldRank && !rank.IsRankRenegade())
			radio = SCR_ERadioMsg.DEMOTION;
		else
			radio = rank.GetRadioMsg();

		faction.SendHQMessage(radio, calledID: playerId, public: false, param: newRank)
	}

	//------------------------------------------------------------------------------------------------
	protected void SetStartingRank(notnull PlayerController playerController)
	{
		if (m_eStartingRank == SCR_ECharacterRank.INVALID)
			return;

		int xp;
		SCR_PlayerXPHandlerComponent handlerXP = SCR_PlayerXPHandlerComponent.Cast(playerController.FindComponent(SCR_PlayerXPHandlerComponent));

		if (handlerXP)
			xp = handlerXP.GetPlayerXP();

		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(FindComponent(SCR_XPHandlerComponent));

		if (!comp)
			return;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());

		if (!factionManager)
			return;

		int requiredXp = factionManager.GetRequiredRankXP(m_eStartingRank) - xp;

		if (requiredXp == 0)
			return;

		// Remove XP only for renegade ranks, also prevent going into negatives for Private rank
		if (requiredXp < 0 && !factionManager.IsRankRenegade(m_eStartingRank))
			return;

		comp.AwardXP(playerController.GetPlayerId(), SCR_EXPRewards.STARTING_RANK, 1, false, requiredXp);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditSuccess(int iPlayerID)
	{
		super.OnPlayerAuditSuccess(iPlayerID);

		// Apply data with a delay so client's game has time to initialize and register faction setting
		GetGame().GetCallqueue().CallLater(ApplyClientData, MINIMUM_DELAY, false, iPlayerID);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		UpdateClientData(playerId); // Must be done before super call to know faction before it is cleared.
		
		super.OnPlayerDisconnected(playerId, cause, timeout);
		m_BaseManager.OnPlayerDisconnected(playerId)
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerEntityChanged_S(int playerId, IEntity previousEntity, IEntity newEntity)
	{
		super.OnPlayerEntityChanged_S(playerId, previousEntity, newEntity);
		if (!newEntity)
			return; // Not spawned

		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc)
			return;

		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));
		if (campaignNetworkComponent)
			campaignNetworkComponent.OnPlayerAliveStateChanged(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnOnPoint_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, IEntity entity, SCR_SpawnPointSpawnData spawnPointData)
	{
		super.OnPlayerSpawnOnPoint_S(requestComponent, handlerComponent, entity, spawnPointData);

		// Award XP for the owner of the respawn radio (if applicable)
		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(FindComponent(SCR_XPHandlerComponent));

		if (compXP)
		{
			SCR_SpawnPoint spawnpoint = spawnPointData.GetSpawnPoint();

			if (spawnpoint)
			{
				SCR_PlayerSpawnPoint playerSpawnpoint = SCR_PlayerSpawnPoint.Cast(spawnpoint);
				SCR_DeployableSpawnPoint radioSpawnpoint = SCR_DeployableSpawnPoint.Cast(spawnpoint);

				if (playerSpawnpoint)
				{
					compXP.AwardXP(playerSpawnpoint.GetPlayerID(), SCR_EXPRewards.SPAWN_PROVIDER);
				}
				else if (radioSpawnpoint)
				{
					SCR_BaseDeployableSpawnPointComponent comp = radioSpawnpoint.GetDeployableSpawnPointComponent();

					if (comp)
					{
						int playerId = comp.GetItemOwnerID();

						// Don't award XP if player respawns on their own spawnpoint
						if (playerId != GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(entity))
							compXP.AwardXP(playerId, SCR_EXPRewards.SPAWN_PROVIDER);
					}
				}
			}
		}

		// Location popup for player
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(requestComponent.GetPlayerId());

		if (playerController)
		{
			SCR_CampaignMilitaryBaseComponent spawnPointParentBase;
			IEntity parent = spawnPointData.GetSpawnPoint();

			//~ Check if spawn target is a base
			while (parent)
			{
				spawnPointParentBase = SCR_CampaignMilitaryBaseComponent.Cast(parent.FindComponent(SCR_CampaignMilitaryBaseComponent));

				if (spawnPointParentBase)
					break;

				parent = parent.GetParent();
			}

			//~ If spawned on base
			if (spawnPointParentBase)
			{
				SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));

				if (campaignNetworkComponent)
					campaignNetworkComponent.RespawnLocationPopup(spawnPointParentBase.GetCallsign());
			}
		}

	}

	//------------------------------------------------------------------------------------------------
	void OnSpawnPointFactionAssigned(SCR_SpawnPoint spawnpoint)
	{
		IEntity owner = spawnpoint.GetParent();
		if (!owner)
			return;

		SCR_CampaignMilitaryBaseComponent parentBase = SCR_CampaignMilitaryBaseComponent.Cast(owner.FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (parentBase)
			parentBase.OnSpawnPointFactionAssigned(spawnpoint.GetFactionKey());
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilledEx(notnull SCR_InstigatorContextData instigatorContextData)
	{
		super.OnPlayerKilledEx(instigatorContextData);

		if (IsProxy())
			return;

		int playerId = instigatorContextData.GetVictimPlayerID();
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc)
			return;

		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));
		if (campaignNetworkComponent)
		{
			campaignNetworkComponent.OnPlayerAliveStateChanged(false);
			campaignNetworkComponent.ResetSavedSupplies();
		}

		UpdateRespawnPenalty(playerId);

		if (instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.SUICIDE))
			OnSuicide(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Handles forgiving of post-suicide deploy timer penalties
	protected void UpdateRespawnPenalty(int playerId)
	{
		if (m_iSuicideRespawnDelay == 0 || m_iSuicideForgiveCooldown == 0)
			return;

		SCR_CampaignClientData clientData = GetClientData(playerId, true);
		float respawnPenalty = clientData.GetRespawnPenalty();
		if (respawnPenalty == 0)
			return;

		float curTime = GetGame().GetWorld().GetWorldTime();
		float penaltyCooldownMs = (float)m_iSuicideForgiveCooldown * 1000;
		float timeSinceLastDeduction = curTime - clientData.GetLastPenaltyDeductionTimestamp();
		float penaltiesForgiven = Math.Floor(timeSinceLastDeduction / penaltyCooldownMs);

		if (penaltiesForgiven < 1)
			return;

		clientData.SetLastPenaltyDeductionTimestamp(curTime);
		float forgivenPenalty = (float)m_iSuicideRespawnDelay * penaltiesForgiven;
		clientData.SetRespawnPenalty(respawnPenalty - forgivenPenalty);

		array<Managed> timers = {};
		FindComponents(SCR_RespawnTimerComponent, timers);

		foreach (Managed timer : timers)
		{
			// Skip this specific type as it's handled separately for radio spawns
			if (timer.Type() == SCR_TimedSpawnPointComponent)
				continue;

			SCR_RespawnTimerComponent timerCast = SCR_RespawnTimerComponent.Cast(timer);

			if (!timerCast)
				continue;

			timerCast.SetRespawnTime(playerId, timerCast.GetRespawnTime() - respawnPenalty - forgivenPenalty);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSuicide(int playerId)
	{
		// Don't issue penalties in WB so it doesn't interfere with debugging etc.
#ifdef WORKBENCH
		return;
#endif
#ifdef NO_SUICIDE_PENALTY
		return;
#endif
		if (m_bIsTutorial)
			return;

		if (m_iSuicideRespawnDelay == 0)
			return;

		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);

		// Do not process suicide if player was unconscious upon death
		if (pc)
		{
			SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.Cast(pc.FindComponent(SCR_CampaignFeedbackComponent));

			if (comp && !comp.IsConscious())
				return;
		}

		SCR_CampaignClientData clientData = GetClientData(playerId, true);
		float respawnPenalty = clientData.GetRespawnPenalty();
		float lastSuicideTimestamp = clientData.GetLastSuicideTimestamp();
		float curTime = GetGame().GetWorld().GetWorldTime();
		clientData.SetLastSuicideTimestamp(curTime);

		if (lastSuicideTimestamp == 0)
			return;

		float timeSinceLastSuicide = curTime - lastSuicideTimestamp;
		float penaltyCooldownMs = (float)m_iSuicidePenaltyCooldown * 1000;

		// Last suicide happened long enough time ago, don't issue a penalty
		if (timeSinceLastSuicide > penaltyCooldownMs)
			return;

		float addedPenalty = m_iSuicideRespawnDelay;

		clientData.SetLastPenaltyDeductionTimestamp(curTime);
		clientData.SetRespawnPenalty(respawnPenalty + addedPenalty);

		array<Managed> timers = {};
		FindComponents(SCR_RespawnTimerComponent, timers);

		foreach (Managed timer : timers)
		{
			// Skip this specific type as it's handled separately for radio spawns
			if (timer.Type() == SCR_TimedSpawnPointComponent)
				continue;

			SCR_RespawnTimerComponent timerCast = SCR_RespawnTimerComponent.Cast(timer);

			if (!timerCast)
				continue;

			timerCast.SetRespawnTime(playerId, timerCast.GetRespawnTime() + respawnPenalty + addedPenalty);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Disable spawning on contested bases and with custom loadout on bases with no armories
	/*!
		Authority:
			Override and implement logic for whether provided player can spawn.
			\param requestComponent The player request component instigating this spawn.
			\param handlerComponent The spawn handler component handling this spawn.
			\param data The request payload.
			\param[out] result Reason why respawn is disabled. Note that if returns true the reason will always be OK
			\return True when spawn is allowed, false otherwise.
	*/
	override bool CanPlayerSpawn_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, out SCR_ESpawnResult result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED)
	{
		if (!super.CanPlayerSpawn_S(requestComponent, handlerComponent, data, result))
			return false;

		SCR_SpawnPointSpawnData spawnpointSpawnData = SCR_SpawnPointSpawnData.Cast(data);

		if (!spawnpointSpawnData)
			return true;

		SCR_SpawnPoint spawnpoint = spawnpointSpawnData.GetSpawnPoint();
		SCR_CampaignSpawnPointGroup spawnpointCampaign = SCR_CampaignSpawnPointGroup.Cast(spawnpoint);
		SCR_CampaignMilitaryBaseComponent base;
		PlayerController pc = requestComponent.GetPlayerController();

		if (spawnpointCampaign)
		{
			IEntity spawnpointParent = spawnpointCampaign.GetParent();

			if (spawnpointParent)
			{
				base = SCR_CampaignMilitaryBaseComponent.Cast(spawnpointParent.FindComponent(SCR_CampaignMilitaryBaseComponent));

				if (base && pc && base.GetCapturingFaction() && base.GetCapturingFaction() != SCR_FactionManager.SGetPlayerFaction(pc.GetPlayerId()))
				{
					result = SCR_ESpawnResult.NOT_ALLOWED_BASE_UNDER_ATTACK;
					return false;
				}
			}
		}

		SCR_PlayerLoadoutComponent loadoutComp = SCR_PlayerLoadoutComponent.Cast(requestComponent.GetPlayerController().FindComponent(SCR_PlayerLoadoutComponent));

		if (!loadoutComp)
			return true;

		// Check if player has the required rank for the picked loadout
		if (pc)
		{
			SCR_PlayerXPHandlerComponent xpHandler = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));
			if (xpHandler)
			{
				SCR_ECharacterRank rankRequirement = SCR_BasePlayerLoadout.GetLoadoutRequiredRank(loadoutComp.GetLoadout(), requestComponent.GetPlayerId());
				if (xpHandler.GetPlayerRankByXP() < rankRequirement)
				{
					result = SCR_ESpawnResult.NOT_ALLOWED_RANK_TOO_LOW;
					return false;
				}
			}
		}

		SCR_PlayerArsenalLoadout loadout = SCR_PlayerArsenalLoadout.Cast(loadoutComp.GetLoadout());
		if (!loadout || !spawnpoint)
			return true;

		// Spawning on MHQ with custom loadouts is not allowed
		if (spawnpoint.FindComponent(SCR_CampaignMobileAssemblyStandaloneComponent))
		{
			result = SCR_ESpawnResult.NOT_ALLOWED_CUSTOM_LOADOUT;
			return false;
		}

		if (!base)
			return true;

		SCR_ServicePointDelegateComponent armory = base.GetServiceDelegateByType(SCR_EServicePointType.ARMORY);

		if (armory)
			return true;

		result = SCR_ESpawnResult.NOT_ALLOWED_NO_ARSENAL;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Award additional XP for enemies killed in friendly bases
	override void OnControllableDestroyedEx(notnull SCR_InstigatorContextData instigatorContextData)
	{
		super.OnControllableDestroyedEx(instigatorContextData);

		if (IsProxy())
			return;

		Instigator instigator = instigatorContextData.GetInstigator();

		// Ignore AI or NONE instigators
		if (instigator.GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER)
			return;

		//~ Only handle cases were the player kills a character (No Suicide or friendly fire) and the player is not a possessed AI
		if (!instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_ENEMY_PLAYER) || !instigatorContextData.HasAnyKillerCharacterControlType(SCR_ECharacterControlType.PLAYER))
			return;

		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(FindComponent(SCR_XPHandlerComponent));
		if (!compXP)
			return;

		//~ Victim is not a character (Safty check)
		SCR_ChimeraCharacter victimCharacter = SCR_ChimeraCharacter.Cast(instigatorContextData.GetVictimEntity());
		if (!victimCharacter)
			return;

		vector victimPos = victimCharacter.GetOrigin();

		//~ Get nearest base
		SCR_CampaignMilitaryBaseComponent nearestBase = m_BaseManager.FindClosestBase(victimPos);
		if (!nearestBase)
			return;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;

		int killerId = instigator.GetInstigatorPlayerID();

		//~ Get killer faction to check if the closest base has the same faction as the killer
		Faction factionKiller = factionManager.GetPlayerFaction(killerId);
		if (!factionKiller)
			return;

		//this awards additional XP to base defenders, so if the instigator is not in his own base, there should be no reward
		if (nearestBase.GetFaction() != factionKiller)
			return;

		//~ Not in defending range
		if (vector.DistanceXZ(victimPos, nearestBase.GetOwner().GetOrigin()) > nearestBase.GetRadius())
			return;

		//~ Award defending XP
		compXP.AwardXP(killerId, SCR_EXPRewards.CUSTOM_1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLocalPlayerFactionAssigned(SCR_PlayerFactionAffiliationComponent component, Faction previous, Faction current)
	{
		const SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(current);
		m_BaseManager.SetLocalPlayerFaction(faction);

		if (m_OnFactionAssignedLocalPlayer)
			m_OnFactionAssignedLocalPlayer.Invoke(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when an entity is spawned by Free Roam Building
	void OnEntityRequested(notnull IEntity spawnedEntity, IEntity user, SCR_Faction faction, SCR_MilitaryBaseLogicComponent service)
	{
		if (IsProxy())
			return;

		if (s_OnEntityRequested)
			s_OnEntityRequested.Invoke(user, spawnedEntity);

		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(spawnedEntity);
		if (aiGroup)
		{
			SCR_CampaignMilitaryBaseManager militaryBaseManager = GetBaseManager();
			if (!militaryBaseManager)
				return;

			militaryBaseManager.OnDefenderGroupSpawned(service, aiGroup);
		}

		if (!spawnedEntity.IsInherited(Vehicle))
			return;

		// Vehicles requested in bases without fuel depot should have only a small amount of fuel
		array<SCR_FuelManagerComponent> fuelManagers = {};
		array<BaseFuelNode> fuelNodes = {};
		SCR_FuelManagerComponent.GetAllFuelManagers(spawnedEntity, fuelManagers);
		array<SCR_MilitaryBaseComponent> serviceBases = {};
		service.GetBases(serviceBases);
		bool fuelDepotNearby;

		foreach (SCR_MilitaryBaseComponent serviceBase : serviceBases)
		{
			if (serviceBase.GetServiceByType(SCR_EServicePointType.FUEL_DEPOT))
			{
				fuelDepotNearby = true;
				break;
			}
		}

		if (!fuelDepotNearby)
		{
			foreach (SCR_FuelManagerComponent fuelManager : fuelManagers)
			{
				fuelNodes.Clear();
				fuelManager.GetFuelNodesList(fuelNodes);

				foreach (BaseFuelNode fuelNode : fuelNodes)
				{
					fuelNode.SetFuel(fuelNode.GetMaxFuel() * 0.3);
				}
			}
		}

		PlayerManager playerManager = GetGame().GetPlayerManager();

		int playerId = playerManager.GetPlayerIdFromControlledEntity(user);

		if (playerId == 0)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));

		if (!playerController)
			return;

		SCR_CampaignNetworkComponent networkComp = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));

		if (!networkComp)
			return;

		ChimeraWorld world = spawnedEntity.GetWorld();
		networkComp.SetLastRequestTimestamp(world.GetServerTimestamp());

		BaseRadioComponent radioComponent = BaseRadioComponent.Cast(spawnedEntity.FindComponent(BaseRadioComponent));

		// Assign faction radio frequency
		if (radioComponent && faction)
		{
			BaseTransceiver transceiver = radioComponent.GetTransceiver(0);

			if (transceiver)
			{
				radioComponent.SetPower(false);
				transceiver.SetFrequency(faction.GetFactionRadioFrequency());
				radioComponent.SetEncryptionKey(faction.GetFactionRadioEncryptionKey());
			}
		}

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(spawnedEntity.FindComponent(SlotManagerComponent));

		if (!slotManager)
			return;

		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);

		IEntity truckBed;
		SCR_CampaignSuppliesComponent suppliesComponent;
		SCR_CampaignMobileAssemblyComponent mobileAssemblyComponent;
		EventHandlerManagerComponent eventHandlerManager;

		// Handle Conflict-specific vehicles
		foreach (EntitySlotInfo slot : slots)
		{
			if (!slot)
				continue;

			truckBed = slot.GetAttachedEntity();

			if (!truckBed)
				continue;

			mobileAssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(truckBed.FindComponent(SCR_CampaignMobileAssemblyComponent));

			// Mobile HQ
			if (mobileAssemblyComponent)
			{
				mobileAssemblyComponent.SetParentFactionID(GetGame().GetFactionManager().GetFactionIndex(faction));
				networkComp.SendVehicleSpawnHint(EHint.CONFLICT_MOBILE_HQ);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Save object with player's current data
	protected void UpdateClientData(int playerID)
	{
		const PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!pc)
			return;

		SCR_CampaignClientData clientData = GetClientData(playerID, true);
		
		SCR_PlayerFactionAffiliationComponent factionComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (factionComp)
			clientData.SetFactionIndex(GetGame().GetFactionManager().GetFactionIndex(factionComp.GetAffiliatedFaction()));

		SCR_PlayerXPHandlerComponent xpComp = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));
		if (xpComp)
			clientData.SetXP(xpComp.GetPlayerXP());

		SCR_FastTravelComponent fastTravel = SCR_FastTravelComponent.Cast(pc.FindComponent(SCR_FastTravelComponent));
		if (fastTravel)
			clientData.SetNextFastTravelTimestamp(fastTravel.GetNextTransportTimestamp());
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyClientData(int playerId)
	{
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!pc)
			return;

		SCR_CampaignClientData clientData = GetClientData(playerId);
		if (!clientData)
		{
			SetStartingRank(GetGame().GetPlayerManager().GetPlayerController(playerId));
			return; // Not a reconnecting client so kick off defaults.
		}

		bool allowFactionLoad = true;

#ifdef ENABLE_DIAG
		if (SCR_RespawnComponent.Diag_IsCLISpawnEnabled())
			allowFactionLoad = false;
#endif

		// Automatically apply the client's previous faction
		int forcedFaction = clientData.GetFactionIndex();
		if (allowFactionLoad && forcedFaction != -1)
		{
			SCR_PlayerFactionAffiliationComponent fac = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
			if (fac)
			{
				Faction faction = GetGame().GetFactionManager().GetFactionByIndex(forcedFaction);
				fac.RequestFaction(faction);
			}
		}

		int xp;
		SCR_PlayerXPHandlerComponent handlerXP = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));
		if (handlerXP)
			xp = handlerXP.GetPlayerXP();

		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(FindComponent(SCR_XPHandlerComponent));
		if (comp)
			comp.AwardXP(playerId, SCR_EXPRewards.UNDEFINED, 1, false, clientData.GetXP() - xp);

		SCR_FastTravelComponent fastTravel = SCR_FastTravelComponent.Cast(pc.FindComponent(SCR_FastTravelComponent));
		if (fastTravel)
			fastTravel.SetNextTransportTimestamp(clientData.GetNextFastTravelTimestamp());
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		super.EOnDiag(owner, timeSlice);

		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_UP))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_UP, 0);

			SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(playerController.FindComponent(SCR_PlayerXPHandlerComponent));
			if (comp)
				comp.CheatRank();
		}

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_DOWN))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_DOWN, 0);

			SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(playerController.FindComponent(SCR_PlayerXPHandlerComponent));
			if (comp)
				comp.CheatRank(true);
		}

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_BECOME_COMMANDER_DEBUG))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_BECOME_COMMANDER_DEBUG, 0);

			SCR_CampaignFactionCommanderPlayerComponent comp = SCR_CampaignFactionCommanderPlayerComponent.Cast(playerController.FindComponent(SCR_CampaignFactionCommanderPlayerComponent));
			if (comp)
				comp.CheatCommander();
		}

		if (!IsProxy())
		{
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_VICTORY_BLUFOR))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_VICTORY_BLUFOR, 0);
				FactionManager fManager = GetGame().GetFactionManager();

				if (fManager)
				{
					Rpc(RPC_DoEndMatch, fManager.GetFactionIndex(GetFactionByEnum(SCR_ECampaignFaction.BLUFOR)));
					RPC_DoEndMatch(fManager.GetFactionIndex(GetFactionByEnum(SCR_ECampaignFaction.BLUFOR)));
				}
			}

			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_VICTORY_OPFOR))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_VICTORY_OPFOR, 0);
				FactionManager fManager = GetGame().GetFactionManager();

				if (fManager)
				{
					Rpc(RPC_DoEndMatch, fManager.GetFactionIndex(GetFactionByEnum(SCR_ECampaignFaction.OPFOR)));
					RPC_DoEndMatch(fManager.GetFactionIndex(GetFactionByEnum(SCR_ECampaignFaction.OPFOR)));
				}
			}
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	void SCR_GameModeCampaign(IEntitySource src, IEntity parent)
	{
		// Attributes check
		if (m_sBLUFORFactionKey == FactionKey.Empty)
			Print("SCR_GameModeCampaign: Empty BLUFOR faction key!", LogLevel.ERROR);

		if (m_sOPFORFactionKey == FactionKey.Empty)
			Print("SCR_GameModeCampaign: Empty OPFOR faction key!", LogLevel.ERROR);

		if (m_sINDFORFactionKey == FactionKey.Empty)
			Print("SCR_GameModeCampaign: Empty INDFOR faction key!", LogLevel.ERROR);

		if (!GetGame().InPlayMode())
			return;

		// Cheat menu
#ifdef ENABLE_DIAG
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_MENU, "Conflict", "");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_INSTANT_BUILDING, "", "Instant composition spawning", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_UP, "", "Promotion", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_DOWN, "", "Demotion", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_BECOME_COMMANDER_DEBUG, "", "Become Commander", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_VICTORY_BLUFOR, "", "Match victory: BLUFOR", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_VICTORY_OPFOR, "", "Match victory: OPFOR", "Conflict");
		ConnectToDiagSystem();
#endif

		// Parameters override from header
		SCR_MissionHeaderCampaign header = SCR_MissionHeaderCampaign.Cast(GetGame().GetMissionHeader());
		if (header)
		{
			m_bIgnoreMinimumVehicleRank = header.m_bIgnoreMinimumVehicleRank;

			int suppliesMax = header.m_iMaximumBaseSupplies;
			int suppliesMin = header.m_iMinimumBaseSupplies;
			int controlPointsLimit = header.m_iControlPointsCap;
			float victoryTimeout = header.m_fVictoryTimeout;
			float supplyAssistanceReward = header.m_fSupplyOffloadAssistanceReward;

			if (suppliesMax != -1)
				m_iMaxStartingSupplies = suppliesMax;

			if (suppliesMin != -1)
				m_iMinStartingSupplies = suppliesMin;

			if (controlPointsLimit != -1)
				m_iControlPointsThreshold = controlPointsLimit;

			if (victoryTimeout != -1)
				m_fVictoryTimer = victoryTimeout;

			if (supplyAssistanceReward >= 0)
				m_fSupplyOffloadAssistanceReward = supplyAssistanceReward;

			m_bCommanderRoleEnabled = header.m_bCommanderRoleEnabled;
			m_bEstablishingBasesEnabled = header.m_bEstablishingBasesEnabled;
			m_bSuppliesAutoRegenerationEnabled = header.m_bSuppliesAutoRegenerationEnabled;
			m_eStartingRank = header.m_eStartingRank;
		}

		// Establishing Bases can only be enabled when Commander Role is enabled
		if (!m_bCommanderRoleEnabled)
			m_bEstablishingBasesEnabled = false;

		// prewarm acp for HQ radio sounds
		if (!m_sHQRadioSoundEntityPrefab.IsEmpty() && !System.IsConsoleApp())
			SCR_HQRadioSoundEntity.GetInstance();
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_GameModeCampaign()
	{
		DisconnectFromDiagSystem();

		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Remove(OnSpawnPointFactionAssigned);

		SCR_AmbientVehicleSystem manager = SCR_AmbientVehicleSystem.GetInstance();

		if (manager)
			manager.GetOnVehicleSpawned().Remove(OnAmbientVehicleSpawned);
	}
}
