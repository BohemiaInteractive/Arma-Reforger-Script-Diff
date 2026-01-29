class SCR_EffectsModuleComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_EffectsModuleComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_EffectsModuleComponent effectComponent = SCR_EffectsModuleComponent.Cast(component);
		const SCR_EffectsModule effectModule = effectComponent.GetEffectsModuleConfig();

		const bool paused = effectModule.IsPaused();

		int barragesLeft = -1;
		int totalProjectilesInBarrage = -1;
		int projectilesLeftInBarrage = -1;
		const SCR_BarrageEffectsModule barrage = SCR_BarrageEffectsModule.Cast(effectModule);
		if (barrage)
		{
			barragesLeft = barrage.GetBarragesLeft();

			if (barragesLeft > 0)
			{
				totalProjectilesInBarrage = barrage.GetTotalProjectilesInBarrage();
				projectilesLeftInBarrage = barrage.GetProjectilesLeftInBarrage();
			}
		}

		if (!paused && barragesLeft == -1 && projectilesLeftInBarrage == -1)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);

		if (effectModule.CanPause())
			context.WriteDefault(paused, false);

		context.WriteDefault(barragesLeft, -1);
		if (barragesLeft > 0)
		{
			context.WriteDefault(totalProjectilesInBarrage, -1);
			context.WriteDefault(projectilesLeftInBarrage, -1);
		}

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_EffectsModuleComponent effectComponent = SCR_EffectsModuleComponent.Cast(component);
		const SCR_EffectsModule effectModule = effectComponent.GetEffectsModuleConfig();

		int version;
		context.Read(version);

		if (effectModule.CanPause())
		{
			bool paused;
			if (context.Read(paused) && paused)
				effectModule.SetPaused(true);
		}

		int barragesLeft;
		context.ReadDefault(barragesLeft, -1);

		int totalProjectilesInBarrage = -1;
		int projectilesLeftInBarrage = -1;
		if (barragesLeft > 0)
		{
			context.ReadDefault(totalProjectilesInBarrage, -1);
			context.ReadDefault(projectilesLeftInBarrage, -1);
		}

		const SCR_BarrageEffectsModule barrage = SCR_BarrageEffectsModule.Cast(effectModule);
		if (barrage)
		{
			if (barragesLeft != -1)
				barrage.SetBarragesLeft(barragesLeft);

			if (totalProjectilesInBarrage != -1)
				barrage.SetTotalProjectilesInBarrage(totalProjectilesInBarrage);

			if (projectilesLeftInBarrage != -1)
				barrage.SetProjectilesLeftInBarrage(projectilesLeftInBarrage);
		}

		return true;
	}
}
