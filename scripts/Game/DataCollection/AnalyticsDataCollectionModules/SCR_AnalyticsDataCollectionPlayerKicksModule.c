[BaseContainerProps()]
class SCR_AnalyticsDataCollectionPlayerKicksModule : SCR_AnalyticsDataCollectionModule
{
	protected ref map<int, int> m_mPlayerVoteKickProposals = new map<int, int>();
	protected ref map<int, int> m_mPlayerVoteKicks = new map<int, int>();

	//------------------------------------------------------------------------------------------------
	protected override void Enable()
	{
		super.Enable();

		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		if (votingManager)
			votingManager.GetOnVotingStart().Insert(OnVotingStart);
	}

	//------------------------------------------------------------------------------------------------
	protected override void Disable()
	{
		super.Disable();

		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		if (votingManager)
			votingManager.GetOnVotingStart().Remove(OnVotingStart);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnGameModeEnd()
	{
		// TODO: Handle data to send them to analytics db
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerConnected(int playerId)
	{
		if (m_mPlayerVoteKickProposals.Contains(playerId) && m_mPlayerVoteKicks.Contains(playerId))
			return;

		m_mPlayerVoteKickProposals.Insert(playerId, 0);
		m_mPlayerVoteKicks.Insert(playerId, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Player vote has started. In case of vote being player kick vote, add it to player data
	//! \param[in] type
	//! \param[in] playerId
	protected void OnVotingStart(EVotingType type, int playerId)
	{
		// Only vote to kick player is relevant
		if (type != EVotingType.KICK)
			return;

		int kickProposals = m_mPlayerVoteKickProposals.Get(playerId) + 1;
		m_mPlayerVoteKickProposals.Set(playerId, kickProposals);

		SCR_SessionDataEvent sessionEvent = GetSessionDataEvent();
		if (!sessionEvent)
			return;

		int allKickProposals;
		foreach (int currentPlayerId, int playerKickProposals : m_mPlayerVoteKickProposals)
		{
			allKickProposals += playerKickProposals; 
		}

		sessionEvent.num_players_kick_proposed = allKickProposals;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerDisconnected(int playerId, KickCauseCode cause,  IEntity controlledEntity = null)
	{
		if (!m_bIsEnabled)
			return;

		string groupId, reasonId;
		KickCauseGroup2 groupInt;
		int reasonInt;

		// Only player kicks due to voting are relevant
		GetGame().GetFullKickReason(cause, groupInt, reasonInt, groupId, reasonId);
		if (groupInt != KickCauseGroup2.PLAYER_MANAGER || reasonInt != PlayerManagerKickReason.KICK_VOTED)
			return;

		int voteKicks = m_mPlayerVoteKicks.Get(playerId) + 1;
		m_mPlayerVoteKicks.Set(playerId, voteKicks);

		SCR_SessionDataEvent sessionEvent = GetSessionDataEvent();
		if (!sessionEvent)
			return;

		int allVoteKicks;
		foreach (int currentPlayerId, int playerKicks : m_mPlayerVoteKicks)
		{
			allVoteKicks += playerKicks; 
		}

		sessionEvent.num_players_kick_voted = allVoteKicks;
	}
	
	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void DrawContent()
	{
		if (!m_bIsEnabled)
		{
			DbgUI.Text("Module disabled!");
			return;
		}

		string playerName;
		int voteKicks;
		foreach (int playerID, int voteKickProposals : m_mPlayerVoteKickProposals)
		{
			playerName = GetGame().GetPlayerManager().GetPlayerName(playerID);
			voteKicks = m_mPlayerVoteKicks.Get(playerID);
			DbgUI.Text(string.Format("Player %1 - %2 voted to be kicked %3 times and has been kicked %4 times", playerID, playerName, voteKickProposals, voteKicks));			
		}
	}
	#endif

	//------------------------------------------------------------------------------------------------
	protected void ~SCR_AnalyticsDataCollectionPlayerKicksModule()
	{
		Disable();
	}
}