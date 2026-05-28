//! Data class for reconnecting players
class SCR_ReconnectData
{
	//! How long this reconnection data can be used
	WorldTimestamp m_ValidUntil;

	//! Entity of the returning player
	IEntity m_ReservedEntity;

	// Add modded properties here from e.g. player controller that should be reapplied on reconnect onto the new controller.
}

[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_ReconnectComponentClass : SCR_BaseGameModeComponentClass
{
}

//! Takes care of managing player reconnects in case of involuntary disconnect
//! Authority-only component attached to gamemode prefab
class SCR_ReconnectComponent : SCR_BaseGameModeComponent
{
	// How many seconds a player has time to reconnect before the data expires and the character is removed.
	protected int m_iReconnectTime = 120;

	static SCR_ReconnectComponent s_Instance;

	ref map<UUID, ref SCR_ReconnectData> m_mReconnectData = new map<UUID, ref SCR_ReconnectData>();

	//------------------------------------------------------------------------------------------------
	static SCR_ReconnectComponent GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		RplComponent rplComp = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!GetGame().InPlayMode() || !rplComp || rplComp.IsProxy())
			return;

		s_Instance = this;

		SCR_DSConfig config();
		if (GetGame().GetBackendApi().GetRunningDSConfig(config))
			m_iReconnectTime = config.operating.slotReservationTimeout;
	}

	//------------------------------------------------------------------------------------------------
	//! For how many seconds a player can reconnect before the controlled character is removed.
	int GetReconnectTimeout()
	{
		return m_iReconnectTime;
	}

	//------------------------------------------------------------------------------------------------
	void UpdateExpieryCheck()
	{
		ScriptCallQueue callqueue = GetGame().GetCallqueue();
		if (m_mReconnectData.IsEmpty())
		{
			callqueue.Remove(CheckExpiery);
			return;
		}

		// Remove expired data and associated char every second (config is in seconds interval. Less frequent check leads to inaccuracy)
		if (callqueue.GetRemainingTime(CheckExpiery) < 0)
			callqueue.CallLater(CheckExpiery, 1000, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove any data that has exceeded the max reconnect time window.
	protected void CheckExpiery()
	{
		if (m_mReconnectData.IsEmpty())
			return;

		const WorldTimestamp timestamp = GetGame().GetWorld().GetTimestamp();

		array<UUID> expired = {};
		foreach (UUID identity, SCR_ReconnectData data : m_mReconnectData)
		{
			if (!IsDataExpired(data, timestamp))
				continue;

			HandleDataExpiery(data);
			expired.Insert(identity);
		}

		if (!expired.IsEmpty())
		{
			foreach (UUID identity : expired)
			{
				m_mReconnectData.Remove(identity);
			}

			UpdateExpieryCheck();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if the players entity has been taken over for reconnection
	bool HandlePlayerDisconnect(int playerId, KickCauseCode cause)
	{
		const KickCauseGroup2 group = KickCauseCodeAPI.GetGroup(cause);
		if (group != RplKickCauseGroup.REPLICATION)
			return false;

		const int reason = KickCauseCodeAPI.GetReason(cause);
		if (reason == RplError.SHUTDOWN)
			return false;

		const SCR_ReconnectData data = StoreData(playerId);
		if (!IsDataRelevant(data))
			return false;

		const UUID identity = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		m_mReconnectData.Set(identity, data);

		UpdateExpieryCheck();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Return the character back to the player controller and assign any other data.
	//! \return True if reconnection was possible, false if respawn from scratch is needed.
	bool HandlePlayerReconnect(int playerId)
	{
		const UUID identity = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		SCR_ReconnectData data;
		if (!m_mReconnectData.Take(identity, data))
			return false;

		if (IsDataRelevant(data))
			ApplyData(playerId, data);

		UpdateExpieryCheck();
		return GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Collect information relevant to reapply after reconnection to player controller / character.
	protected notnull SCR_ReconnectData StoreData(int playerId)
	{
		SCR_ReconnectData data();
		data.m_ReservedEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (m_iReconnectTime > 0)
			data.m_ValidUntil = GetGame().GetWorld().GetTimestamp().PlusSeconds(m_iReconnectTime);

		return data;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if the data is relevant for reconnect and we should store it in the system.
	protected bool IsDataRelevant(notnull SCR_ReconnectData data)
	{
		const ChimeraCharacter character = ChimeraCharacter.Cast(data.m_ReservedEntity);
		return character && !character.GetCharacterController().IsDead();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the reconnect time window has been exceeded.
	protected bool IsDataExpired(notnull SCR_ReconnectData data, WorldTimestamp timestamp)
	{
		return data.m_ValidUntil && data.m_ValidUntil.Less(timestamp);
	}

	//------------------------------------------------------------------------------------------------
	//! Data has expired, handle graceful removal of presence in the world.
	protected void HandleDataExpiery(notnull SCR_ReconnectData data)
	{
		if (!IsDataRelevant(data))
			return; // Not relevant anymore, so we are not going to do anything with it.

		auto respawn = SCR_RespawnSystemComponent.GetInstance();
		if (respawn)
			respawn.OnPlayerEntityCleanup_S(data.m_ReservedEntity);

		RplComponent.DeleteRplEntity(data.m_ReservedEntity, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Apply still relevant data back on successful reconnect.
	protected void ApplyData(int playerId, notnull SCR_ReconnectData data)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		const ChimeraCharacter character = ChimeraCharacter.Cast(data.m_ReservedEntity);
		playerController.SetInitialMainEntity(character);

		const Faction faction = SCR_FactionManager.SGetFaction(character);
		if (faction)
		{
			SCR_PlayerFactionAffiliationComponent playerFactionComp = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));
			if (playerFactionComp)
				playerFactionComp.SetFaction_S(faction);
		}

		SCR_RespawnSystemComponent.GetInstance().EmitPlayerEntityChange_S(playerId, null, character);

		SCR_ReconnectSynchronizationComponent syncComp = SCR_ReconnectSynchronizationComponent.Cast(playerController.FindComponent(SCR_ReconnectSynchronizationComponent));
		if (syncComp)
			syncComp.CreateReconnectDialog();
	}
}
