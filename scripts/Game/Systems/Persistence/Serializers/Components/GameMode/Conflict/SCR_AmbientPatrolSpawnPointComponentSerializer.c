class SCR_AmbientPatrolSpawnPointComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_AmbientPatrolSpawnPointComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		auto ambientPatrol = SCR_AmbientPatrolSpawnPointComponent.Cast(component);
		auto world = ChimeraWorld.CastFrom(owner.GetWorld());
		const WorldTimestamp currentTime = world.GetServerTimestamp();

		const bool spawned = ambientPatrol.GetIsSpawned();
		const bool paused = ambientPatrol.GetIsPaused();
		const int membersAlive = ambientPatrol.GetMembersAlive();

		if (!spawned &&	!paused && (membersAlive == -1))
			return ESerializeResult.DEFAULT;

		const UUID groupId = GetSystem().GetId(ambientPatrol.GetSpawnedGroup());
		const bool groupActive = ambientPatrol.IsGroupActive();
		const UUID waypointId = GetSystem().GetId(ambientPatrol.GetWaypoint());

		const WorldTimestamp despawnTimestamp = ambientPatrol.GetDespawnTimestamp();
		float despawnTime = 0;
		if (despawnTimestamp)
			despawnTime = currentTime.DiffSeconds(despawnTimestamp);

		const WorldTimestamp respawnTimestamp = ambientPatrol.GetRespawnTimestamp();
		float respawnTime = 0;
		if (respawnTimestamp)
			respawnTime = currentTime.DiffSeconds(respawnTimestamp);

		context.WriteValue("version", 1);
		context.WriteDefault(spawned, false);
		context.WriteDefault(paused, false);
		context.WriteDefault(groupId, UUID.NULL_UUID);
		context.WriteDefault(groupActive, false);
		context.WriteDefault(membersAlive, -1);
		context.WriteDefault(waypointId, UUID.NULL_UUID);
		context.WriteDefault(despawnTime, 0.0);
		context.WriteDefault(respawnTime, 0.0);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto ambientPatrol = SCR_AmbientPatrolSpawnPointComponent.Cast(component);
		auto world = ChimeraWorld.CastFrom(owner.GetWorld());
		const WorldTimestamp currentTime = world.GetServerTimestamp();

		int version;
		context.Read(version);

		bool spawned;
		if (context.Read(spawned))
			ambientPatrol.SetIsSpawned(spawned);

		bool paused;
		if (context.Read(paused))
			ambientPatrol.SetIsPaused(paused);

		UUID groupId;
		if (context.Read(groupId) && !groupId.IsNull())
		{
			Tuple1<SCR_AmbientPatrolSpawnPointComponent> ctx(ambientPatrol);
			PersistenceWhenAvailableTask task(OnGroupAvailable, ctx);
			GetSystem().WhenAvailable(groupId, task);
		}

		bool groupActive;
		if (context.Read(groupActive))
		{
			if (groupActive)
			{
				ambientPatrol.ActivateGroup();
			}
			else
			{
				ambientPatrol.DeactivateGroup();
			}
		}

		int membersAlive;
		if (context.Read(membersAlive))
			ambientPatrol.SetMembersAlive(membersAlive);

		UUID waypointId;
		if (context.Read(waypointId) && !waypointId.IsNull())
		{
			Tuple1<SCR_AmbientPatrolSpawnPointComponent> ctx(ambientPatrol);
			PersistenceWhenAvailableTask task(OnWaypointAvailable, ctx);
			GetSystem().WhenAvailable(waypointId, task);
		}

		float despawnTime;
		if (context.Read(despawnTime))
			ambientPatrol.SetDespawnTimestamp(currentTime.PlusSeconds(despawnTime));

		float respawnTime;
		if (context.Read(respawnTime))
			ambientPatrol.SetRespawnTimestamp(currentTime.PlusSeconds(respawnTime));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnGroupAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto group = SCR_AIGroup.Cast(instance);
		if (!group)
			return;

		auto ctx = Tuple1<SCR_AmbientPatrolSpawnPointComponent>.Cast(context);
		if (ctx.param1)
			ctx.param1.SetspawnedGroup(group);
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnWaypointAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto wp = SCR_AIWaypoint.Cast(instance);
		if (!wp)
			return;

		auto ctx = Tuple1<SCR_AmbientPatrolSpawnPointComponent>.Cast(context);
		if (ctx.param1)
			ctx.param1.SetWaypoint(wp);
	}
}
