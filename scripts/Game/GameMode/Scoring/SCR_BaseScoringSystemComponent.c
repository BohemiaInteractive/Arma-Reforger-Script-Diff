class SCR_BaseScoringSystemComponentClass : SCR_BaseGameModeComponentClass
{
}

void OnPlayerEventDelegate(int playerId);
typedef func OnPlayerEventDelegate;
typedef ScriptInvokerBase<OnPlayerEventDelegate> OnPlayerEventInvoker;

void OnPlayerScoreChangedDelegate(int playerId, SCR_ScoreInfo scoreInfo);
typedef func OnPlayerScoreChangedDelegate;
typedef ScriptInvokerBase<OnPlayerScoreChangedDelegate> OnPlayerScoreChangedInvoker;

void OnFactionScoreChangedDelegate(Faction faction, SCR_ScoreInfo scoreInfo);
typedef func OnFactionScoreChangedDelegate;
typedef ScriptInvokerBase<OnFactionScoreChangedDelegate> OnFactionScoreChangedInvoker;

//! This component serves as a base and interface for game mode scoring system.
//!
//! It keeps track of individual player scores and manages replication of such values to
//! all clients when necessary.
//!
//! Scoring is implemented in authoritative way, ie. only the server can update score.
//!
//! This component only keeps track of scores, but does not react or change the game flow in
//! any way. For specialised logic inherit this component and implement custom logic.
class SCR_BaseScoringSystemComponent : SCR_BaseGameModeComponent
{
	//! Map of scores per player.
	//! 	key: playerId
	protected ref map<int, ref SCR_ScoreInfo> m_mPlayerScores = new map<int, ref SCR_ScoreInfo>();

	//! Map of scores per faction.
	protected ref map<Faction, ref SCR_ScoreInfo> m_mFactionScores = new map<Faction, ref SCR_ScoreInfo>();
	
	//! This invoker is invoked when score of provided player changes.
	protected ref OnPlayerScoreChangedInvoker m_OnPlayerScoreChangedInvoker;
	
	//------------------------------------------------------------------------------------------------
	//! \return invoker that is invoked when score of provided player changes.
	OnPlayerScoreChangedInvoker GetOnPlayerScoreChanged()
	{
		if (!m_OnPlayerScoreChangedInvoker)
			m_OnPlayerScoreChangedInvoker = new OnPlayerScoreChangedInvoker();

		return m_OnPlayerScoreChangedInvoker;
	}
	
	//! This invoker is invoked when score of provided faction changes.
	protected ref OnFactionScoreChangedInvoker m_OnFactionScoreChangedInvoker;
	
	//------------------------------------------------------------------------------------------------
	//! \return invoker that is invoked when score of provided player changes.
	OnFactionScoreChangedInvoker GetOnFactionScoreChanged()
	{
		if (!m_OnFactionScoreChangedInvoker)
			m_OnFactionScoreChangedInvoker = new OnFactionScoreChangedInvoker();

		return m_OnFactionScoreChangedInvoker;
	}
	
	//! This invoker is invoked when a player is registered to the scoreboard.
	protected ref OnPlayerEventInvoker m_OnPlayerAdded;
	
	//------------------------------------------------------------------------------------------------
	//! \return invoker that is invoked when any score changes.
	OnPlayerEventInvoker GetOnPlayerAdded()
	{
		if (!m_OnPlayerAdded)
			m_OnPlayerAdded = new OnPlayerEventInvoker();

		return m_OnPlayerAdded;
	}
	
	//! This invoker is invoked when a player is registered to the scoreboard.
	protected ref OnPlayerEventInvoker m_OnPlayerRemoved;
	
	//------------------------------------------------------------------------------------------------
	//! \return invoker that is invoked when any score changes.
	OnPlayerEventInvoker GetOnPlayerRemoved()
	{
		if (!m_OnPlayerRemoved)
			m_OnPlayerRemoved = new OnPlayerEventInvoker();

		return m_OnPlayerRemoved;
	}

