class SCR_PersistentReconnectData
{
	UUID m_sPlayerIdentity;
	float m_fRemainingValidity;
	UUID m_sReservedEntityId;

	//------------------------------------------------------------------------------------------------
	void ReadFrom(notnull PersistenceSystem persistence, notnull SCR_ReconnectData data)
	{
		if (data.m_ValidUntil)
			m_fRemainingValidity = data.m_ValidUntil.DiffSeconds(GetGame().GetWorld().GetTimestamp());

		m_sReservedEntityId = persistence.GetId(data.m_ReservedEntity);
	}

	//------------------------------------------------------------------------------------------------
	void ApplyTo(notnull PersistenceSystem persistence, notnull SCR_ReconnectData data)
	{
		if (m_fRemainingValidity > 0)
			data.m_ValidUntil = GetGame().GetWorld().GetTimestamp().PlusSeconds(m_fRemainingValidity);

		Tuple1<SCR_ReconnectData> context(data);
		PersistenceWhenAvailableTask task(OnEntityAvailable, context);
		persistence.WhenAvailable(m_sReservedEntityId, task);
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnEntityAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto entity = IEntity.Cast(instance);
		if (!entity)
			return;

		auto ctx = Tuple1<SCR_ReconnectData>.Cast(context);
		if (ctx.param1)
			ctx.param1.m_ReservedEntity = entity;
	}
}

class SCR_PlayerReconnectData : PersistentState
{
	ref set<UUID> m_aPlayerCharacters = new set<UUID>();

	//------------------------------------------------------------------------------------------------
	void HandleStateChange(EPersistenceSystemState oldState, EPersistenceSystemState newState)
	{
		if (newState != EPersistenceSystemState.ACTIVE)
			return;

		int removeAfterMs = 60000; // 1 Minute by default to connect back or character is deleted.

		// Remove after reconnect period would end
		const SCR_ReconnectComponent reconnect = SCR_ReconnectComponent.GetInstance();
		if (reconnect)
			removeAfterMs = reconnect.GetReconnectTimeout() * 1000;

		GetGame().GetCallqueue().CallLater(RemoveUnusedCharacters, removeAfterMs);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveUnusedCharacters()
	{
		const PersistenceSystem persistence = PersistenceSystem.GetInstance();
		const PlayerManager playerManager = GetGame().GetPlayerManager();
		const SCR_RespawnSystemComponent respawn = SCR_RespawnSystemComponent.GetInstance();
		foreach (UUID charId : m_aPlayerCharacters)
		{
			IEntity character = IEntity.Cast(persistence.FindById(charId));
			if (!character)
				continue;

			if (playerManager.GetPlayerIdFromControlledEntity(character) > 0)
				continue; // Player connected back and has active control

			respawn.OnPlayerEntityCleanup_S(character);
			SCR_EntityHelper.DeleteEntityAndChildren(character);
		}
		m_aPlayerCharacters.Clear();
	}
}

class SCR_PlayerReconnectDataSerializer : ScriptedStateSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_PlayerReconnectData;
	}

	//------------------------------------------------------------------------------------------------
	override ESerializeResult Serialize(notnull Managed instance, notnull SaveContext context)
	{
		const SCR_PlayerReconnectData state = SCR_PlayerReconnectData.Cast(instance);
		array<int> players();
		const PlayerManager manager = GetGame().GetPlayerManager();
		manager.GetPlayers(players);
		foreach (int player : players)
		{
			const UUID characterId = GetSystem().GetId(manager.GetPlayerControlledEntity(player));
			if (!characterId.IsNull())
				state.m_aPlayerCharacters.Insert(characterId);
		}

		array<ref SCR_PersistentReconnectData> reconnectData();
		const SCR_ReconnectComponent reconnect = SCR_ReconnectComponent.GetInstance();
		if (reconnect)
		{
			foreach (UUID playerIdentity, SCR_ReconnectData data : reconnect.m_mReconnectData)
			{
				SCR_PersistentReconnectData persistentData();
				persistentData.m_sPlayerIdentity = playerIdentity;
				persistentData.ReadFrom(GetSystem(), data);
				reconnectData.Insert(persistentData);
			}
		}

		if (state.m_aPlayerCharacters.IsEmpty() && reconnectData.IsEmpty())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);

		if (!state.m_aPlayerCharacters.IsEmpty() || !context.CanSeekMembers())
			context.WriteValue("active", state.m_aPlayerCharacters);

		if (!reconnectData.IsEmpty() || !context.CanSeekMembers())
		{
			const bool prev = context.EnableTypeDiscriminator(false);
			context.WriteValue("reconnect", reconnectData);
			context.EnableTypeDiscriminator(prev);
		}

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull LoadContext context)
	{
		SCR_PlayerReconnectData state = SCR_PlayerReconnectData.Cast(instance);

		int version;
		context.Read(version);

		context.ReadValue("active", state.m_aPlayerCharacters);
		if (!state.m_aPlayerCharacters.IsEmpty())
			SCR_PersistenceSystem.Cast(GetSystem()).GetOnStateChanged().Insert(state.HandleStateChange);

		const SCR_ReconnectComponent reconnect = SCR_ReconnectComponent.GetInstance();
		if (!reconnect)
			return true;

		array<ref SCR_PersistentReconnectData> data;
		const bool prev = context.EnableTypeDiscriminator(false);
		context.ReadValue("reconnect", data);
		context.EnableTypeDiscriminator(prev);
		if (data)
		{
			foreach (SCR_PersistentReconnectData persistentData : data)
			{
				SCR_ReconnectData entry();
				persistentData.ApplyTo(GetSystem(), entry);
				reconnect.m_mReconnectData.Set(persistentData.m_sPlayerIdentity, entry);
			}
		}

		reconnect.UpdateExpieryCheck();
		return true;
	}
}
