[ComponentEditorProps(category: "GameScripted/GameMode", description: "Communicator for RespawnSystemComponent. Should be attached to PlayerController.")]
class SCR_RespawnComponentClass : RespawnComponentClass
{
}

//#define RESPAWN_COMPONENT_DIAG
//	This define can be used to enable RespawnComponent diag menu which can be accessed in-game
//	via the Game Diag (Cmd+Alt) -> Network -> Respawn System Diag

//#define RESPAWN_COMPONENT_VERBOSE
//	Can be used to enable verbose logging of RPCs sent via SCR_RespawnComponent switch,
//	which can be especially useful when debugging a dedicated server or so.

// #define RESPAWN_COMPONENT_DIAG
// #define RESPAWN_COMPONENT_VERBOSE
// #define RESPAWN_COMPONENT_LOCKS_VERBOSE

//! Result code for request/assign response
enum ERespawnSelectionResult
{
	OK = 0,
	ERROR = 1,

	ERROR_FORBIDDEN = 2, //!< Can happen if we are setting a loadout from a faction to which we do not belong to or similar
}

//! Dummy communicator for RespawnSystem.
//! Must be attached to PlayerController entity.
class SCR_RespawnComponent : RespawnComponent
{
	//! Parent entity (owner) - has to be a player controller for RPCs
	protected PlayerController m_PlayerController;

	//! Parent entity's rpl component
	protected RplComponent m_RplComponent;
	// RespawnSystemComponent - has to be attached on a gameMode entity

	//! List of all request components - children of this component, stored by their assigned type.
	//! See also:SCR_SpawnRequestComponent.GetDataType()
	protected ref map<typename, SCR_SpawnRequestComponent> m_mRequestComponents = new map<typename, SCR_SpawnRequestComponent>();
	
	private static ref ScriptInvokerVoid s_OnLocalPlayerSpawned;
	
	//------------------------------------------------------------------------------------------------
	//! Called when player spawns locally
	//! \return
	static ScriptInvokerVoid SGetOnLocalPlayerSpawned() 
	{ 
		if (!s_OnLocalPlayerSpawned)
			s_OnLocalPlayerSpawned = new ScriptInvokerVoid();

		return s_OnLocalPlayerSpawned;
	}
	
	// ON RESPAWN READY (notification from server to e.g. open respawn menu)
	protected ref OnRespawnReadyInvoker m_OnRespawnReadyInvoker_O = new OnRespawnReadyInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this player is "ready" to spawn. (E.g. open deployment menu)
	OnRespawnReadyInvoker GetOnRespawnReadyInvoker_O()
	{
		return m_OnRespawnReadyInvoker_O;
	}

	// ON CAN RESPAWN REQUEST
	protected ref OnCanRespawnRequestInvoker m_OnCanRespawnRequestInvoker_O = new OnCanRespawnRequestInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component *sends* a can-request.
	OnCanRespawnRequestInvoker GetOnCanRespawnRequestInvoker_O()
	{
		return m_OnCanRespawnRequestInvoker_O;
	}

	protected ref OnCanRespawnRequestInvoker m_OnCanRespawnRequestInvoker_S = new OnCanRespawnRequestInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component *sends* a can-request.
	OnCanRespawnRequestInvoker GetOnCanRespawnRequestInvoker_S()
	{
		return m_OnCanRespawnRequestInvoker_S;
	}

	// ON CAN RESPAWN RESPONSE
	protected ref OnCanRespawnResponseInvoker m_OnCanRespawnResponseInvoker_O = new OnCanRespawnResponseInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component *receives* a response from the authority about
	//! prior sent ask can-request.
	OnCanRespawnResponseInvoker GetOnCanRespawnResponseInvoker_O()
	{
		return m_OnCanRespawnResponseInvoker_O;
	}

	protected ref OnCanRespawnResponseInvoker m_OnCanRespawnResponseInvoker_S = new OnCanRespawnResponseInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component *receives* a response from teh authority about
	//! prior sent ask can-request.
	OnCanRespawnResponseInvoker GetOnCanRespawnResponseInvoker_S()
	{
		return m_OnCanRespawnResponseInvoker_S;
	}

