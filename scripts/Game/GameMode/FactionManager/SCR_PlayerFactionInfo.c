class SCR_PlayerFactionInfo
{
	protected int m_iPlayerId;
	protected int m_iFactionIndex = -1;

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetFactionIndex()
	{
		return m_iFactionIndex;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] factionIndex
	void SetFactionIndex(int factionIndex)
	{
		m_iFactionIndex = factionIndex;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetPlayerId()
	{
		return m_iPlayerId;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	protected void SCR_PlayerFactionInfo();

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerId
	//! \return
	static SCR_PlayerFactionInfo Create(int playerId)
	{
		SCR_PlayerFactionInfo info = new SCR_PlayerFactionInfo();
		info.m_iPlayerId = playerId;
		info.m_iFactionIndex = -1;
		return info;
	}

	//################################################################################################
	//! Codec methods
	//################################################################################################

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] instance
	//! \param[in] ctx
	//! \param[in] snapshot
	//! \return
	static bool Extract(SCR_PlayerFactionInfo instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeInt(instance.m_iPlayerId);
		snapshot.SerializeInt(instance.m_iFactionIndex);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \param[in] instance
	//! \return
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_PlayerFactionInfo instance)
	{
		snapshot.SerializeInt(instance.m_iPlayerId);
		snapshot.SerializeInt(instance.m_iFactionIndex);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \param[in] packet
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.EncodeInt(packet);	// m_iPlayerId
		snapshot.EncodeInt(packet);	// m_iFactionIndex
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] packet
	//! \param[in] ctx
	//! \param[in] snapshot
	//! \return
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.DecodeInt(packet);	// m_iPlayerId
		snapshot.DecodeInt(packet);	// m_iFactionIndex
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] lhs
	//! \param[in] rhs
	//! \param[in] ctx
	//! \return
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs , ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, 4 + 4); // m_iPlayerId, m_iFactionIndex
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] instance
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \return
	static bool PropCompare(SCR_PlayerFactionInfo instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.CompareInt(instance.m_iPlayerId)
		    && snapshot.CompareInt(instance.m_iFactionIndex);
	}
}
