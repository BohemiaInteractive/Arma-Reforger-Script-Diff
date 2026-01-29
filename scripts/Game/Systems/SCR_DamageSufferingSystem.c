class SCR_DamageSufferingSystem : GameSystem
{
	protected ref map<SCR_ExtendedDamageManagerComponent, ref SCR_ManagedDamageEffectsContainer> m_mManagedDamageEffects;
	protected SCR_ExtendedDamageManagerComponent m_LocallyControlledCharactersDmgMgr;
	protected SCR_ManagedDamageEffectsContainer m_LocallyControlledCharacterData;

	private static const ref map<typename, ref SCR_PersistentDamageEffect> MAP_OF_VIRTUAL_INSTANCES = new map<typename, ref SCR_PersistentDamageEffect>();

	//------------------------------------------------------------------------------------------------
	override bool ShouldBePaused()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo.SetAbstract(false);
		outInfo.SetLocation(WorldSystemLocation.Both);
		outInfo.AddPoint(WorldSystemPoint.Frame);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_DamageSufferingSystem GetInstance()
	{
		const World world = GetGame().GetWorld();
		return SCR_DamageSufferingSystem.Cast(world.FindSystem(SCR_DamageSufferingSystem));
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a cached virtual instance of the damage effect of the provided type
	//! \param[in] t type of a class which inherits from SCR_PersistentDamageEffect
	//! \return cached instance of the provided type
	static SCR_PersistentDamageEffect GetVirtualInstanceOfDamageEffect(typename t)
	{
		SCR_PersistentDamageEffect dmgEffect = MAP_OF_VIRTUAL_INSTANCES.Get(t);
		if (dmgEffect)
			return dmgEffect;

		dmgEffect = SCR_PersistentDamageEffect.Cast(t.Spawn());
		if (dmgEffect)
			MAP_OF_VIRTUAL_INSTANCES.Insert(t, dmgEffect);
		else
			Debug.Error("SCR_DamageSufferingSystem.GetVirtualInstanceOfDamageEffect: Failed to create an instance of " + t.ToString() + "! Ensure that it inherits from SCR_PersistentDamageEffect.");

		return dmgEffect;
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to find batched data about specified damage effect
	//! \param[in] dmgMgr
	//! \param[in] t type of the damage effect
	//! \return found batched set of data, or null
	SCR_BatchedDamageEffects GetBatchedDataOfType(notnull SCR_ExtendedDamageManagerComponent dmgMgr, typename t)
	{
		if (!m_mManagedDamageEffects)
			return null;

		SCR_ManagedDamageEffectsContainer container = m_mManagedDamageEffects.Get(dmgMgr);
		if (!container)
			return null;

		return container.GetBatchedDataOfType(t);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to lookup batched data for locally controlled character
	//! \param[in] t type of the damage effect
	//! \return found batched set of data, or null
	SCR_BatchedDamageEffects GetBatchedDataForLocalCharacter(typename t)
	{
		if (!m_LocallyControlledCharacterData)
			return null;

		return m_LocallyControlledCharacterData.GetBatchedDataOfType(t);
	}

	//------------------------------------------------------------------------------------------------
	//! Tries to add damage effect to the list of managed effects for the provided damage manager
	//! \param[in] dmgMgr
	//! \param[in] dmgEffect
	void RegisterEffect(notnull SCR_ExtendedDamageManagerComponent dmgMgr, notnull SCR_PersistentDamageEffect dmgEffect)
	{
		if (!m_mManagedDamageEffects)
			m_mManagedDamageEffects = new map<SCR_ExtendedDamageManagerComponent, ref SCR_ManagedDamageEffectsContainer>();

		SCR_ManagedDamageEffectsContainer container = m_mManagedDamageEffects.Get(dmgMgr);
		if (!container)
		{
			container = new SCR_ManagedDamageEffectsContainer();
			container.AddEffect(dmgEffect);
			m_mManagedDamageEffects.Set(dmgMgr, container);
			if (!IsEnabled())
				Enable(true);

			if (m_LocallyControlledCharactersDmgMgr == dmgMgr)
				m_LocallyControlledCharacterData = container;

			return;
		}

		if (container.AddEffect(dmgEffect))
			Enable(true);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] dmgMgr
	//! \param[in] dmgEffect
	void UnregisterEffect(notnull SCR_ExtendedDamageManagerComponent dmgMgr, notnull SCR_PersistentDamageEffect dmgEffect)
	{
		if (!m_mManagedDamageEffects || m_mManagedDamageEffects.IsEmpty())
			return;

		SCR_ManagedDamageEffectsContainer container = m_mManagedDamageEffects.Get(dmgMgr);
		if (!container)
			return;

		if (container.RemoveEffect(dmgEffect) == 0)
			m_mManagedDamageEffects.Set(dmgMgr, null); // make it garbage so it can be cleared next time we are going to be processing this
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		if (!m_mManagedDamageEffects || m_mManagedDamageEffects.IsEmpty())
		{
			Enable(false);
			return;
		}

		const bool isAuthority = GetNode().GetRole() == RplRole.Authority;
		const array<SCR_ExtendedDamageManagerComponent> garbageData = {};
		const float avgTimeSlice = System.GetFrameTimeS();
		map<typename, ref SCR_BatchedDamageEffects> newBatchedData = new map<typename, ref SCR_BatchedDamageEffects>();

		foreach (SCR_ExtendedDamageManagerComponent dmgMgr, SCR_ManagedDamageEffectsContainer container : m_mManagedDamageEffects)
		{
			if (!dmgMgr || !container || !container.BatchData(newBatchedData, avgTimeSlice, dmgMgr))
			{
				garbageData.Insert(dmgMgr);
				continue;
			}

			container.UpdateBachedData(newBatchedData);
			container.ProcessBatchedData(dmgMgr, isAuthority);
		}

		foreach (SCR_ExtendedDamageManagerComponent garbageKey : garbageData)
		{
			m_mManagedDamageEffects.Remove(garbageKey);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method for PlayerController ownership transfer. For more information see PlayerController.OnOwnershipChanged
	//! \param[in] controller
	//! \param[in] changing true if ownership is being transferred
	//! \param[in] becameOwner true if local client is now the owner of this controller
	void StartObservingControlledEntityChanges(notnull SCR_PlayerController controller, bool changing, bool becameOwner)
	{
		if (changing)
		{
			controller.m_OnControlledEntityChanged.Remove(OnControlledEntityChanged);
		}
		else if (becameOwner)
		{
			controller.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
			OnControlledEntityChanged(null, controller.GetLocalControlledEntity());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to inform that locally controlled entity has changed
	//! \param[in] from previously controlled entity
	//! \param[in] to currently controlled entity
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		if (!to)
		{
			m_LocallyControlledCharacterData = null;
			return;
		}

		ChimeraCharacter character = ChimeraCharacter.Cast(to);
		if (!character)
		{
			m_LocallyControlledCharactersDmgMgr = SCR_ExtendedDamageManagerComponent.Cast(SCR_DamageManagerComponent.GetDamageManager(to));
			if (!m_LocallyControlledCharactersDmgMgr)
			{
				m_LocallyControlledCharacterData = null;
				return;
			}
		}
		else
		{
			m_LocallyControlledCharactersDmgMgr = SCR_ExtendedDamageManagerComponent.Cast(character.GetDamageManager());
		}

		if (m_mManagedDamageEffects)
			m_LocallyControlledCharacterData = m_mManagedDamageEffects.Get(m_LocallyControlledCharactersDmgMgr);
		else
			m_LocallyControlledCharacterData = null;
	}
}
