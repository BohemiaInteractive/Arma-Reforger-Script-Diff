class SCR_AdditionalGameModeSettingsComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_AdditionalGameModeSettingsComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.Cast(component);
		const BaseContainer source = settings.GetComponentSource(owner);

		const bool vehicleSalvageEnabled = settings.IsVehicleSalvageEnabled();
		bool vehicleSalvageEnabledDefault = true;
		if (source)
			source.Get("m_bEnableVehicleSalvage", vehicleSalvageEnabledDefault);
		
		const bool entityRefundingActionAllowed = settings.IsEntityRefundingActionAllowed();
		bool entityRefundingActionAllowedDefault = true;
		if (source)
			source.Get("m_bAllowEntityRefundingAction", entityRefundingActionAllowedDefault);

		const bool teamKillingPunished = settings.IsTeamKillingPunished();
		bool teamKillingPunishedDefault = true;
		if (source)
			source.Get("m_bEnableTeamKillPunishment", teamKillingPunishedDefault);

		const bool projectileBallisticInfoVisibility = settings.GetProjectileBallisticInfoVisibility();
		bool projectileBallisticInfoVisibilityDefault = true;
		if (source)
			source.Get("m_bProjectileBallisticInfoVisibility", projectileBallisticInfoVisibilityDefault);

		const SCR_ENametagControllerFilter nametagControllerFilters = settings.GetNametagControllerFilters();
		SCR_ENametagControllerFilter nametagControllerFiltersDefault = SCR_ENametagControllerFilter.SHOW_AI | SCR_ENametagControllerFilter.SHOW_PLAYERS;
		if (source)
			source.Get("m_eNametagControllerFilters", nametagControllerFiltersDefault);

		const SCR_ENametagRelationFilter nametagRelationFilters = settings.GetNametagRelationFilters();
		SCR_ENametagRelationFilter nametagRelationFiltersDefault = SCR_ENametagRelationFilter.SHOW_SAME_FACTION | SCR_ENametagRelationFilter.SHOW_GROUP_MEMBERS | SCR_ENametagRelationFilter.SHOW_MILITARY_ALLIES;
		if (source)
			source.Get("m_eNametagRelationFilters", nametagRelationFiltersDefault);

		// Not directly on component but we save it here anyway
		const float fuelConsumptionScale = SCR_FuelConsumptionComponent.GetGlobalFuelConsumptionScale();
		const float fuelConsumptionScaleDefault = 8.0;
		
		if (vehicleSalvageEnabled == vehicleSalvageEnabledDefault &&
			entityRefundingActionAllowed == entityRefundingActionAllowedDefault &&
			teamKillingPunished == teamKillingPunishedDefault &&
			projectileBallisticInfoVisibility == projectileBallisticInfoVisibilityDefault &&
			nametagControllerFilters == nametagControllerFiltersDefault &&
			fuelConsumptionScale == fuelConsumptionScaleDefault)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(vehicleSalvageEnabled, vehicleSalvageEnabledDefault);
		context.WriteDefault(entityRefundingActionAllowed, entityRefundingActionAllowedDefault);
		context.WriteDefault(teamKillingPunished, teamKillingPunishedDefault);
		context.WriteDefault(projectileBallisticInfoVisibility, projectileBallisticInfoVisibilityDefault);
		context.WriteDefault(nametagControllerFilters, nametagControllerFiltersDefault);
		context.WriteDefault(nametagRelationFilters, nametagRelationFiltersDefault);
		context.WriteDefault(fuelConsumptionScale, fuelConsumptionScaleDefault);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.Cast(component);

		int version;
		context.Read(version);

		bool vehicleSalvageEnabled;
		if (context.Read(vehicleSalvageEnabled))
			settings.SetEnableVehicleSalvage_S(vehicleSalvageEnabled);

		bool entityRefundingActionAllowed;
		if (context.Read(entityRefundingActionAllowed))
			settings.SetAllowEntityRefundingAction_S(entityRefundingActionAllowed);

		bool teamKillingPunished;
		if (context.Read(teamKillingPunished))
			settings.SetEnableTeamKillPunishment_S(teamKillingPunished);
		
		bool projectileBallisticInfoVisibility;
		if (context.Read(projectileBallisticInfoVisibility))
			settings.SetProjectileBallisticInfoVisibility_S(projectileBallisticInfoVisibility);

		SCR_ENametagControllerFilter nametagControllerFilters;
		if (context.Read(nametagControllerFilters))
			settings.SetNametagControllerFilters(nametagControllerFilters);

		SCR_ENametagControllerFilter nametagRelationFilters;
		if (context.Read(nametagRelationFilters))
			settings.SetNametagRelationFilters(nametagRelationFilters);

		float fuelConsumptionScale;
		if (context.Read(fuelConsumptionScale))
			SCR_FuelConsumptionComponent.SetGlobalFuelConsumptionScale(fuelConsumptionScale);

		return true;
	}
}
