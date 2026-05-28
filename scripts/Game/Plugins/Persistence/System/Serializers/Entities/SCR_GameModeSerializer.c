class SCR_GameModeSerializer : ScriptedEntitySerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_BaseGameMode;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity entity, notnull SaveContext context)
	{
		const SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(entity);
		const BaseContainer source = entity.GetPrefabData().GetPrefab();

		array<EResourceType> defaultDisabled();
		source.Get("m_aDisabledResourceTypes", defaultDisabled);

		array<EResourceType> disabledResourceTypes();
		gameMode.GetDisabledResourceTypes(disabledResourceTypes);

		if (SCR_ArrayHelperT<EResourceType>.AreEqual(defaultDisabled, disabledResourceTypes))
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.Write(disabledResourceTypes);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity entity, notnull LoadContext context)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(entity);

		int version;
		context.Read(version);

		array<EResourceType> disabledResourceTypes;
		context.Read(disabledResourceTypes);
		if (disabledResourceTypes)
		{
			// Enable those who are not disabled in save-game
			array<EResourceType> currentDisabled();
			gameMode.GetDisabledResourceTypes(currentDisabled);
			foreach (EResourceType type : currentDisabled)
			{
				if (!disabledResourceTypes.Contains(type))
					gameMode.SetResourceTypeEnabled(true, type);
			}

			// Disable all from save-game
			foreach (EResourceType type : disabledResourceTypes)
			{
				gameMode.SetResourceTypeEnabled(false, type);
			}
		}

		return true;
	}
}
