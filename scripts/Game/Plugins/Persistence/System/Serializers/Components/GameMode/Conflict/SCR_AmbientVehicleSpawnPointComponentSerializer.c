class SCR_AmbientVehicleSpawnPointComponentSerializer : ScriptedComponentSerializer
{
	[Attribute("120.0", desc: "Seconds of lifetime added to a vehicle after loading a save, to give players a chance to connect before it despawns.", params: "0 3600 1")]
	protected float m_fDespawnGracePeriod;

	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_AmbientVehicleSpawnPointComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull SaveContext context)
	{
		auto ambientVehicle = SCR_AmbientVehicleSpawnPointComponent.Cast(component);
		auto world = ChimeraWorld.CastFrom(owner.GetWorld());
		const WorldTimestamp currentTime = world.GetServerTimestamp();

		const bool depleted = ambientVehicle.GetIsDepleted();

		const IEntity vehicle = ambientVehicle.GetSpawnedVehicle();
		const bool spawned = vehicle != null;
		UUID vehicleId;
		if (spawned)
			vehicleId = GetSystem().GetId(vehicle);
		else
			vehicleId = ambientVehicle.GetDespawnedDataId();

		const WorldTimestamp despawnTimestamp = ambientVehicle.GetDespawnTimestamp();
		float despawnTime = 0;
		if (despawnTimestamp)
			despawnTime = currentTime.DiffSeconds(despawnTimestamp);

		const WorldTimestamp respawnTimestamp = ambientVehicle.GetRespawnTimestamp();
		float respawnTime = 0;
		if (respawnTimestamp)
			respawnTime = currentTime.DiffSeconds(respawnTimestamp);

		if (!depleted &&
			vehicleId.IsNull() &&
			float.AlmostEqual(despawnTime, 0.0) &&
			float.AlmostEqual(respawnTime, 0.0))
		{
			return ESerializeResult.DEFAULT;
		}

		context.WriteValue("version", 2);
		context.WriteDefault(depleted, false);
		context.WriteDefault(spawned, false);
		context.WriteDefault(vehicleId, UUID.NULL_UUID);
		context.WriteDefault(despawnTime, 0.0);
		context.WriteDefault(respawnTime, 0.0);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull LoadContext context)
	{
		auto ambientVehicle = SCR_AmbientVehicleSpawnPointComponent.Cast(component);
		auto world = ChimeraWorld.CastFrom(owner.GetWorld());
		const WorldTimestamp currentTime = world.GetServerTimestamp();

		int version;
		context.Read(version);

		bool depleted;
		if (context.Read(depleted))
			ambientVehicle.SetIsDepleted(depleted);

		bool spawned = true;
		if (version >= 2)
			context.ReadDefault(spawned, false);

		UUID vehicleId;
		if (context.Read(vehicleId) && !vehicleId.IsNull())
		{
			if (spawned)
			{
				Tuple1<SCR_AmbientVehicleSpawnPointComponent> ctx(ambientVehicle);
				PersistenceWhenAvailableTask task(OnVehicleAvailable, ctx);
				GetSystem().WhenAvailable(vehicleId, task);
			}
			else
			{
				ambientVehicle.SetDespawnedDataId(vehicleId);
			}
		}

		float despawnTime;
		context.Read(despawnTime);
		ambientVehicle.SetDespawnTimestamp(currentTime.PlusSeconds(despawnTime + Math.Max(m_fDespawnGracePeriod, 0)));

		float respawnTime;
		if (context.Read(respawnTime))
			ambientVehicle.SetRespawnTimestamp(currentTime.PlusSeconds(respawnTime));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnVehicleAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto vehicle = Vehicle.Cast(instance);
		if (!vehicle)
			return;

		auto ctx = Tuple1<SCR_AmbientVehicleSpawnPointComponent>.Cast(context);
		if (ctx.param1)
			ctx.param1.SetSpawnedVehicle(vehicle);
	}
}
