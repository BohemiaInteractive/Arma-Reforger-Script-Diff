/*!
	Serializable end game date replicated to all users.
*/
class SCR_GameModeEndData
{
	private int m_iEndReason;

	private ref array<int> m_aWinnerIds = {};
	private ref array<int> m_aWinnerFactionIds = {};

	/*!
		Creates and returns new end data.
	*/
	static SCR_GameModeEndData CreateSimple(EGameOverTypes reason = EGameOverTypes.ENDREASON_UNDEFINED, int winnerId = -1, int winnerFactionId = -1)
	{
		ref SCR_GameModeEndData data = new SCR_GameModeEndData();
		data.m_iEndReason = reason;

		if (winnerId > -1)
		{
			data.m_aWinnerIds.Clear();
			data.m_aWinnerIds.Insert(winnerId);
		}

		if (winnerFactionId > -1)
		{
			data.m_aWinnerFactionIds.Clear();
			data.m_aWinnerFactionIds.Insert(winnerFactionId);
		}
		return data;
	}

	/*!
		Creates and returns new end data with multiple potential winner(s).
	*/
	static SCR_GameModeEndData Create(EGameOverTypes reason = EGameOverTypes.ENDREASON_UNDEFINED, array<int> winnerIds = null, array<int> winnerFactionIds = null)
	{
		ref SCR_GameModeEndData data = new SCR_GameModeEndData();
		data.m_iEndReason = reason;

		if (winnerIds)
		{
			for (int i = 0, cnt = winnerIds.Count(); i < cnt; i++)
				data.m_aWinnerIds.Insert(winnerIds[i]);
		}

		if (winnerFactionIds)
		{
			for (int i = 0, cnt = winnerFactionIds.Count(); i < cnt; i++)
				data.m_aWinnerFactionIds.Insert(winnerFactionIds[i]);
		}

		return data;
	}

	/*!
		Returns reason why game mode ended.
		Can be left empty or implemented as desired per game mode specialization.

		Reserved values:
		-1: ENDREASON_UNDEFINED: Undefined
		-2: ENDREASON_TIMELIMIT: Time limit reached
		-3: ENDREASON_SCORELIMIT: Score limit reached
		-4: ENDREASON_DRAW: Game ends in draw
		-5: ENDREASON_SERVER_RESTART: On server being restarted
	*/
	int GetEndReason()
	{
		return m_iEndReason;
	}

	/*!
		If game mode has a winning player, their id can be delivered via this prop.
		Can be left empty or implemented as desired per game mode specialization.
	*/
	int GetWinnerId()
	{
		if (m_aWinnerIds && m_aWinnerIds.Count() > 0)
			return m_aWinnerIds[0];

		return -1;
	}

	/*!
		Fill provided array with list of all winners.
	*/
	void GetWinnerIds(out notnull array<int> winnerIds)
	{
		winnerIds.Clear();
		for (int i = 0, cnt = m_aWinnerIds.Count(); i < cnt; i++)
			winnerIds.Insert(m_aWinnerIds[i]);
	}

	/*!
		If game mode has a winning faction, their id can be delivered via this prop.
		Can be left empty or implemented as desired per game mode specialization.
	*/
	int GetWinnerFactionId()
	{
		if (m_aWinnerFactionIds && m_aWinnerFactionIds.Count() > 0)
			return m_aWinnerFactionIds[0];

		return -1;
	}

	/*!
		Fill provided array with list of all winners.
	*/
	void GetFactionWinnerIds(out notnull array<int> winnerIds)
	{
		winnerIds.Clear();
		for (int i = 0, cnt = m_aWinnerFactionIds.Count(); i < cnt; i++)
			winnerIds.Insert(m_aWinnerFactionIds[i]);
	}

	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_GameModeEndData prop, ScriptCtx hint, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeInt(prop.m_iEndReason);

		int winnerCount = prop.m_aWinnerIds.Count();
		snapshot.SerializeInt(winnerCount);
		for (int i = 0; i < winnerCount; i++)
		{
			int id = prop.m_aWinnerIds[i]; 			
			snapshot.SerializeInt(id);
		}

