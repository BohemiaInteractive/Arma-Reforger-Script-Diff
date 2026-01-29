class SCR_RespawnSystemComponentClass : RespawnSystemComponentClass
{
}

//! Scripted implementation that handles spawning and respawning of players.
//! Should be attached to a GameMode entity.
[ComponentEditorProps(icon: HYBRID_COMPONENT_ICON)]
class SCR_RespawnSystemComponent : RespawnSystemComponent
{
	[Attribute(category: "Respawn System")]
	protected ref SCR_SpawnLogic m_SpawnLogic;

	[Attribute("1", uiwidget: UIWidgets.CheckBox, category: "Respawn System")]
	protected bool m_bEnableRespawn;

	[Attribute("1", desc: "Handles visibility of the Respawn button in Pause menu.", category: "Respawn System")]
	protected bool m_bEnablePauseMenuRespawn;

	[Attribute("{A1CE9D1EC16DA9BE}UI/layouts/Menus/MainMenu/SplashScreen.layout", desc: "Optional: Layout shown during world load completion and respawn menu opening or automatically completing.", category: "Respawn System")]
	protected ResourceName m_sLoadingLayout;

	[Attribute("{A39BE59EB6F41125}Configs/Respawn/SpawnPointRequestResultInfoConfig.conf", desc: "Holds a config of all reasons why a specific spawnpoint can be disabled", category: "Respawn System")]
	protected ResourceName m_sSpawnPointRequestResultInfoHolder;

	protected ref SCR_SpawnPointRequestResultInfoConfig m_SpawnPointRequestResultInfoHolder;

	// Instance of this component
	private static SCR_RespawnSystemComponent s_Instance = null;

	// The parent of this entity which should be a gamemode
	protected SCR_BaseGameMode m_pGameMode;

	// Parent entity's rpl component
	protected RplComponent m_RplComponent;

	// Preload
	protected ref SimplePreload m_Preload;
	protected ref ScriptInvoker Event_OnRespawnEnabledChanged;

	// Loading placeholder
	protected bool m_bAudioMuted;
	protected Widget m_wLoadingPlaceholder;
	protected SCR_LoadingSpinner m_LoadingSpinner;

	//------------------------------------------------------------------------------------------------
	//! \param[in] requestComponent
	//! \param[in] response
	//! \param[in] data
	//! \return
	SCR_BaseSpawnPointRequestResultInfo GetSpawnPointRequestResultInfo(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response, SCR_SpawnData data)
	{
		if (!m_SpawnPointRequestResultInfoHolder)
			return null;

		return m_SpawnPointRequestResultInfoHolder.GetFirstValidRequestResultInfo(requestComponent, response, data);
	}

