void OnDeployMenuOpenDelegate();
typedef func OnDeployMenuOpenDelegate;
typedef ScriptInvokerBase<OnDeployMenuOpenDelegate> OnDeployMenuOpenInvoker;

void OnDeployMenuCloseDelegate();
typedef func OnDeployMenuCloseDelegate;
typedef ScriptInvokerBase<OnDeployMenuCloseDelegate> OnDeployMenuCloseInvoker;

class SCR_PlayerDeployMenuHandlerComponentClass : ScriptComponentClass
{
}

//! Component responsible for deploy menu management.
class SCR_PlayerDeployMenuHandlerComponent : ScriptComponent
{
	private SCR_RespawnComponent m_RespawnComponent;
	private SCR_PlayerController m_PlayerController;
	private SCR_PlayerFactionAffiliationComponent m_PlyFactionAffil;
	private bool m_bReady = true;
	private bool m_bWelcomeOpened;
	private bool m_bWelcomeClosed;
	protected RplId m_iLastUsedSpawnPoint = RplId.Invalid();

	private static ref OnDeployMenuCloseInvoker s_OnMenuClose;

	private DeployMenuSystem m_DeployMenuSystem;
	private SCR_MenuSpawnLogic m_SpawnLogic;
	private SCR_RespawnSystemComponent m_RespawnSystem;

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!GetGame().InPlayMode())
			return;

		m_RespawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (m_RespawnSystem)
			m_SpawnLogic = SCR_MenuSpawnLogic.Cast(m_RespawnSystem.GetSpawnLogic());

		if (!m_SpawnLogic)
			return;

		World world = GetOwner().GetWorld();
		m_DeployMenuSystem = DeployMenuSystem.Cast(world.FindSystem(DeployMenuSystem));
		m_DeployMenuSystem.Register(this);

		m_RespawnComponent = SCR_RespawnComponent.Cast(owner.FindComponent(SCR_RespawnComponent));
		if (!m_RespawnComponent)
		{
			Print(string.Format("%1 could not find %2!",
				Type().ToString(), SCR_RespawnComponent),
				LogLevel.ERROR);
		}

		m_PlayerController = SCR_PlayerController.Cast(owner);
		if (!m_PlayerController)
		{
			Print(string.Format("%1 is not attached in %2 hierarchy! (%1 should be a child of %3!)",
				Type().ToString(), SCR_PlayerController, SCR_RespawnComponent),
				LogLevel.ERROR);
		}

		m_PlyFactionAffil = SCR_PlayerFactionAffiliationComponent.Cast(owner.FindComponent(SCR_PlayerFactionAffiliationComponent));

		m_RespawnComponent.GetOnRespawnReadyInvoker_O().Insert(OnRespawnReady);
		m_RespawnComponent.GetOnRespawnResponseInvoker_O().Insert(SetNotReady);

		SCR_ReconnectSynchronizationComponent reconnectComp = SCR_ReconnectSynchronizationComponent.Cast(owner.FindComponent(SCR_ReconnectSynchronizationComponent));
		if (reconnectComp)
			reconnectComp.GetOnPlayerReconnect().Insert(OnPlayerReconnect);

		SCR_WelcomeScreenComponent welcomeScreenComp = SCR_WelcomeScreenComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_WelcomeScreenComponent));
		if (!welcomeScreenComp)
			SetWelcomeClosed();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] timeSlice
	void Update(float timeSlice)
	{
#ifdef ENABLE_DIAG
		if (SCR_RespawnComponent.Diag_IsCLISpawnEnabled())
			return;
#endif
		if (!m_bReady)
			return;

		if (CanOpenWelcomeScreen())
		{
			if (!IsWelcomeScreenOpen())
				GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.WelcomeScreenMenu);

			return;

		}
		else if (IsWelcomeScreenOpen())
		{
			CloseWelcomeScreen();
			return;
		}

		if (CanOpenMenu())
		{
			if (HasPlayableFaction() && !SCR_RoleSelectionMenu.GetRoleSelectionMenu())
			{
				if (!SCR_DeployMenuMain.GetDeployMenu())
				{
					SCR_RoleSelectionMenu.CloseRoleSelectionMenu();
					SCR_DeployMenuMain.OpenDeployMenu();
				}
			}
			else if (!SCR_RoleSelectionMenu.GetRoleSelectionMenu())
			{
				SCR_DeployMenuMain.CloseDeployMenu();
				SCR_RoleSelectionMenu.OpenRoleSelectionMenu();
			}
		}
		else if (IsMenuOpen())
		{
			CloseMenu();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsMenuOpen()
	{
		return SCR_DeployMenuMain.GetDeployMenu() || SCR_RoleSelectionMenu.GetRoleSelectionMenu();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsWelcomeScreenOpen()
	{
		return GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.WelcomeScreenMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseWelcomeScreen()
	{
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.WelcomeScreenMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseMenu()
	{
		if (m_PlayerController.GetPlayerId() != GetGame().GetPlayerController().GetPlayerId())
			return;

		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.WelcomeScreenMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.RespawnSuperMenu);
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.RoleSelectionDialog);

		if (s_OnMenuClose)
			s_OnMenuClose.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasPlayableFaction()
	{
		if (!m_PlyFactionAffil)
			return false;

		SCR_Faction faction = SCR_Faction.Cast(m_PlyFactionAffil.GetAffiliatedFaction());

		return (faction && faction.IsPlayable());
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanOpenMenu()
	{
		if (!m_bWelcomeClosed)
			return false;

		bool hasEntity = (m_PlayerController.IsPossessing() || m_PlayerController.GetControlledEntity());
		bool hasDeadEntity = false;
		ChimeraCharacter character = ChimeraCharacter.Cast(m_PlayerController.GetControlledEntity());
		if (character)
			hasDeadEntity = character.GetCharacterController().IsDead();

		bool canOpen = (
			!SCR_EditorManagerEntity.IsOpenedInstance() &&
			(!hasEntity || hasDeadEntity) &&
			(SCR_BaseGameMode.Cast(GetGame().GetGameMode()).GetState() != SCR_EGameModeState.POSTGAME)
		);

		return canOpen;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanOpenWelcomeScreen()
	{
		bool hasEntity = (m_PlayerController.IsPossessing() || m_PlayerController.GetControlledEntity());

		if (m_bWelcomeClosed || hasEntity)
			return false;

		bool hasDeadEntity = false;
		ChimeraCharacter character = ChimeraCharacter.Cast(m_PlayerController.GetControlledEntity());
		if (character)
			hasDeadEntity = character.GetCharacterController().IsDead();

		bool canOpen = (
			!SCR_EditorManagerEntity.IsOpenedInstance() &&
			(!hasEntity || hasDeadEntity) &&
			(SCR_BaseGameMode.Cast(GetGame().GetGameMode()).GetState() != SCR_EGameModeState.POSTGAME)
		);

		return canOpen;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRespawnReady()
	{
		if (m_SpawnLogic && m_SpawnLogic.GetWaitForSpawnPoints())
		{
			HandleWaitForSpawnPoints();
			return;
		}

		// Delay to next frame as current character deleted replication could by applied later than the respawn system RPC this event is raised from.
		GetGame().GetCallqueue().Call(SetReadyDelayed);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetReadyDelayed()
	{
		IEntity controlledEntity = m_PlayerController.GetControlledEntity();
		if (!controlledEntity || controlledEntity.IsDeleted())
		{
			SetReady();
			return;
		}

		const float delay = m_SpawnLogic.GetDeployMenuOpenDelay() * 1000.0;
		GetGame().GetCallqueue().CallLater(SetReady, delay, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetReady()
	{
		if (!m_bReady)
			return;

		if (m_RespawnSystem)
			m_RespawnSystem.DestroyLoadingPlaceholder();

		m_DeployMenuSystem.SetReady(true);
	}

	//------------------------------------------------------------------------------------------------
	void SetWelcomeClosed()
	{
		m_bWelcomeClosed = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerReconnect(int state)
	{
		SetWelcomeClosed();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetNotReady(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response)
	{
		if (response == SCR_ESpawnResult.OK)
		{
			SCR_DeployMenuMain.CloseDeployMenu();
			SCR_RoleSelectionMenu.CloseRoleSelectionMenu();

			CloseWelcomeScreen();
			SetWelcomeClosed();

			m_DeployMenuSystem.SetReady(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static OnDeployMenuCloseInvoker SGetOnMenuClosed()
	{
		if (!s_OnMenuClose)
			s_OnMenuClose = new OnDeployMenuCloseInvoker();

		return s_OnMenuClose;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDelete(IEntity owner)
	{
		if (s_OnMenuClose)
			delete s_OnMenuClose;

		if (m_DeployMenuSystem)
			m_DeployMenuSystem.Unregister(this);

		if (m_RespawnSystem)
			m_RespawnSystem.DestroyLoadingPlaceholder();

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_PlayerController GetPlayerController()
	{
		return m_PlayerController;
	}

	//------------------------------------------------------------------------------------------------
	RplId GetLastUsedSpawnPointId()
	{
		return m_iLastUsedSpawnPoint;
	}

	//------------------------------------------------------------------------------------------------
	void SetLastUsedSpawnPointId(RplId id)
	{
		if (id != m_iLastUsedSpawnPoint)
			m_iLastUsedSpawnPoint = id;
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleWaitForSpawnPoints()
	{
		if (!m_PlyFactionAffil)
			return;

		const FactionKey factionKey = m_PlyFactionAffil.GetAffiliatedFactionKey();
		if (factionKey.IsEmpty())
		{
			m_PlyFactionAffil.GetOnPlayerFactionChangedInvoker().Insert(OnPlayerFactionAssigned);
			return; // We must wait until we have a faction assigned
		}

		CheckSpawnsOrWait(factionKey);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerFactionAssigned(SCR_PlayerFactionAffiliationComponent component, Faction previous, Faction current)
	{
		if (!current)
			return;

		m_PlyFactionAffil.GetOnPlayerFactionChangedInvoker().Remove(OnPlayerFactionAssigned);
		CheckSpawnsOrWait(current.GetFactionKey());
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckSpawnsOrWait(const FactionKey factionKey)
	{
		if (SCR_SpawnPoint.GetSpawnPointCountForFaction(factionKey) > 0)
		{
			m_bReady = true;
			GetGame().GetCallqueue().Call(SetReadyDelayed);
			return;
		}
		
		// Wait for more to be added
		SCR_SpawnPoint.Event_SpawnPointAdded.Insert(OnPlayerSpawnPointAdded);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawnPointAdded(SCR_SpawnPoint sp)
	{
		if (sp.GetFactionKey() != m_PlyFactionAffil.GetAffiliatedFactionKey())
			return;
		
		SCR_SpawnPoint.Event_SpawnPointAdded.Remove(OnPlayerSpawnPointAdded);

		m_bReady = true;
		GetGame().GetCallqueue().Call(SetReadyDelayed);
	}
}
