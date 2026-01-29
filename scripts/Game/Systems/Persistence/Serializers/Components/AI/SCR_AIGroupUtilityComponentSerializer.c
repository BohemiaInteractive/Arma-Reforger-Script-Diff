class SCR_AIGroupUtilityComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_AIGroupUtilityComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_AIGroupUtilityComponent aiGroupUtility = SCR_AIGroupUtilityComponent.Cast(component);
		
		const float autonomousDistance = aiGroupUtility.GetMaxAutonomousDistance();
		const EAIGroupCombatMode combatMode = aiGroupUtility.GetCombatModeExternal();

		if (autonomousDistance == SCR_AIGroupUtilityComponent.AUTONOMOUS_DISTANCE_MAX_DEFAULT && 
			combatMode == EAIGroupCombatMode.FIRE_AT_WILL)
		{
			return ESerializeResult.DEFAULT;
		}

		context.WriteValue("version", 1);
		context.WriteDefault(autonomousDistance, SCR_AIGroupUtilityComponent.AUTONOMOUS_DISTANCE_MAX_DEFAULT);
		context.WriteDefault(combatMode, EAIGroupCombatMode.FIRE_AT_WILL);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto aiGroupUtility = SCR_AIGroupUtilityComponent.Cast(component);

		int version;
		context.Read(version);

		float autonomousDistance;
		context.ReadDefault(autonomousDistance, SCR_AIGroupUtilityComponent.AUTONOMOUS_DISTANCE_MAX_DEFAULT);
		
		EAIGroupCombatMode combatMode;
		context.ReadDefault(combatMode, EAIGroupCombatMode.FIRE_AT_WILL);
		
		if (autonomousDistance != SCR_AIGroupUtilityComponent.AUTONOMOUS_DISTANCE_MAX_DEFAULT)
			aiGroupUtility.SetMaxAutonomousDistance(autonomousDistance);
		
		if (combatMode != EAIGroupCombatMode.FIRE_AT_WILL)
			aiGroupUtility.SetCombatMode(combatMode);
		
		return true;
	}
}
