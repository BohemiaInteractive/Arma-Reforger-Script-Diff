enum SCR_ESpawnLogicDisconnectBehaviour
{
	NOTHING,
	SAVE,
	DELETE
}

//------------------------------------------------------------------------------------------------
/*
	Authority:
		Object responsible for defining respawn logic.

		This object receives callbacks from parent SCR_RespawnSystemComponent that can be used
		to either spawn the player on the authority side or just notify the remote player that
		they can process to spawn, or any combination based on derived implementations.
*/
[BaseContainerProps(category: "Respawn")]
class SCR_SpawnLogic
{
	[Attribute(SCR_ESpawnLogicDisconnectBehaviour.SAVE.ToString(), UIWidgets.ComboBox, "Decide what happens to playercontroller persistence data on disconnect.", enums: ParamEnumArray.FromEnum(SCR_ESpawnLogicDisconnectBehaviour))]
	protected SCR_ESpawnLogicDisconnectBehaviour m_eDisconnectPlayerControllerBehaviour;

	[Attribute(SCR_ESpawnLogicDisconnectBehaviour.SAVE.ToString(), UIWidgets.ComboBox, "Decide what happens to character persistence data on disconnect.", enums: ParamEnumArray.FromEnum(SCR_ESpawnLogicDisconnectBehaviour))]
	protected SCR_ESpawnLogicDisconnectBehaviour m_eDisconnectCharacterBehaviour;

	protected SCR_RespawnSystemComponent m_RespawnSystem;

	protected SCR_PersistenceSystem m_Persistence;
	protected PersistenceCollection m_PlayerCollection;
	protected PersistenceCollection m_CharacterCollection;
	protected ref array<ref Tuple3<SCR_PlayerController, UUID, UUID>> m_aStoredControlledEntityIds = {};
	protected ref map<int, UUID> m_mPendingPosessions = new map<int, UUID>();

	#ifdef WORKBENCH
	protected static vector s_vPlayFromCameraPos;
	protected static vector s_vPlayFromCameraYpr;
	#endif