	//------------------------------------------------------------------------------------------------
	//! \return an instance of RespawnSystemComponent
	static SCR_RespawnSystemComponent GetInstance()
	{
		if (!s_Instance)
		{
			BaseGameMode pGameMode = GetGame().GetGameMode();
			if (pGameMode)
				s_Instance = SCR_RespawnSystemComponent.Cast(pGameMode.FindComponent(SCR_RespawnSystemComponent));
		}

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! Access to replication component
	RplComponent GetRplComponent()
	{
		return m_RplComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! UI management
	static MenuBase OpenRespawnMenu()
	{
		MenuManager pMenuManager = GetGame().GetMenuManager();
		if (!pMenuManager)
			return null;

		return pMenuManager.OpenMenu(ChimeraMenuPreset.RespawnSuperMenu);
	}

	//------------------------------------------------------------------------------------------------
	//! Close all menus operated by Respawn System
	static void CloseRespawnMenu()
	{
		MenuManager pMenuManager = GetGame().GetMenuManager();
		if (!pMenuManager)
			return;

		MenuBase menu = pMenuManager.FindMenuByPreset(ChimeraMenuPreset.RespawnSuperMenu);
		if (menu)
			pMenuManager.CloseMenu(menu);
	}

	//------------------------------------------------------------------------------------------------
	//! Set respawn enabled Server only
	//! \param[in] enableSpawning set respawn enabled or not
	void ServerSetEnableRespawn(bool enableSpawning)
	{
		if (enableSpawning == m_bEnableRespawn || !Replication.IsServer())
			return;

		SetEnableRespawnBroadcast(enableSpawning);
		Rpc(SetEnableRespawnBroadcast, enableSpawning);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] enableSpawning
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetEnableRespawnBroadcast(bool enableSpawning)
	{
		m_bEnableRespawn = enableSpawning;
		if (Event_OnRespawnEnabledChanged)
			Event_OnRespawnEnabledChanged.Invoke(m_bEnableRespawn);
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if respawn is enabled, false otherwise
	bool IsRespawnEnabled()
	{
		return m_bEnableRespawn;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if respawn from pause menu is enabled, false otherwise
	bool IsPauseMenuRespawnEnabled()
	{
		return m_bEnablePauseMenuRespawn;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if faction change is allowed by the game mode, false otherwise.
	bool IsFactionChangeAllowed()
	{
		return m_pGameMode.IsFactionChangeAllowed();
	}

	//------------------------------------------------------------------------------------------------
	//! \return script invoker which is called when server enables or disables respawn
	ScriptInvoker GetOnRespawnEnabledChanged()
	{
		if (!Event_OnRespawnEnabledChanged)
			Event_OnRespawnEnabledChanged = new ScriptInvoker();

		return Event_OnRespawnEnabledChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority only:
	//! Whenever a SCR_SpawnHandlerComponent receives a request from SCR_SpawnRequestComponent that needs to
	//! verify whether a player can spawn in addition to the SCR_SpawnHandlerComponent logic (per-case logic),
	//! this method is called to allow handling logic on a global scale.
	//! \param[in] requestComponent The player request component (instigator).
	//! \param[in] handlerComponent The handler that passes the event to this manager.
	//! \param[in] data The data passed from the request
	//! \param[out] result Reason why respawn is disabled. Note that if returns true the reason will always be OK
	//! \return true If request is allowed, false otherwise.
	bool CanRequestSpawn_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, out SCR_ESpawnResult result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::CanRequestSpawn_S(playerId: %2, handler: %2, data: %3)", Type().ToString(),
					requestComponent.GetPlayerId(),
					handlerComponent,
					data), LogLevel.NORMAL);
		#endif

		if (!m_bEnableRespawn)
		{
			result = SCR_ESpawnResult.NOT_ALLOWED_SPAWNING_DISABLED;
			return false;
		}

		return m_pGameMode.CanPlayerSpawn_S(requestComponent, handlerComponent, data, result);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority only:
	//! During the spawn process (after validation pass), the SCR_SpawnHandlerComponent can opt to prepare
	//! spawned entity. This process first happens on affiliated SCR_SpawnHandlerComponent and if it succeeds,
	//! it additionally raises this method, which can prepare entity on a global scale. (E.g. game mode logic)
	//! Preparation can still fail (e.g. desire to seat a character, but an error occurs) and by returning false
	//! the sender is informed of such failure and can respond accordingly.
	//! \param[in] requestComponent Instigator of the request.
	//! \param[in] handlerComponent Handler that processed the request.
	//! \param[in] data The payload of the request.
	//! \param[in] entity Spawned (or generally assigned) entity to be prepared.
	//! \return true on success (continue to next step), fail on failure (terminate spawn process).
	bool PreparePlayerEntity_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		return m_pGameMode.PreparePlayerEntity_S(requestComponent, handlerComponent, data, entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority only:
	//! During the spawn process the SCR_SpawnHandlerComponent can opt to handle changes of previous (and next) controlled
	//! (or newly spawned) entity for the given player. Such process additionally raises this method, which can handle
	//! entity changes on a global scale. (E.g. game mode logic).
	//! \param[in] requestComponent Instigator of the request.
	//! \param[in] handlerComponent Handler that processed the request.
	//! \param[in] previousEntity Previously controlled entity. (May be null)
	//! \param[in] newEntity Entity to be controlled.
	//! \param[in] data The payload of the request.
	void OnPlayerEntityChange_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, IEntity previousEntity, IEntity newEntity, SCR_SpawnData data)
	{
		EmitPlayerEntityChange_S(requestComponent.GetPlayerId(), previousEntity, newEntity)
	}

	//------------------------------------------------------------------------------------------------
	void EmitPlayerEntityChange_S(int playerId, IEntity previousEntity, IEntity newEntity)
	{
		m_pGameMode.OnPlayerEntityChanged_S(playerId, previousEntity, newEntity);
		m_SpawnLogic.OnPlayerEntityChanged_S(playerId, previousEntity, newEntity);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority only:
	//! Whenever a request to spawn is denied by the authority, this callback is raised.
	void OnSpawnPlayerEntityFailure_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, IEntity entity, SCR_SpawnData data, SCR_ESpawnResult reason)
	{
		m_pGameMode.OnSpawnPlayerEntityFailure_S(requestComponent, handlerComponent, entity, data, reason);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority only:
	//! Whenever a SCR_SpawnHandlerComponent processes a spawn request and finished the finalization stage
	//! (awaits finalization, passes control to client) this method is called. This is the final step in the respawn
	//! process and after this point the owner of SCR_SpawnRequestComponent is spawned.
	//! \param[in] requestComponent Instigator of the request.
	//! \param[in] handlerComponent Handler that processed the request.
	//! \param[in] data The payload of the request.
	//! \param[in] entity Spawned (or generally assigned) entity.
	void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		m_pGameMode.OnPlayerSpawnFinalize_S(requestComponent, handlerComponent, data, entity);
		m_SpawnLogic.OnPlayerSpawned_S(requestComponent.GetPlayerId(), entity);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	void OnPlayerRegistered_S(int playerId)
	{
		m_SpawnLogic.OnPlayerRegistered_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	void OnPlayerAuditSuccess_S(int playerId)
	{
		m_SpawnLogic.OnPlayerAuditSuccess_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	//! \param[in] cause
	//! \param[in] timeout
	void OnPlayerDisconnected_S(int playerId, KickCauseCode cause, int timeout)
	{
		m_SpawnLogic.OnPlayerDisconnected_S(playerId, cause, timeout);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	//! \param[in] playerEntity
	//! \param[in] killerEntity
	//! \param[in] killer
	void OnPlayerKilled_S(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		m_SpawnLogic.OnPlayerKilled_S(playerId, playerEntity, killerEntity, killer);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	void OnPlayerDeleted_S(int playerId)
	{
		const bool isDisconnect = !GetGame().GetPlayerManager().IsPlayerConnected(playerId);
		m_SpawnLogic.OnPlayerDeleted_S(playerId, isDisconnect);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_SpawnLogic GetSpawnLogic()
	{
		return m_SpawnLogic;
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		m_SpawnPointRequestResultInfoHolder = SCR_ConfigHelperT<SCR_SpawnPointRequestResultInfoConfig>.GetConfigObject(m_sSpawnPointRequestResultInfoHolder);
		if (!m_SpawnPointRequestResultInfoHolder)
			Print("'SCR_RespawnSystemComponent' has no valid m_SpawnPointRequestResultInfoHolder! This means the disabled reason cannot be disabled spawn point!", LogLevel.ERROR);

		m_pGameMode = SCR_BaseGameMode.Cast(owner);
		if (!m_pGameMode)
			Print("SCR_RespawnSystemComponent has to be attached to a SCR_BaseGameMode (or inherited) entity!", LogLevel.ERROR);
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		if (!m_SpawnLogic)
			Print("SCR_RespawnSystemComponent is missing SCR_SpawnLogic!", LogLevel.ERROR);

		if (GetGame().InPlayMode())
		{
			m_SpawnLogic.OnInit(this);

			// Validate faction manager
			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			if (!factionManager)
			{
				string text = string.Format("No %1 found in the world, %2 might not work as intended!",
						SCR_FactionManager, SCR_RespawnSystemComponent);
				Print(text, LogLevel.WARNING);
			}

			// Validate loadout manager
			SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
			if (!loadoutManager)
			{
				string text = string.Format("No %1 found in the world, %2 might not work as intended!",
						SCR_LoadoutManager, SCR_RespawnSystemComponent);
				Print(text, LogLevel.WARNING);
			}

			if (!System.IsConsoleApp())
				CreateLoadingPlaceholder();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override event bool OnRplSave(ScriptBitWriter w)
	{
		w.WriteBool(m_bEnableRespawn);

		return super.OnRplSave(w);
	}

	//------------------------------------------------------------------------------------------------
	protected override event bool OnRplLoad(ScriptBitReader r)
	{
		bool enableRespawn;
		r.ReadBool(enableRespawn);

		SetEnableRespawnBroadcast(enableRespawn);

		return super.OnRplLoad(r);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateLoadingPlaceholder()
	{
		if (!UsesLoadingPlaceholder())
			return;

		m_wLoadingPlaceholder = GetGame().GetWorkspace().CreateWidgets(m_sLoadingLayout);
		if (!m_wLoadingPlaceholder)
			return;

		AudioSystem.Pause(1 << AudioSystem.SFX);
		m_bAudioMuted = true;

		const Widget spinner = m_wLoadingPlaceholder.FindAnyWidget("Spinner");
		if (spinner)
			m_LoadingSpinner = SCR_LoadingSpinner.Cast(spinner.FindHandler(SCR_LoadingSpinner));
	}

	//------------------------------------------------------------------------------------------------
	bool UsesLoadingPlaceholder()
	{
		return !m_sLoadingLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	void UpdateLoadingPlaceholder(float dt)
	{
		if (!m_wLoadingPlaceholder)
			return;

		const bool placeHolderVisible = !GetGame().IsPlayingCinematic();
		m_wLoadingPlaceholder.SetVisible(placeHolderVisible);

		if (placeHolderVisible && !m_bAudioMuted)
		{
			AudioSystem.Pause(1 << AudioSystem.SFX);
			m_bAudioMuted = true;
		}
		else if (!placeHolderVisible && m_bAudioMuted)
		{
			AudioSystem.Resume(1 << AudioSystem.SFX);
			m_bAudioMuted = false;
		}

		if (m_LoadingSpinner)
			m_LoadingSpinner.Update(dt);
	}

	//------------------------------------------------------------------------------------------------
	void DestroyLoadingPlaceholder()
	{
		if (m_bAudioMuted) // Unmute audio regardless of widget in case it was implicitly destoryed early.
		{
			AudioSystem.Resume(1 << AudioSystem.SFX);
			m_bAudioMuted = false;
		}

		if (!m_wLoadingPlaceholder)
			return;

		m_wLoadingPlaceholder.RemoveFromHierarchy();
		m_wLoadingPlaceholder = null;
		m_LoadingSpinner = null;
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_RespawnSystemComponent()
	{
		DestroyLoadingPlaceholder();
		s_Instance = null;
	}
}
