class SCR_GameModeHealthSettingsSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_GameModeHealthSettings;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_GameModeHealthSettings settings = SCR_GameModeHealthSettings.Cast(component);
		const BaseContainer source = settings.GetComponentSource(owner);

		const float bleedingScale = settings.GetBleedingScale();
		float bleedingScaleDefault = 1.0;
		if (source)
			source.Get("m_fDOTScale", bleedingScaleDefault);

		const float regenScale = settings.GetRegenScale();
		float regenScaleDefault = 1.0;
		if (source)
			source.Get("m_fRegenScale", regenScaleDefault);

		const bool unconsciousnessPermitted = settings.IsUnconsciousnessPermitted();
		bool unconsciousnessPermittedDefault = true;
		if (source)
			source.Get("m_bPermitUnconsciousness", unconsciousnessPermittedDefault);

		const bool unconsciousnessPermittedVON = settings.IsUnconsciousVONPermitted();
		bool unconsciousnessPermittedVONDefault = false;
		if (source)
			source.Get("m_bPermitUnconsciousVON", unconsciousnessPermittedVONDefault);

		const float tourniquettedLegMovementSlowdow = settings.GetTourniquettedLegMovementSlowdown();
		float tourniquettedLegMovementSlowdowDefault = 0.75;
		if (source)
			source.Get("m_fTourniquettedLegMovementSlowdown", tourniquettedLegMovementSlowdowDefault);

		const float tourniquetStrengthMultiplier = settings.GetTourniquetStrengthMultiplier();
		float tourniquetStrengthMultiplierDefault = 0.1;
		if (source)
			source.Get("m_fTourniquetStrengthMultiplier", tourniquetStrengthMultiplierDefault);

		const float compartmentRegenRateMultiplier = settings.GetCompartmentRegenRateMultiplier();
		float compartmentRegenRateMultiplierDefault = 1.0;
		if (source)
			source.Get("m_fCompartmentRegenerationRateMultiplier", compartmentRegenRateMultiplierDefault);

		const float regenerationDelay = settings.GetRegenerationDelay();
		float regenerationDelayDefault = 10.0;
		if (source)
			source.Get("m_fRegenerationDelay", regenerationDelayDefault);

		const float resilienceHzRegenScale = settings.GetResilienceHzRegenScale();
		float resilienceHzRegenScaleDefault = 1.0;
		if (source)
			source.Get("m_fRegenerationScale", resilienceHzRegenScaleDefault);

		if (bleedingScale == bleedingScaleDefault &&
			regenScale == regenScaleDefault &&
			unconsciousnessPermitted == unconsciousnessPermittedDefault &&
			unconsciousnessPermittedVON == unconsciousnessPermittedVONDefault &&
			tourniquettedLegMovementSlowdow == tourniquettedLegMovementSlowdowDefault &&
			tourniquetStrengthMultiplier == tourniquetStrengthMultiplierDefault &&
			compartmentRegenRateMultiplier == compartmentRegenRateMultiplierDefault &&
			regenerationDelay == regenerationDelayDefault &&
			resilienceHzRegenScale == resilienceHzRegenScaleDefault)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(bleedingScale, bleedingScaleDefault);
		context.WriteDefault(regenScale, regenScaleDefault);
		context.WriteDefault(unconsciousnessPermitted, unconsciousnessPermittedDefault);
		context.WriteDefault(unconsciousnessPermittedVON, unconsciousnessPermittedVONDefault);
		context.WriteDefault(tourniquettedLegMovementSlowdow, tourniquettedLegMovementSlowdowDefault);
		context.WriteDefault(tourniquetStrengthMultiplier, tourniquetStrengthMultiplierDefault);
		context.WriteDefault(compartmentRegenRateMultiplier, compartmentRegenRateMultiplierDefault);
		context.WriteDefault(regenerationDelay, regenerationDelayDefault);
		context.WriteDefault(resilienceHzRegenScale, resilienceHzRegenScaleDefault);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_GameModeHealthSettings settings = SCR_GameModeHealthSettings.Cast(component);

		int version;
		context.Read(version);

		float bleedingScale;
		if (context.Read(bleedingScale))
			settings.SetBleedingScale(bleedingScale);

		float regenScale;
		if (context.Read(regenScale))
			settings.SetRegenScale(regenScale);

		bool unconsciousnessPermitted;
		if (context.Read(unconsciousnessPermitted))
			settings.SetUnconsciousnessPermitted(unconsciousnessPermitted);

		bool unconsciousnessPermittedVON;
		if (context.Read(unconsciousnessPermittedVON))
			settings.SetUnconsciousVONPermitted(unconsciousnessPermittedVON);

		float tourniquettedLegMovementSlowdow;
		if (context.Read(tourniquettedLegMovementSlowdow))
			settings.SetTourniquettedLegMovementSlowdown(tourniquettedLegMovementSlowdow);

		float tourniquetStrengthMultiplier;
		if (context.Read(tourniquetStrengthMultiplier))
			settings.SetTourniquetStrengthMultiplier(tourniquetStrengthMultiplier);

		float compartmentRegenRateMultiplier;
		if (context.Read(compartmentRegenRateMultiplier))
			settings.SetCompartmentRegenRateMultiplier(compartmentRegenRateMultiplier);

		float regenerationDelay;
		if (context.Read(regenerationDelay))
			settings.SetRegenerationDelay(regenerationDelay);

		float resilienceHzRegenScale;
		if (context.Read(resilienceHzRegenScale))
			settings.SetResilienceHzRegenScale(resilienceHzRegenScale);

		return true;
	}
}
