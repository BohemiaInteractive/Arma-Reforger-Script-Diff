class SCR_AnalyticsPlayerXPData
{
	protected SCR_ECharacterRank m_eRank = SCR_ECharacterRank.PRIVATE;
	protected ref map<SCR_EXPRewards, int> m_mXPRewards = new map<SCR_EXPRewards, int>();

	//------------------------------------------------------------------------------------------------
	//! Aggregates gained xp from all saved XP Rewards
	//! \return current gained XP
	int GetCurrentXP()
	{
		int currentXP, i;
		for (i; i < m_mXPRewards.Count(); i++)
			currentXP += m_mXPRewards.GetElement(i);

		return currentXP;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetRank()
	{
		return m_eRank;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] newRank
	void SetRank(SCR_ECharacterRank newRank)
	{
		if (m_eRank != newRank)
			m_eRank = newRank;
	}

	//------------------------------------------------------------------------------------------------
	map<SCR_EXPRewards, int> GetXPRewards()
	{
		return m_mXPRewards;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds new XP reward type and gained xp to the list
	//! If reward is already in the list, adds gained xp to saved xp
	//! \param[in] rewardID
	//! \param[in] xp
	void AddXPReward(SCR_EXPRewards rewardID, int xp)
	{
		if (xp == 0)
			return;

		m_mXPRewards.Set(rewardID, m_mXPRewards.Get(rewardID) + xp);
	}
}
