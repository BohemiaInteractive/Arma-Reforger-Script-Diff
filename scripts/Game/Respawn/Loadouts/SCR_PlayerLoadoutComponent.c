[ComponentEditorProps(category: "GameScripted/Respawn/PlayerController")]
class SCR_PlayerLoadoutComponentClass : ScriptComponentClass
{
}

void PlayerLoadoutRequestDelegate(SCR_PlayerLoadoutComponent component, int loadoutIndex);
typedef func PlayerLoadoutRequestDelegate;
typedef ScriptInvokerBase<PlayerLoadoutRequestDelegate> OnPlayerLoadoutRequestInvoker;

void PlayerLoadoutResponseDelegate(SCR_PlayerLoadoutComponent component, int loadoutIndex, bool response);
typedef func PlayerLoadoutResponseDelegate;
typedef ScriptInvokerBase<PlayerLoadoutResponseDelegate> OnPlayerLoadoutResponseInvoker;

//! This component should be attached to a PlayerController.
//! It manages player-specific loadout and the communication between player and authority regarding so.
class SCR_PlayerLoadoutComponent : ScriptComponent
{
	private PlayerController m_PlayerController;
	private SCR_RespawnComponent m_RespawnComponent;
	private RplComponent m_RplComponent;
	private SCR_PlayerFactionAffiliationComponent m_FactionAffiliation;
	private SCR_SpawnLockComponent m_Lock;

	//! Assigned loadout index. Relevant to authority only.
	protected SCR_BasePlayerLoadout m_Loadout;

