class SCR_ShellConfig
{
	protected EntityPrefabData m_PrefabData;
	protected float m_fTime;
	protected bool m_bUseManualTime;
	protected int m_iChargeRingConfigId;

	//------------------------------------------------------------------------------------------------
	//! \return
	EntityPrefabData GetPrefabData()
	{
		return m_PrefabData;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetSavedTime()
	{
		return m_fTime;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] newTime
	void SetSavedTime(float newTime)
	{
		m_fTime = newTime;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsUsingManualTime()
	{
		return m_bUseManualTime;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] shouldUse
	void SetManualTimeUsage(bool shouldUse)
	{
		m_bUseManualTime = shouldUse;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetChargeRingConfigId()
	{
		return m_iChargeRingConfigId;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] configId
	void SetChargeRingConfigId(int configId)
	{
		m_iChargeRingConfigId = configId;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] prefabData
	//! \param[in] time
	//! \param[in] useManualTime
	void SCR_ShellConfig (notnull EntityPrefabData prefabData, float time, bool useManualTime, int chargeRingConfigId = -1)
	{
		m_PrefabData = prefabData;
		m_fTime = time;
		m_bUseManualTime = useManualTime;
		m_iChargeRingConfigId = chargeRingConfigId;
	}
}
