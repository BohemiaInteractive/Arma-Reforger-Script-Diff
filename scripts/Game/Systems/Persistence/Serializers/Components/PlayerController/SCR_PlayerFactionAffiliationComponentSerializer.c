class SCR_PlayerFactionAffiliationComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_PlayerFactionAffiliationComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_PlayerController playerController = SCR_PlayerController.Cast(owner);
		auto playerFactionAffiliation = SCR_PlayerFactionAffiliationComponent.Cast(component);

		FactionKey factionKey = playerFactionAffiliation.GetAffiliatedFactionKey();
		const FactionKey defaultFactionKey = playerFactionAffiliation.GetDefaultFactionKey();

		const Faction faction = SCR_FactionManager.SGetPlayerFaction(playerController.GetPlayerId());
		if (faction)
			factionKey = faction.GetFactionKey();

		if (factionKey == defaultFactionKey)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.Write(factionKey);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto playerController = SCR_PlayerController.Cast(owner);
		auto playerFactionAffiliation = SCR_PlayerFactionAffiliationComponent.Cast(component);

		int version;
		context.Read(version);

		FactionKey factionKey;
		context.Read(factionKey);

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		const Faction faction = factionManager.GetFactionByKey(factionKey);
		const Faction currentFaction = SCR_FactionManager.SGetPlayerFaction(playerController.GetPlayerId());
		if (faction != currentFaction)
			playerFactionAffiliation.SetFaction_S(faction);

		return true;
	}
}
