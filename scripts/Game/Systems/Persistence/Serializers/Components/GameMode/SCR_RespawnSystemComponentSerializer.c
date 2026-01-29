class SCR_RespawnSystemComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_RespawnSystemComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.Cast(component);
		const BaseContainer source = respawnSystem.GetComponentSource(owner);

		const bool respawnEnabled = respawnSystem.IsRespawnEnabled();
		bool respawnEnabledDefault = true;
		if (source)
			source.Get("m_bEnableRespawn", respawnEnabledDefault);

		// Not directly on component but info belongs to it anyway
		float respawnTime = 10.0;
		float respawnTimeDefault = 10.0;
		const SCR_RespawnTimerComponent respawnTimeComponent = SCR_RespawnTimerComponent.Cast(owner.FindComponent(SCR_RespawnTimerComponent));
		if (respawnTimeComponent)
		{
			respawnTime = respawnTimeComponent.GetRespawnTime();
			const BaseContainer timerSource = respawnTimeComponent.GetComponentSource(owner);
			if (timerSource)
				timerSource.Get("m_fRespawnTime", respawnTimeDefault);
		}

		if (respawnEnabled == respawnEnabledDefault && float.AlmostEqual(respawnTime, respawnTimeDefault))
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(respawnEnabled, respawnEnabledDefault);
		context.WriteDefault(respawnTime, respawnTimeDefault);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.Cast(component);

		int version;
		context.Read(version);

		bool respawnEnabled;
		if (context.Read(respawnEnabled))
			respawnSystem.ServerSetEnableRespawn(respawnEnabled);

		float respawnTime;
		if (context.Read(respawnTime))
		{
			const SCR_RespawnTimerComponent respawnTimeComponent = SCR_RespawnTimerComponent.Cast(owner.FindComponent(SCR_RespawnTimerComponent));
			if (respawnTimeComponent)
				respawnTimeComponent.SetRespawnTime(respawnTime);
		}

		return true;
	}
}
