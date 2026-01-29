[EntityEditorProps(category: "GameScripted/GameMode", description: "temp loadout selection.", color: "0 0 255 255")]
class SCR_LoadoutManagerClass : GenericEntityClass
{
}

class SCR_LoadoutManager : GenericEntity
{
	protected static const int INVALID_LOADOUT_INDEX = -1;
	
	[Attribute("", UIWidgets.Object, category: "Loadout Manager")]
	protected ref array<ref SCR_BasePlayerLoadout> m_aPlayerLoadouts;
	
	//! List of all player loadout infos in no particular order. Maintained by the authority.
	[RplProp(onRplName: "OnPlayerLoadoutInfoChanged")]
	protected ref array<ref SCR_PlayerLoadoutInfo> m_aPlayerLoadoutInfo = {};

	//! Map of previous player <playerId : loadoutIndex>.
	protected ref map<int, int> m_PreviousPlayerLoadouts = new map<int, int>();

	//! List of indices of loadouts whose count has changed since last update.
	protected ref set<int> m_ChangedLoadouts = new set<int>();

	//! Local mapping of playerId to player loadout info.
	protected ref map<int, ref SCR_PlayerLoadoutInfo> m_MappedPlayerLoadoutInfo = new map<int, ref SCR_PlayerLoadoutInfo>();

	//! Mapping of loadout id:player count
	protected ref map<int, int> m_PlayerCount = new map<int, int>();

	protected ref ScriptInvoker<SCR_BasePlayerLoadout, int> m_OnMappedPlayerLoadoutInfoChanged;
	
