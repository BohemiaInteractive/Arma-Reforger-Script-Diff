class SCR_AICombatComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_AICombatComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_AICombatComponent aiCombat = SCR_AICombatComponent.Cast(component);

		const EAISkill skill = aiCombat.GetAISkill();
		const EAISkill skillDefault = aiCombat.GetAISkillDefault();
		const float perceptionFactor = aiCombat.GetPerceptionFactor();

		if (skill == skillDefault && float.AlmostEqual(perceptionFactor, 1.0))
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(skill, skillDefault);
		context.WriteDefault(perceptionFactor, 1.0);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto aiCombat = SCR_AICombatComponent.Cast(component);

		int version;
		context.Read(version);

		const EAISkill skillDefault = aiCombat.GetAISkillDefault();
		EAISkill skill;
		context.ReadDefault(skill, skillDefault);
		
		float perceptionFactor;
		context.ReadDefault(perceptionFactor, 1.0);
		
		if (skill != skillDefault)
			aiCombat.SetAISkill(skill);
		
		if (!float.AlmostEqual(perceptionFactor, 1.0))
			aiCombat.SetPerceptionFactor(perceptionFactor);

		return true;
	}
}
