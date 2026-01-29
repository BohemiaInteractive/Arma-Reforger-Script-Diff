//------------------------------------------------------------------------------------------------
//! Used for storing client data to be reapplied for reconnecting clients
class SCR_CampaignClientData
{
	protected string m_sID;
	protected int m_iXP;
	protected int m_iFaction = -1;
	protected float m_fLastSuicideTimestamp;
	protected float m_fRespawnPenalty;
	protected float m_fLastPenaltyDeductionTimestamp;
	protected WorldTimestamp m_fNextTravelAvailableAt;

	//------------------------------------------------------------------------------------------------
	void SetID(string ID)
	{
		m_sID = ID;
	}

	//------------------------------------------------------------------------------------------------
	string GetID()
	{
		return m_sID;
	}

	//------------------------------------------------------------------------------------------------
	void SetXP(int XP)
	{
		m_iXP = XP;
	}

	//------------------------------------------------------------------------------------------------
	int GetXP()
	{
		return m_iXP;
	}

	//------------------------------------------------------------------------------------------------
	void SetFactionIndex(int faction)
	{
		m_iFaction = faction;
	}

	//------------------------------------------------------------------------------------------------
	int GetFactionIndex()
	{
		return m_iFaction;
	}

	//------------------------------------------------------------------------------------------------
	void SetRespawnPenalty(float penalty)
	{
		m_fRespawnPenalty = Math.Max(0, penalty);
	}

	//------------------------------------------------------------------------------------------------
	float GetRespawnPenalty()
	{
		return m_fRespawnPenalty;
	}

	//------------------------------------------------------------------------------------------------
	void SetLastSuicideTimestamp(float timestamp)
	{
		m_fLastSuicideTimestamp = timestamp;
	}

	//------------------------------------------------------------------------------------------------
	float GetLastSuicideTimestamp()
	{
		return m_fLastSuicideTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	void SetLastPenaltyDeductionTimestamp(float timestamp)
	{
		m_fLastPenaltyDeductionTimestamp = timestamp;
	}

	//------------------------------------------------------------------------------------------------
	float GetLastPenaltyDeductionTimestamp()
	{
		return m_fLastPenaltyDeductionTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	void SetNextFastTravelTimestamp(WorldTimestamp timestamp)
	{
		m_fNextTravelAvailableAt = timestamp;
	}

	//------------------------------------------------------------------------------------------------
	WorldTimestamp GetNextFastTravelTimestamp()
	{
		return m_fNextTravelAvailableAt;
	}
}
