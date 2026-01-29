class SCR_NightModeGameModeComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_NightModeGameModeComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_NightModeGameModeComponent nightMode = SCR_NightModeGameModeComponent.Cast(component);

		if (!nightMode.IsGlobalNightModeEnabled())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteValue("globalNightMode", true);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_NightModeGameModeComponent nightMode = SCR_NightModeGameModeComponent.Cast(component);

		int version;
		context.Read(version);

		bool globalNightMode;
		if (context.Read(globalNightMode))
			nightMode.EnableGlobalNightMode(true);

		return true;
	}
}