	// ON RESPAWN REQUEST
	protected ref OnRespawnRequestInvoker m_OnRespawnRequestInvoker_O = new OnRespawnRequestInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component sends a respawn request.
	OnRespawnRequestInvoker GetOnRespawnRequestInvoker_O()
	{
		return m_OnRespawnRequestInvoker_O;
	}

	protected ref OnRespawnRequestInvoker m_OnRespawnRequestInvoker_S = new OnRespawnRequestInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component sends a respawn request.
	OnRespawnRequestInvoker GetOnRespawnRequestInvoker_S()
	{
		return m_OnRespawnRequestInvoker_S;
	}

	// ON RESPAWN RESPONSE
	protected ref OnRespawnResponseInvoker m_OnRespawnResponseInvoker_O = new OnRespawnResponseInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component receives a respawn request response from the authority.
	OnRespawnResponseInvoker GetOnRespawnResponseInvoker_O()
	{
		return m_OnRespawnResponseInvoker_O;
	}

	protected ref OnRespawnResponseInvoker m_OnRespawnResponseInvoker_S = new OnRespawnResponseInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component receives a respawn request response from the authority.
	OnRespawnResponseInvoker GetOnRespawnResponseInvoker_S()
	{
		return m_OnRespawnResponseInvoker_S;
	}
	
	//------------------------------------------------------------------------------------------------
	// ON FINALIZE START
	protected ref OnRespawnRequestInvoker m_OnRespawnFinalizeBeginInvoker_O = new OnRespawnRequestInvoker();
	//------------------------------------------------------------------------------------------------
	//! When the spawn process reaches it end, the authority notifies the client about the last state starting ("finalization").
	//! This is the last state after which the player gains control of the desired controllable, or receives a response
	//! (see GetOnRespawnResponseInvoker_O) about a possible (rare?) failure.
	OnRespawnRequestInvoker GetOnRespawnFinalizeBeginInvoker_O()
	{
		return m_OnRespawnFinalizeBeginInvoker_O;
	}

	#ifdef DEBUGUI_RESPAWN_REQUEST_COMPONENT_DIAG
		static bool s_DebugRegistered;
	#endif

	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_RespawnComponent GetInstance()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (playerController)
			return SCR_RespawnComponent.Cast(playerController.FindComponent(SCR_RespawnComponent));
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Find SCR_RespawnComponent affiliated with the local player.
	//! Returns null if no PlayerController exists.
	//! \return SCR_RespawnComponent instance for local player or null if none is present.
	static SCR_RespawnComponent SGetLocalRespawnComponent()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return null;

