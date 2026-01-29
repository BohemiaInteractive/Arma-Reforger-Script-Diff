class SCR_AIGroupSerializer : ScriptedEntitySerializer
{
	[Attribute("60.0", desc: "Maximum time after group creation a player has time to be reconnected to automatically join back into it.")]
	protected float m_fMaxPlayerReconnectTime;

	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_AIGroup;
	}

	//------------------------------------------------------------------------------------------------
	override ESerializeResult SerializeSpawnData(notnull IEntity entity, notnull BaseSerializationSaveContext context, SerializerDefaultSpawnData defaultData)
	{
		context.WriteValue("version", 1);
		context.WriteValue("prefab", SCR_ResourceNameUtils.GetPrefabName(entity));
		return ESerializeResult.DEFAULT;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity entity, notnull BaseSerializationSaveContext context)
	{
		const SCR_AIGroup group = SCR_AIGroup.Cast(entity);

		// Skip commander group that is managed by commander system in conflict
		const SCR_EGroupRole role = group.GetGroupRole();
		const SCR_GameModeCampaign campaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (role == SCR_EGroupRole.COMMANDER && campaign)
			return ESerializeResult.DEFAULT;

		const bool useCommanding = SCR_CommandingManagerComponent.GetInstance() != null;
		const UUID commandingGroup = GetSystem().GetId(group.GetMaster());

		array<UUID> aiMembers();
		array<AIAgent> outAgents();
		group.GetAgents(outAgents);
		foreach (auto agent : outAgents)
		{
			const IEntity character = agent.GetControlledEntity();
			if (EntityUtils.IsPlayer(character))
				continue; // Players are stored via player controller so we can decide if and how they join on reconnect.

			const UUID uuid = GetSystem().GetId(character);
			if (!uuid.IsNull())
				aiMembers.Insert(uuid);
		}

		array<UUID> waypoints();
		array<AIWaypoint> outWaypoints();
		group.GetWaypoints(outWaypoints);
		foreach (auto waypoint : outWaypoints)
		{
			const UUID uuid = GetSystem().GetId(waypoint);
			if (!uuid.IsNull())
				waypoints.Insert(uuid);
		}

		FactionKey factionKey;
		auto faction = SCR_Faction.Cast(group.GetFaction());
		if (faction)
			factionKey = faction.GetFactionKey();

		const bool playable = group.IsPlayable();
		const bool preDefined = group.IsPredefinedGroup();

		const array<int> playerIds = group.GetPlayerIDs();

		// Ensure the leader id is at idx 0
		if (playerIds.RemoveItem(group.GetLeaderID()))
			playerIds.InsertAt(group.GetLeaderID(), 0);

		if (aiMembers.IsEmpty() && (!playable || preDefined) && playerIds.IsEmpty())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.Write(factionKey);

		if (useCommanding)
			context.WriteDefault(commandingGroup, UUID.NULL_UUID);

		if (!aiMembers.IsEmpty() || !context.CanSeekMembers())
			context.Write(aiMembers);

		if (!waypoints.IsEmpty() || !context.CanSeekMembers())
			context.Write(waypoints);

		if (playable)
		{
			const int radioFrequency = group.GetRadioFrequency();
			const string flag = group.GetGroupFlag();
			const bool isPrivate = group.IsPrivate();
			const bool isPrivateChangeable = group.IsPrivacyChangeable();
			const string customName = group.GetCustomName();
			const string customDesc = group.GetCustomDescription();
			const int maxMembers = group.GetMaxMembers();
			const SCR_ECharacterRank requiredRank = group.GetRequiredRank();
			const bool deleteIfNoPlayer = group.GetDeleteIfNoPlayer();

			UUID rallyBase = UUID.NULL_UUID;
			const bool rallyForced = group.IsRallyPointForced();
			if (campaign)
			{
				const SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
				if (baseManager)
				{
					const SCR_CampaignMilitaryBaseComponent base = baseManager.FindBaseByCallsign(group.GetRallyPointId());
					if (base)
						rallyBase = GetSystem().GetId(base.GetOwner());
				}
			}

			context.Write(radioFrequency);
			context.WriteDefault(preDefined, false);
			context.WriteDefault(role, SCR_EGroupRole.NONE);
			context.WriteDefault(flag, string.Empty);
			context.WriteDefault(isPrivate, false);
			context.WriteDefault(isPrivateChangeable, false);
			context.WriteDefault(customName, string.Empty);
			context.WriteDefault(customDesc, string.Empty);
			context.WriteDefault(maxMembers, 0);
			context.WriteDefault(requiredRank, SCR_ECharacterRank.PRIVATE);
			context.WriteDefault(deleteIfNoPlayer, true);
			context.WriteDefault(rallyBase, UUID.NULL_UUID);
			if (!rallyBase.IsNull())
				context.WriteDefault(rallyForced, false);

			array<UUID> playerIdentities();
			foreach (auto playerId : playerIds)
			{
				UUID playerIdentity = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
				if (!playerIdentity.IsNull())
					playerIdentities.Insert(playerIdentity);
			}

			if (!playerIdentities.IsEmpty() || !context.CanSeekMembers())
				context.Write(playerIdentities);
		}

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override bool DeserializeSpawnData(out ResourceName prefab, out EntitySpawnParams params, notnull BaseSerializationLoadContext context)
	{
		int version;
		context.Read(version);
		context.ReadValue("prefab", prefab);

		// Avoid group prefab spawning child units automatically, persistence logic will handle it.
		SCR_AIGroup.IgnoreSpawning(true);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity entity, notnull BaseSerializationLoadContext context)
	{
		// Allow group spawning again
		SCR_AIGroup.IgnoreSpawning(false);

		auto group = SCR_AIGroup.Cast(entity);
		group.ActivateAI();

		int version;
			context.Read(version);

		FactionKey factionKey;
		context.Read(factionKey);
		Faction faction = GetGame().GetFactionManager().GetFactionByKey(factionKey);
		if (faction)
			group.SetFaction(faction);

		UUID commandingGroup = UUID.NULL_UUID;
		auto commanding = SCR_CommandingManagerComponent.GetInstance();
		if (commanding)
		{
			context.ReadDefault(commandingGroup, UUID.NULL_UUID);
			if (!commandingGroup.IsNull())
			{
				Tuple1<SCR_AIGroup> commandingContext(group);
				PersistenceWhenAvailableTask commandingTask(OnCommandingAvailable, commandingContext);
				GetSystem().WhenAvailable(commandingGroup, commandingTask);
			}
		}

		// Join back all aiMembers
		array<UUID> aiMembers();
		context.Read(aiMembers);
		foreach (int idx, auto member : aiMembers)
		{
			Tuple2<SCR_AIGroup, bool> memberContext(group, idx == 0); //Leader at idx 0 of aiMembers
			PersistenceWhenAvailableTask memberTask(OnAiMemberAvailable, memberContext);
			GetSystem().WhenAvailable(member, memberTask);
		}

		// Reconnect to all waypoints in the right order. For now we simply unlink all waypoints
		array<AIWaypoint> outWaypoints();
		group.GetWaypoints(outWaypoints);
		foreach (auto waypoint : outWaypoints)
		{
			group.RemoveWaypoint(waypoint);
		}

		array<UUID> waypoints();
		context.Read(waypoints);
		foreach (int idx, auto waypoint : waypoints)
		{
			Tuple2<SCR_AIGroup, int> waypointContext(group, idx);
			PersistenceWhenAvailableTask waypointTask(OnWaypointAvailable, waypointContext);
			GetSystem().WhenAvailable(waypoint, waypointTask);
		}

		if (group.IsPlayable())
		{
			int radioFrequency;
			context.Read(radioFrequency);
			group.SetRadioFrequency(radioFrequency);

			bool preDefined;
			if (context.Read(preDefined))
				group.SetPredefinedGroup(preDefined);

			SCR_EGroupRole role;
			if (context.Read(role))
				group.SetGroupRole(role);

			string flag;
			context.ReadDefault(flag, string.Empty);
			if (!flag.IsEmpty())
			{
				group.SetCustomGroupFlag(flag);
				if (!flag.StartsWith("{"))
					group.SetFlagIsFromImageSet(true);
			}

			bool isPrivate;
			if (context.Read(isPrivate))
				group.SetPrivate(isPrivate);

			bool isPrivateChangeable;
			if (context.Read(isPrivateChangeable))
				group.SetPrivacyChangeable(isPrivateChangeable);

			string customName;
			context.ReadDefault(customName, string.Empty);

			string customDesc;
			context.ReadDefault(customDesc, string.Empty);

			int maxMembers;
			if (context.Read(maxMembers))
				group.SetMaxGroupMembers(maxMembers);

			SCR_ECharacterRank requiredRank;
			if (context.Read(requiredRank))
				group.SetRequiredRank(requiredRank);

			bool deleteIfNoPlayer;
			context.ReadDefault(deleteIfNoPlayer, true);
			group.SetCanDeleteIfNoPlayer(deleteIfNoPlayer);

			UUID rallyBase;
			context.ReadDefault(rallyBase, UUID.NULL_UUID);
			if (!rallyBase.IsNull())
			{
				bool rallyForced;
				context.ReadDefault(rallyForced, false);

				Tuple2<SCR_AIGroup, bool> rallyContext(group, rallyForced);
				PersistenceWhenAvailableTask rallyTask(OnRallyBaseAvailable, rallyContext);
				GetSystem().WhenAvailable(rallyBase, rallyTask);
			}

			auto groupsManager = SCR_GroupsManagerComponent.GetInstance();
			if (groupsManager)
			{
				groupsManager.AssignGroupID(group);
				groupsManager.RegisterGroup(group);
				groupsManager.ClaimFrequency(radioFrequency, faction);
				groupsManager.OnGroupCreated(group);
			}

			array<UUID> playerIdentities();
			context.Read(playerIdentities);
			foreach (int idx, auto playerIdentity : playerIdentities)
			{
				Tuple4<SCR_AIGroup, bool, string, string> playerContext(group, idx == 0, string.Empty, string.Empty); // Player Identity 0 is from the leader
				if (playerContext.param2)
				{
					// Name and desc are only re-applied if the leader joins back as per UGC requirements
					playerContext.param3 = customName;
					playerContext.param4 = customDesc;
				}

				PersistenceWhenAvailableTask AddPlayerIdTask(OnPlayerMemberAvailable, playerContext);
				GetSystem().WhenAvailable(playerIdentity, AddPlayerIdTask, m_fMaxPlayerReconnectTime);
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnPlayerMemberAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto playerController = SCR_PlayerController.Cast(instance);
		if (!playerController)
			return;

		auto playerContext = Tuple4<SCR_AIGroup, bool, string, string>.Cast(context);
		if (!playerContext.param1)
			return; // Group no longer exists

		auto playerGroupController = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!playerGroupController)
			return;

		const int groupId = playerContext.param1.GetGroupID();
		playerGroupController.RPC_AskJoinGroup(groupId);
		if (playerGroupController.GetGroupID() != groupId)
			return; // Failed to join group - e.g. was full already.

		if (playerContext.param2 || playerContext.param1.GetLeaderID() == -1)
		{
			const int playerId = playerController.GetPlayerId();
			playerContext.param1.SetGroupLeader(playerId);

			if (!playerContext.param3.IsEmpty())
				playerContext.param1.SetCustomName(playerContext.param3, playerId);

			if (!playerContext.param4.IsEmpty())
				playerContext.param1.SetCustomDescription(playerContext.param4, playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnAiMemberAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto member = SCR_ChimeraCharacter.Cast(instance);
		if (!member)
			return;

		auto aiControl = AIControlComponent.Cast(member.FindComponent(AIControlComponent));
		if (!aiControl)
			return;

		auto aiAgent = aiControl.GetControlAIAgent();
		aiAgent.ActivateAI();

		auto groupContext = Tuple2<SCR_AIGroup, bool>.Cast(context);
		if (!groupContext.param1)
			return; // Group gone

		groupContext.param1.AddAgent(aiAgent);

		if (groupContext.param2)
			groupContext.param1.SetNewLeader(aiAgent);

		// Commanding
		if (groupContext.param1.GetMaster())
		{
			auto aiMembers = groupContext.param1.GetAIMembers();
			if (!aiMembers.Contains(member))
			{
				aiMembers.Insert(member);
				member.SetRecruited(true);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnWaypointAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto waypoint = AIWaypoint.Cast(instance);
		if (!waypoint)
			return;

		auto waypointContext = Tuple2<SCR_AIGroup, int>.Cast(context);
		if (waypointContext.param1)
			waypointContext.param1.AddWaypointAt(waypoint, waypointContext.param2);
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnCommandingAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto masterGroup = SCR_AIGroup.Cast(instance);
		if (!masterGroup)
			return;

		auto commandingContext = Tuple1<SCR_AIGroup>.Cast(context);
		if (!commandingContext.param1)
			return;

		auto masterRpl = RplComponent.Cast(masterGroup.FindComponent(RplComponent));
		auto slaveRpl = RplComponent.Cast(commandingContext.param1.FindComponent(RplComponent));
		if (!masterRpl || !slaveRpl)
			return;

		auto groupsManager = SCR_GroupsManagerComponent.GetInstance();
		groupsManager.RequestSetGroupSlave(masterRpl.Id(), slaveRpl.Id());

		// Go through all members of the slave group and ensure they are added to the scripted array and marked as recruited.
		// If a member is loaded after this setup the individual member available callback will recheck and add them.
		array<AIAgent> outAgents();
		commandingContext.param1.GetAgents(outAgents);
		auto aiMembers = commandingContext.param1.GetAIMembers();
		foreach (auto agent : outAgents)
		{
			auto member = SCR_ChimeraCharacter.Cast(agent.GetControlledEntity());
			if (!aiMembers.Contains(member))
			{
				aiMembers.Insert(member);
				member.SetRecruited(true);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnRallyBaseAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto owner = IEntity.Cast(instance);
		if (!owner)
			return;

		auto base = SCR_CampaignMilitaryBaseComponent.Cast(owner.FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (!base)
			return;

		auto rallyContext = Tuple2<SCR_AIGroup, bool>.Cast(context);
		if (rallyContext.param1)
			rallyContext.param1.SetRallyPoint(base, rallyContext.param2);
	}
}