	//------------------------------------------------------------------------------------------------
	//! \return owner PlayerController this component is attached to.
	PlayerController GetPlayerController()
	{
		return m_PlayerController;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return owner player PlayerController id.
	int GetPlayerId()
	{
		return GetPlayerController().GetPlayerId();
	}

	//------------------------------------------------------------------------------------------------
	//! \return owner PlayerController lock component (if any).
	protected SCR_SpawnLockComponent GetLock()
	{
		return m_Lock;
	}

	//------------------------------------------------------------------------------------------------
	//! \return player faction affiliation, if any is present.
	protected SCR_PlayerFactionAffiliationComponent GetPlayerFactionAffiliationComponent()
	{
		return m_FactionAffiliation;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return player faction affiliation, if any is present.
	SCR_BasePlayerLoadout GetLoadout()
	{
		return m_Loadout;
	}

	// ON CAN LOADOUT REQUEST
	protected ref OnPlayerLoadoutRequestInvoker m_OnCanPlayerLoadoutRequestInvoker_O = new OnPlayerLoadoutRequestInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component requests a loadout change from the authority.
	OnPlayerLoadoutRequestInvoker GetOnCanPlayerLoadoutRequestInvoker_O()
	{
		return m_OnCanPlayerLoadoutRequestInvoker_O;
	}

	protected ref OnPlayerLoadoutRequestInvoker m_OnCanPlayerLoadoutRequestInvoker_S = new OnPlayerLoadoutRequestInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component requests a loadout change from the authority.
	OnPlayerLoadoutRequestInvoker GetOnCanPlayerLoadoutRequestInvoker_S()
	{
		return m_OnCanPlayerLoadoutRequestInvoker_S;
	}

	// ON CAN LOADOUT RESPONSE
	protected ref OnPlayerLoadoutResponseInvoker m_OnCanPlayerLoadoutResponseInvoker_O = new OnPlayerLoadoutResponseInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component receives a response from the authority regarding loadout change.
	OnPlayerLoadoutResponseInvoker GetOnCanPlayerLoadoutResponseInvoker_O()
	{
		return m_OnCanPlayerLoadoutResponseInvoker_O;
	}

	protected ref OnPlayerLoadoutResponseInvoker m_OnCanPlayerLoadoutResponseInvoker_S = new OnPlayerLoadoutResponseInvoker();
	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component receives a response from the authority regarding loadout change.
	OnPlayerLoadoutResponseInvoker GetOnCanPlayerLoadoutResponseInvoker_S()
	{
		return m_OnCanPlayerLoadoutResponseInvoker_S;
	}

	// ON LOADOUT REQUEST
	protected ref OnPlayerLoadoutRequestInvoker m_OnPlayerLoadoutRequestInvoker_O = new OnPlayerLoadoutRequestInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component requests a loadout change from the authority.
	OnPlayerLoadoutRequestInvoker GetOnPlayerLoadoutRequestInvoker_O()
	{
		return m_OnPlayerLoadoutRequestInvoker_O;
	}

	protected ref OnPlayerLoadoutRequestInvoker m_OnPlayerLoadoutRequestInvoker_S = new OnPlayerLoadoutRequestInvoker();
	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component requests a loadout change from the authority.
	OnPlayerLoadoutRequestInvoker GetOnPlayerLoadoutRequestInvoker_S()
	{
		return m_OnPlayerLoadoutRequestInvoker_S;
	}

	// ON LOADOUT RESPONSE
	protected ref OnPlayerLoadoutResponseInvoker m_OnPlayerLoadoutResponseInvoker_O = new OnPlayerLoadoutResponseInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component receives a response from the authority regarding loadout change.
	OnPlayerLoadoutResponseInvoker GetOnPlayerLoadoutResponseInvoker_O()
	{
		return m_OnPlayerLoadoutResponseInvoker_O;
	}

	protected ref OnPlayerLoadoutResponseInvoker m_OnPlayerLoadoutResponseInvoker_S = new OnPlayerLoadoutResponseInvoker();

	//------------------------------------------------------------------------------------------------
	//! \return an invoker that is invoked after this component receives a response from the authority regarding loadout change.
	OnPlayerLoadoutResponseInvoker GetOnPlayerLoadoutResponseInvoker_S()
	{
		return m_OnPlayerLoadoutResponseInvoker_S;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsOwner()
	{
		return !m_RplComponent || m_RplComponent.IsOwner();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsProxy()
	{
		return m_RplComponent && m_RplComponent.IsProxy();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		m_PlayerController = PlayerController.Cast(owner);
		if (!m_PlayerController)
			Debug.Error(string.Format("%1 is not attached to a %2", Type().ToString(), PlayerController));

		m_RespawnComponent = SCR_RespawnComponent.Cast(owner.FindComponent(SCR_RespawnComponent));
		m_FactionAffiliation = SCR_PlayerFactionAffiliationComponent.Cast(owner.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (m_FactionAffiliation)
			m_FactionAffiliation.GetOnFactionChanged().Insert(OnFactionChanged);
		
		m_Lock = SCR_SpawnLockComponent.Cast(owner.FindComponent(SCR_SpawnLockComponent));
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_RESPAWN_PLAYER_LOADOUT_DIAG, "", "Player Loadouts", "GameMode");
		GetGame().GetCallqueue().CallLater(OnDiag, 0, true);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnDelete(IEntity owner)
	{
		if (m_FactionAffiliation)
			m_FactionAffiliation.GetOnFactionChanged().Remove(OnFactionChanged);
		
		super.OnDelete(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFactionChanged(FactionAffiliationComponent owner, Faction old, Faction current)
	{
		// If a faction changes, as the authority, always clear this player's
		// loadout as there is no guarantee that new faction will
		if (old != current && !m_RplComponent || !m_RplComponent.IsProxy())
		{
			SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
			if (!loadoutManager)
			{
				Print("Loadout manager is missing in the world!", LogLevel.ERROR);
				return;
			}

			int previous = loadoutManager.GetLoadoutIndex(m_Loadout);	
			
			// Loadout is existant, see whether it is possible to be kept,
			// or whether this loadout is no longer available after faction change
			if (previous != -1)
			{
				// If it cannot be assigned (not part of faction, etc)
				if (!CanAssignLoadout_S(previous))
				{
					// Directly clear loadout of target player,
					// for previous loadout can no longer be deemed valid
					AssignLoadout_S(-1);
					SendRequestLoadoutResponse_S(-1, true);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Authority
	//! \return assigned loadout for this player.
	SCR_BasePlayerLoadout GetAssignedLoadout()
	{
		return m_Loadout;
	}

	//------------------------------------------------------------------------------------------------
	//! Sends a request to get assignedf provided loadout.
	//! \param[in] loadout
	//! \return True if request was sent, false if request was caught (on owner, still!) because it was invalid.
	//! NOTE: This is not the final result of the assignation. That result is can be listened to by hooking
	//! onto GetOnPlayerLoadoutResponseInvoker(), successful request will have a response of SCR_ESpawnResult.OK.
	bool RequestLoadout(SCR_BasePlayerLoadout loadout)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::RequestLoadout(loadout: %2)", Type().ToString(), loadout);
		#endif

		// Lock this
		int loadoutIndex = GetGame().GetLoadoutManager().GetLoadoutIndex(loadout);
		SCR_SpawnLockComponent lock = GetLock();
		if (lock && !lock.TryLock(this, false))
		{
			Print("SCR_PlayerLoadoutComponent::RequestLoadout - Caught request on locked player!", LogLevel.DEBUG);
			return false;
		}

		// Notify owner
		if (IsOwner())
			GetOnPlayerLoadoutRequestInvoker_O().Invoke(this, loadoutIndex);
		// Notify authority
		if (!IsProxy())
			GetOnPlayerLoadoutRequestInvoker_S().Invoke(this, loadoutIndex);

		Rpc(Rpc_RequestLoadout_S, loadoutIndex);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Ask the authority to assign provided loadout.
	//! \param[in] loadoutIndex
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_RequestLoadout_S(int loadoutIndex)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::Rpc_RequestLoadout_S(loadoutIdx: %2)", Type().ToString(), loadoutIndex);
		#endif

		// Lock server
		SCR_SpawnLockComponent lock = GetLock();
		if (lock && !lock.TryLock(this, true))
		{
			Print("SCR_PlayerLoadoutComponent::Rpc_RequestLoadout_S - Caught request on locked player!", LogLevel.DEBUG);
			return;
		}

		// Notify server
		GetOnPlayerLoadoutRequestInvoker_S().Invoke(this, loadoutIndex);

		// See whether loadout can be be set
		if (CanAssignLoadout_S(loadoutIndex))
		{
			// Assign loadout
			if (AssignLoadout_S(loadoutIndex))
			{
				// Success
				SendRequestLoadoutResponse_S(loadoutIndex, true);
				return;
			}
		}

		// Failure
		SendRequestLoadoutResponse_S(loadoutIndex, false);
	}

	//------------------------------------------------------------------------------------------------
	protected bool AssignLoadout_S(int loadoutIndex)
	{
		// Assign new data
		SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
		SCR_BasePlayerLoadout loadout = loadoutManager.GetLoadoutByIndex(loadoutIndex);
		m_Loadout = loadout;

		// Notify loadout manager
		loadoutManager.UpdatePlayerLoadout_S(this);
		
		// Notify game mode
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		gameMode.OnPlayerLoadoutSet_S(this, loadout);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority:
	//! 	Sends response to the owner whether loadout assignation was successfull or not.
	//! \param[in] loadoutIndex
	//! \param[in] response Was loadout assigned?
	protected void SendRequestLoadoutResponse_S(int loadoutIndex, bool response)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::SendRequestLoadoutResponse_S(loadoutIdx: %2, response: %3)", Type().ToString(), loadoutIndex, response);
		#endif

		// Unlock server
		SCR_SpawnLockComponent lock = GetLock();
		if (lock)
		{
			lock.Unlock(this, true);
			lock.Unlock(this, false);
		}

		// Notify this
		GetOnPlayerLoadoutResponseInvoker_S().Invoke(this, loadoutIndex, response);

		// Notify owner
		Rpc(RequestLoadoutResponse_O, loadoutIndex, response);
	}

	//------------------------------------------------------------------------------------------------
	//! Owner:
	//! 	Response from the authority about whether loadout was set successfully or not.
	//! \param[in] loadoutIndex
	//! \param[in] response
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RequestLoadoutResponse_O(int loadoutIndex, bool response)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::RequestLoadoutResponse_O(loadoutIdx: %2, response: %3)", Type().ToString(), loadoutIndex, response);
		#endif

		// Unlock this
		SCR_SpawnLockComponent lock = GetLock();
		if (lock)
		{
			lock.Unlock(this, false);
		}

		// Set loadout
		SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
		m_Loadout = loadoutManager.GetLoadoutByIndex(loadoutIndex);

		// Notify this
		if (IsOwner())
			GetOnPlayerLoadoutResponseInvoker_O().Invoke(this, loadoutIndex, response);
	}

	//------------------------------------------------------------------------------------------------
	//! Sends a can-ask request to the authority.
	//! \param[in] loadout
	//! \return True if request was sent, false if request was caught (on owner, still!) because it was invalid.
	//! NOTE: This is not the final result of the assignation. That result is can be listened to by hooking
	//! onto GetOnCanPlayerLoadoutResponseInvoker(), successful request will have a response of SCR_ESpawnResult.OK.
	bool CanRequestLoadout(SCR_BasePlayerLoadout loadout)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::CanRequestLoadout(loadout: %2)", Type().ToString(), loadout), LogLevel.NORMAL);
		#endif

		// Lock this
		SCR_SpawnLockComponent lock = GetLock();
		if (lock && !lock.TryLock(this, false))
		{
			Print("SCR_PlayerLoadoutComponent::CanRequestLoadout - Caught request on locked player!", LogLevel.DEBUG);
			return false;
		}
		
		int loadoutIndex = GetGame().GetLoadoutManager().GetLoadoutIndex(loadout);
		// Notify owner
		if (IsOwner())
			GetOnCanPlayerLoadoutRequestInvoker_O().Invoke(this, loadoutIndex);

		// Notify authority
		if (!IsProxy())
			GetOnCanPlayerLoadoutRequestInvoker_S().Invoke(this, loadoutIndex);

		Rpc(Rpc_CanRequestLoadout_S, loadoutIndex);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Ask the authority to whether provided loadout can be assigned.
	//! \param[in] loadoutIndex
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_CanRequestLoadout_S(int loadoutIndex)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::Rpc_CanRequestLoadout_S(loadoutIdx: %2)", Type().ToString(), loadoutIndex);
		#endif

		// Lock server
		SCR_SpawnLockComponent lock = GetLock();
		if (lock && !lock.TryLock(this, true))
		{
			Print("SCR_PlayerLoadoutComponent::Rpc_CanRequestLoadout_S - Caught request on locked player!", LogLevel.DEBUG);
			return;
		}

		// Notify server
		GetOnCanPlayerLoadoutRequestInvoker_S().Invoke(this, loadoutIndex);

		// See whether game logic allows for assigning this loadout
		if (!CanAssignLoadout_S(loadoutIndex))
		{
			SendCanRequestLoadoutResponse_S(loadoutIndex, false);
			return;
		}

		SendCanRequestLoadoutResponse_S(loadoutIndex, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority:
	//! 	Returns whether provided loadout can be assigned for this player.
	//! \param[in] loadoutIndex
	protected bool CanAssignLoadout_S(int loadoutIndex)
	{
		SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
		SCR_BasePlayerLoadout loadout = loadoutManager.GetLoadoutByIndex(loadoutIndex);
		
		// If loadout is faction based, ensure loadout can be set
		Faction playerFaction;
	 	SCR_PlayerFactionAffiliationComponent factionAffiliation = GetPlayerFactionAffiliationComponent();
		if (factionAffiliation)
			playerFaction = factionAffiliation.GetAffiliatedFaction();
		
		// Loadout is not of allowed faction, disallow mismatches
		if (!IsLoadoutUseableByFaction(loadout, playerFaction))
			return false;
		
		return loadoutManager.CanAssignLoadout_S(this, loadout);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] loadout
	//! \param[in] faction
	//! \return whether the provided loadout is useable by the provided faction (if provided).
	protected bool IsLoadoutUseableByFaction(SCR_BasePlayerLoadout loadout, Faction faction)
	{
		// Loadout can not belong to any faction
		SCR_FactionPlayerLoadout factionLoadout = SCR_FactionPlayerLoadout.Cast(loadout);
		if (!factionLoadout)
			return true;
		
		FactionKey key;
		if (faction)
			key = faction.GetFactionKey();
		
		return key == factionLoadout.GetFactionKey();
	}

	//------------------------------------------------------------------------------------------------
	//! Authority:
	//! 	Sends response to the owner whether loadout assignation can be done or not.
	//! \param[in] loadoutIndex
	//! \param[in] response Can loadout be assigned?
	protected void SendCanRequestLoadoutResponse_S(int loadoutIndex, bool response)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::SendCanRequestLoadoutResponse_S(loadoutIdx: %2, response: %3)", Type().ToString(), loadoutIndex, response);
		#endif

		// Unlock server
		SCR_SpawnLockComponent lock = GetLock();
		if (lock)
		{
			lock.Unlock(this, true);
			lock.Unlock(this, false);
		}

		// Notify owner
		GetOnCanPlayerLoadoutResponseInvoker_S().Invoke(this, loadoutIndex, response);

		// Notify user
		Rpc(CanRequestLoadoutResponse_O, loadoutIndex, response);
	}

	//------------------------------------------------------------------------------------------------
	//! Owner:
	//! 	Response from the authority about whether loadout was set successfully or not.
	//! \param[in] loadoutIndex
	//! \param[in] response
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void CanRequestLoadoutResponse_O(int loadoutIndex, bool response)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::CanRequestLoadoutResponse_O(loadoutIdx: %2, response: %3)", Type().ToString(), loadoutIndex, response);
		#endif

		// Unlock this
		SCR_SpawnLockComponent lock = GetLock();
		if (lock)
		{
			lock.Unlock(this, false);
		}

		// Notify owner
		GetOnCanPlayerLoadoutResponseInvoker_O().Invoke(this, loadoutIndex, response);
	}

	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	//! Draw diagnostics for this component.
	protected void OnDiag()
	{
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_RESPAWN_PLAYER_LOADOUT_DIAG))
			return;

		int playerId = GetPlayerController().GetPlayerId();
		DbgUI.Begin(string.Format("PlayerLoadout (id: %1)", playerId));
		{
			SCR_BasePlayerLoadout loadout = GetAssignedLoadout();
			string loadoutName = "None";
			if (loadout)
				loadoutName = loadout.GetLoadoutName();

			DbgUI.Text(string.Format("Current: %1 (%2)", loadout, loadout));

			int wantedIdx;
			DbgUI.InputInt("Wanted Loadout Idx", wantedIdx);

			SCR_BasePlayerLoadout wantedLoadout = GetGame().GetLoadoutManager().GetLoadoutByIndex(wantedIdx);

			if (wantedLoadout)
				DbgUI.Text(string.Format("Wanted: %1 (%2)", wantedLoadout.GetLoadoutName(), wantedLoadout));
			else
				DbgUI.Text(string.Format("Wanted: None (Clear)"));

			if (DbgUI.Button("CanRequest"))
				CanRequestLoadout(wantedLoadout);
			if (DbgUI.Button("Request"))
				RequestLoadout(wantedLoadout);
		}
		DbgUI.End();
	}
	#endif
}
