//~ Event when player faction changed
void SCR_FactionManager_PlayerFactionChanged(int playerId, SCR_PlayerFactionAffiliationComponent playerFactionAffiliationComponent, Faction faction);
typedef func SCR_FactionManager_PlayerFactionChanged;

class SCR_FactionManagerClass : FactionManagerClass
{
}

class SCR_FactionManager : FactionManager
{
	[Attribute(defvalue: "1", desc: "Whether  or not the isPlayable state of a faction can be changed on run time")]
	protected bool m_bCanChangeFactionsPlayable;
	
	[Attribute("", UIWidgets.Object, "List of rank types")]
	protected ref array<ref SCR_RankID> m_aRanks;
	
	protected ref SCR_SortedArray<SCR_Faction> m_SortedFactions = new SCR_SortedArray<SCR_Faction>();
	protected ref map<string, ref array<string>> m_aAncestors = new map<string, ref array<string>>();

	//! List of all player faction infos in no particular order. Maintained by the authority.
	[RplProp(onRplName: "OnPlayerFactionInfoChanged")]
	protected ref array<ref SCR_PlayerFactionInfo> m_aPlayerFactionInfo = {};

	//! Map of previous players <playerId : factionIndex>.
	protected ref map<int, int> m_PreviousPlayerFactions = new map<int, int>();

	//! List of indices of factions whose count has changed since last update.
	protected ref set<int> m_ChangedFactions = new set<int>();

	//! Local mapping of playerId to player faction info.
	protected ref map<int, ref SCR_PlayerFactionInfo> m_MappedPlayerFactionInfo = new map<int, ref SCR_PlayerFactionInfo>();

	//! Mapping of faction id : player count
	protected ref map<int, int> m_PlayerCount = new map<int, int>();
	
