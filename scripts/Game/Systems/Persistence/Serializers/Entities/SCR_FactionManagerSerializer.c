class SCR_PersistentFaction
{
	FactionKey m_sKey;
	bool m_bPlayable;
	ref array<FactionKey> m_aFriendlies;
	ref array<FactionKey> m_aHostiles;
}

class SCR_FactionManagerSerializer : ScriptedEntitySerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_FactionManager;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity entity, notnull BaseSerializationSaveContext context)
	{
		const SCR_FactionManager factionManager = SCR_FactionManager.Cast(entity);
		array<Faction> factions();
		factionManager.GetFactionsList(factions);

		array<ref SCR_PersistentFaction> changedFactions();

		map<FactionKey, ref set<FactionKey>> friendlyDefaultMapping();
		foreach (auto faction : factions)
		{
			friendlyDefaultMapping.Insert(faction.GetFactionKey(), new set<FactionKey>);
		}

		for (int i = 0, count = factions.Count(); i < count; ++i)
		{
			const SCR_Faction faction = SCR_Faction.Cast(factions[i]);
			if (!faction)
				continue;

			const FactionKey key = faction.GetFactionKey();
			auto defaultFriendlies = faction.GetDefaultFriendlyFactions();
			bool defaultFriendlySelf = faction.IsDefaultSelfFriendly();

			set<FactionKey> friendlies = friendlyDefaultMapping.Get(key);

			if (defaultFriendlySelf)
				friendlies.Insert(key);

			foreach (auto friendly : defaultFriendlies)
			{
				friendlies.Insert(friendly);
				friendlyDefaultMapping.Get(friendly).Insert(key);
			}
		}

		for (int i = 0, count = factions.Count(); i < count; ++i)
		{
			const SCR_Faction faction = SCR_Faction.Cast(factions[i]);
			if (!faction)
				continue;

			const FactionKey key = faction.GetFactionKey();

			array<FactionKey> friendlyFactions();
			foreach (int idx, auto otherFaction : factions)
			{
				if (faction.IsFactionFriendly(otherFaction))
					friendlyFactions.Insert(otherFaction.GetFactionKey());
			}

			bool isPlayable = faction.IsPlayable();
			bool isPlayableDefault = faction.IsDefaultPlayable();

			array<FactionKey> hostileFactions();
			foreach (auto defaultFriendly : friendlyDefaultMapping.Get(key))
			{
				if (!friendlyFactions.RemoveItem(defaultFriendly))
					hostileFactions.Insert(defaultFriendly);
			}

			if (isPlayable == isPlayableDefault && friendlyFactions.IsEmpty() && hostileFactions.IsEmpty())
				continue;

			SCR_PersistentFaction changedFaction();
			changedFaction.m_sKey = faction.GetFactionKey();
			changedFaction.m_bPlayable = isPlayable;
			changedFaction.m_aFriendlies = friendlyFactions;
			changedFaction.m_aHostiles = hostileFactions;
			changedFactions.Insert(changedFaction);
		}

		if (changedFactions.IsEmpty())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		const bool prev = context.EnableTypeDiscriminator(false);
		context.WriteValue("factions", changedFactions);
		context.EnableTypeDiscriminator(prev);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity entity, notnull BaseSerializationLoadContext context)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(entity);
		SCR_DelegateFactionManagerComponent delegateFactionManager = SCR_DelegateFactionManagerComponent.GetInstance();

		int version;
		context.Read(version);

		array<ref SCR_PersistentFaction> changedFactions();
		const bool prev = context.EnableTypeDiscriminator(false);
		context.ReadValue("factions", changedFactions);
		context.EnableTypeDiscriminator(prev);

		array<Faction> factions();
		factionManager.GetFactionsList(factions);

		foreach (SCR_PersistentFaction changedFaction : changedFactions)
		{
			SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByKey(changedFaction.m_sKey));
			if (!faction)
				continue;

			if (faction.IsPlayable() != changedFaction.m_bPlayable)
			{
				faction.SetIsPlayable(changedFaction.m_bPlayable, false);
				if (delegateFactionManager)
				{
					// Make visible for GM editing UI
					SCR_EditableFactionComponent delegate = delegateFactionManager.GetFactionDelegate(faction);
					if (delegate)
						delegate.SetVisible(changedFaction.m_bPlayable);
				}
			}

			bool anyChanged = false;
			foreach (auto otherFaction : factions)
			{
				const SCR_Faction scrOtherFaction = SCR_Faction.Cast(otherFaction);
				if (!scrOtherFaction)
					continue;

				const FactionKey otherKey = scrOtherFaction.GetFactionKey();
				const bool currentFriendly = faction.IsFactionFriendly(scrOtherFaction);
				if (changedFaction.m_aFriendlies.Contains(otherKey) && !currentFriendly)
				{
					anyChanged = true;
					factionManager.SetFactionsFriendly(faction, scrOtherFaction, updateAIs: false);
				}
				else if (changedFaction.m_aHostiles.Contains(otherKey) && currentFriendly)
				{
					anyChanged = true;
					factionManager.SetFactionsHostile(faction, scrOtherFaction, updateAIs: false);
				}
			}

			if (anyChanged)
				SCR_FactionManager.RequestUpdateAllTargetsFactions();
		}

		return true;
	}
}
