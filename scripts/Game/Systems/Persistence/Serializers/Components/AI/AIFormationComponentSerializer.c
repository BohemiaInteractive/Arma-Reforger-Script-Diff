class AIFormationComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return AIFormationComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const AIFormationComponent aiFormation = AIFormationComponent.Cast(component);
		auto formation = aiFormation.GetFormation();
		if (!formation)
			return ESerializeResult.DEFAULT;

		string defaultFormation;
		if (component.GetComponentSource(owner).Get("DefaultFormation", defaultFormation) && defaultFormation == formation.GetName())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteValue("formation", formation.GetName());
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		AIFormationComponent aiFormation = AIFormationComponent.Cast(component);

		int version;
		context.Read(version);

		string formation;
		context.Read(formation);

		aiFormation.SetFormation(formation);
		return true;
	}
}
