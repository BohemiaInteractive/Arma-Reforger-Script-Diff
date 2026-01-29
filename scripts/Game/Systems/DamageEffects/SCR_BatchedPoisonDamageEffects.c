class SCR_BatchedPoisonDamageEffects : SCR_BatchedDamageEffects
{
	float m_fDamageValue;
	int m_iNumberOfEffects;
	WorldTimestamp m_NextSoundEventTime;

	//------------------------------------------------------------------------------------------------
	override void CopyBatchedData(notnull SCR_BatchedDamageEffects newBatchedDataContainer)
	{
		SCR_BatchedPoisonDamageEffects newPoisonData = SCR_BatchedPoisonDamageEffects.Cast(newBatchedDataContainer);
		if (!newPoisonData)
			return;

		m_fDamageValue = newPoisonData.m_fDamageValue;
		m_iNumberOfEffects = newPoisonData.m_iNumberOfEffects;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] dmgVal
	//! \param[in] count
	void SCR_BatchedPoisonDamageEffects(float dmgVal, int count = 1)
	{
		m_fDamageValue = dmgVal;
		m_iNumberOfEffects = count;
	}
}