		int factionWinnerCount = prop.m_aWinnerFactionIds.Count();
		snapshot.SerializeInt(factionWinnerCount);
		for (int i = 0; i < factionWinnerCount; i++)
		{
			int id = prop.m_aWinnerFactionIds[i];
			snapshot.SerializeInt(id);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx hint, SCR_GameModeEndData prop)
	{
		snapshot.SerializeInt(prop.m_iEndReason);

		int winnerCount;
		snapshot.SerializeInt(winnerCount);

		int tmp;
		prop.m_aWinnerIds.Clear();
		for (int i = 0; i < winnerCount; i++)
		{
			snapshot.SerializeInt(tmp);
			prop.m_aWinnerIds.Insert(tmp);
		}

		int winnerFactionCount;
		snapshot.SerializeInt(winnerFactionCount);

		prop.m_aWinnerFactionIds.Clear();
		for (int i = 0; i < winnerFactionCount; i++)
		{
			snapshot.SerializeInt(tmp);
			prop.m_aWinnerFactionIds.Insert(tmp);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx hint, ScriptBitSerializer packet)
	{
		int endReason;
		snapshot.SerializeBytes(endReason, 4);
		packet.Serialize(endReason, 32);

		int tmp;

		int winnerCount;
		snapshot.SerializeBytes(winnerCount, 4);
		packet.Serialize(winnerCount, 32);
		
		for (int i = 0; i < winnerCount; i++)
		{
			snapshot.SerializeBytes(tmp, 4);
			packet.Serialize(tmp, 32);

		}

		int factionWinnerCount;
		snapshot.SerializeBytes(factionWinnerCount, 4);
		packet.Serialize(factionWinnerCount, 32);

		for (int i = 0; i < factionWinnerCount; i++)
		{
			snapshot.SerializeBytes(tmp, 4);
			packet.Serialize(tmp, 32);
		}
	}

	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx hint, SSnapSerializerBase snapshot)
	{
		// How many int do i .Decode() ?
		int endReason;
		packet.Serialize(endReason, 32);
		snapshot.SerializeBytes(endReason, 4);

		int winnerCount;
		packet.Serialize(winnerCount, 32);
		snapshot.SerializeBytes(winnerCount, 4);

		int tmp;
		for (int i = 0; i < winnerCount; i++)
		{
			packet.Serialize(tmp, 32);
			snapshot.SerializeBytes(tmp, 4);
		}

		int factionWinnerCount;
		packet.Serialize(factionWinnerCount, 32);
		snapshot.SerializeBytes(factionWinnerCount, 4);
		for (int i = 0; i < factionWinnerCount; i++)
		{
			packet.Serialize(tmp, 32);
			snapshot.SerializeBytes(tmp, 4);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx hint)
	{
		int lhsReason;
		lhs.SerializeInt(lhsReason);

		int rhsReason;
		rhs.SerializeInt(rhsReason);

		if (lhsReason != rhsReason)
			return false;

		// array 1
		int lCnt;
		lhs.SerializeInt(lCnt);

		int rCnt;
		rhs.SerializeInt(rCnt);

		if (lCnt != rCnt)
			return false;

		int lTmp;
		int rTmp;
		for (int i = 0; i < lCnt; i++)
		{
			lhs.SerializeInt(lTmp);
			rhs.SerializeInt(rTmp);
			if (lTmp != rTmp)
				return false;
		}

		// array 2
		lhs.SerializeInt(lCnt);
		rhs.SerializeInt(rCnt);

		if (lCnt != rCnt)
			return false;

		for (int i = 0; i < lCnt; i++)
		{
			lhs.SerializeInt(lTmp);
			rhs.SerializeInt(rTmp);
			if (lTmp != rTmp)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_GameModeEndData prop, SSnapSerializerBase snapshot, ScriptCtx hint)
	{
		if (!snapshot.CompareInt(prop.m_iEndReason))
			return false;

		int winnerCount = prop.m_aWinnerIds.Count();
		if (!snapshot.CompareInt(winnerCount))
			return false;

		for (int i = 0; i < winnerCount; i++)
		{
			if (!snapshot.CompareInt(prop.m_aWinnerIds[i]))
				return false;
		}

		int winnerFactionCount = prop.m_aWinnerFactionIds.Count();
		if (!snapshot.CompareInt(winnerFactionCount))
			return false;

		for (int i = 0; i < winnerFactionCount; i++)
		{
			if (!snapshot.CompareInt(prop.m_aWinnerFactionIds[i]))
				return false;
		}

		return true;
	}
};