		return SCR_RespawnComponent.Cast(playerController.GetRespawnComponent());
	}

	//------------------------------------------------------------------------------------------------
	//! Authority only:
	//! 	Find SCR_RespawnComponent affiliated with the provided player by their Id.
	//! \param[in] playerId Id of target player corresponding to id in PlayerController/PlayerManager.
	//! Returns null if no PlayerController exists.
	//! \return SCR_RespawnComponent instance for target player or null if none is present.
	static SCR_RespawnComponent SGetPlayerRespawnComponent(int playerId)
	{
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return null;

		return SCR_RespawnComponent.Cast(playerController.GetRespawnComponent());
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	RplComponent GetRplComponent()
	{
		return m_RplComponent;
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_PlayerFactionAffiliationComponent instead!")]
	bool RequestClearPlayerFaction();

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_PlayerLoadoutComponent instead!")]
	bool RequestClearPlayerLoadout();

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_RespawnComponent.RequestSpawn directly instead!")]
	void RequestClearPlayerSpawnPoint();

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_PlayerLoadoutComponent instead!")]
	bool RequestPlayerLoadout(SCR_BasePlayerLoadout loadout);

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_PlayerFactionAffiliationComponent instead!")]
	bool RequestPlayerFaction(Faction faction);

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_RespawnComponent.RequestSpawn directly instead!")]
	bool RequestPlayerSpawnPoint(SCR_SpawnPoint spawnPoint);

	//------------------------------------------------------------------------------------------------
	//!
	void RequestPlayerSuicide()
	{
		if (!m_PlayerController)
			return;

		GenericEntity controlledEntity = GenericEntity.Cast(m_PlayerController.GetControlledEntity());
		if (!controlledEntity)
			return;

		CharacterControllerComponent characterController = CharacterControllerComponent.Cast(controlledEntity.FindComponent(CharacterControllerComponent));
		if (characterController)
		{
			characterController.ForceDeath();
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_PlayerLoadoutComponent instead!")]
	protected void RequestPlayerLoadoutIndex(int loadoutIndex);

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_PlayerFactionAffiliationComponent instead!")]
	protected void RequestPlayerFactionIndex(int factionIndex);

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_RespawnComponent.RequestSpawn directly instead!")]
	protected void RequestPlayerSpawnPointIdentity(RplId spawnPointIdentity);
	
	//------------------------------------------------------------------------------------------------
	//! Authority:
	//! 	Send notification to this player that they are ready to spawn.
	void NotifyReadyForSpawn_S()
	{
		Rpc(Rpc_NotifyReadyForSpawn_O);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void Rpc_NotifyReadyForSpawn_O()
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::Rpc_NotifyReadyForSpawn_O(playerId: %2)", Type().ToString(), GetPlayerController().GetPlayerId()), LogLevel.NORMAL);
		#endif
		
		GetOnRespawnReadyInvoker_O().Invoke();

		#ifdef ENABLE_DIAG
		if (Diag_IsCLISpawnEnabled())
			Diag_RequestCLISpawn();
		#endif
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("Unsupported")]
	void RequestQuickRespawn();
	
	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_RespawnComponent.RequestSpawn instead.")]
	void RequestRespawn();

	//------------------------------------------------------------------------------------------------
	#ifdef ENABLE_DIAG
	override void OnDiag(IEntity owner, float timeSlice)
	{
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_RESPAWN_COMPONENT_DIAG))
			return;
		
		foreach (SCR_SpawnRequestComponent requestComponent : m_mRequestComponents)
		{
			requestComponent.DrawDiag();
		}
	}
	#endif

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		#ifdef ENABLE_DIAG
		ConnectToDiagSystem(owner);
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_RESPAWN_COMPONENT_DIAG, "", "Respawn Component", "GameMode");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_RESPAWN_COMPONENT_TIME, "", "Respawn Time Measures", "GameMode");
		#endif
		
		m_PlayerController = PlayerController.Cast(owner);
		if (!m_PlayerController)
		{
			Print("SCR_RespawnComponent must be attached to PlayerController!", LogLevel.ERROR);
			return;
		}
		
		SCR_SpawnLockComponent lockComponent = SCR_SpawnLockComponent.Cast(owner.FindComponent(SCR_SpawnLockComponent));
		if (!lockComponent)
		{
			Print(string.Format("%1 does not have a %2 attached!", 
				Type().ToString(), SCR_SpawnLockComponent), 
			LogLevel.ERROR);
		}

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		RegisterRespawnRequestComponents(owner);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
		{
			Print("No game mode found in the world, SCR_RespawnComponent will not function correctly!", LogLevel.ERROR);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromDiagSystem(owner);
		
		super.OnDelete(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Request a spawn with the provided data.
	//!
	//! The request is partially validated on client before transmission to the authority occurs.
	//! It is then further evaluated and processed by a SCR_RespawnHandlerComponent corresponding to
	//! each SCR_SpawnRequestComponent.
	//!
	//! Notable callbacks:
	//! - GetOnRespawnRequestInvoker_O -> Raised on owner request ('sender' has asked)
	//! - GetOnRespawnResponseInvoker_O -> Raised on owner response (authority has responded or in certain cases early reject is done by self)
	//! - GetOnRespawnRequestInvoker_S -> Authority received request from this component
	//! - GetOnRespawnResponseInvoker_S -> Authority dispatched response to this component
	//! \param[in] data
	//! \return Returns true if the request was dispatched into the system, false if there was an error on the requesting side.
	//! Such case can occur e.g. when there is a missing respawn handler for provided data type or similar.
	bool RequestSpawn(SCR_SpawnData data)
	{
		SCR_SpawnRequestComponent requestComponent = GetRequestComponent(data);
		if (!requestComponent)
		{
			Print(string.Format("%1::RequestRespawn(data: %2) could not find associated %3!", 
				Type().ToString(), data, SCR_SpawnRequestComponent), LogLevel.ERROR);
			return false;
		}
		
		return requestComponent.RequestRespawn(data);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Request an authority confirmation whether spawn with the provided data is possible.
	//!
	//! The request is partially validated on client before transmission to the authority occurs.
	//! It is then further evaluated and processed by a SCR_RespawnHandlerComponent corresponding to
	//! each SCR_SpawnRequestComponent.
	//!
	//! Notable callbacks:
	//! - GetOnCanRespawnRequestInvoker_O -> Raised on owner request ('sender' has asked)
	//! - GetOnCanRespawnResponseInvoker_O -> Raised on owner response (authority has responded or in certain cases early reject is done by self)
	//! - GetOnCanRespawnRequestInvoker_S -> Authority received request from this component
	//! - GetOnCanRespawnResponseInvoker_S -> Authority dispatched response to this component
	//! \param[in] data
	//! \return Returns true if the request was dispatched into the system, false if there was an error on the requesting side.
	//! Such case can occur e.g. when there is a missing respawn handler for provided data type or similar.
	bool CanSpawn(SCR_SpawnData data)
	{
		SCR_SpawnRequestComponent requestComponent = GetRequestComponent(data);
		if (!requestComponent)
		{
			Print(string.Format("%1::RequestRespawn(data: %2) could not find associated %3!", 
				Type().ToString(), data, SCR_SpawnRequestComponent), LogLevel.ERROR);
			return false;
		}
		
		return requestComponent.CanRequestRespawn(data);
	}

	//------------------------------------------------------------------------------------------------
	//! Register all SCR_SpawnRequestComponent found in the hierarchy.
	//! \param[in] owner
	protected void RegisterRespawnRequestComponents(IEntity owner)
	{
		array<GenericComponent> components = {};
		FindComponents(SCR_SpawnRequestComponent, components);
		foreach (GenericComponent genericComponent : components)
		{
			SCR_SpawnRequestComponent requestComponent = SCR_SpawnRequestComponent.Cast(genericComponent);
			typename dataType = requestComponent.GetDataType();
			if (m_mRequestComponents.Contains(dataType))
			{
				Debug.Error(string.Format("Cannot register %1! %2 already contains a mapping for %3 data type! (old: %4)",
						requestComponent.Type().ToString(),
						Type().ToString(),
						dataType,
						m_mRequestComponents[dataType].Type().ToString()));

				continue;
			}

			m_mRequestComponents.Insert(dataType, requestComponent);

			#ifdef _ENABLE_RESPAWN_LOGS
			string typeName = "null";
			if (dataType != typename.Empty)
				typeName = dataType.ToString();
			Print(string.Format("%1::RegisterRespawnRequestComponents() registered %2 component for requests of %3 type.", Type().ToString(),
						requestComponent.Type().ToString(),
						typeName), LogLevel.NORMAL);
			#endif
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Find a request component based on provided data instance type.
	//! \param[in] data
	//! \return
	protected SCR_SpawnRequestComponent GetRequestComponent(SCR_SpawnData data)
	{
		if (!data)
			return null;

		SCR_SpawnRequestComponent component;
		m_mRequestComponents.Find(data.Type(), component);
		return component;
	}

	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	//! \return whether diagnostics CLI spawning is enabled, in which case default spawning behaviour
	//! is overridden by supplied commands for quicker deployment for diagnostic purposes.
	static bool Diag_IsCLISpawnEnabled()
	{
		return System.IsCLIParam("autodeployFaction") || System.IsCLIParam("autodeployLoadout") ||
			System.IsCLIParam("tdmf") || System.IsCLIParam("tdml");
	}

	//------------------------------------------------------------------------------------------------
	//!
	void Diag_RequestCLISpawn()
	{
		int factionId = -1;
		string fs;
		if (System.GetCLIParam("autodeployFaction", fs) || System.GetCLIParam("tdmf", fs))
		{
			Faction factionFromKey = GetGame().GetFactionManager().GetFactionByKey(fs);

			if (factionFromKey != null)
				factionId = GetGame().GetFactionManager().GetFactionIndex(factionFromKey);
			else
				factionId = fs.ToInt();
		}

		int loadoutId = -1;
		string ls;
		if (System.GetCLIParam("autodeployLoadout", fs) || System.GetCLIParam("tdml", ls))
		{
			SCR_BasePlayerLoadout loadoutFromKey = GetGame().GetLoadoutManager().GetLoadoutByName(ls);

			if (loadoutFromKey != null)
				loadoutId = GetGame().GetLoadoutManager().GetLoadoutIndex(loadoutFromKey);
			else
				loadoutId = ls.ToInt();
		}

		Rpc(Rpc_RequestCLISpawn_S, factionId, loadoutId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_RequestCLISpawn_S(int factionIdx, int loadoutIdx)
	{
		int ok = 0;
		Faction faction = GetGame().GetFactionManager().GetFactionByIndex(factionIdx);
		SCR_BasePlayerLoadout loadout = GetGame().GetLoadoutManager().GetLoadoutByIndex(loadoutIdx);

		// backtrack faction index from loadout, if possible
		if (!faction && loadout)
		{
			SCR_FactionPlayerLoadout factionLoadout = SCR_FactionPlayerLoadout.Cast(loadout);
			if (factionLoadout)
				faction = GetGame().GetFactionManager().GetFactionByKey(factionLoadout.GetFactionKey());
		}

		// select loadout at random, faction was given
		if (faction && !loadout)
			loadout = GetGame().GetLoadoutManager().GetRandomFactionLoadout(faction);

		SCR_PlayerFactionAffiliationComponent factionComponent = SCR_PlayerFactionAffiliationComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (factionComponent)
		{
			if (factionComponent.RequestFaction(faction))
				ok |= (1 << 1);
		}

		SCR_PlayerLoadoutComponent loadoutComponent = SCR_PlayerLoadoutComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerLoadoutComponent));
		if (loadoutComponent)
		{
			if (loadoutComponent.RequestLoadout(loadout))
				ok |= (1 << 2);
		}

		if (loadout)
		{
			ResourceName resource = loadout.GetLoadoutResource();
			SCR_SpawnPoint spawnPoint = SCR_SpawnPoint.GetSpawnPointsForFaction(faction.GetFactionKey()).GetRandomElement();
			if (!resource.IsEmpty() && spawnPoint)
			{
				SCR_SpawnPointSpawnData spsd = new SCR_SpawnPointSpawnData(resource, spawnPoint.GetRplId());
				if (RequestSpawn(spsd))
					ok |= (1 << 3);
			}
		}

		Rpc(Rpc_ResponseCLISpawn_O, ok);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void Rpc_ResponseCLISpawn_O(int response)
	{
		bool faction = (response & (1 << 1));
		bool loadout = (response & (1 << 2));
		bool spawn = (response & (1 << 3));

		// TODO: check for good const usage
		const string msg = "Server request to spawn using diagnostics mode processed (-autodeployFaction/-autodeployLoadout), result:\n\tFaction: %1, Loadout: %2, Spawn: %3";

		string fs = "ERR";
		if (faction)
			fs = "OK";

		string ls = "ERR";
		if (loadout)
			ls = "OK";

		string ss = "ERR";
		if (spawn)
			ss = "OK";

		Print(string.Format(msg, fs, ls, ss), LogLevel.NORMAL);
	}

	#endif

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_RespawnComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}
}
