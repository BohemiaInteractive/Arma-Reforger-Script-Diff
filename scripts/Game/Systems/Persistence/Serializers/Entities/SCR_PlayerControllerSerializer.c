class SCR_PlayerControllerSerializer : ScriptedEntitySerializer
{
	[Attribute("1", desc: "Enable if the gamemode allows for posessing another character besides the main player entity.")]
	protected bool m_bPosessionPossible;

	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_PlayerController;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity entity, notnull BaseSerializationSaveContext context)
	{
		const SCR_PlayerController playerController = SCR_PlayerController.Cast(entity);
		const UUID playerEntity = GetSystem().GetId(playerController.GetMainEntity());
		UUID currentEntity = GetSystem().GetId(playerController.GetControlledEntity());
		if (currentEntity.IsNull())
			currentEntity = playerEntity; // In case the currently controlled entity is not persistent

		if (playerEntity.IsNull() && (!m_bPosessionPossible || currentEntity == playerEntity))
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(playerEntity, UUID.NULL_UUID);

		if (m_bPosessionPossible)
			context.WriteDefault(currentEntity, playerEntity);

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity entity, notnull BaseSerializationLoadContext context)
	{
		auto playerController = SCR_PlayerController.Cast(entity);

		int version;
		context.ReadValue("version", version);

		UUID playerEntity;
		context.ReadDefault(playerEntity, UUID.NULL_UUID);

		UUID currentEntity;
		if (m_bPosessionPossible)
		{
			context.ReadDefault(currentEntity, playerEntity);
		}
		else
		{
			currentEntity = UUID.NULL_UUID;
		}

		// For vanilla we go through the respawn system to assign main and currently controlled entity to not break existing logic that relies on it.
		auto respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (respawnSystem)
		{
			respawnSystem.GetSpawnLogic().StoreControlledEntityIds(playerController, playerEntity, currentEntity);
			return true;
		}

		// TODO(@langepau): Support for direct loading of player and controlled entity with spawn request on both and WhenAvailable if a mod wants to not have a respawn system.
		return false;
	}
}
