class SCR_ManagedDamageEffectsContainer
{
	protected ref map<typename, ref SCR_BatchedDamageEffects> m_mBatchedEffects;
	protected ref set<SCR_PersistentDamageEffect> m_EffectsSet;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] dmgEffect
	//! \return
	bool AddEffect(notnull SCR_PersistentDamageEffect dmgEffect)
	{
		if (!m_EffectsSet)
			m_EffectsSet = new set<SCR_PersistentDamageEffect>();

		return m_EffectsSet.Insert(dmgEffect);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] dmgEffect
	//! \return number of effects in this set
	int RemoveEffect(notnull SCR_PersistentDamageEffect dmgEffect)
	{
		if (!m_EffectsSet)
			return 0;

		m_EffectsSet.RemoveItem(dmgEffect);
		return m_EffectsSet.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] t of the effect for which we are looking for the batch data
	//! \return batched damage effects container or null
	SCR_BatchedDamageEffects GetBatchedDataOfType(typename t)
	{
		if (!m_mBatchedEffects)
			return null;

		return m_mBatchedEffects.Get(t);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in,out] newBatchedData
	//! \param[in] avgTimeSlice
	//! \param[in] dmgMgr
	bool BatchData(notnull inout map<typename, ref SCR_BatchedDamageEffects> newBatchedData, const float avgTimeSlice, SCR_ExtendedDamageManagerComponent dmgMgr)
	{
		if (dmgMgr.GetState() == EDamageState.DESTROYED)
			return false;

		newBatchedData.Clear();
		if (!m_EffectsSet || m_EffectsSet.IsEmpty())
			return false;

		typename effectType;
		SCR_DotDamageEffect dotEffect;
		SCR_BatchedDamageEffects batchContainer;
		SCR_PersistentDamageEffect dmgEffectVirtualInstance;
		foreach (SCR_PersistentDamageEffect effect : m_EffectsSet)
		{
			dotEffect = SCR_DotDamageEffect.Cast(effect);
			if (dotEffect)
				dotEffect.RecalculateDPS(avgTimeSlice, dmgMgr);

			if (effect.UseBatchProcessing())
			{
				effectType = effect.Type();
				dmgEffectVirtualInstance = SCR_DamageSufferingSystem.GetVirtualInstanceOfDamageEffect(effectType);
				if (!dmgEffectVirtualInstance)
					continue;

				batchContainer = newBatchedData.Get(effectType);
				if (batchContainer)
				{
					dmgEffectVirtualInstance.BatchData(batchContainer, effect);
				}
				else
				{
					dmgEffectVirtualInstance.BatchData(batchContainer, effect);
					if (!batchContainer) // if it failed to create a batch container then ignore it - but it is sus (O_O")
					{
						Debug.Error("SCR_DamageSufferingSystem.OnUpdatePoint: " + effectType.ToString() + ".BatchData() failed to create an instance of the batched data container!");
						continue;
					}

					newBatchedData.Insert(effectType, batchContainer);
				}
			}
			else
			{
				effect.IndividualProcessing(dmgMgr);
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Updates previously hald information with the new batch data
	//! \param[in] newBatchedData from which information is going to be transferred to the old one
	void UpdateBachedData(map<typename, ref SCR_BatchedDamageEffects> newBatchedData)
	{
		if (m_mBatchedEffects && (!newBatchedData || newBatchedData.IsEmpty()))
		{
			m_mBatchedEffects.Clear();
			return;
		}

		if (!m_mBatchedEffects)
			m_mBatchedEffects = new map<typename, ref SCR_BatchedDamageEffects>();

		// simple lookup table of what is now in the batch to allow us later to discard no longer valid data
		const set<typename> batchedTypes = new set<typename>();
		bool needsCleanup = newBatchedData.Count() < m_mBatchedEffects.Count();

		// transfer information from new to the old set
		SCR_BatchedDamageEffects oldBatchContainer;
		foreach (typename effectType, ref SCR_BatchedDamageEffects batchContainer : newBatchedData)
		{
			batchedTypes.Insert(effectType);
			oldBatchContainer = m_mBatchedEffects.Get(effectType);
			if (!oldBatchContainer)
			{
				needsCleanup = true;
				m_mBatchedEffects.Insert(effectType, batchContainer);
			}
			else
			{
				oldBatchContainer.CopyBatchedData(batchContainer);
			}
		}

		if (!needsCleanup)
			return;

		// now cleanup old elements which are no longer being batched
		const set<typename> garbageTypes = new set<typename>();
		foreach (typename effectType, ref SCR_BatchedDamageEffects batchContainer : m_mBatchedEffects)
		{
			if (batchedTypes.Contains(effectType))
				continue;

			garbageTypes.Insert(effectType);
		}

		foreach (typename garbageKey : garbageTypes)
		{
			m_mBatchedEffects.Remove(garbageKey);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Iterates through the batched data and executes their respective processing methods
	//! \param[in] dmgMgr
	//! \param[in] isAuthority
	void ProcessBatchedData(notnull SCR_ExtendedDamageManagerComponent dmgMgr, bool isAuthority)
	{
		if (!m_mBatchedEffects)
			return;

		SCR_PersistentDamageEffect dmgEffectVirtualInstance;
		foreach (typename effectType, ref SCR_BatchedDamageEffects container : m_mBatchedEffects)
		{
			dmgEffectVirtualInstance = SCR_DamageSufferingSystem.GetVirtualInstanceOfDamageEffect(effectType);
			if (!dmgEffectVirtualInstance)
				continue;

			dmgEffectVirtualInstance.BatchProcessing(dmgMgr, container, isAuthority);
		}
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] effectsSet
	//! \param[in] batchedEffects
	void SCR_ManagedDamageEffectsContainer(set<SCR_PersistentDamageEffect> effectsSet = null, map<typename, ref SCR_BatchedDamageEffects> batchedEffects = null)
	{
		m_EffectsSet = effectsSet;
		m_mBatchedEffects = batchedEffects;
	}
}
