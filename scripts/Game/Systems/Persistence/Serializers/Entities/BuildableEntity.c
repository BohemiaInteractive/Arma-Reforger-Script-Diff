class BuildableEntitySerializer : GenericEntitySerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return GenericEntity;
	}

	//------------------------------------------------------------------------------------------------
	override bool DeserializeSpawnData(out ResourceName prefab, out EntitySpawnParams params, notnull BaseSerializationLoadContext context)
	{
		// Avoid buildable compositions loading as finished
		SCR_EditorLinkComponent.IgnoreSpawning(true);
		return super.DeserializeSpawnData(prefab, params,  context);
	}
}
