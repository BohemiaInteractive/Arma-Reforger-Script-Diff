//------------------------------------------------------------------------------------------------
//! Holds scoring data of players.
class SCR_PlayerScoreInfoFiringRange
{
	int m_iID = -1;
	int m_iScore = 0;
	int m_iScoreMax = 0;
	
	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, 12);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, 12);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx) 
	{	
		return lhs.CompareSnapshots(rhs, 12);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_PlayerScoreInfoFiringRange prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return snapshot.Compare(prop.m_iID, 4) 
			&& snapshot.Compare(prop.m_iScore, 4)
			&& snapshot.Compare(prop.m_iScoreMax, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_PlayerScoreInfoFiringRange prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{		
		snapshot.SerializeBytes(prop.m_iID, 4);
		snapshot.SerializeBytes(prop.m_iScore, 4);
		snapshot.SerializeBytes(prop.m_iScoreMax, 4);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_PlayerScoreInfoFiringRange prop) 
	{
		snapshot.SerializeBytes(prop.m_iID, 4);
		snapshot.SerializeBytes(prop.m_iScore, 4);
		snapshot.SerializeBytes(prop.m_iScoreMax, 4);
		
		return true;
	}
	
	//################################################################################################
	//------------------------------------------------------------------------------------------------
	//! Dynamically returns the player name.
	string GetName()
	{
		return SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(m_iID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Resets the player score info.
	void Clear()
	{
		m_iScore = 0;
		m_iScoreMax = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update data in this player info with data from another one.
	//! \param playerScoreInfoFiringRange feeds the new info.
	void Insert(SCR_PlayerScoreInfoFiringRange playerScoreInfoFiringRange)
	{
		m_iScore = playerScoreInfoFiringRange.m_iScore;
	}

	//------------------------------------------------------------------------------------------------
	//! Dynamically calculates and returns the player score.
	int GetScore(SCR_FiringRangeScoringComponent scoringSystemFiringRange)
	{
		if (!scoringSystemFiringRange)
			return 0;

		return m_iScore; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Dynamically calculates and returns the player score.
	int GetScoreMax()
	{
		return m_iScoreMax; 
	}
	
	//------------------------------------------------------------------------------------------------
	int GetScore()
	{
		return m_iScore;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerID()
	{
		return m_iID;
	}
};