	//------------------------------------------------------------------------------------------------
	//! Serialise current state of scoring into buffer via provided writer.
	//! \param[in] writer
	//! \return
	override bool RplSave(ScriptBitWriter writer)
    {
		super.RplSave(writer);

		int players = m_mPlayerScores.Count();
        writer.WriteInt(players);

		foreach (int playerId, SCR_ScoreInfo scoreInfo : m_mPlayerScores)
		{
			writer.WriteInt(playerId);
			scoreInfo.RplSave(writer);
		}

		int factions = m_mFactionScores.Count();
        writer.WriteInt(factions);
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
		{
			Print("Score serialization fail, no FactionManager present in the world!", LogLevel.WARNING);
			return false;
		}

		foreach (Faction faction, SCR_ScoreInfo scoreInfo : m_mFactionScores)
		{
			int factionId = factionManager.GetFactionIndex(faction);
			writer.WriteInt(factionId);
			scoreInfo.RplSave(writer);
		}

		return true;
    }

	//------------------------------------------------------------------------------------------------
	//! Deserialise state of scoring into buffer via provided reader.
	//! \param[in] reader
	//! \return
    override bool RplLoad(ScriptBitReader reader)
    {
		super.RplLoad(reader);

		int players;
		if (!reader.ReadInt(players))
			return false;

		array<int> playerIds = {};
		array<ref SCR_ScoreInfo> playerScores = {};

		for (int i = 0; i < players; i++)
		{
			int playerId;
			SCR_ScoreInfo scoreInfo = new SCR_ScoreInfo();

			if (!reader.ReadInt(playerId))
				return false;

			if (!scoreInfo.RplLoad(reader))
				return false;

			playerIds.Insert(playerId);
			playerScores.Insert(scoreInfo);
		}

		int factions;
		if (!reader.ReadInt(factions))
			return false;

		array<int> factionIds = {};
		array<ref SCR_ScoreInfo> factionScores = {};

		SCR_ScoreInfo scoreInfo;
		for (int i = 0; i < factions; i++)
		{
			int factionId;
			scoreInfo = new SCR_ScoreInfo();

			if (!reader.ReadInt(factionId))
				return false;

			if (!scoreInfo.RplLoad(reader))
				return false;

			factionIds.Insert(factionId);
			factionScores.Insert(scoreInfo);
		}

		// Fill our instance with newly received data
		m_mPlayerScores.Clear();
		for (int i = 0; i < players; i++)
		{
			m_mPlayerScores.Insert(playerIds[i], playerScores[i]);
		}

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
		{
			Print("Score deserialization fail, no FactionManager present in the world, faction scoring will not work properly!", LogLevel.WARNING);
			return false;
		}

		array<Faction> _ = {};
		int facCnt = factionManager.GetFactionsList(_);
		m_mFactionScores.Clear();
		Faction faction;
		for (int i = 0; i < factions; i++)
		{
			int factionIndex = factionIds[i];
			Print(string.Format("idx=%1, facIdx=%2, facMan=%3, facCnt=%4", i, factionIndex, factionManager, facCnt), LogLevel.NORMAL);
			faction = factionManager.GetFactionByIndex(factionIndex);
			m_mFactionScores.Insert(faction, factionScores[i]);

		}

        return true;
    }

