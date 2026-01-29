//------------------------------------------------------------------------------------------------
/*!
	Holds data used in game mode score system. 
	See SCR_BaseScoringSystemComponent for more inforamtion.
*/
class SCR_ScoreInfo
{
	int m_iKills = 0;
	int m_iTeamKills = 0;
	int m_iDeaths = 0;
	int m_iSuicides = 0;
	int m_iObjectives = 0;
	
	//------------------------------------------------------------------------------------------------
	/*
		Serialize current state into buffer via provided writer.
	*/
	bool RplSave(ScriptBitWriter writer)
    {
        writer.WriteInt(m_iKills);
		writer.WriteInt(m_iTeamKills);
		writer.WriteInt(m_iDeaths);
		writer.WriteInt(m_iSuicides);
		writer.WriteInt(m_iObjectives);
		return true;
    }

	//------------------------------------------------------------------------------------------------
	/*
		Deserialize state of scoring into buffer via provided reader.
	*/
    bool RplLoad(ScriptBitReader reader)
    {
		reader.ReadInt(m_iKills);
		reader.ReadInt(m_iTeamKills);
		reader.ReadInt(m_iDeaths);
		reader.ReadInt(m_iSuicides);
		reader.ReadInt(m_iObjectives);
        return true;
    }
	
	//------------------------------------------------------------------------------------------------
	/*!
		Clear all data in this object, resetting values to 0.
	*/
	void Clear()
	{
		m_iKills = 0;
		m_iTeamKills = 0;
		m_iDeaths = 0;
		m_iSuicides = 0;
		m_iObjectives = 0;
	}
};