	//~ Script invokers
	protected ref ScriptInvoker s_OnPlayerFactionCountChanged = new ScriptInvoker();
	//~ Server only \/
	protected ref ScriptInvokerBase<SCR_FactionManager_PlayerFactionChanged> m_OnPlayerFactionChanged_S;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnPlayerFactionCountChanged()
	{
		if (!s_OnPlayerFactionCountChanged)
			s_OnPlayerFactionCountChanged = new ScriptInvoker();

		return s_OnPlayerFactionCountChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! return Script invoker on player faction changed (Server only)
	ScriptInvokerBase<SCR_FactionManager_PlayerFactionChanged> GetOnPlayerFactionChanged_S()
	{
		if (!m_OnPlayerFactionChanged_S)
			m_OnPlayerFactionChanged_S = new ScriptInvokerBase<SCR_FactionManager_PlayerFactionChanged>();
		
		return m_OnPlayerFactionChanged_S;
	}

	//------------------------------------------------------------------------------------------------
	//! Update local player faction mapping.
	protected void OnPlayerFactionInfoChanged()
	{
		// Store previous factions, so we can raise events
		m_ChangedFactions.Clear();
		m_PreviousPlayerFactions.Clear();
		foreach (int id, SCR_PlayerFactionInfo factionInfo : m_MappedPlayerFactionInfo)
		{
			int factionIndex = -1;
			if (factionInfo)
				factionIndex = factionInfo.GetFactionIndex();

			m_PreviousPlayerFactions.Set(id, factionIndex);
		}

		// Clear all records and rebuild them from scratch
		m_MappedPlayerFactionInfo.Clear();
		m_PlayerCount.Clear();
		for (int i = 0, cnt = m_aPlayerFactionInfo.Count(); i < cnt; i++)
		{
			// Map player to info
			int playerId = m_aPlayerFactionInfo[i].GetPlayerId();
			m_MappedPlayerFactionInfo.Insert(playerId, m_aPlayerFactionInfo[i]);

			// Resolve player-faction count
			int playerFactionIndex = m_aPlayerFactionInfo[i].GetFactionIndex();
			if (playerFactionIndex != -1)
			{
				int previousCount;
				m_PlayerCount.Find(playerFactionIndex, previousCount);
				m_PlayerCount.Set(playerFactionIndex, previousCount+1);
			}

			// If player count changed, append to temp list
			int previousFactionIndex;
			if (!m_PreviousPlayerFactions.Find(playerId, previousFactionIndex))
				previousFactionIndex = -1; // If player had no affiliated faction previously, always assume none instead
			
			if (previousFactionIndex != playerFactionIndex)
			{
				m_ChangedFactions.Insert(previousFactionIndex);
				m_ChangedFactions.Insert(playerFactionIndex);
			}
		}

		// Raise callback for all factions of which the count has changed
		foreach (int factionIndex : m_ChangedFactions)
		{
			// Null faction
			if (factionIndex == -1)
				continue;

			Faction faction = GetFactionByIndex(factionIndex);
			int count;
			m_PlayerCount.Find(factionIndex, count);
			OnPlayerFactionCountChanged(faction, count);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Authority:
	//! 	Event raised when provided player component has a faction set.
	protected void OnPlayerFactionSet_S(SCR_PlayerFactionAffiliationComponent playerComponent, Faction faction)
	{
		if (m_OnPlayerFactionChanged_S)
			m_OnPlayerFactionChanged_S.Invoke(playerComponent.GetPlayerId(), playerComponent, faction);
		
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::OnPlayerFactionSet_S(playerId: %2, faction: %3)", Type().ToString(), playerComponent.GetPlayerId(), faction), LogLevel.NORMAL);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Anyone:
	//! 	Event raised when provided faction's player count changes.
	//!
	//! Note: Order of changes is not fully deterministic, e.g. when changing faction from A to B,
	//! 	this method might be invoked in the order B, A instead.
	//! \param[in] faction The faction for which affiliated player count changed.
	//! \param[in] newCount The new number of players that are part of this faction.
	protected void OnPlayerFactionCountChanged(Faction faction, int newCount)
	{
		if (s_OnPlayerFactionCountChanged)
			s_OnPlayerFactionCountChanged.Invoke(faction, newCount);
		
		#ifdef _ENABLE_RESPAWN_LOGS
		FactionKey key;
		if (faction)
			key = faction.GetFactionKey();

		Print(string.Format("%1::OnPlayerFactionCountChanged(faction: %2 [%3], count: %4)", Type().ToString(), faction, key, newCount), LogLevel.NORMAL);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Return affiliated faction of provided player by their id.
	//! \param[in] playerId Id of target player corresponding to PlayerController/PlayerManager player id.
	//! \throws Exception if no FactionManager is present in the world.
	//! \return Faction instance if faction is assigned, null otherwise.
	Faction GetPlayerFaction(int playerId)
	{
		SCR_PlayerFactionInfo info;
		if (m_MappedPlayerFactionInfo.Find(playerId, info))
		{
			int factionIndex = info.GetFactionIndex();
			return GetFactionByIndex(factionIndex);
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	static Faction SGetFaction(IEntity entity)
	{
		if (!entity)
			return null;
		
		const FactionAffiliationComponent factionManager = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
		if (!factionManager)
			return null;

		return factionManager.GetAffiliatedFaction();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return affiliated faction of provided player by their id.
	//! Static variant of SCR_FactionManager.GetLocalPlayerFaction that uses
	//! registered FactionManager from the ArmaReforgerScripted game instance.
	//! \param[in] playerId Id of target player corresponding to PlayerController/PlayerManager player id.
	//! \throws Exception if no FactionManager is present in the world.
	//! \return Faction instance if faction is assigned, null otherwise.
	static Faction SGetPlayerFaction(int playerId)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return null;

		return factionManager.GetPlayerFaction(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return affiliated faction of local player.
	//! \throws Exception if no FactionManager is present in the world.
	//! \return Faction instance if faction is assigned, null otherwise.
	Faction GetLocalPlayerFaction()
	{
		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		return GetPlayerFaction(localPlayerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return affiliated faction of local player.
	//! Static variant of SCR_FactionManager.GetLocalPlayerFaction that uses
	//! registered FactionManager from the ArmaReforgerScripted game instance.
	//! \throws Exception if no FactionManager is present in the world.
	//! \return Faction instance if faction is assigned, null otherwise.
	static Faction SGetLocalPlayerFaction()
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return null;

		return factionManager.GetLocalPlayerFaction();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current count of players assigned to the provided faction.
	//! \param[in] faction
	//! \return Number of players or always 0 if no faction is provided.
	int GetFactionPlayerCount(Faction faction)
	{
		if (!faction)
			return 0;

		int playerCount;
		m_PlayerCount.Find(GetFactionIndex(faction), playerCount);
		return playerCount;
	}

	//------------------------------------------------------------------------------------------------
	//! Return count of players assigned to the provided faction.
	//! Static variant of SCR_FactionManager.GetFactionPlayerCount that uses
	//! registered FactionManager from the ArmaReforgerScripted game instance.
	//! \throws Exception if no FactionManager is present in the world.
	//! \param[in] faction
	//! \return Player count for provided faction or 0 if no faction is provided.
	static int SGetFactionPlayerCount(Faction faction)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return 0;
		
		return factionManager.GetFactionPlayerCount(faction);
	}

	//------------------------------------------------------------------------------------------------
	//! Get factions sorted according to their own custom order.
	//! \param[out] outFactions Array to be filled with factions
	//! \return Number of factions
	int GetSortedFactionsList(out notnull SCR_SortedArray<SCR_Faction> outFactions)
	{
		return outFactions.CopyFrom(m_SortedFactions);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] rankID
	//! \return
	SCR_RankID GetRankByID(SCR_ECharacterRank rankID)
	{		
		if (!m_aRanks)
			return null;
		
		foreach (SCR_RankID rank: m_aRanks)
		{	
			if (rank && rank.GetRankID() == rankID)
				return rank;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_RankID> GetAllAvailableRanks()
	{
		array<ref SCR_RankID> outArray = {};
		foreach (SCR_RankID rank: m_aRanks)
		{
			outArray.Insert(rank);
		}
		
		return outArray;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] rankID
	//! \return
	bool IsRankRenegade(SCR_ECharacterRank rankID)
	{
		SCR_RankID rank = GetRankByID(rankID);
		
		if (rank)
			return rank.IsRankRenegade();
		else
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] rankID
	//! \return
	int GetRequiredRankXP(SCR_ECharacterRank rankID)
	{
		SCR_RankID rank = GetRankByID(rankID);
		if (!rank)
			return int.MAX;

		return rank.GetRequiredRankXP();
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ECharacterRank GetRenegadeRank()
	{	
		foreach (SCR_RankID rank: m_aRanks)
		{
			if (rank && rank.IsRankRenegade())
				return rank.GetRankID();
		}
		
		return SCR_ECharacterRank.INVALID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] XP
	//! \return
	SCR_ECharacterRank GetRankByXP(int XP)
	{
		if (!m_aRanks)
			return SCR_ECharacterRank.INVALID;
		
		int maxFoundXP = -100000;
		SCR_ECharacterRank rankFound = GetRenegadeRank();
		
		foreach (SCR_RankID rank: m_aRanks)
		{
			int reqXP = GetRequiredRankXP(rank.GetRankID());
			
			if (reqXP <= XP && reqXP > maxFoundXP)
			{
				maxFoundXP = reqXP;
				rankFound = rank.GetRankID();
			}
		}
		
		return rankFound;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] rank
	//! \return the next higher rank
	SCR_ECharacterRank GetRankNext(SCR_ECharacterRank rank)
	{
		int rankXP = GetRequiredRankXP(rank);
		int higherXP = 99999;
		SCR_ECharacterRank foundID = SCR_ECharacterRank.INVALID;
		
		foreach (SCR_RankID r: m_aRanks)
		{
			if (!r)
				continue;
			
			SCR_ECharacterRank ID = r.GetRankID();
			int thisXP = GetRequiredRankXP(ID);
			
			if (thisXP > rankXP && thisXP < higherXP)
			{
				higherXP = thisXP;
				foundID = ID;
			}
		}
		
		return foundID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] rank
	//! \return the next lower rank
	SCR_ECharacterRank GetRankPrev(SCR_ECharacterRank rank)
	{
		int rankXP = GetRequiredRankXP(rank);
		int lowerXP = -99999;
		SCR_ECharacterRank foundID = SCR_ECharacterRank.INVALID;
		
		foreach (SCR_RankID r: m_aRanks)
		{
			if (!r)
				continue;
			
			SCR_ECharacterRank ID = r.GetRankID();
			int thisXP = GetRequiredRankXP(ID);
			
			if (thisXP < rankXP && thisXP > lowerXP)
			{
				lowerXP = thisXP;
				foundID = ID;
			}
		}
		
		return foundID;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		array<string> ancestors;
		array<Faction> factions = {};
		string scriptedFactionKey;
		GetFactionsList(factions);
		SCR_Faction scriptedFaction;
		SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		map<string, int> missionFactionLimitMap = new map<string, int>();

		if (missionHeader)
			missionFactionLimitMap = missionHeader.GetFactionLimitMap();
		
		for (int i = factions.Count() - 1; i >= 0; i--)
		{
			scriptedFaction = SCR_Faction.Cast(factions[i]);
			if (scriptedFaction)
			{
				scriptedFactionKey = scriptedFaction.GetFactionKey();
				if (m_aAncestors.Find(scriptedFactionKey, ancestors))
					scriptedFaction.SetAncestors(ancestors);
				
				m_SortedFactions.Insert(scriptedFaction.GetOrder(), scriptedFaction);
				
				if (missionFactionLimitMap && missionFactionLimitMap.Contains(scriptedFactionKey))
					scriptedFaction.SetPlayerLimit(missionFactionLimitMap.Get(scriptedFactionKey));
				
				scriptedFaction.InitializeFaction();
			}
		}
		m_aAncestors = null; //--- Don't keep in the memory anymore, stored on factions now
		
		//--- Initialise components (OnPostInit doesn't work in them)
		SCR_BaseFactionManagerComponent component;
		array<Managed> components = {};
		for (int i = 0, count = owner.FindComponents(SCR_BaseFactionManagerComponent, components); i < count; i++)
		{
			component = SCR_BaseFactionManagerComponent.Cast(components[i]);
			component.OnFactionsInit(factions);
		}
		
		//--- Hook player disconnection event
		#ifdef WORKBENCH
		if (!GetGame().InPlayMode())
			return;
		#endif
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnected);
	}
	
	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	protected override void EOnDiag(IEntity owner, float timeSlice)
	{
		super.EOnDiag(owner, timeSlice);
		
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_RESPAWN_PLAYER_FACTION_DIAG))
			return;
		
		DbgUI.Begin("SCR_FactionManager");
		{
			DbgUI.Text("* Faction Player Count *");
			array<Faction> factions = {};
			GetFactionsList(factions);
			foreach (Faction faction : factions)
			{
				DbgUI.Text(string.Format("%1: %2 player(s)", faction.GetFactionKey(), GetFactionPlayerCount(faction)));
			}
		}
		DbgUI.End();
	}
	#endif

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_FactionManager(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		#ifdef ENABLE_DIAG
		ConnectToDiagSystem();
		#endif
		
		//--- Save faction ancestors
		BaseContainerList factionSources = src.GetObjectArray("Factions");
		BaseContainer factionSource;
		string factionKey, parentKey;
		for (int i, count = factionSources.Count(); i < count; i++)
		{
			factionSource = factionSources.Get(i);
			factionSource.Get("FactionKey", factionKey);
			
			array<string> ancestors = {};
			while (factionSource)
			{
				factionSource.Get("FactionKey", parentKey);
				if (!ancestors.Contains(parentKey))
					ancestors.Insert(parentKey);
				
				factionSource = factionSource.GetAncestor();
			}
			
			m_aAncestors.Insert(factionKey, ancestors);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_FactionManager()
	{
		#ifdef ENABLE_DIAG
		DisconnectFromDiagSystem();
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the faction is playable.
	//! Non-playable factions will not appear in the respawn menu.
	//! \return True when playable
	bool CanChangeFactionsPlayable()
	{
		return m_bCanChangeFactionsPlayable;
	}
	
	//======================================== FACTION RELATIONS ========================================\\	

	//------------------------------------------------------------------------------------------------
	//! Set given factions friendly towards eachother (Replicated if called by server)
	//! It is possible to set the same faction friendly towards itself to prevent faction infighting
	//! \param[in] factionA faction to set friendly to factionB
	//! \param[in] factionB faction to set friendly to factionA
	//! \param[in] playerChanged id of player who changed it to show notification. Leave -1 to not show notification
	//! \param[in] updateAIs If true it will update all AI in the world. This is rather expensive so call RequestUpdateAllTargetsFactions() separately to update all AI if setting multiple factions friendly (or hostile)
	void SetFactionsFriendly(notnull SCR_Faction factionA, notnull SCR_Faction factionB, int playerChanged = -1, bool updateAIs = true)
	{		
		//~ Already friendly
		if (factionA.DoCheckIfFactionFriendly(factionB))
			return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		bool isServer = (gameMode && gameMode.IsMaster()) || (!gameMode && Replication.IsServer());
		
		//~ Only call once if setting self as friendly
		if (factionA == factionB)
		{
			factionA.SetFactionFriendly(factionA);
			
			if (playerChanged > 0 && isServer)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_FACTION_SET_FRIENDLY_TO_SELF, playerChanged, GetFactionIndex(factionA));
		}
		else 
		{
			factionA.SetFactionFriendly(factionB);
			factionB.SetFactionFriendly(factionA);
			
			if (playerChanged > 0 && isServer)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_FACTION_SET_FRIENDLY_TO, playerChanged, GetFactionIndex(factionA), GetFactionIndex(factionB));
		}
		
		//~ Replicate the setting of friendly Factions
		if (isServer)
		{
			//~ Update AIs
			if (updateAIs)
				RequestUpdateAllTargetsFactions();
			
			SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();
			if (!delegateFactionManager)
				return;
			
			SCR_EditableFactionComponent factionDelegate = delegateFactionManager.GetFactionDelegate(factionA);
			if (!factionDelegate)
				return;
			
			//~ Replicate set friendly
			factionDelegate.SetFactionFriendly_S(GetFactionIndex(factionB));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set given factions hostile towards eachother (Replicated if called by server)
	//! It is possible to set the same faction hostile towards itself to allow faction infighting
	//! \param[in] factionA faction to set hostile to factionB
	//! \param[in] factionB faction to set hostile to factionA
	//! \param[in] playerChanged id of player who changed it to show notification. Leave -1 to not show notification
	//! \param[in] updateAIs If true it will update all AI in the world. This is rather expensive so call RequestUpdateAllTargetsFactions() separately to update all AI if setting multiple factions hostile (or friendly)
	void SetFactionsHostile(notnull SCR_Faction factionA, notnull SCR_Faction factionB, int playerChanged = -1, bool updateAIs = true)
	{
		//~ Already Hostile
		if (!factionA.DoCheckIfFactionFriendly(factionB))
			return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		bool isServer = (gameMode && gameMode.IsMaster()) || (!gameMode && Replication.IsServer());
		
		//~ Only call once if setting self as hostile
		if (factionA == factionB)
		{
			factionA.SetFactionHostile(factionA);
			
			if (playerChanged > 0 && isServer)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_FACTION_SET_HOSTILE_TO_SELF, playerChanged, GetFactionIndex(factionA));
		}
		else 
		{
			factionA.SetFactionHostile(factionB);
			factionB.SetFactionHostile(factionA);
			
			if (playerChanged > 0 && isServer)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_FACTION_SET_HOSTILE_TO, playerChanged, GetFactionIndex(factionA), GetFactionIndex(factionB));
		}
		
		//~ Replicate the setting of hostile Factions
		if (isServer)
		{
			//~ Update AIs
			if (updateAIs)
				RequestUpdateAllTargetsFactions();
			
			SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();
			if (!delegateFactionManager)
				return;
			
			SCR_EditableFactionComponent factionDelegate = delegateFactionManager.GetFactionDelegate(factionA);
			if (!factionDelegate)
				return;
			
			//~ Replicate set hostile
			factionDelegate.SetFactionHostile_S(GetFactionIndex(factionB));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update all AI perception.
	//! Used when faction friendly is changed to make sure AI in the area attack or stop attacking each other
	static void RequestUpdateAllTargetsFactions()
	{
		PerceptionManager pm = GetGame().GetPerceptionManager();
		if (pm)
			pm.RequestUpdateAllTargetsFactions();
	}

	//------------------------------------------------------------------------------------------------
	//! Authority:
	//! 	Update player faction info for target player with their up-to-date state.
	//! \param[in] playerFactionComponent
	void UpdatePlayerFaction_S(SCR_PlayerFactionAffiliationComponent playerFactionComponent)
	{
		int targetPlayerId = playerFactionComponent.GetPlayerController().GetPlayerId();
		Faction targetFaction = playerFactionComponent.GetAffiliatedFaction();
		int targetFactionIndex = GetFactionIndex(targetFaction);

		// See if we have a record of player in the map
		SCR_PlayerFactionInfo foundInfo;
		if (m_MappedPlayerFactionInfo.Find(targetPlayerId, foundInfo))
		{
			// Adjust player counts
			Faction previousFaction = GetPlayerFaction(targetPlayerId);
			if (previousFaction)
			{
				// But only if previous entry was valid
				int previousIndex = GetFactionIndex(previousFaction);
				if (previousIndex != -1)
				{
					int previousCount;
					m_PlayerCount.Find(previousIndex, previousCount); // Will not set value if not found
					int newCount = previousCount - 1;
					m_PlayerCount.Set(previousIndex, newCount); // Remove this player
					OnPlayerFactionCountChanged(previousFaction, newCount);
				}
			}

			// Update existing record
			foundInfo.SetFactionIndex(targetFactionIndex);
			
			// If new faction is valid, add to player count
			if (targetFactionIndex != -1)
			{
				int previousCount;
				m_PlayerCount.Find(targetFactionIndex, previousCount); // Will not set value if not found
				int newCount = previousCount + 1;
				m_PlayerCount.Set(targetFactionIndex, newCount); // Remove this player
				OnPlayerFactionCountChanged(targetFaction, newCount);
			}
			
			// Raise authority callback
			OnPlayerFactionSet_S(playerFactionComponent, targetFaction);
			
			Replication.BumpMe();
			return;
		}

		// Insert new record
		SCR_PlayerFactionInfo newInfo = SCR_PlayerFactionInfo.Create(targetPlayerId);
		newInfo.SetFactionIndex(targetFactionIndex);
		m_aPlayerFactionInfo.Insert(newInfo);
		// And map it
		m_MappedPlayerFactionInfo.Set(targetPlayerId, newInfo);
		// And since this player was not assigned, increment the count of players for target faction
		if (targetFactionIndex != -1)
		{
			int previousCount;
			m_PlayerCount.Find(targetFactionIndex, previousCount);
			int newCount = previousCount + 1;
			m_PlayerCount.Set(targetFactionIndex, newCount);
			OnPlayerFactionCountChanged(targetFaction, newCount);
		}
		
		// Raise authority callback
		OnPlayerFactionSet_S(playerFactionComponent, targetFaction);
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Authority:
	//! 	Handle disconnected player.
	//! \param[in] playerId
	//! \param[in] cause
	//! \param[in] timeout
	protected void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		
		// If faction is unused, no need to do anything
		SCR_PlayerFactionAffiliationComponent playerFactionAffiliation = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!playerFactionAffiliation)
			return;
		
		// No faction does not need update
		if (!playerFactionAffiliation.GetAffiliatedFaction())
			return;
		
		// Clear faction, this will result in proper update of things
		playerFactionAffiliation.RequestFaction(null);
	}
}
