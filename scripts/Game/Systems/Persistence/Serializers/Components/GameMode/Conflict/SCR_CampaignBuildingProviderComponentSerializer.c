class SCR_CampaignBuildingProviderComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_CampaignBuildingProviderComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_CampaignBuildingProviderComponent provider = SCR_CampaignBuildingProviderComponent.Cast(component);

		const int objectBudget = provider.GetCurrentPropValue();
		const int aiBudget = provider.GetCurrentAIValue();

		if (objectBudget == 0 && aiBudget == 0)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(objectBudget, 0);
		context.WriteDefault(aiBudget, 0);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_CampaignBuildingProviderComponent provider = SCR_CampaignBuildingProviderComponent.Cast(component);

		int version;
		context.Read(version);

		int objectBudget;
		if (context.Read(objectBudget))
			GetGame().GetCallqueue().Call(provider.SetPropValue, objectBudget);

		int aiBudget;
		if (context.Read(aiBudget))
			GetGame().GetCallqueue().Call(provider.SetAIValue, aiBudget);

		return true;
	}
}