	//------------------------------------------------------------------------------------------------
	//! Internal helper method.
	//! Returns player's affiliated faction index or -1 if none.
	private int GetPlayerFactionIndex(int playerId)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
		{
			Faction playerFaction = factionManager.GetPlayerFaction(playerId);
			return factionManager.GetFactionIndex(playerFaction);
		}
		
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Internal helper method.
	//! \return faction from provided index.
	private Faction GetFactionByIndex(int factionIndex)
	{
		if (factionIndex < 0)
			return null;

		return GetGame().GetFactionManager().GetFactionByIndex(factionIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Internal helper method.
	//! \return index of provided faction.
	private int GetFactionIndex(Faction faction)
	{
		if (!faction)
			return -1;

		return GetGame().GetFactionManager().GetFactionIndex(faction);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RpcDo_AddKill(int playerId, int factionIdx, int count)
	{
		SCR_ScoreInfo playerScore = m_mPlayerScores[playerId];
		playerScore.m_iKills += count;
		OnPlayerScoreChanged(playerId, playerScore);

		Faction faction = GetFactionByIndex(factionIdx);
		if (faction)
		{
			SCR_ScoreInfo factionScore = m_mFactionScores[faction];
			factionScore.m_iKills += count;
			OnFactionScoreChanged(faction, factionScore);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Add count x kills to playerId's score info and their affiliated faction's.
	//! Server-only, propagated to all clients via BC reliable RPC.
	//! \param[in] playerId
	//! \param[in] count
	void AddKill(int playerId, int count = 1)
	{
		// Server only
		if (!m_pGameMode.IsMaster())
			return;

		int factionIdx = GetPlayerFactionIndex(playerId);
		RpcDo_AddKill(playerId, factionIdx, count);
		Rpc(RpcDo_AddKill, playerId, factionIdx, count);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RpcDo_AddDeath(int playerId, int factionIdx, int count)
	{
		SCR_ScoreInfo playerScore = m_mPlayerScores[playerId];
		playerScore.m_iDeaths += count;
		OnPlayerScoreChanged(playerId, playerScore);

		Faction faction = GetFactionByIndex(factionIdx);
		if (faction)
		{
			SCR_ScoreInfo factionScore = m_mFactionScores[faction];
			factionScore.m_iDeaths += count;
			OnFactionScoreChanged(faction, factionScore);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Add count x deaths to playerId's score info and their affiliated faction's.
	//! Server-only, propagated to all clients via BC reliable RPC.
	//! \param[in] playerId
	//! \param[in] count
	void AddDeath(int playerId, int count = 1)
	{
		// Server only
		if (!m_pGameMode.IsMaster())
			return;

		int factionIdx = GetPlayerFactionIndex(playerId);
		RpcDo_AddDeath(playerId, factionIdx, count);
		Rpc(RpcDo_AddDeath, playerId, factionIdx, count);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RpcDo_AddTeamKill(int playerId, int factionIdx, int count)
	{
		SCR_ScoreInfo playerScore = m_mPlayerScores[playerId];
		playerScore.m_iTeamKills += count;
		OnPlayerScoreChanged(playerId, playerScore);

		Faction faction = GetFactionByIndex(factionIdx);
		if (faction)
		{
			SCR_ScoreInfo factionScore = m_mFactionScores[faction];
			factionScore.m_iTeamKills += count;
			OnFactionScoreChanged(faction, factionScore);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Add count x teamKills to playerId's score info and their affiliated faction's.
	//! Server-only, propagated to all clients via BC reliable RPC.
	//! \param[in] playerId
	//! \param[in] count
	void AddTeamKill(int playerId, int count = 1)
	{
		// Server only
		if (!m_pGameMode.IsMaster())
			return;

		int factionIdx = GetPlayerFactionIndex(playerId);
		RpcDo_AddTeamKill(playerId, factionIdx, count);
		Rpc(RpcDo_AddTeamKill, playerId, factionIdx, count);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RpcDo_AddSuicide(int playerId, int factionIdx, int count)
	{
		SCR_ScoreInfo playerScore = m_mPlayerScores[playerId];
		playerScore.m_iSuicides += count;
		OnPlayerScoreChanged(playerId, playerScore);

		Faction faction = GetFactionByIndex(factionIdx);
		if (faction)
		{
			SCR_ScoreInfo factionScore = m_mFactionScores[faction];
			factionScore.m_iSuicides += count;
			OnFactionScoreChanged(faction, factionScore);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Add count x suicides to playerId's score info and their affiliated faction's.
	//! Server-only, propagated to all clients via BC reliable RPC.
	//! \param[in] playerId
	//! \param[in] count
	void AddSuicide(int playerId, int count = 1)
	{
		// Server only
		if (!m_pGameMode.IsMaster())
			return;

		int factionIdx = GetPlayerFactionIndex(playerId);
		RpcDo_AddSuicide(playerId, factionIdx, count);
		Rpc(RpcDo_AddSuicide, playerId, factionIdx, count);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RpcDo_AddObjective(int playerId, int factionIdx, int count)
	{
		SCR_ScoreInfo playerScore = m_mPlayerScores[playerId];
		playerScore.m_iObjectives += count;
		OnPlayerScoreChanged(playerId, playerScore);

		Faction faction = GetFactionByIndex(factionIdx);
		if (faction)
		{
			SCR_ScoreInfo factionScore = m_mFactionScores[faction];
			factionScore.m_iObjectives += count;
			OnFactionScoreChanged(faction, factionScore);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Add count x objectives to playerId's score info.
	//! Server-only, propagated to all clients via BC reliable RPC.
	//! \param[in] playerId
	//! \param[in] count
	//! \param[in] addToFaction If false, score is not added to faction automatically
	void AddObjective(int playerId, int count = 1, bool addToFaction = true)
	{
		// Server only
		if (!m_pGameMode.IsMaster())
			return;

		int factionIdx = -1;
		if (addToFaction)
			factionIdx = GetPlayerFactionIndex(playerId);

		RpcDo_AddObjective(playerId, factionIdx, count);
		Rpc(RpcDo_AddObjective, playerId, factionIdx, count);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	private void RpcDo_AddFactionObjective(int factionIdx, int count)
	{
		Faction faction = GetFactionByIndex(factionIdx);
		if (faction)
		{
			SCR_ScoreInfo factionScore = m_mFactionScores[faction];
			factionScore.m_iObjectives += count;
			OnFactionScoreChanged(faction, factionScore);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Add count x objectives to provided faction.
	//! Server-only, propagated to all clients via BC reliable RPC.
	//! \param[in] faction
	//! \param[in] count
	void AddFactionObjective(notnull Faction faction, int count = 1)
	{
		// Server only
		if (!m_pGameMode.IsMaster())
			return;

		int factionIdx = GetFactionIndex(faction);
		RpcDo_AddFactionObjective(factionIdx, count);
		Rpc(RpcDo_AddFactionObjective, factionIdx, count);
	}

	//------------------------------------------------------------------------------------------------
	//! Called on all machines when player score changes.
	//! \param[in] playerId
	//! \param[in] scoreInfo
	protected void OnPlayerScoreChanged(int playerId, SCR_ScoreInfo scoreInfo)
	{
		if (m_OnPlayerScoreChangedInvoker)
			m_OnPlayerScoreChangedInvoker.Invoke(playerId, scoreInfo);

		GameStatsApi statsApi = GetGame().GetStatsApi();
		if (statsApi)
			statsApi.PlayerScore(playerId, scoreInfo);
	}

	//------------------------------------------------------------------------------------------------
	//! Called on all machines when faction score changes.
	//! \param[in] faction
	//! \param[in] scoreInfo
	protected void OnFactionScoreChanged(Faction faction, SCR_ScoreInfo scoreInfo)
	{
		if (m_OnFactionScoreChangedInvoker)
			m_OnFactionScoreChangedInvoker.Invoke(faction, scoreInfo);
	}

	//------------------------------------------------------------------------------------------------
	//! Calculate score for provided score info.
	//! Different kinds of score multipliers and evaluation can be calculated by overriding this method.
	//! \param[in] info
	//! \return
	protected int CalculateScore(SCR_ScoreInfo info)
	{
		int val = (info.m_iKills + info.m_iObjectives) - info.m_iSuicides - info.m_iTeamKills;
		if (val < 0)
			return 0;

		return val;
	}

	//------------------------------------------------------------------------------------------------
	//! \return score for given player by their playerId or 0 if none.
	int GetPlayerScore(int playerId)
	{
		SCR_ScoreInfo info = GetPlayerScoreInfo(playerId);
		if (!info)
			return false;

		return CalculateScore(info);
	}

	//------------------------------------------------------------------------------------------------
	//! \return score info of provided player or null if none.
	SCR_ScoreInfo GetPlayerScoreInfo(int playerId)
	{
		if (!m_mPlayerScores.Contains(playerId))
			return null;

		return m_mPlayerScores[playerId];
	}

	//------------------------------------------------------------------------------------------------
	//! \return score of provided faction or 0 if none.
	int GetFactionScore(notnull Faction faction)
	{
		SCR_ScoreInfo info = GetFactionScoreInfo(faction);
		if (!info)
			return false;

		return CalculateScore(info);
	}

	//------------------------------------------------------------------------------------------------
	//! \return score info of provided faction or null if none.
	SCR_ScoreInfo GetFactionScoreInfo(notnull Faction faction)
	{
		if (!m_mFactionScores.Contains(faction))
			return null;

		return m_mFactionScores[faction];
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);

		// Register player info
		if (!m_mPlayerScores.Contains(playerId))
		{
			SCR_ScoreInfo scoreInfo = new SCR_ScoreInfo();
			m_mPlayerScores.Insert(playerId, scoreInfo);
			if (m_OnPlayerScoreChangedInvoker)
				m_OnPlayerScoreChangedInvoker.Invoke(playerId, scoreInfo);
		}
		
		if (m_OnPlayerAdded)
			m_OnPlayerAdded.Invoke(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);
		
		if (m_OnPlayerRemoved)
			m_OnPlayerRemoved.Invoke(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		super.EOnDiag(owner, timeSlice);

		#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SCORING_SYSTEM))
		{
			string title = Type().ToString() + " Diagnostics";
			DbgUI.Begin(title);
			{
				DbgUI.Text("--Faction Scoring--");
				const string factionFormat = "[%1] Score=%2 || Kills=%3 | Deaths=%4 | TeamKills=%5 | Suicides=%6 | Objectives=%7";
				foreach (Faction faction, SCR_ScoreInfo scoreInfo : m_mFactionScores)
				{
					int score = CalculateScore(scoreInfo);
					string factionText = string.Format(factionFormat, faction.GetFactionKey(), score, scoreInfo.m_iKills, scoreInfo.m_iDeaths, scoreInfo.m_iTeamKills, scoreInfo.m_iSuicides, scoreInfo.m_iObjectives);
					DbgUI.Text(factionText);
				}

				DbgUI.Text("--Player Scoring--");
				const string playerFormat = "pId=%1: \"%2\" Score=%3 || Kills=%4 | Deaths=%5 | TeamKills=%6 | Suicides=%7 | Objectives=%8";
				foreach (int playerId, SCR_ScoreInfo scoreInfo : m_mPlayerScores)
				{
					string name = GetGame().GetPlayerManager().GetPlayerName(playerId);
					int score = CalculateScore(scoreInfo);
					string playerText = string.Format(playerFormat, playerId, name, score, scoreInfo.m_iKills, scoreInfo.m_iDeaths, scoreInfo.m_iTeamKills, scoreInfo.m_iSuicides, scoreInfo.m_iObjectives);
					DbgUI.Text(playerText);
				}
			}
			DbgUI.End();

			int targetId = 1;
			DbgUI.InputInt("Player Id", targetId);
			
			int cnt = 1;
			DbgUI.InputInt("n (count): ", cnt);

			if (DbgUI.Button("Add n Kills"))
				AddKill(targetId, cnt);
			if (DbgUI.Button("Add n Deaths"))
				AddDeath(targetId, cnt);
			if (DbgUI.Button("Add n TeamKills"))
				AddTeamKill(targetId, cnt);
			if (DbgUI.Button("Add n Suicides"))
				AddSuicide(targetId, cnt);
			if (DbgUI.Button("Add n Objectives"))
				AddObjective(targetId, cnt);
			if (DbgUI.Button("Add n Faction Objectives (Player Faction Only)"))
			{
				int facIdx = GetPlayerFactionIndex(targetId);
				Faction f = GetFactionByIndex(facIdx);
				AddFactionObjective(f, cnt);
			}
		}
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return maximum allowed score or -1 if undefined.
	int GetScoreLimit() 
	{
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
		{
			array<Faction> factions = {};
			factionManager.GetFactionsList(factions);

			foreach (Faction faction : factions)
			{
				m_mFactionScores.Insert(faction, new SCR_ScoreInfo);
			}
		}

		#ifdef ENABLE_DIAG
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SCORING_SYSTEM, "", "Scoring System", "GameMode");
		#endif
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		SetEventMask(owner, EntityEvent.INIT);
		#ifdef ENABLE_DIAG
		ConnectToDiagSystem(owner);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		#ifdef ENABLE_DIAG
		DisconnectFromDiagSystem(owner);
		#endif
		
		super.OnDelete(owner);
	}
}
