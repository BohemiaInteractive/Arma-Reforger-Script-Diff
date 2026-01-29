[EntityEditorProps(category: "GameScripted/FiringRange", description: "Handles Score on Firing Range.", color: "0 0 255 255")]
class SCR_FiringRangeScoringComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_FiringRangeScoringComponent : SCR_BaseGameModeComponent
{
	SCR_FiringRangeManager s_Manager;
	
	[RplProp()]
	protected ref array<ref SCR_PlayerScoreInfoFiringRange> m_aAllPlayersInfo = {}; //!< Contains score info of all the players.
		
	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	void OnDisconnected(int playerID)
	{
		RemovePlayer(playerID);	
		
		// Remove player from assigned firing line
		s_Manager.RemoveAssignedPlayerFromFireline(playerID);	
		
		ClearScore(playerID);
		// If player is in Firing line area and has his line in scoreborad, remove him from it.
		if (s_Manager.IsPlayerInFiringRangeArea(playerID))
			s_Manager.RemovePlayerFromArea(playerID);	
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! What happens when a player is killed. 
	//! \param[in] playerId
	//! \param[in] playerEntity
	//! \param[in] killerEntity
	//! \param[in] killer
	void OnKill(notnull SCR_InstigatorContextData instigatorContextData)
	{
		int playerId = instigatorContextData.GetVictimPlayerID();
		
		// Remove player from assigned firing line
		s_Manager.RemoveAssignedPlayerFromFireline(playerId);	
		
		ClearScore(playerId);
		// If player is in Firing line area and has his line in scoreborad, remove him from it.
		if (s_Manager.IsPlayerInFiringRangeArea(playerId))
			s_Manager.RemovePlayerFromArea(playerId);	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add a player to the player array m_aAllPlayersInfo.
	//! \param[in] playerID, is unique player identifier.
	SCR_PlayerScoreInfoFiringRange AddPlayer(int playerID)
	{
		SCR_PlayerScoreInfoFiringRange playerInfo = new SCR_PlayerScoreInfoFiringRange();
		playerInfo.m_iID = playerID;
		m_aAllPlayersInfo.Insert(playerInfo);
		Replication.BumpMe();

		return playerInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove a player from the player array m_aAllPlayersInfo.
	//! \param[in] playerID, is unique player identifier.
	void RemovePlayer(int playerID)
	{
		m_aAllPlayersInfo.RemoveItem(GetPlayerScoreInfo(playerID));
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	void ClearScore(int playerID)
	{
		SCR_PlayerScoreInfoFiringRange playerScoreInfoFiringRange = GetPlayerScoreInfo(playerID);
		if (playerScoreInfoFiringRange)
			playerScoreInfoFiringRange.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerID
	//! \param[in] scorePoints
	void AddScore(int playerID, int scorePoints)
	{
		SCR_PlayerScoreInfoFiringRange playerScoreInfoFiringRange = GetPlayerScoreInfo(playerID);
		if (playerScoreInfoFiringRange)
			playerScoreInfoFiringRange.m_iScore = playerScoreInfoFiringRange.GetScore() + scorePoints;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the maximal score of given firing line
	//! \param[in] playerID
	//! \param[in] scorePointsMax
	void SetScoreMax(int playerID, int scorePointsMax)
	{
		SCR_PlayerScoreInfoFiringRange playerScoreInfoFiringRange = GetPlayerScoreInfo(playerID);
		if (playerScoreInfoFiringRange)
			playerScoreInfoFiringRange.m_iScoreMax = scorePointsMax;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get the integer equal to the player's kill count.
	//! \param[in] playerID, is unique player identifier.
	//! \return the player's kill count.
	int GetScore(int playerID)
	{
		SCR_PlayerScoreInfoFiringRange playerScoreInfoFiringRange = GetPlayerScoreInfo(playerID);
		if (playerScoreInfoFiringRange)
			return playerScoreInfoFiringRange.GetScore();
		else return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the number of players.
	int GetPlayersCount()
	{
		return m_aAllPlayersInfo.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Finds a SCR_PlayerScoreInfoFiringRange by playerID from m_aAllPlayersInfo array.
	//! \param[in] playerID, is unique player identifier.
	private SCR_PlayerScoreInfoFiringRange GetPlayerScoreInfo(int playerID)
	{
		for (int i = 0, count = m_aAllPlayersInfo.Count(); i < count; i++)
		{
			if (m_aAllPlayersInfo[i].m_iID == playerID)
				return m_aAllPlayersInfo[i];
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] output
	//! \return score info of all players.
	int GetAllPlayersScoreInfo(notnull out array<SCR_PlayerScoreInfoFiringRange> output)
	{
		output.Clear();
		int y = 0;
		
		for (int i = 0, count = m_aAllPlayersInfo.Count(); i < count; i++)
		{
			output.Insert(m_aAllPlayersInfo[i]);
			Replication.BumpMe();
			y++;
		}

		return y;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
		    return;
		
		gameMode.GetOnPlayerConnected().Insert(AddPlayer);
		gameMode.GetOnPlayerDisconnected().Insert(OnDisconnected);
		gameMode.GetOnPlayerKilled().Insert(OnKill);
		
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetPlayers(playerIds);
		
		foreach (int id : playerIds)
		{
			AddPlayer(id);
		}
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_FiringRangeScoringComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		s_Manager = SCR_FiringRangeManager.Cast(ent);
	}
}
