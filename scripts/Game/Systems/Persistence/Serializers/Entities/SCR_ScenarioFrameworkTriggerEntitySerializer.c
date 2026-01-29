class SCR_ScenarioFrameworkTriggerEntitySerializer : GenericEntitySerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_ScenarioFrameworkTriggerEntity;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity entity, notnull BaseSerializationSaveContext context)
	{
		const SCR_BaseTriggerEntity trigger = SCR_BaseTriggerEntity.Cast(entity);

		const bool periodicQueriesEnabled = trigger.IsPeriodicQueriesEnabled();

		context.StartObject("base");
		const ESerializeResult baseResult = super.Serialize(entity, context);
		context.EndObject();
		if (baseResult == ESerializeResult.ERROR)
			return baseResult;

		if (baseResult == ESerializeResult.DEFAULT && periodicQueriesEnabled)
			return baseResult;

		context.WriteValue("version", 1);
		context.Write(periodicQueriesEnabled);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity entity, notnull BaseSerializationLoadContext context)
	{
		SCR_BaseTriggerEntity trigger = SCR_BaseTriggerEntity.Cast(entity);

		if (context.DoesObjectExist("base"))
		{
			if (!context.StartObject("base") ||
				!super.Deserialize(entity, context) ||
				!context.EndObject())
			{
				return false;
			}
		}

		int version;
		context.Read(version);

		bool periodicQueriesEnabled;
		context.Read(periodicQueriesEnabled);
		trigger.EnablePeriodicQueries(periodicQueriesEnabled);

		return true;
	}
}
