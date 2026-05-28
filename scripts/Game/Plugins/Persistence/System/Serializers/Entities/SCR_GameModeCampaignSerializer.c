class SCR_GameModeCampaignSerializer : SCR_GameModeSerializer
{
	[Attribute("120.0", desc: "Maximum time a faction commander can reconnect at and automatically get his role back.")]
	protected float m_fMaxCommanderReconnectTime;

	[Attribute("60.0", desc: "How much time, if any, is added to victory countdown to account for players needing time to reconnect after loading a save.")]
	protected float m_fVictoryGracePeriod;

	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_GameModeCampaign;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity entity, notnull SaveContext context)
	{
		const SCR_GameModeCampaign conflict = SCR_GameModeCampaign.Cast(entity);

		const int callsignOffset = conflict.GetCallsignOffset();

		const WorldTimestamp currentTime = entity.GetWorld().GetTimestamp();

		map<FactionKey, UUID> factionCommanders();
		map<FactionKey, int> factionVictoryTimestamps();
		const FactionManager factionManager = GetGame().GetFactionManager();
		array<Faction> factions();
		factionManager.GetFactionsList(factions);
		foreach (auto faction : factions)
		{
			const SCR_CampaignFaction campaignFaction = SCR_CampaignFaction.Cast(faction);
			if (!campaignFaction)
				continue;

			if (!campaignFaction.IsAICommander())
			{
				const UUID playerIdentity = GetSystem().GetId(GetGame().GetPlayerManager().GetPlayerController(campaignFaction.GetCommanderId()));
				if (!playerIdentity.IsNull())
					factionCommanders.Set(campaignFaction.GetFactionKey(), playerIdentity);
			}

			const float victoryTime = Math.Max(campaignFaction.GetVictoryTimestamp().DiffSeconds(currentTime), 0.0);
			if (victoryTime)
				factionVictoryTimestamps.Set(campaignFaction.GetFactionKey(), victoryTime);
		}

		context.StartObject("base");
		const ESerializeResult baseResult = super.Serialize(entity, context);
		context.EndObject();
		if (baseResult == ESerializeResult.ERROR)
			return baseResult;

		context.WriteValue("version", 1);
		context.Write(callsignOffset);

		if (!factionCommanders.IsEmpty() || !context.CanSeekMembers())
			context.Write(factionCommanders);

		if (!factionVictoryTimestamps.IsEmpty() || !context.CanSeekMembers())
			context.Write(factionVictoryTimestamps);

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity entity, notnull LoadContext context)
	{
		auto conflict = SCR_GameModeCampaign.Cast(entity);

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

		int callsignOffset;
		context.Read(callsignOffset);
		conflict.SetCallsignOffset(callsignOffset);

		map<FactionKey, UUID> factionCommanders;
		if (context.Read(factionCommanders))
		{
			foreach (FactionKey factionKey, UUID commanderId : factionCommanders)
			{
				Tuple1<FactionKey> ctx(factionKey);
				PersistenceWhenAvailableTask task(OnPlayerAvailable, ctx);
				GetSystem().WhenAvailable(commanderId, task, m_fMaxCommanderReconnectTime);
			}
		}

		map<FactionKey, int> factionVictoryTimestamps;
		if (context.Read(factionVictoryTimestamps))
		{
			const FactionManager factionManager = GetGame().GetFactionManager();
			const WorldTimestamp currentTime = entity.GetWorld().GetTimestamp();
			foreach (FactionKey factionKey, int timestamp : factionVictoryTimestamps)
			{
				auto campaignFaction = SCR_CampaignFaction.Cast(factionManager.GetFactionByKey(factionKey));
				if (campaignFaction && timestamp > 0)
					campaignFaction.SetVictoryTimestamp(currentTime.PlusSeconds(timestamp + m_fVictoryGracePeriod));
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