	//------------------------------------------------------------------------------------------------
	//! Return assigned loadout of provided player by their id.
	//! \param[in] playerId Id of target player corresponding to PlayerController/PlayerManager player id.
	//! \return loadout instance if loadout is assigned, null otherwise.
	SCR_BasePlayerLoadout GetPlayerLoadout(int playerId)
	{
		SCR_PlayerLoadoutInfo info;
		if (m_MappedPlayerLoadoutInfo.Find(playerId, info))
		{
			int loadoutIndex = info.GetLoadoutIndex();
			return GetLoadoutByIndex(loadoutIndex);
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return loadout of provided player by their id.
	//! Static variant of SCR_LoadoutManager.GetPlayerLoadout that uses
	//! registered SCR_LoadoutManager from the ArmaReforgerScripted game instance.
	//! \param[in] playerId Id of target player corresponding to PlayerController/PlayerManager player id.
	//! \throws Exception if no SCR_LoadoutManager is present in the world.
	//! \return SCR_BasePlayerLoadout instance if loadout is assigned, null otherwise.
	static SCR_BasePlayerLoadout SGetPlayerLoadout(int playerId)
	{
		SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
		return loadoutManager.GetPlayerLoadout(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return loadout of of local player.
	//! \throws Exception if no SCR_LoadoutManager is present in the world.
	//! \return SCR_BasePlayerLoadout instance if loadout is assigned, null otherwise.
	SCR_BasePlayerLoadout GetLocalPlayerLoadout()
	{
		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		return GetPlayerLoadout(localPlayerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return loadout of local player.
	//! Static variant of SCR_LoadoutManager.GetLocalPlayerLoadout that uses
	//! registered SCR_LoadoutManager from the ArmaReforgerScripted game instance.
	//! \throws Exception if no SCR_LoadoutManager is present in the world.
	//! \return SCR_BasePlayerLoadout instance if loadout is assigned, null otherwise.
	static SCR_BasePlayerLoadout SGetLocalPlayerLoadout()
	{
		SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
		return loadoutManager.GetLocalPlayerLoadout();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns current count of players assigned to the provided loadout.
	//! \param[in] loadout
	//! \return Number of players or always 0 if no loadout is provided.
	int GetLoadoutPlayerCount(SCR_BasePlayerLoadout loadout)
	{
		if (!loadout)
			return 0;

		int playerCount;
		m_PlayerCount.Find(GetLoadoutIndex(loadout), playerCount);
		return playerCount;
	}

	//------------------------------------------------------------------------------------------------
	//! Return count of players assigned to the provided loadout.
	//! Static variant of SCR_LoadoutManager.GetLoadoutPlayerCount that uses
	//! registered FactionManager from the ArmaReforgerScripted game instance.
	//! \throws Exception if no FactionManager is present in the world.
	//! \param[in] loadout
	//! \return Player count for provided loadout or 0 if no loadout is provided.
	static int SGetLoadoutPlayerCount(SCR_BasePlayerLoadout loadout)
	{
		SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
		return loadoutManager.GetLoadoutPlayerCount(loadout);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update local player loadout mapping.
	protected void OnPlayerLoadoutInfoChanged()
	{
		// Store previous loadouts, so we can raise events
		m_ChangedLoadouts.Clear();
		m_PreviousPlayerLoadouts.Clear();
		foreach (int id, SCR_PlayerLoadoutInfo loadoutInfo : m_MappedPlayerLoadoutInfo)
		{
			int loadoutIndex = -1;
			if (loadoutInfo)
				loadoutIndex = loadoutInfo.GetLoadoutIndex();

			m_PreviousPlayerLoadouts.Set(id, loadoutIndex);
		}
		
		// Clear all records and rebuild them from scratch
		m_MappedPlayerLoadoutInfo.Clear();
		m_PlayerCount.Clear();
		for (int i = 0, cnt = m_aPlayerLoadoutInfo.Count(); i < cnt; i++)
		{
			int playerId = m_aPlayerLoadoutInfo[i].GetPlayerId();
			m_MappedPlayerLoadoutInfo.Insert(playerId, m_aPlayerLoadoutInfo[i]);
			
			// Resolve player-loadout count
			int playerLoadoutIndex = m_aPlayerLoadoutInfo[i].GetLoadoutIndex();
			if (playerLoadoutIndex != -1)
			{
				int previousCount;
				m_PlayerCount.Find(playerLoadoutIndex, previousCount);
				m_PlayerCount.Set(playerLoadoutIndex, previousCount + 1);
			}
			
			// If player count changed, append to temp list
			int previousLoadoutIndex;
			if (!m_PreviousPlayerLoadouts.Find(playerId, previousLoadoutIndex))
				previousLoadoutIndex = -1;	 // If player had no affiliated loadout previously, always assume none instead
			
			if (previousLoadoutIndex != playerLoadoutIndex)
			{
				m_ChangedLoadouts.Insert(previousLoadoutIndex);
				m_ChangedLoadouts.Insert(playerLoadoutIndex);
			}
		}
		
		// Raise callback for all loadouts of which the count has changed
		foreach (int loadoutIndex : m_ChangedLoadouts)
		{
			// Null loadout
			if (loadoutIndex == -1)
				continue;

			SCR_BasePlayerLoadout loadout = GetLoadoutByIndex(loadoutIndex);
			int count;
			m_PlayerCount.Find(loadoutIndex, count);
			OnPlayerLoadoutCountChanged(loadout, count);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Anyone:
	//! 	Event raised when provided faction's player count changes.
	//!
	//! Note: Order of changes is not fully deterministic, e.g. when changing faction from A to B,
	//! this method might be invoked in the order B, A instead.
	//! \param[in] loadout The loadout for which affiliated player count changed.
	//! \param[in] newCount The new number of players that are using this loadout.
	protected void OnPlayerLoadoutCountChanged(SCR_BasePlayerLoadout loadout, int newCount)
	{
		if (m_OnMappedPlayerLoadoutInfoChanged)
			m_OnMappedPlayerLoadoutInfoChanged.Invoke(loadout, newCount);
		
		#ifdef _ENABLE_RESPAWN_LOGS
		ResourceName res;
		if (loadout) res = loadout.GetLoadoutResource();
		PrintFormat("%1::OnPlayerLoadoutCountChanged(loadout: %2 [%3], count: %4)", Type().ToString(), loadout, res, newCount);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//! Authority:
	//! 	Event raised when provided player component has a loadout set.
	protected void OnPlayerLoadoutSet_S(SCR_PlayerLoadoutComponent playerComponent, SCR_BasePlayerLoadout loadout)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerLoadoutSet_S(playerId: %2, loadout: %3)", Type().ToString(), playerComponent.GetPlayerId(), loadout);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//! Authority:
	//! 	Update player loadout info for target player with their up-to-date state.
	//! \param[in] playerLoadoutComponent
	void UpdatePlayerLoadout_S(SCR_PlayerLoadoutComponent playerLoadoutComponent)
	{
		int targetPlayerId = playerLoadoutComponent.GetPlayerController().GetPlayerId();
		SCR_BasePlayerLoadout targetLoadout = playerLoadoutComponent.GetAssignedLoadout();
		int targetLoadoutIndex = GetLoadoutIndex(targetLoadout);

		// See if we have a record of player in the map
		SCR_PlayerLoadoutInfo foundInfo;
		if (m_MappedPlayerLoadoutInfo.Find(targetPlayerId, foundInfo))
		{
			// Adjust player counts
			SCR_BasePlayerLoadout previousLoadout = GetPlayerLoadout(targetPlayerId);
			if (previousLoadout)
			{
				// But only if previous entry was valid
				int previousIndex = GetLoadoutIndex(previousLoadout);
				if (previousIndex != -1)
				{
					int previousCount;
					m_PlayerCount.Find(previousIndex, previousCount); // Will not set value if not found
					int newCount = previousCount - 1;
					m_PlayerCount.Set(previousIndex, newCount); // Remove this player
					OnPlayerLoadoutCountChanged(previousLoadout, newCount);	
				}
			}
			
			// Update existing record
			foundInfo.SetLoadoutIndex(targetLoadoutIndex);
			
			// If new loadout is valid, add to player count
			if (targetLoadoutIndex != -1)
			{
				int previousCount;
				m_PlayerCount.Find(targetLoadoutIndex, previousCount); // Will not set value if not found
				int newCount = previousCount + 1;
				m_PlayerCount.Set(targetLoadoutIndex, newCount); // Remove this player
				OnPlayerLoadoutCountChanged(targetLoadout, newCount);
			}
			
			// Raise authority callback
			OnPlayerLoadoutSet_S(playerLoadoutComponent, targetLoadout);
			
			Replication.BumpMe();
			return;
		}

		// Insert new record
		SCR_PlayerLoadoutInfo newInfo = SCR_PlayerLoadoutInfo.Create(targetPlayerId);
		newInfo.SetLoadoutIndex(targetLoadoutIndex);
		m_aPlayerLoadoutInfo.Insert(newInfo);
		// And map it
		m_MappedPlayerLoadoutInfo.Set(targetPlayerId, newInfo);
		
		// And since this player was not assigned, increment the count of players for target faction
		if (targetLoadoutIndex != -1)
		{
			int previousCount;
			m_PlayerCount.Find(targetLoadoutIndex, previousCount);
			int newCount = previousCount + 1;
			m_PlayerCount.Set(targetLoadoutIndex, newCount);
			OnPlayerLoadoutCountChanged(targetLoadout, newCount);
		}
		
		// Raise authority callback
		OnPlayerLoadoutSet_S(playerLoadoutComponent, targetLoadout);
		
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Authority:
	//! Return whether provided loadout can be set for the requesting player.
	//! Game logic can be implemented here, e.g. maximum slots.
	//! \param[in] playerLoadoutComponent
	//! \param[in] loadout
	//! \return
	bool CanAssignLoadout_S(SCR_PlayerLoadoutComponent playerLoadoutComponent, SCR_BasePlayerLoadout loadout)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_BasePlayerLoadout> GetPlayerLoadouts()
	{ 
		return m_aPlayerLoadouts;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the number of loadouts provided by this manager or 0 if none.
	int GetLoadoutCount()
	{
		if (!m_aPlayerLoadouts)
			return 0;
		
		return m_aPlayerLoadouts.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns index of provided loadout or -1 if none.
	//! \param[in] loadout
	int GetLoadoutIndex(SCR_BasePlayerLoadout loadout)
	{
		foreach (int i, SCR_BasePlayerLoadout inst : m_aPlayerLoadouts)
		{
			if (inst == loadout)
				return i;
		}

		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns loadout at provided index or null if none.
	//! \param[in] index
	//! \return
	SCR_BasePlayerLoadout GetLoadoutByIndex(int index)
	{
		if (index < 0 || index >= m_aPlayerLoadouts.Count())
			return null;
		
		return m_aPlayerLoadouts[index];
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the first loadout with the provided name, or null if none were found.
	//! \param[in] name
	//! \return
	SCR_BasePlayerLoadout GetLoadoutByName(string name, FactionKey faction = string.Empty)
	{
		foreach (SCR_BasePlayerLoadout candidate : m_aPlayerLoadouts)
		{
			if (candidate.GetLoadoutName() == name)
			{
				auto factionLoadout = SCR_FactionPlayerLoadout.Cast(candidate);
				if (faction && factionLoadout && factionLoadout.GetFactionKey() != faction)
					continue;
				
				return candidate;
			}
		}

		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns random loadout that belongs to provided faction or null if none.
	//! \param[in] faction
	SCR_BasePlayerLoadout GetRandomFactionLoadout(Faction faction)
	{
		array<ref SCR_BasePlayerLoadout> loadouts = {};
		if (GetPlayerLoadoutsByFaction(faction, loadouts) == 0)
			return null;
		
		return loadouts.GetRandomElement();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \param[out] outLoadouts
	//! \return
	int GetPlayerLoadoutsByFaction(Faction faction, out notnull array<ref SCR_BasePlayerLoadout> outLoadouts)
	{
		outLoadouts.Clear();
		
		if (!m_aPlayerLoadouts)
			return 0;
		
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return 0;
		
		FactionManager factionManager = game.GetFactionManager();
		if (!factionManager)
			return 0;
		
		int outCount = 0;
			
		int count = m_aPlayerLoadouts.Count();
		for (int i = 0; i < count; i++)
		{
#ifdef DISABLE_ARSENAL_LOADOUTS
			if (SCR_PlayerArsenalLoadout.Cast(m_aPlayerLoadouts[i]))
				continue;
#endif			
			SCR_FactionPlayerLoadout factionLoadout = SCR_FactionPlayerLoadout.Cast(m_aPlayerLoadouts[i]);
			if (factionLoadout)
			{
				Faction ldFaction = factionManager.GetFactionByKey(factionLoadout.m_sAffiliatedFaction);
				if (faction == ldFaction)
				{
					outLoadouts.Insert(factionLoadout);
					outCount++;
				}
			}
		}
		
		return outCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	//! \param[in] faction
	//! \param[out] outLoadouts
	//! \return
	int GetPlayerLoadoutsByGroup(notnull SCR_AIGroup group, notnull Faction faction, out notnull array<ref SCR_BasePlayerLoadout> outLoadouts)
	{
		outLoadouts.Clear();

		if (!m_aPlayerLoadouts)
			return 0;

		ArmaReforgerScripted game = GetGame();
		if (!game)
			return 0;

		FactionManager factionManager = game.GetFactionManager();
		if (!factionManager)
			return 0;

		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if (!scrFaction)
			return 0;

		SCR_FactionPlayerLoadout factionLoadout;
		Faction ldFaction;
		int outCount;
		foreach (SCR_BasePlayerLoadout loadout : m_aPlayerLoadouts)
		{
#ifdef DISABLE_ARSENAL_LOADOUTS
			if (SCR_PlayerArsenalLoadout.Cast(loadout))
				continue;
#endif
			factionLoadout = SCR_FactionPlayerLoadout.Cast(loadout);
			if (!factionLoadout)
				continue;

			ldFaction = factionManager.GetFactionByKey(factionLoadout.m_sAffiliatedFaction);
			if (faction == ldFaction && group.IsLoadoutInGroup(factionLoadout))
			{
				outLoadouts.Insert(factionLoadout);
				outCount++;
			}
		}

		return outCount;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] outLoadouts
	//! \return
	int GetPlayerLoadouts(out notnull array<SCR_BasePlayerLoadout> outLoadouts)
	{
		int outCount = 0;
		outLoadouts.Clear();
		
		if (!m_aPlayerLoadouts)
			return 0;	
			
		int count = m_aPlayerLoadouts.Count();
		for (int i = 0; i < count; i++)
		{
			outLoadouts.Insert(m_aPlayerLoadouts[i]);
			outCount++;
		}
		
		return outCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \return
	int GetRandomLoadoutIndex(Faction faction)
	{
		if (!m_aPlayerLoadouts)
			return -1;	
			
		array<ref SCR_BasePlayerLoadout> loadouts = {};
		int count = GetPlayerLoadoutsByFaction(faction, loadouts);
		if (count <= 0)
			return -1;
		
		return Math.RandomInt(0, count);
	}

	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetRandomLoadoutIndex()
	{
		if (!m_aPlayerLoadouts)
			return -1;	
			
		int count = m_aPlayerLoadouts.Count();
		if (count <= 0)
			return -1;
		
		return Math.RandomInt(0, count);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_BasePlayerLoadout GetRandomLoadout()
	{
		int randomIndex = GetRandomLoadoutIndex();
		if (randomIndex < 0)
			return null;

		return m_aPlayerLoadouts[randomIndex];
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnMappedPlayerLoadoutInfoChanged()
	{
		if (!m_OnMappedPlayerLoadoutInfoChanged)
			m_OnMappedPlayerLoadoutInfoChanged = new ScriptInvoker();

		return m_OnMappedPlayerLoadoutInfoChanged;
	}
	
	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in] timeSlice
	protected override void EOnDiag(IEntity owner, float timeSlice)
	{
		super.EOnDiag(owner, timeSlice);
		
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_RESPAWN_PLAYER_LOADOUT_DIAG))
			return;
		
		DbgUI.Begin("SCR_LoadoutManager");
		{
			DbgUI.Text("* Loadout Player Count *");
			foreach (SCR_BasePlayerLoadout ld : m_aPlayerLoadouts)
			{
				DbgUI.Text(string.Format("%1: %2 player(s)", ld.GetLoadoutName(), GetLoadoutPlayerCount(ld)));
			}
		}
		DbgUI.End();
	}
	#endif

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_LoadoutManager(IEntitySource src, IEntity parent)
	{
		GetGame().RegisterLoadoutManager(this);
		
		#ifdef ENABLE_DIAG
		ConnectToDiagSystem();
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//! destructor
	void ~SCR_LoadoutManager()
	{
		#ifdef ENABLE_DIAG
		DisconnectFromDiagSystem();
		#endif
		GetGame().UnregisterLoadoutManager(this);
	}
}
