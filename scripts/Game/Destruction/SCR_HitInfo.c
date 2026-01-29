//! Class to temporarily store information about the last hit that dealt damage
class SCR_HitInfo
{
	float m_LastHealth;
	float m_HitDamage;
	EDamageType m_DamageType;
	vector m_HitPosition;
	vector m_HitDirection;
	vector m_HitNormal;
}

//! Class to temporarily store information about the last hit that caused destruction
class SCR_DestructionHitInfo : SCR_HitInfo
{
	bool m_TotalDestruction;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hitInfo
	//! \param[in] totalDestruction
	//! \return
	static SCR_DestructionHitInfo FromHitInfo(SCR_HitInfo hitInfo, bool totalDestruction)
	{
		SCR_DestructionHitInfo destHitInfo = new SCR_DestructionHitInfo;
		destHitInfo.m_TotalDestruction = totalDestruction;
		destHitInfo.m_LastHealth = hitInfo.m_LastHealth;
		destHitInfo.m_HitDamage = hitInfo.m_HitDamage;
		destHitInfo.m_DamageType = hitInfo.m_DamageType;
		destHitInfo.m_HitPosition = hitInfo.m_HitPosition;
		destHitInfo.m_HitDirection = hitInfo.m_HitDirection;
		destHitInfo.m_HitNormal = hitInfo.m_HitNormal;
		return destHitInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] destructionData
	//! \return
	static SCR_DestructionHitInfo FromDestructionData(SCR_DestructionData destructionData)
	{
		SCR_DestructionHitInfo destructionHitInfo = new SCR_DestructionHitInfo();
		destructionHitInfo.m_TotalDestruction = destructionData.m_bTotalDestruction;
		destructionHitInfo.m_HitDamage = destructionData.m_fHitDamage;
		destructionHitInfo.m_DamageType = destructionData.m_eDamageType;
		destructionHitInfo.m_HitPosition = destructionData.m_vHitPosition;
		destructionHitInfo.m_HitDirection = destructionData.m_vHitDirection;
		destructionHitInfo.m_HitNormal = destructionData.m_vHitNormal;
		return destructionHitInfo;
	}
}