	//------------------------------------------------------------------------------------------------
	void OnInit(SCR_RespawnSystemComponent owner)
	{
		m_RespawnSystem = owner;
		auto gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		m_Persistence = SCR_PersistenceSystem.GetByEntityWorld(gameMode);
		if (m_Persistence)
			SetupPersistenceCollections(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Override with your own collection name if customized.
	protected void SetupPersistenceCollections(SCR_RespawnSystemComponent owner)
	{
		m_PlayerCollection = m_Persistence.FindCollection("Player");
		m_CharacterCollection = m_Persistence.FindCollection("Character");
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerRegistered_S(int playerId)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerRegistered_S(playerId: %2)", Type().ToString(), playerId);
		#endif

		SCR_RespawnComponent respawnComponent = GetPlayerRespawnComponent_S(playerId);
		respawnComponent.GetOnRespawnRequestInvoker_S().Insert(OnPlayerSpawnRequest_S);
		respawnComponent.GetOnRespawnResponseInvoker_S().Insert(OnPlayerSpawnResponse_S);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerAuditSuccess_S(int playerId)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerAuditSuccess_S(playerId: %2)", Type().ToString(), playerId);
		#endif

		if (!m_Persistence)
			return;

		// Assign players identity id to playercontroller in persistence tracking
		const SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		const UUID identity = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		m_Persistence.SetId(playerController, identity);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconnected_S(int playerId, KickCauseCode cause, int timeout)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerDisconnected_S(playerId: %2)", Type().ToString(), playerId);
		#endif

		SCR_RespawnComponent respawnComponent = GetPlayerRespawnComponent_S(playerId);
		respawnComponent.GetOnRespawnRequestInvoker_S().Remove(OnPlayerSpawnRequest_S);
		respawnComponent.GetOnRespawnResponseInvoker_S().Remove(OnPlayerSpawnResponse_S);

		if (!m_Persistence)
			return;

		auto playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		ForgetControlledEntityIds(playerController);

		switch (m_eDisconnectPlayerControllerBehaviour)
		{
			case SCR_ESpawnLogicDisconnectBehaviour.SAVE:
			{
				// Save controller data and release tracking to ignore it being deleted when player manager is done with disconnect procedure.
				m_Persistence.Save(playerController);
				m_Persistence.ReleaseTracking(playerController);
				break;
			}

			case SCR_ESpawnLogicDisconnectBehaviour.DELETE:
			{
				m_Persistence.StopTracking(playerController);
				break;
			}
		}

		/*
		If the player has a chance to reconnect, we do not need to save now
		Should the player never come back then:
			a) The body remains (dead or alive) until next auto/shutdown where it gets saved, or
			b) it is deleted by the next save and consequently removed from persistence.
		*/
		SCR_ReconnectComponent reconnect = SCR_ReconnectComponent.GetInstance();
		if (reconnect && reconnect.GetReconnectState(playerId) == SCR_EReconnectState.ENTITY_AVAILABLE)
			return;

		const IEntity character = playerController.GetControlledEntity();
		if (!character)
			return;

		switch (m_eDisconnectCharacterBehaviour)
		{
			case SCR_ESpawnLogicDisconnectBehaviour.SAVE:
			{
				// Save character data and release tracking to ignore it being deleted during player controller cleanup
				m_Persistence.Save(character);
				m_Persistence.ReleaseTracking(character);
				break;
			}

			case SCR_ESpawnLogicDisconnectBehaviour.DELETE:
			{
				m_Persistence.StopTracking(character);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerSpawnRequest_S(SCR_SpawnRequestComponent requestComponent)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerSpawnRequest_S(playerId: %2)", Type().ToString(), requestComponent.GetPlayerId());
		#endif
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerSpawnResponse_S(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerSpawnResponse_S(playerId: %2, response: %3)",
			Type().ToString(), requestComponent.GetPlayerId(), typename.EnumToString(SCR_ESpawnResult, response));
		#endif

		if (response != SCR_ESpawnResult.OK)
			OnPlayerSpawnFailed_S(requestComponent.GetPlayerId());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawnFailed_S(int playerId)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerSpawned_S(int playerId, IEntity entity)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerSpawned_S(playerId: %2, entity: %3)", Type().ToString(), playerId, entity);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerEntityChanged_S(int playerId, IEntity previousEntity, IEntity newEntity)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerEntityChanged_S(playerId: %2, previousEntity: %3, newEntity: %4)",
			Type().ToString(), playerId, previousEntity, newEntity);
		#endif

		if (!newEntity)
			OnPlayerEntityLost_S(playerId);

		if (!m_Persistence)
			return;

		// Check that the old entity does not count as player anymore
		if (previousEntity)
			m_Persistence.ReloadConfig(previousEntity);

		// Refresh config to recongize what is a player and what might be a dead char.
		if (newEntity)
			m_Persistence.ReloadConfig(newEntity);

		ApplyPendingPosession(playerId);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerKilled_S(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerKilled_S(playerId: %2, playerEntity: %3, killerEntity: %4, killerId: %5)",
			Type().ToString(), playerId, playerEntity, killerEntity, killer.GetInstigatorPlayerID());
		#endif

		OnPlayerEntityLost_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDeleted_S(int playerId, bool isDisconnect)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerDeleted_S(playerId: %2, isDisconnect:%3)", Type().ToString(), playerId, isDisconnect);
		#endif

		if (!isDisconnect)
			OnPlayerEntityLost_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Called whenever provided player loses controlled entity, this can occur e.g.
		when a player dies or their entity is deleted.
	*/
	protected void OnPlayerEntityLost_S(int playerId)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerEntityLost_S(playerId: %2)", Type().ToString(), playerId);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Notify the target player that they are ready for spawn. Useful for cases of manual spawning,
		e.g. when user should open respawn menu and similar.
	*/
	protected void NotifyPlayerReadyForSpawn(int playerId)
	{
		GetPlayerRespawnComponent_S(playerId).NotifyReadyForSpawn_S();
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyPendingPosession(int playerId)
	{
		UUID posessCharacterId;
		if (!m_mPendingPosessions.Take(playerId, posessCharacterId))
			return;

		Tuple2<int, bool> characterAvailableContext(playerId, false);
		PersistenceWhenAvailableTask linkControlledEntityTask(OnControlledCharacterAvailable, characterAvailableContext);
		m_Persistence.WhenAvailable(posessCharacterId, linkControlledEntityTask);
	}

	//------------------------------------------------------------------------------------------------
	protected void ExcuteInitialLoadOrSpawn_S(int playerId)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::ExcuteInitialLoadOrSpawn_S(playerId: %2)", Type().ToString(), playerId);
		#endif

		#ifdef WORKBENCH
		// Wait one frame for inital play from camera entity to be available (or not).
		GetGame().GetCallqueue().Call(RequestPlayerData_S, playerId);
		#else
		RequestPlayerData_S(playerId);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	protected void RequestPlayerData_S(int playerId)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::RequestPlayerData_S(playerId: %2)", Type().ToString(), playerId);
		#endif

		const PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);

		#ifdef WORKBENCH
		IEntity controlledEntity = playerController.GetControlledEntity();
		if (controlledEntity)
		{
			s_vPlayFromCameraPos = controlledEntity.GetOrigin();
			s_vPlayFromCameraYpr = controlledEntity.GetYawPitchRoll();
		}
		#endif

		if (!m_Persistence || !m_CharacterCollection)
		{
			// No persistence data to request, proceed with spawning
			OnPlayerDataLoaded_S(EPersistenceStatusCode.UNAVAILABLE, null, true, new Tuple1<PlayerController>(playerController));
			return;
		}

		const EPersistenceSystemState state = m_Persistence.GetState();
		if (state != EPersistenceSystemState.ACTIVE)
		{
			OnPlayerDataLoaded_S(EPersistenceStatusCode.UNAVAILABLE, null, true, new Tuple1<PlayerController>(playerController));
			return;
		}

		// Load existing data about the player to see which character, faction, group etc to connect him with again on load
		PersistenceLoadRequest request();
		request.Instances = {playerController};
		// Pass controller as weakptr via tuple
		PersistenceResultCallback callback(OnPlayerDataLoaded_S, new Tuple1<PlayerController>(playerController));
		m_Persistence.RequestLoad(request, callback);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerDataLoaded_S(EPersistenceStatusCode statusCode, Managed result, bool isLast, Managed context)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerDataLoaded_S(%2, %3, %4)", Type().ToString(), typename.EnumToString(EPersistenceStatusCode, statusCode), result, context);
		#endif

		auto playerController = SCR_PlayerController.Cast(Tuple1<PlayerController>.Cast(context).param1);
		if (!playerController)
			return; // Response arrived after player already disconnected
		
		const int playerId = playerController.GetPlayerId();
		if (ResolveReconnection(playerId))
		{
			ForgetControlledEntityIds(playerController);
			return; // User was reconnected, their entity was returned
		}

		if (statusCode != EPersistenceStatusCode.OK)
		{
			// Abort and proceed with default spawn
			DoInitialSpawn_S(playerId);
			return;
		}

		if (playerController != result)
			return; // Something went terribly wrong.

		UUID playerCharacterId, controlledCharacterId;
		ConsumeControlledEntityIds(playerController, playerCharacterId, controlledCharacterId);

		if (playerCharacterId.IsNull())
		{
			// Player did not have its own character, but posessed other entity (possible in e.g. GM)
			if (!controlledCharacterId.IsNull())
			{
				Tuple2<int, bool> characterAvailableContext(playerController.GetPlayerId(), true);
				PersistenceWhenAvailableTask linkControlledEntityTask(OnControlledCharacterAvailable, characterAvailableContext);
				m_Persistence.WhenAvailable(controlledCharacterId, linkControlledEntityTask);
				return;
			}

			// No player or posessed entity = abort and proceed with default spawn.
			DoInitialSpawn_S(playerController.GetPlayerId());
			return;
		}

		// Queue up posession of another entity after the main entity spawn has been completed
		if (!controlledCharacterId.IsNull() && controlledCharacterId != playerCharacterId)
			m_mPendingPosessions.Set(playerId, controlledCharacterId);

		PersistenceSpawnRequest request();
		request.Collection = m_CharacterCollection;
		request.Include = {playerCharacterId};

		Tuple1<int> playerCharContext(playerId);
		PersistenceResultCallback callback(OnPlayerCharacterLoaded_S, playerCharContext);
		m_Persistence.RequestSpawn(request, callback);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerCharacterLoaded_S(EPersistenceStatusCode statusCode, Managed result, bool isLast, Managed context)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::OnPlayerCharacterLoaded_S(%2, %3, %4)", Type().ToString(), typename.EnumToString(EPersistenceStatusCode, statusCode), result, context);
		#endif

		auto playerDataContext = Tuple1<int>.Cast(context);

		// Hand over
		auto player = BaseGameEntity.Cast(result);

		// Apply play from camera pose to new char and delete the system spawned one, as we have our own from DB
		// Also consume the saved data on first spawn back, afterwards save and load will use its own data.
		#ifdef WORKBENCH
		if (player)
		{
			bool needsChange;
			vector transform[4];
			player.GetWorldTransform(transform);

			if (s_vPlayFromCameraPos != vector.Zero)
			{
				transform[3] = s_vPlayFromCameraPos;
				needsChange = true;
				s_vPlayFromCameraPos = vector.Zero;
			}

			if (s_vPlayFromCameraYpr != vector.Zero)
			{
				Math3D.AnglesToMatrix(s_vPlayFromCameraYpr, transform);
				needsChange = true;
				s_vPlayFromCameraYpr = vector.Zero;
			}

			if (needsChange)
				player.Teleport(transform);
		}

		// Remove old player
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerDataContext.param1);
		IEntity controlledEntity = playerController.GetControlledEntity();
		SCR_EntityHelper.DeleteEntityAndChildren(controlledEntity);
		#endif

		// Dead players will not work for respawn, as no events for additional death on them are raised after posession.
		if (player)
		{
			auto charController = CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent));
			if (charController && charController.GetLifeState() == ECharacterLifeState.DEAD)
				player = null;
		}

		// Check that we have a validi player to posess back
		if (!player)
		{
			DoInitialSpawn_S(playerDataContext.param1);
			return;
		}

		auto data = SCR_PossessSpawnData.FromEntity(player);
		GetPlayerRespawnComponent_S(playerDataContext.param1).RequestSpawn(data);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnControlledCharacterAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto characterAvailableContext = Tuple2<int, bool>.Cast(context);

		auto entity = IEntity.Cast(instance);
		if (entity)
		{
			// Check that the char is not dead by the time we hand over controls
			auto charController = CharacterControllerComponent.Cast(entity.FindComponent(CharacterControllerComponent));
			if (charController && charController.GetLifeState() != ECharacterLifeState.DEAD)
			{
				auto playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(characterAvailableContext.param1));
				playerController.SetPossessedEntity(entity);
				return;
			}
		}

