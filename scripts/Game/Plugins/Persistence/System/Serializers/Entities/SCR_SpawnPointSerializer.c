class SCR_SpawnPointSerializer : GenericEntitySerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_SpawnPoint;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity entity, notnull SaveContext context)
	{
		const SCR_SpawnPoint spawnPoint = SCR_SpawnPoint.Cast(entity);
		const BaseContainer source = entity.GetPrefabData().GetPrefab();

		const bool enabled = spawnPoint.IsSpawnPointEnabled();
		bool enabledDefault;
		source.Get("m_bSpawnPointEnabled", enabledDefault);

		if (enabled == enabledDefault)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(enabled, enabledDefault);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity entity, notnull LoadContext context)
	{
		SCR_SpawnPoint spawnPoint = SCR_SpawnPoint.Cast(entity);

		int version;
		context.ReadValue("version", version);

		bool enabled;
		if (context.Read(enabled))
			spawnPoint.SetSpawnPointEnabled_S(enabled);

		return true;
	}
}
