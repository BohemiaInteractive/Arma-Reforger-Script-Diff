class FactionAffiliationComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return FactionAffiliationComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(component);

		const FactionKey factionKey = factionAffiliation.GetAffiliatedFactionKey();
		const FactionKey defaultFactionKey = factionAffiliation.GetDefaultFactionKey();

		if (factionKey == defaultFactionKey)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.Write(factionKey);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto factionAffiliation = FactionAffiliationComponent.Cast(component);

		int version;
		context.Read(version);

		FactionKey factionKey;
		context.Read(factionKey);

		const FactionKey currentFactionKey = factionAffiliation.GetAffiliatedFactionKey();
		if (factionKey != currentFactionKey)
		{
			FactionManager factionManager = GetGame().GetFactionManager();
			if (factionManager)
			{
				const Faction faction = factionManager.GetFactionByKey(factionKey);
				factionAffiliation.SetAffiliatedFaction(faction);
			}
		}

		return true;
	}
}
