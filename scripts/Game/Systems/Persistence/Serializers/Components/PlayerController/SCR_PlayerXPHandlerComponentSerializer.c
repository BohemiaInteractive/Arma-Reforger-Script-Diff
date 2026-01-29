class SCR_PlayerXPHandlerComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_PlayerXPHandlerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		auto playerXpHandler = SCR_PlayerXPHandlerComponent.Cast(component);

		const int xp = playerXpHandler.GetPlayerXP();
		const int xpSinceLastSpawn = playerXpHandler.GetPlayerXPSinceLastSpawn();
		const int survivalCycle = playerXpHandler.GetSurvivalRewardCycle();

		if (xp == 0 && xpSinceLastSpawn == 0 && survivalCycle == 0)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(xp, 0);
		context.WriteDefault(xpSinceLastSpawn, 0);
		context.WriteDefault(survivalCycle, 0);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto playerXpHandler = SCR_PlayerXPHandlerComponent.Cast(component);

		int version;
		context.Read(version);

		int xp;
		if (context.Read(xp))
			playerXpHandler.AddPlayerXP(SCR_EXPRewards.UNDEFINED, addDirectly: (xp - playerXpHandler.GetPlayerXP()));

		int xpSinceLastSpawn;
		if (context.Read(xpSinceLastSpawn))
			playerXpHandler.SetPlayerXPSinceLastSpawn(xpSinceLastSpawn);
		
		int survivalCycle;
		if (context.Read(survivalCycle))
			playerXpHandler.SetSurvivalRewardCycle(survivalCycle);

		return true;
	}
}
