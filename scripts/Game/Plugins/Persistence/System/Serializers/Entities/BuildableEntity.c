class BuildableEntitySerializer : GenericEntitySerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return GenericEntity;
	}

	//------------------------------------------------------------------------------------------------
	override bool DeserializeSpawnData(out ResourceName prefab, notnull out EntitySpawnParams params, notnull LoadContext context)
	{
		// Avoid buildable compositions loading as finished
		SCR_EditorLinkComponent.IgnoreSpawning(true);
		return super.DeserializeSpawnData(prefab, params, context);
	}
}