		// No alive character was found during inital join process so proceed with default spawn
		if (characterAvailableContext.param2)
			DoInitialSpawn_S(characterAvailableContext.param1);
	}

	//------------------------------------------------------------------------------------------------
	void StoreControlledEntityIds(notnull SCR_PlayerController playerController, UUID playerCharacterId, UUID controlledCharacterId)
	{
		m_aStoredControlledEntityIds.Insert(new Tuple3<SCR_PlayerController, UUID, UUID>(playerController, playerCharacterId, controlledCharacterId));
	}

	//------------------------------------------------------------------------------------------------
	protected bool ConsumeControlledEntityIds(notnull SCR_PlayerController playerController, out UUID playerCharacterId, out UUID controlledCharacterId)
	{
		for (int i = 0, count = m_aStoredControlledEntityIds.Count(); i < count; i++)
		{
			if (m_aStoredControlledEntityIds[i].param1 == playerController)
			{
				playerCharacterId = m_aStoredControlledEntityIds[i].param2;
				controlledCharacterId = m_aStoredControlledEntityIds[i].param3;
				m_aStoredControlledEntityIds.Remove(i);
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool ForgetControlledEntityIds(notnull SCR_PlayerController playerController)
	{
		for (int i = 0, count = m_aStoredControlledEntityIds.Count(); i < count; i++)
		{
			if (m_aStoredControlledEntityIds[i].param1 == playerController)
			{
				m_aStoredControlledEntityIds.Remove(i);
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void DoInitialSpawn_S(int playerId)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::DoInitialSpawn_S(playerId: %2)", Type().ToString(), playerId);
		#endif

		#ifdef WORKBENCH
		if (GetGame().GetPlayerController().GetPlayerId() == playerId && HandlePlayFromCamera())
			return;
		#endif

		DoSpawn_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Implement the actual spawn behaviour
	protected void DoSpawn_S(int playerId)
	{
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Resolves spawn using the SCR_ReconnectComponent for player of given playerId.
		If such player is eligible for spawning this way, action is taken and true is
		returned on success (entity given over), false otherwise.
		\param playerId Player
		\return True if existing entity was re-assigned to him
	*/
	protected bool ResolveReconnection(int playerId)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::ResolveReconnection(playerId: %2)", Type().ToString(), playerId);
		#endif

		SCR_ReconnectComponent reconnect = SCR_ReconnectComponent.GetInstance();
		if (!reconnect || !reconnect.IsReconnectEnabled())
			return false;

		const SCR_EReconnectState state = reconnect.GetReconnectState(playerId);
		if (state == SCR_EReconnectState.NOT_RECONNECT)
		{
			reconnect.Forget(playerId);
			return false;
		}

		if (state == SCR_EReconnectState.ENTITY_DISCARDED)
		{
			// Discarded (killed and possibly already deleted) entity means we stop here and force respawn.
			// No attempt to use persistence even if it has data!
			reconnect.Forget(playerId);
			DoInitialSpawn_S(playerId);
			return true;
		}

		const IEntity assignedEntity = reconnect.ReturnControlledEntity(playerId);
		if (assignedEntity)
		{
			const PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);

			const Faction faction = SCR_FactionManager.SGetFaction(assignedEntity);
			if (faction)
			{

				SCR_PlayerFactionAffiliationComponent playerFactionComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
				if (playerFactionComp)
					playerFactionComp.SetFaction_S(faction);
			}

			m_RespawnSystem.EmitPlayerEntityChange_S(playerId, null, assignedEntity);

			SCR_ReconnectSynchronizationComponent syncComp = SCR_ReconnectSynchronizationComponent.Cast(pc.FindComponent(SCR_ReconnectSynchronizationComponent));
			if (syncComp)
				syncComp.CreateReconnectDialog(state);
		}

		return assignedEntity != null;
	}

	#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	protected bool HandlePlayFromCamera()
	{
		// Use play from camera entity if it is available
		PlayerController playerController = GetGame().GetPlayerController();
		IEntity controlledEntity = playerController.GetControlledEntity();
		if (controlledEntity)
		{
			auto respawnSystem = SCR_RespawnSystemComponent.GetInstance();
			if (respawnSystem)
				respawnSystem.DestroyLoadingPlaceholder();

			return true;
		}

		return false;
	}
	#endif

	//------------------------------------------------------------------------------------------------
	SCR_RespawnComponent GetPlayerRespawnComponent_S(int playerId)
	{
		return SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(playerId));
	}

	//------------------------------------------------------------------------------------------------
	SCR_RespawnComponent GetLocalPlayerRespawnComponent()
	{
		return SCR_RespawnComponent.Cast(GetGame().GetPlayerController().GetRespawnComponent());
	}

	//------------------------------------------------------------------------------------------------
	SCR_PlayerFactionAffiliationComponent GetPlayerFactionComponent_S(int playerId)
	{
		return SCR_PlayerFactionAffiliationComponent.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId).FindComponent(SCR_PlayerFactionAffiliationComponent));
	}

	//------------------------------------------------------------------------------------------------
	SCR_PlayerLoadoutComponent GetPlayerLoadoutComponent_S(int playerId)
	{
		return SCR_PlayerLoadoutComponent.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId).FindComponent(SCR_PlayerLoadoutComponent));
	}
}
