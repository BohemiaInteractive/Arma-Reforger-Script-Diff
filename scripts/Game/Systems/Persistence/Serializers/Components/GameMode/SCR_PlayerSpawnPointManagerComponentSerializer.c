class SCR_PlayerSpawnPointManagerComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_PlayerSpawnPointManagerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(component);
		const BaseContainer source = playerSpawnPointManager.GetComponentSource(owner);

		const bool playerSpawnPoints = playerSpawnPointManager.IsPlayerSpawnPointsEnabled();
		bool playerSpawnPointsDefault = true;
		if (source)
			source.Get("m_bEnablePlayerSpawnPoints", playerSpawnPointsDefault);

		const bool radioVehicleSpawning = playerSpawnPointManager.IsRadioVehicleSpawningEnabled();
		bool radioVehicleSpawningDefault = false;
		if (source)
			source.Get("m_bRadioVehicleSpawningEnabled", radioVehicleSpawningDefault);

		const bool deployingSpawnPoints = playerSpawnPointManager.IsDeployingSpawnPointsEnabled();
		bool deployingSpawnPointsDefault = true;
		if (source)
			source.Get("m_bDeployableSpawnPointsEnabled", deployingSpawnPointsDefault);

		const SCR_ESpawnPointBudgetType deployableSpawnPointBudgetType = playerSpawnPointManager.GetDeployableSpawnPointBudgetType();
		SCR_ESpawnPointBudgetType deployableSpawnPointBudgetTypeDefault = SCR_ESpawnPointBudgetType.SUPPLIES;
		if (source)
			source.Get("m_eDeployableSpawnPointBudgetType", deployableSpawnPointBudgetTypeDefault);

		const int deployableSpawnPointTicketAmount = playerSpawnPointManager.GetDeployableSpawnPointTicketAmount();
		int deployableSpawnPointTicketAmountDefault = 5;
		if (source)
			source.Get("m_iDeployableSpawnPointTicketAmount", deployableSpawnPointTicketAmountDefault);

		if (playerSpawnPoints == playerSpawnPointsDefault &&
			radioVehicleSpawning == radioVehicleSpawningDefault &&
			deployingSpawnPoints == deployingSpawnPointsDefault &&
			deployableSpawnPointBudgetType == deployableSpawnPointBudgetTypeDefault &&
			deployableSpawnPointTicketAmount == deployableSpawnPointTicketAmountDefault)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(playerSpawnPoints, playerSpawnPointsDefault);
		context.WriteDefault(radioVehicleSpawning, radioVehicleSpawningDefault);
		context.WriteDefault(deployingSpawnPoints, deployingSpawnPointsDefault);
		context.WriteDefault(deployableSpawnPointBudgetType, deployableSpawnPointBudgetTypeDefault);
		context.WriteDefault(deployableSpawnPointTicketAmount, deployableSpawnPointTicketAmountDefault);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(component);

		int version;
		context.Read(version);

		bool playerSpawnPoints;
		if (context.Read(playerSpawnPoints))
			playerSpawnPointManager.EnablePlayerSpawnPoints(playerSpawnPoints);

		bool radioVehicleSpawning;
		if (context.Read(radioVehicleSpawning))
			playerSpawnPointManager.EnableRadioVehicleSpawning(radioVehicleSpawning);

		bool deployingSpawnPoints;
		if (context.Read(deployingSpawnPoints))
			playerSpawnPointManager.EnableDeployableSpawnPoints(deployingSpawnPoints);

		SCR_ESpawnPointBudgetType deployableSpawnPointBudgetType;
		if (context.Read(deployableSpawnPointBudgetType))
			playerSpawnPointManager.SetDeployableSpawnPointBudgetType(deployableSpawnPointBudgetType);

		int deployableSpawnPointTicketAmount;
		if (context.Read(deployableSpawnPointTicketAmount))
			playerSpawnPointManager.SetDeployableSpawnPointTicketAmount(deployableSpawnPointTicketAmount);

		return true;
	}
}
