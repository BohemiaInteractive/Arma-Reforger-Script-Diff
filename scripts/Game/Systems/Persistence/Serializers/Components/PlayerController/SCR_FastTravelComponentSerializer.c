class SCR_FastTravelComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_FastTravelComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		auto fastTravel = SCR_FastTravelComponent.Cast(component);
		auto world = ChimeraWorld.CastFrom(owner.GetWorld());

		const WorldTimestamp nextTransportTime = fastTravel.GetNextTransportTimestamp();
		if (!nextTransportTime)
			return ESerializeResult.DEFAULT;

		const WorldTimestamp currentTime = world.GetServerTimestamp();
		const float cooldown = currentTime.DiffSeconds(nextTransportTime);

		if (float.AlmostEqual(cooldown, 0.0))
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.Write(cooldown);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto fastTravel = SCR_FastTravelComponent.Cast(component);
		auto world = ChimeraWorld.CastFrom(owner.GetWorld());
		
		int version;
		context.Read(version);

		float cooldown;
		context.Read(cooldown);
		fastTravel.SetNextTransportTimestamp(world.GetServerTimestamp().PlusSeconds(cooldown));

		return true;
	}
}
