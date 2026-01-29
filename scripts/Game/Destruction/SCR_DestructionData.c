class SCR_DestructionData : BaseDestructibleData
{
	float m_fHitDamage;
	EDamageType m_eDamageType;
	vector m_vHitPosition;
	vector m_vHitDirection; //Normalized
	vector m_vHitNormal; //Normalized
	bool m_bTotalDestruction;
	int m_iPreviousPhase;
	
	const int MAX_PHASES_BIT_SIZE = 4; // Max 16 phases are supported now = 4 bits
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] w
	//! \param[in] norm
	void SaveNormalizedVector(ScriptBitWriter w, vector norm)
	{
		w.WriteHalf(norm[0]);
		w.WriteHalf(norm[1]);
		w.WriteHalf(norm[2]);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] r
	//! \param[out] norm
	void LoadNormalizedVector(ScriptBitReader r, out vector norm)
	{
		float x;
		float y;
		float z;
		
		r.ReadHalf(x);
		r.ReadHalf(y);
		r.ReadHalf(z);
		
		norm = {x,y,z};
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] w
	//! \return
	bool Save(ScriptBitWriter w)
	{
		SaveNormalizedVector(w, m_vHitDirection);
		w.WriteFloat(m_fHitDamage);
		SaveNormalizedVector(w, m_vHitNormal);
		w.WriteInt(m_eDamageType);
		
		w.WriteVector(m_vHitPosition); // Maybe we should make this relative to the entity, so it's smaller numbers and we can also write it as half
		w.WriteBool(m_bTotalDestruction);
		w.WriteInt(m_iPreviousPhase);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] r
	//! \return
	bool Load(ScriptBitReader r)
	{
		LoadNormalizedVector(r, m_vHitDirection);
		r.ReadFloat(m_fHitDamage);
		LoadNormalizedVector(r, m_vHitNormal);
		r.ReadInt(m_eDamageType);
		r.ReadVector(m_vHitPosition);
		r.ReadBool(m_bTotalDestruction);
		r.ReadInt(m_iPreviousPhase);
		
		return true;
	}
}