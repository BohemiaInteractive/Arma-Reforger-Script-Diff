class SCR_GameModeCampaignSerializer : ScriptedEntitySerializer
{
	[Attribute("120.0", desc: "Maximum time a faction commander can reconnect at and automatically get his role back.")]
	protected float m_fMaxCommanderReconnectTime;
	
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_GameModeCampaign;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity entity, notnull BaseSerializationSaveContext context)
	{
		const SCR_GameModeCampaign conflict = SCR_GameModeCampaign.Cast(entity);

		const int callsignOffset = conflict.GetCallsignOffset();

		map<FactionKey, UUID> factionCommanders();
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
		{
			array<Faction> factions();
			factionManager.GetFactionsList(factions);
			foreach (auto faction : factions)
			{
				const SCR_Faction scrFaction = SCR_Faction.Cast(faction);
				if (scrFaction.IsAICommander())
					continue;

				const UUID playerIdentity = GetSystem().GetId(GetGame().GetPlayerManager().GetPlayerController(scrFaction.GetCommanderId()));
				if (!playerIdentity.IsNull())
					factionCommanders.Set(scrFaction.GetFactionKey(), playerIdentity);
			}
		}

		context.WriteValue("version", 1);
		context.Write(callsignOffset);

		if (!factionCommanders.IsEmpty() || !context.CanSeekMembers())
			context.Write(factionCommanders);

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity entity, notnull BaseSerializationLoadContext context)
	{
		auto conflict = SCR_GameModeCampaign.Cast(entity);

		int version;
		context.Read(version);

		int callsignOffset;
		context.Read(callsignOffset);
		conflict.SetCallsignOffset(callsignOffset);

		map<FactionKey, UUID> factionCommanders;
		if (context.Read(factionCommanders))
		{
			foreach (FactionKey factionKey, UUID commanderId : factionCommanders)
			{
				Tuple1<FactionKey> ctx(factionKey);
				PersistenceWhenAvailableTask task(OnPlayerAvailable,  ctx);
				GetSystem().WhenAvailable(commanderId, task, m_fMaxCommanderReconnectTime);
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnPlayerAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto playerController = PlayerController.Cast(instance);
		if (!playerController)
			return;

		SCR_FactionCommanderHandlerComponent component = SCR_FactionCommanderHandlerComponent.GetInstance();
		if (!component)
			return;

		auto factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		auto ctx = Tuple1<FactionKey>.Cast(context);
		auto faction = SCR_Faction.Cast(factionManager.GetFactionByKey(ctx.param1));
		if (faction && faction.IsAICommander())
			component.SetFactionCommander(faction, playerController.GetPlayerId());
	}
}
