class SCR_AmbientVehicleSpawnPointComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_AmbientVehicleSpawnPointComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		auto ambientVehicle = SCR_AmbientVehicleSpawnPointComponent.Cast(component);
		auto world = ChimeraWorld.CastFrom(owner.GetWorld());
		const WorldTimestamp currentTime = world.GetServerTimestamp();

		const bool depleted = ambientVehicle.GetIsDepleted();
		const UUID vehicleId = GetSystem().GetId(ambientVehicle.GetSpawnedVehicle());

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

		context.WriteValue("version", 1);
		context.WriteDefault(depleted, false);
		context.WriteDefault(vehicleId, UUID.NULL_UUID);
		context.WriteDefault(despawnTime, 0.0);
		context.WriteDefault(respawnTime, 0.0);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto ambientVehicle = SCR_AmbientVehicleSpawnPointComponent.Cast(component);
		auto world = ChimeraWorld.CastFrom(owner.GetWorld());
		const WorldTimestamp currentTime = world.GetServerTimestamp();

		int version;
		context.Read(version);

		bool depleted;
		if (context.Read(depleted))
			ambientVehicle.SetIsDepleted(depleted);

		UUID vehicleId;
		if (context.Read(vehicleId) && !vehicleId.IsNull())
		{
			Tuple1<SCR_AmbientVehicleSpawnPointComponent> ctx(ambientVehicle);
			PersistenceWhenAvailableTask task(OnVehicleAvailable, ctx);
			GetSystem().WhenAvailable(vehicleId, task);
		}

		float despawnTime;
		if (context.Read(despawnTime))
			ambientVehicle.SetDespawnTimestamp(currentTime.PlusSeconds(despawnTime));

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
