class SCR_PerceivedFactionManagerComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_PerceivedFactionManagerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_PerceivedFactionManagerComponent perceivedFactionManager = SCR_PerceivedFactionManagerComponent.Cast(component);
		const BaseContainer source = perceivedFactionManager.GetComponentSource(owner);

		const SCR_EPerceivedFactionOutfitType characterPerceivedFactionOutfitType = perceivedFactionManager.GetCharacterPerceivedFactionOutfitType();
		SCR_EPerceivedFactionOutfitType characterPerceivedFactionOutfitTypeDefault = SCR_EPerceivedFactionOutfitType.DISABLED;
		if (source)
			source.Get("m_eCharacterPerceivedFactionOutfitType", characterPerceivedFactionOutfitTypeDefault);

		const SCR_EDisguisedKillingPunishment punishmentKillingWhileDisguisedFlags = perceivedFactionManager.GetPunishmentKillingWhileDisguisedFlags();
		SCR_EDisguisedKillingPunishment punishmentKillingWhileDisguisedFlagsDefault = 0;
		if (source)
			source.Get("m_ePunishmentKillingWhileDisguised", punishmentKillingWhileDisguisedFlagsDefault);

		if (characterPerceivedFactionOutfitType == characterPerceivedFactionOutfitTypeDefault &&
			punishmentKillingWhileDisguisedFlags == punishmentKillingWhileDisguisedFlagsDefault)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(characterPerceivedFactionOutfitType, characterPerceivedFactionOutfitTypeDefault);
		context.WriteDefault(punishmentKillingWhileDisguisedFlags, punishmentKillingWhileDisguisedFlagsDefault);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_PerceivedFactionManagerComponent perceivedFactionManager = SCR_PerceivedFactionManagerComponent.Cast(component);

		int version;
		context.Read(version);

		SCR_EDisguisedKillingPunishment characterPerceivedFactionOutfitType;
		if (context.Read(characterPerceivedFactionOutfitType))
			perceivedFactionManager.SetCharacterPerceivedFactionOutfitType_S(characterPerceivedFactionOutfitType);

		SCR_EDisguisedKillingPunishment punishmentKillingWhileDisguisedFlags;
		if (context.Read(punishmentKillingWhileDisguisedFlags))
			perceivedFactionManager.SetPunishmentKillingWhileDisguisedFlags_S(punishmentKillingWhileDisguisedFlags);

		return true;
	}
}

