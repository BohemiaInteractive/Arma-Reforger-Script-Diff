class SCR_ArsenalManagerComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_ArsenalManagerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_ArsenalManagerComponent arsenalManager = SCR_ArsenalManagerComponent.Cast(component);
		const BaseContainer source = arsenalManager.GetComponentSource(owner);

		const float loadoutSpawnSupplyCostMultiplier = arsenalManager.GetLoadoutSpawnSupplyCostMultiplier();
		float loadoutSpawnSupplyCostMultiplierDefault = 0.0;
		if (source)
			source.Get("m_fLoadoutSpawnSupplyCostMultiplier", loadoutSpawnSupplyCostMultiplierDefault);

		const SCR_EArsenalGameModeType gameModeType = arsenalManager.GetArsenalGameModeType();
		SCR_EArsenalGameModeType gameModeTypeDefault = -1;
		if (source)
			source.Get("m_eArsenalGameModeType", gameModeTypeDefault);

		const SCR_EArsenalTypes enabledTypes = arsenalManager.GetEnabledArsenalTypes();
		SCR_EArsenalTypes enabledTypesDefault = SCR_Enum.GetFlagValues(SCR_EArsenalTypes);
		if (source)
			source.Get("m_eArsenalTypesEnabled", enabledTypesDefault);

		const bool militarySupplyAllocation = SCR_ArsenalManagerComponent.IsMilitarySupplyAllocationEnabled();
		bool militarySupplyAllocationDefault = false;
		if (source)
			source.Get("m_bUseMilitarySupplyAllocation", militarySupplyAllocationDefault);

		if (loadoutSpawnSupplyCostMultiplier == loadoutSpawnSupplyCostMultiplierDefault &&
			gameModeType == gameModeTypeDefault &&
			enabledTypes == enabledTypesDefault &&
			militarySupplyAllocation == militarySupplyAllocationDefault)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(loadoutSpawnSupplyCostMultiplier, loadoutSpawnSupplyCostMultiplierDefault);
		context.WriteDefault(gameModeType, gameModeTypeDefault);
		context.WriteDefault(enabledTypes, enabledTypesDefault);
		context.WriteDefault(militarySupplyAllocation, militarySupplyAllocationDefault);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_ArsenalManagerComponent arsenalManager = SCR_ArsenalManagerComponent.Cast(component);

		int version;
		context.Read(version);

		float loadoutSpawnSupplyCostMultiplier;
		if (context.Read(loadoutSpawnSupplyCostMultiplier))
			arsenalManager.SetLoadoutSpawnCostMultiplier_S(loadoutSpawnSupplyCostMultiplier);

		SCR_EArsenalGameModeType gameModeType;
		if (context.Read(gameModeType))
			arsenalManager.SetArsenalGameModeType_S(gameModeType);

		SCR_EArsenalTypes enabledTypes;
		if (context.Read(enabledTypes))
			arsenalManager.SetEnabledArsenalTypes(enabledTypes);

		bool militarySupplyAllocation;
		if (context.Read(militarySupplyAllocation))
			arsenalManager.SetMilitarySupplyAllocationEnabled(militarySupplyAllocation);

		return true;
	}
}
