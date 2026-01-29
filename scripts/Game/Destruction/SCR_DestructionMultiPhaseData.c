class SCR_DestructionMultiPhaseData
{
	//! this integer is public as we do not want to have unnecessary getter & setter calls for it
	int m_iNextFreeIndex = -1;
	protected int m_iDamagePhase = 0;
	protected int m_iTargetDamagePhase = 0;
	protected ResourceName m_OriginalResourceName;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetTargetDamagePhase()
	{
		return m_iTargetDamagePhase;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] targetDamagePhase
	void SetTargetDamagePhase(int targetDamagePhase)
	{
		m_iTargetDamagePhase = targetDamagePhase;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetOriginalResourceName()
	{
		return m_OriginalResourceName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] originalResourceName
	void SetOriginalResourceName(ResourceName originalResourceName)
	{
		m_OriginalResourceName = originalResourceName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetDamagePhase()
	{
		return m_iDamagePhase;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] damagePhase
	void SetDamagePhase(int damagePhase)
	{
		m_iDamagePhase = damagePhase;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void Reset()
	{
		m_iDamagePhase = 0;
		m_iTargetDamagePhase = 0;
		m_OriginalResourceName = string.Empty;
	}
}
