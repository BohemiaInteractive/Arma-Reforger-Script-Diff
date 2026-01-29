class SCR_CampaignMilitaryBaseComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_CampaignMilitaryBaseComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_CampaignMilitaryBaseComponent militaryBase = SCR_CampaignMilitaryBaseComponent.Cast(component);

		const bool isHQ = militaryBase.IsHQ();
		const UUID buildingId = GetSystem().GetId(militaryBase.GetBaseBuildingComposition());
		const int callsign = militaryBase.GetCallsign();
		const bool builtByPlayers = militaryBase.GetBuiltByPlayers();
		const FactionKey builtFaction = militaryBase.GetBuiltFaction();

		if (!isHQ && buildingId.IsNull() && callsign == SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN && !builtByPlayers && builtFaction.IsEmpty())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteDefault(isHQ, false);
		context.WriteDefault(buildingId, UUID.NULL_UUID);
		context.WriteDefault(callsign, SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN);
		context.WriteDefault(builtByPlayers, false);
		context.WriteDefault(builtFaction, FactionKey.Empty);

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		SCR_CampaignMilitaryBaseComponent militaryBase = SCR_CampaignMilitaryBaseComponent.Cast(component);

		int version;
		context.Read(version);

		bool isHQ;
		if (context.Read(isHQ))
			militaryBase.SetAsHQ(isHQ);

		UUID buildingId;
		if (context.Read(buildingId) && !buildingId.IsNull())
		{
			Tuple1<SCR_CampaignMilitaryBaseComponent> ctx(militaryBase);
			PersistenceWhenAvailableTask task(OnBuildingAvailable, ctx);
			GetSystem().WhenAvailable(buildingId, task);
		}

		int callsign;
		context.Read(callsign);
		militaryBase.SetCallsignIndex(callsign);

		bool builtByPlayers;
		if (context.Read(builtByPlayers))
			militaryBase.SetBuiltByPlayers(builtByPlayers);
		
		FactionKey builtFaction;
		if (context.Read(builtFaction))
		{
			const Faction faction = GetGame().GetFactionManager().GetFactionByKey(builtFaction);
			if (faction)
				militaryBase.SetBuiltFaction(faction);
		}

		militaryBase.Initialize();

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (campaign)
			campaign.GetBaseManager().UpdateBases();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnBuildingAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto building = IEntity.Cast(instance);
		if (!building)
			return;

		auto ctx = Tuple1<SCR_CampaignMilitaryBaseComponent>.Cast(context);
		if (ctx.param1)
			ctx.param1.SetBaseBuildingComposition(building);
	}
}
