class SCR_RestrictedDeployableSpawnPointComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_RestrictedDeployableSpawnPointComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_RestrictedDeployableSpawnPointComponent spawnPoint = SCR_RestrictedDeployableSpawnPointComponent.Cast(component);
		const BaseContainer source = component.GetComponentSource(owner);

		const bool deployed = spawnPoint.IsDeployed();

		const SCR_ESpawnPointBudgetType budgetType = spawnPoint.GetBudgetType();
		SCR_ESpawnPointBudgetType budgetTypeDefault = SCR_ESpawnPointBudgetType.SUPPLIES;
		if (source)
			source.Get("m_eRespawnBudgetType", budgetTypeDefault);

		const float supplies = spawnPoint.GetSuppliesValue();
		float suppliesDefault = 100;
		if (source)
			source.Get("m_fSuppliesValue", suppliesDefault);

		const int respawnCount = spawnPoint.GetRespawnCount();
		const int respawnCountDefault = 0;

		const int spawnTicketsMax = spawnPoint.GetMaxRespawns();
		float spawnTicketsMaxDefault = 5;
		if (source)
			source.Get("m_iMaxRespawns", spawnTicketsMaxDefault);

		const int allowCustomLoadout = spawnPoint.IsCustomLoadoutsAllowed();
		bool allowCustomLoadoutDefault = false;
		if (source)
			source.Get("m_bAllowCustomLoadouts", allowCustomLoadoutDefault);

		if (deployed == false &&
			budgetType == budgetTypeDefault &&
			supplies == suppliesDefault &&
			respawnCount == respawnCountDefault &&
			spawnTicketsMax == spawnTicketsMaxDefault &&
			allowCustomLoadout == allowCustomLoadoutDefault)
		{
			return ESerializeResult.DEFAULT;
		}

		UUID groupId = UUID.NULL_UUID;
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (groupsManager)
			groupId = GetSystem().GetId(groupsManager.FindGroup(spawnPoint.GetGroupID()));

		const UUID ownerId = GetSystem().GetId(GetGame().GetPlayerManager().GetPlayerController(spawnPoint.GetItemOwnerID()));

		context.WriteValue("version", 1);
		context.WriteDefault(deployed, false);
		context.WriteDefault(supplies, suppliesDefault);
		context.WriteDefault(respawnCount, respawnCountDefault);
		context.WriteDefault(spawnTicketsMax, spawnTicketsMax);
		context.WriteDefault(allowCustomLoadout, allowCustomLoadoutDefault);
		context.WriteDefault(groupId, UUID.NULL_UUID);
		context.WriteDefault(ownerId, UUID.NULL_UUID);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_RestrictedDeployableSpawnPointComponent spawnPoint = SCR_RestrictedDeployableSpawnPointComponent.Cast(component);

		int version;
		context.Read(version);

		bool deployed;
		context.ReadDefault(deployed, false);

		float supplies;
		if (context.Read(supplies))
			spawnPoint.SetSuppliesValue(supplies, false);

		int respawnCount;
		if (context.Read(respawnCount))
			spawnPoint.SetRespawnCount(respawnCount);

		int spawnTicketsMax;
		if (context.Read(spawnTicketsMax))
			spawnPoint.SetRespawnCount(spawnTicketsMax);

		bool allowCustomLoadout;
		if (context.Read(allowCustomLoadout))
			spawnPoint.SetSpawningWithLoadout(allowCustomLoadout);

		UUID groupId;
		context.ReadDefault(groupId, UUID.NULL_UUID);

		UUID ownerId;
		if (context.Read(ownerId))
		{
			Tuple1<SCR_RestrictedDeployableSpawnPointComponent> ctx(spawnPoint);
			PersistenceWhenAvailableTask task(OnOwnerAvailable, ctx);
			GetSystem().WhenAvailable(ownerId, task);
		}

		if (deployed && !groupId.IsNull())
		{
			Tuple1<SCR_RestrictedDeployableSpawnPointComponent> ctx(spawnPoint);
			PersistenceWhenAvailableTask task(OnGroupAvailable, ctx);
			GetSystem().WhenAvailable(groupId, task);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnGroupAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto group = SCR_AIGroup.Cast(instance);
		if (!group)
			return;

		auto ctx = Tuple1<SCR_RestrictedDeployableSpawnPointComponent>.Cast(context);
		if (ctx.param1)
			ctx.param1.DeployByGroup(group, reload: true);
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnOwnerAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto owner = PlayerController.Cast(instance);
		if (!owner)
			return;

		auto ctx = Tuple1<SCR_RestrictedDeployableSpawnPointComponent>.Cast(context);
		if (ctx.param1)
			ctx.param1.SetItemOwner(owner.GetPlayerId());
	}
}
