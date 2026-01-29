void OnSpawnPointRplIdSet(RplId id);
typedef func OnSpawnPointRplIdSet;
typedef ScriptInvokerBase<OnSpawnPointRplIdSet> OnSpawnPointRplIdSetInvoker;
//------------------------------------------------------------------------------------------------
//! Base deploy menu class.
class SCR_DeployMenuBase : ChimeraMenuBase
{
	protected SCR_InputButtonComponent m_PauseButton;
	protected SCR_InputButtonComponent m_GameMasterButton;
	protected SCR_InputButtonComponent m_ChatButton;

	protected SCR_ChatPanel m_ChatPanel;
	protected SCR_MapEntity m_MapEntity;

	protected static ref OnDeployMenuOpenInvoker s_OnMenuOpen;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		if (s_OnMenuOpen)
			s_OnMenuOpen.Invoke();

		m_GameMasterButton = SCR_InputButtonComponent.GetInputButtonComponent("Editor", GetRootWidget());
		if (m_GameMasterButton)
		{
			SCR_EditorManagerEntity editor = SCR_EditorManagerEntity.GetInstance();
			if (editor)
			{
				m_GameMasterButton.SetVisible(!editor.IsLimitedInstance());
				editor.GetOnLimitedChange().Insert(OnEditorLimitedChanged);
			}
		}
		
		super.OnMenuOpen();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpened()
	{
		// Mute sounds
		// If menu is opened before loading screen is closed, wait for closing
		if (ArmaReforgerLoadingAnim.IsOpen())
			ArmaReforgerLoadingAnim.m_onExitLoadingScreen.Insert(MuteSounds);
		else
			MuteSounds();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		MuteSounds(false);
		if (m_MapEntity && m_MapEntity.IsOpen())
			m_MapEntity.CloseMap();

		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		MuteSounds(false);
		if (m_MapEntity && m_MapEntity.IsOpen())
			m_MapEntity.CloseMap();		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OpenPlayerList()
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.PlayerListMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] mute
	void MuteSounds(bool mute = true)
	{
		if (!IsOpen())
			return;
		
		AudioSystem.SetMasterVolume(AudioSystem.SFX, !mute);
		AudioSystem.SetMasterVolume(AudioSystem.VoiceChat, !mute);
		AudioSystem.SetMasterVolume(AudioSystem.Dialog, !mute);
		
		ArmaReforgerLoadingAnim.m_onExitLoadingScreen.Remove(MuteSounds);
	}

	//! If limited, don't show the game master switch button.
	protected void OnEditorLimitedChanged(bool limited)
	{
		m_GameMasterButton.SetVisible(!limited);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static OnDeployMenuOpenInvoker SGetOnMenuOpen()
	{
		if (!s_OnMenuOpen)
			s_OnMenuOpen = new OnDeployMenuOpenInvoker();
		
		return s_OnMenuOpen;
	}
}

//------------------------------------------------------------------------------------------------
//! Main deploy menu with the map present.
class SCR_DeployMenuMain : SCR_DeployMenuBase
{	
	protected SCR_DeployMenuHandler m_MenuHandler;

	protected SCR_LoadoutRequestUIComponent m_LoadoutRequestUIHandler;
	protected SCR_GroupRequestUIComponent m_GroupRequestUIHandler;
	protected SCR_SpawnPointRequestUIComponent m_SpawnPointRequestUIHandler;

	protected ref MapConfiguration m_MapConfigDeploy = new MapConfiguration();
	protected SCR_MapUIElementContainer m_UIElementContainer;

	protected SCR_BaseGameMode m_GameMode;
	protected SCR_RespawnComponent m_SpawnRequestManager;
	protected RplId m_iSelectedSpawnPointId = RplId.Invalid();
	protected SCR_PlayerDeployMenuHandlerComponent m_PlayerMenuHandler;

	protected Widget m_wLoadingSpinner;
	protected SCR_LoadingSpinner m_LoadingSpinner;

	protected FactionManager m_FactionManager;
	protected SCR_PlayerFactionAffiliationComponent m_PlyFactionAffilComp;

	protected SCR_PlayerLoadoutComponent m_PlyLoadoutComp;

	protected Widget m_wRespawnButton;
	protected SCR_DeployButton m_RespawnButton;

	protected Widget m_wMenuFrame;

	protected SCR_RespawnTimerComponent m_ActiveRespawnTimer;	
	protected SCR_RespawnTimerComponent m_PlayerRespawnTimer;
	protected SCR_TimedSpawnPointComponent m_TimedSpawnPointTimer;
	protected int m_iPreviousTime = 0;
	protected bool m_bRespawnRequested = false;
	protected bool m_bSuppliesEnabled;
	protected SCR_RespawnSystemComponent m_RespawnSystemComp;
	protected SCR_RespawnComponent m_RespawnComponent;

	protected int m_iPlayerId;
	
	protected bool m_bMapContextAllowed = true;
	protected bool m_bInitialSpawnPointSet = false;
	protected bool m_bCanRespawnAtSpawnPoint;
	
	protected SCR_UIInfoSpawnRequestResult m_UIInfoSpawnRequestResult;
	protected bool m_bDisplayTime;
	
	protected float m_fCurrentCanSpawnUpdateTime;
	protected const float CHECK_CAN_SPAWN_SPAWNPOINT_TIME = 1; //~ Time (in seconds) when the system should request if the player can spawn. Note this request is send to server
	protected const string FALLBACK_DEPLOY_STRING = "#AR-ButtonSelectDeploy"; //~ Time (in seconds) when the system should request if the player can spawn. Note this request is send to server
	
	protected SCR_InputButtonComponent m_GroupOpenButton;
	
	//~ A timer that disables the respawn button for x seconds after it has been pressed to prevent players from sending multiple spawn requests
	protected float m_fCurrentDeployTimeOut;
	protected const float DEPLOY_TIME_OUT = 0.5;
	
	protected ref OnSpawnPointRplIdSetInvoker m_OnSpawnPointSet;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] component
	//! \return
	//----------------------------------------------------------------------------------------------
	int GetLoadoutCost(SCR_PlayerLoadoutComponent component)
	{
		int cost;
		
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return 0;
			
		SCR_Faction faction = SCR_Faction.Cast(m_PlyFactionAffilComp.GetAffiliatedFaction());
		if (!faction)
			return 0;
			
		SCR_BasePlayerLoadout playerLoadout = component.GetLoadout();
		if (!playerLoadout)
			return 0;
			
		ResourceName loadoutResource = playerLoadout.GetLoadoutResource();
		if (!loadoutResource)
			return 0;
			
		Resource resource = Resource.Load(loadoutResource);
		if (!resource)
			return 0;
			
		SCR_EntityCatalogEntry entry = entityCatalogManager.GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType.CHARACTER, resource.GetResource().GetResourceName(), faction);
		if (!entry)
			return 0;
			
		SCR_EntityCatalogSpawnerData data = SCR_EntityCatalogSpawnerData.Cast(entry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (!data)
			return 0;
	
		cost = data.GetSupplyCost();
		
		return cost;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Sets map context active based on whether or not any of the selectors are focused with a gamepad.
	void AllowMapContext(bool allow)
	{
		m_bMapContextAllowed = allow;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if map context is allowed
	//! \return
	bool GetAllowMapContext()
	{
		return m_bMapContextAllowed;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		ResetRespawnResultVars();
		
		m_wMenuFrame = GetRootWidget().FindAnyWidget("MenuFrame");

		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		m_MenuHandler = SCR_DeployMenuHandler.Cast(GetRootWidget().FindHandler(SCR_DeployMenuHandler));
		m_RespawnSystemComp = SCR_RespawnSystemComponent.GetInstance();
		m_RespawnComponent = SCR_RespawnComponent.GetInstance();
		if (m_RespawnComponent)
			m_RespawnComponent.GetOnCanRespawnResponseInvoker_O().Insert(OnCanRespawnRequestResponse);	
		
		m_MapEntity = SCR_MapEntity.GetMapInstance();

		if (!m_MapEntity)
		{
			Debug.Error("Map entity is missing in the world! Deploy menu won't work correctly.");
		}

		FindRequestHandlers();

		m_wRespawnButton = GetRootWidget().FindAnyWidget("RespawnButton");

		m_RespawnButton = SCR_DeployButton.Cast(m_wRespawnButton.FindHandler(SCR_DeployButton));
		if (m_RespawnButton)
		{
			//~ Set enabled false so players cannot press the button right after init
			m_RespawnButton.SetEnabled(false);
			m_RespawnButton.m_OnActivated.Insert(RequestRespawn);
		}
		
		Widget spinnerRoot = GetRootWidget().FindAnyWidget("LoadingSpinner");
		m_wLoadingSpinner = spinnerRoot.FindAnyWidget("Spinner");
		if (m_wLoadingSpinner)
			m_LoadingSpinner = SCR_LoadingSpinner.Cast(m_wLoadingSpinner.FindHandler(SCR_LoadingSpinner));

		m_FactionManager = GetGame().GetFactionManager();
		if (!m_FactionManager)
		{
			Print("Cannot find faction manager, respawn menu functionality will be broken.", LogLevel.ERROR);
		}

		m_PlayerRespawnTimer = SCR_RespawnTimerComponent.Cast(m_GameMode.FindComponent(SCR_RespawnTimerComponent));
		m_TimedSpawnPointTimer = SCR_TimedSpawnPointComponent.Cast(m_GameMode.FindComponent(SCR_TimedSpawnPointComponent));
		m_ActiveRespawnTimer = m_PlayerRespawnTimer;

		PlayerController pc = GetGame().GetPlayerController();
		m_iPlayerId = pc.GetPlayerId();

		m_PlyFactionAffilComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!m_PlyFactionAffilComp)
		{
			Print("Cannot find player faction affiliation component!", LogLevel.ERROR);
		}

		m_PlyLoadoutComp = SCR_PlayerLoadoutComponent.Cast(pc.FindComponent(SCR_PlayerLoadoutComponent));
		if (!m_PlyLoadoutComp)
		{
			Print("Cannot find player loadout component!", LogLevel.ERROR);
		}

		m_SpawnRequestManager = SCR_RespawnComponent.Cast(pc.GetRespawnComponent());
		m_PlayerMenuHandler = SCR_PlayerDeployMenuHandlerComponent.Cast(pc.FindComponent(SCR_PlayerDeployMenuHandlerComponent));

		Widget chat = GetRootWidget().FindAnyWidget("ChatPanel");
		if (chat)
			m_ChatPanel = SCR_ChatPanel.Cast(chat.FindHandler(SCR_ChatPanel));

		m_ChatButton = SCR_InputButtonComponent.GetInputButtonComponent("ChatButton", GetRootWidget());
		if (m_ChatButton)
			m_ChatButton.m_OnActivated.Insert(OnChatToggle);

		m_PauseButton = SCR_InputButtonComponent.GetInputButtonComponent("PauseButton", GetRootWidget());
		if (m_PauseButton)
			m_PauseButton.m_OnActivated.Insert(OnPauseMenu);

		m_GroupOpenButton = SCR_InputButtonComponent.GetInputButtonComponent("GroupManager", GetRootWidget());
		if (m_GroupOpenButton)
			m_GroupOpenButton.m_OnActivated.Insert(OpenGroupMenu);
		
		HookEvents();
		InitMapDeploy();

		bool pause = m_ActiveRespawnTimer == null;
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		gameMode.PauseGame(pause, SCR_EPauseReason.MENU);

		SCR_MenuSpawnLogic logic = SCR_MenuSpawnLogic.Cast(m_RespawnSystemComp.GetSpawnLogic());
		if (logic.GetUseFadeEffect())
		{
			Widget fade = GetRootWidget().FindAnyWidget("FadeEffect");
			fade.SetVisible(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();

		if (m_MapEntity && m_MapEntity.IsOpen())
			m_MapEntity.CloseMap();
		
		if (m_RespawnComponent)
			m_RespawnComponent.GetOnCanRespawnResponseInvoker_O().Remove(OnCanRespawnRequestResponse);	
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		GetGame().GetInputManager().RemoveActionListener("ShowScoreboard", EActionTrigger.DOWN, OpenPlayerList);
		GetGame().GetInputManager().RemoveActionListener("DeployMenuSelect", EActionTrigger.DOWN, RequestRespawn);
		
		GetGame().GetInputManager().RemoveActionListener("SpawnPointNext", EActionTrigger.DOWN, NextSpawn);
		GetGame().GetInputManager().RemoveActionListener("SpawnPointPrev", EActionTrigger.DOWN, PrevSpawn);
		
		super.OnMenuFocusLost();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{ 
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.AutoInit();

		GetGame().GetInputManager().AddActionListener("ShowScoreboard", EActionTrigger.DOWN, OpenPlayerList);
		GetGame().GetInputManager().AddActionListener("DeployMenuSelect", EActionTrigger.DOWN, RequestRespawn);

		GetGame().GetInputManager().AddActionListener("SpawnPointNext", EActionTrigger.DOWN, NextSpawn);
		GetGame().GetInputManager().AddActionListener("SpawnPointPrev", EActionTrigger.DOWN, PrevSpawn);
		
		super.OnMenuFocusGained();
	}

	//! Select next available spawn point.
	protected void NextSpawn()
	{
		if (!m_bRespawnRequested && !m_LoadoutRequestUIHandler.IsSelectorFocused() && !m_SpawnPointRequestUIHandler.IsSelectorFocused())
			m_SpawnPointRequestUIHandler.CycleSpawnPoints();
	}

	//! Select previous available spawn point.
	protected void PrevSpawn()
	{
		if (!m_bRespawnRequested && !m_LoadoutRequestUIHandler.IsSelectorFocused() && !m_SpawnPointRequestUIHandler.IsSelectorFocused())
			m_SpawnPointRequestUIHandler.CycleSpawnPoints(false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpened()
	{
		super.OnMenuOpened();
		Faction plyFaction = m_PlyFactionAffilComp.GetAffiliatedFaction();

		m_LoadoutRequestUIHandler.ShowAvailableLoadouts(plyFaction);
		m_GroupRequestUIHandler.ShowAvailableGroups(plyFaction);
		if (!m_GroupRequestUIHandler.GetPlayerGroup())
			m_GroupRequestUIHandler.JoinGroupAutomatically();
		m_SpawnPointRequestUIHandler.ShowAvailableSpawnPoints(plyFaction);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();

		if (m_MapEntity && m_MapEntity.IsOpen())
			m_MapEntity.CloseMap();
		
		m_fCurrentDeployTimeOut = 0;
		
		// Unpause
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		gameMode.PauseGame(false, SCR_EPauseReason.MENU);
	}
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		m_RespawnButton.SetSuppliesEnabled(m_bSuppliesEnabled);
		GetGame().GetInputManager().ActivateContext("DeployMenuContext");
		GetGame().GetInputManager().ActivateContext("DeployMenuMapContext");
		
		if (m_bMapContextAllowed && m_MapEntity && m_MapEntity.IsOpen())
			GetGame().GetInputManager().ActivateContext("MapContext");
		
		if (m_LoadingSpinner && m_wLoadingSpinner.IsVisible())
		{
			m_LoadingSpinner.Update(tDelta);			
			m_RespawnButton.UpdateSpinner(tDelta);
		}

		if (m_ChatPanel)
			m_ChatPanel.OnUpdateChat(tDelta);
		
		//~ Update if player can spawn
		m_fCurrentCanSpawnUpdateTime += tDelta;
		if (m_fCurrentCanSpawnUpdateTime >= CHECK_CAN_SPAWN_SPAWNPOINT_TIME)
		{
			m_fCurrentCanSpawnUpdateTime -= CHECK_CAN_SPAWN_SPAWNPOINT_TIME;
			
			//~ Only check if can spawn if spawn is requested
			if (!m_bRespawnRequested && m_LoadoutRequestUIHandler.GetPlayerLoadout())
				m_RespawnComponent.CanSpawn(new SCR_SpawnPointSpawnData(m_LoadoutRequestUIHandler.GetPlayerLoadout().GetLoadoutResource(), m_iSelectedSpawnPointId));
		}
		
		if (m_fCurrentDeployTimeOut > 0)
			m_fCurrentDeployTimeOut -= tDelta;
		
		//~ If can respawn
		int remainingTime = -1;
			
		//~ Timer calculation and sound event
		if (m_ActiveRespawnTimer)
		{
			float spawnPointTime = 0;
			if (SCR_SpawnPoint.GetSpawnPointByRplId(m_iSelectedSpawnPointId))
				spawnPointTime = SCR_SpawnPoint.GetSpawnPointByRplId(m_iSelectedSpawnPointId).GetRespawnTime();
			remainingTime = m_ActiveRespawnTimer.GetPlayerRemainingTime(m_iPlayerId, spawnPointTime);
			
			if (remainingTime > 0)
			{
				if (remainingTime != m_iPreviousTime)
				{
					if (m_bDisplayTime)
					{
						SCR_UISoundEntity.SetSignalValueStr("countdownValue", remainingTime);
						SCR_UISoundEntity.SetSignalValueStr("maxCountdownValue", m_ActiveRespawnTimer.GetRespawnTime());					
						SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RESPAWN_COUNTDOWN);
					}
					
					m_iPreviousTime = remainingTime;
				}
				
				if (m_bDisplayTime)
				{
					if (m_UIInfoSpawnRequestResult)
						m_RespawnButton.SetText(m_bCanRespawnAtSpawnPoint, m_UIInfoSpawnRequestResult.GetNameWithTimer(), remainingTime);
					else 
						m_RespawnButton.SetText(m_bCanRespawnAtSpawnPoint, FALLBACK_DEPLOY_STRING, remainingTime);
				}
				else 
				{
					if (m_UIInfoSpawnRequestResult)
						m_RespawnButton.SetText(m_bCanRespawnAtSpawnPoint, m_UIInfoSpawnRequestResult.GetName());
					else 
						m_RespawnButton.SetText(m_bCanRespawnAtSpawnPoint, FALLBACK_DEPLOY_STRING);
				}
			}
			else
			{
				if (m_iPreviousTime > 0)
				{
					if (m_bDisplayTime)
						SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RESPAWN_COUNTDOWN_END);
					
					m_iPreviousTime = remainingTime;
				}
			
				if (m_UIInfoSpawnRequestResult)
					m_RespawnButton.SetText(m_bCanRespawnAtSpawnPoint, m_UIInfoSpawnRequestResult.GetName());
				else 
					m_RespawnButton.SetText(m_bCanRespawnAtSpawnPoint, FALLBACK_DEPLOY_STRING);
			}				
		}
		
		UpdateRespawnButton();
	}
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	protected void ResetRespawnResultVars()
	{
		m_bDisplayTime = false;
		m_bCanRespawnAtSpawnPoint = false;
		m_fCurrentCanSpawnUpdateTime = CHECK_CAN_SPAWN_SPAWNPOINT_TIME; //~ Makes sure to send a request if spawnpoint can be spawned at
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Response from server when player sends a request to spawn
	protected void OnCanRespawnRequestResponse(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response, SCR_SpawnData data)
	{
		m_bCanRespawnAtSpawnPoint = (response == SCR_ESpawnResult.OK);
		SCR_BaseSpawnPointRequestResultInfo spawnPointResultInfo = m_RespawnSystemComp.GetSpawnPointRequestResultInfo(requestComponent, response, data);
		
		if (!spawnPointResultInfo)
		{
			m_RespawnButton.SetText(m_bCanRespawnAtSpawnPoint, FALLBACK_DEPLOY_STRING);
			UpdateRespawnButton();
			
			return;
		}
		
		m_UIInfoSpawnRequestResult = spawnPointResultInfo.GetUIInfo();
		m_bDisplayTime = spawnPointResultInfo.ShowRespawnTime();
		
		m_RespawnButton.SetSupplyCost(GetLoadoutCost(m_PlyLoadoutComp));
		
		m_bSuppliesEnabled = false;
		
		SCR_SpawnPoint spawnPoint = SCR_SpawnPoint.GetSpawnPointByRplId(m_SpawnPointRequestUIHandler.GetCurrentRplId());
		
		if (!spawnPoint)
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(spawnPoint);
		
		if (resourceComponent)
			m_bSuppliesEnabled = true;
		else
		{
			IEntity parentEntity = spawnPoint.GetParent();
			
			if (parentEntity)
			{
				resourceComponent = SCR_ResourceComponent.FindResourceComponent(parentEntity);
				
				if (resourceComponent)
					m_bSuppliesEnabled = true;
			}
		}
		
		m_RespawnButton.SetSuppliesEnabled(m_bSuppliesEnabled);
		
		//~ Timer calculation and sound event
		if (m_ActiveRespawnTimer)
		{
			int remainingTime = m_ActiveRespawnTimer.GetPlayerRemainingTime(m_iPlayerId);
			
			if (remainingTime > 0)
			{
				if (m_UIInfoSpawnRequestResult)
					m_RespawnButton.SetText(m_bCanRespawnAtSpawnPoint, m_UIInfoSpawnRequestResult.GetNameWithTimer(), remainingTime);
				else 
					m_RespawnButton.SetText(m_bCanRespawnAtSpawnPoint, FALLBACK_DEPLOY_STRING, remainingTime);
				
				UpdateRespawnButton();
				return;
			}
		}
		
		if (m_UIInfoSpawnRequestResult)
			m_RespawnButton.SetText(m_bCanRespawnAtSpawnPoint, m_UIInfoSpawnRequestResult.GetName());
		else 
			m_RespawnButton.SetText(m_bCanRespawnAtSpawnPoint, FALLBACK_DEPLOY_STRING);
		
		UpdateRespawnButton();
	}

	//------------------------------------------------------------------------------------------------
	//! Find all components in the layout that are needed in order for deploy menu to function correctly.
	protected void FindRequestHandlers()
	{
		m_LoadoutRequestUIHandler = m_MenuHandler.GetLoadoutRequestHandler();
		if (!m_LoadoutRequestUIHandler)
		{
			Print("Failed to find SCR_LoadoutRequestUIComponent!", LogLevel.ERROR);
		}
		else
		{
			m_LoadoutRequestUIHandler.RegisterOnSpawnPointInvoker(GetOnSpawnPointRplIdSet());
		}

		m_GroupRequestUIHandler = m_MenuHandler.GetGroupRequestHandler();
		if (!m_GroupRequestUIHandler)
		{
			Print("Failed to find SCR_GroupRequestUIComponent!", LogLevel.ERROR);
		}

		m_SpawnPointRequestUIHandler = m_MenuHandler.GetSpawnPointRequestHandler();
		if (!m_SpawnPointRequestUIHandler)
		{
			Print("Failed to find SCR_SpawnPointRequestUIComponent!", LogLevel.ERROR);
		}
	}

	//! Initialize necessary callbacks.
	protected void HookEvents()
	{
		m_PlyFactionAffilComp.GetOnPlayerFactionRequestInvoker_O().Insert(OnPlayerFactionRequest);
		m_PlyFactionAffilComp.GetOnPlayerFactionResponseInvoker_O().Insert(OnPlayerFactionResponse);

		m_PlyLoadoutComp.GetOnPlayerLoadoutRequestInvoker_O().Insert(OnPlayerLoadoutRequest);
		m_PlyLoadoutComp.GetOnPlayerLoadoutResponseInvoker_O().Insert(OnPlayerLoadoutResponse);

		m_SpawnRequestManager.GetOnRespawnRequestInvoker_O().Insert(OnRespawnRequest);
		m_SpawnRequestManager.GetOnRespawnResponseInvoker_O().Insert(OnRespawnResponse);
		
		m_SpawnPointRequestUIHandler.GetOnSpawnPointSelected().Insert(OnSpawnPointSelected);
		m_GroupRequestUIHandler.GetOnLocalPlayerGroupJoined().Insert(OnLocalPlayerGroupJoined);
		m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);

		m_GameMode.GetOnPreloadFinished().Insert(HideLoading);
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----	
	//------------------------------------------------------------------------------------------------
	protected void OnMapOpen(MapConfiguration config)
	{
		m_MapEntity.SetZoom(1);
		
		m_UIElementContainer = SCR_MapUIElementContainer.Cast(m_MapEntity.GetMapUIComponent(SCR_MapUIElementContainer));
		if (m_UIElementContainer)
			m_UIElementContainer.GetOnSpawnPointSelected().Insert(SetSpawnPointExt);
		
		SCR_SpawnPoint sp = SCR_SpawnPoint.GetSpawnPointByRplId(m_iSelectedSpawnPointId);
		if (sp)
			GetGame().GetCallqueue().CallLater(SetInitialSpawnPoint, 0, false, m_iSelectedSpawnPointId); // called the next frame because of widget init order

		SCR_MenuSpawnLogic logic = SCR_MenuSpawnLogic.Cast(m_RespawnSystemComp.GetSpawnLogic());
		if (logic.GetUseFadeEffect())
		{
			Widget fade = GetRootWidget().FindAnyWidget("FadeEffect");
			const int timeBeforeFade = 1000;
			const float fadeTime = 0.3;
			GetGame().GetCallqueue().CallLater(AnimateWidget.Opacity, timeBeforeFade, false, fade, 0, fadeTime, true);
		}
			
	}
	//---- REFACTOR NOTE END ----

	//! Callback when player joins a group
	protected void OnLocalPlayerGroupJoined(SCR_AIGroup group)
	{
		if (m_SpawnPointRequestUIHandler)
			m_SpawnPointRequestUIHandler.UpdateRelevantSpawnPoints();
	}

	//! Initializes the map with deploy menu config.
	protected void InitMapDeploy()
	{
		if (!m_MapEntity || m_MapEntity.IsOpen())
			return;

		SCR_MapConfigComponent configComp = SCR_MapConfigComponent.Cast(m_GameMode.FindComponent(SCR_MapConfigComponent));
		if (!configComp)
			return;

		m_MapConfigDeploy = m_MapEntity.SetupMapConfig(EMapEntityMode.SPAWNSCREEN, configComp.GetSpawnMapConfig(), GetRootWidget());
		m_MapEntity.OpenMap(m_MapConfigDeploy);
	}

	//! Sets spawn point from an external source (ie. by clicking the spawn point icon).
	protected void SetSpawnPointExt(RplId id)
	{
		m_SpawnPointRequestUIHandler.SelectSpawnPointExt(id);
	}

	//! Sets initial spawn point when the deploy map is open for the first time.
	protected void SetInitialSpawnPoint(RplId spawnPointId)
	{
		SCR_SpawnPoint sp = SCR_SpawnPoint.GetSpawnPointByRplId(m_iSelectedSpawnPointId);
		m_bInitialSpawnPointSet = true;
		SetSpawnPoint(spawnPointId, false);
	}

	protected void OnSpawnPointSelected(RplId id)
	{
		SetSpawnPoint(id); // Handle default args
	}
	
	//! Sets the currently selected spawn point.
	protected void SetSpawnPoint(RplId id, bool smoothPan = true)
	{	
		ResetRespawnResultVars();
		m_iSelectedSpawnPointId = id;

		SCR_SpawnPoint sp = SCR_SpawnPoint.GetSpawnPointByRplId(id);
		if (!sp)
			return;

		FocusOnPoint(sp, smoothPan);

		if (m_UIElementContainer)
			m_UIElementContainer.OnSpawnPointSelectedExt(id);
		
		if (m_TimedSpawnPointTimer)
		{
			if (sp.IsTimed())
				m_ActiveRespawnTimer = m_TimedSpawnPointTimer;
			else
				m_ActiveRespawnTimer = m_PlayerRespawnTimer;
		}
		
		UpdateRespawnButton();
		if (m_OnSpawnPointSet)
			m_OnSpawnPointSet.Invoke(id);
	}

	//! Centers map to a specific spawn point.
	protected void FocusOnPoint(notnull SCR_SpawnPoint spawnPoint, bool smooth = true)
	{
		if (!m_bInitialSpawnPointSet || !m_MapEntity || !m_MapEntity.IsOpen())
			return;

		vector o = spawnPoint.GetOrigin();

		float x, y;
		m_MapEntity.WorldToScreen(o[0], o[2], x, y);

		if (smooth)
			m_MapEntity.PanSmooth(x, y);
		else
			m_MapEntity.PanSmooth(x, y, 0.001); // since SetPan doesn't work correctly in some cases, just PanSmooth super fast
	}

	//! Hides loading spinner widget.
	protected void HideLoading()
	{
		m_wLoadingSpinner.SetVisible(false);
	}

	//! Sends a respawn request based on assigned loadout and selected spawn point.
	protected void RequestRespawn()
	{
		UpdateRespawnButton();
		
		if (!m_RespawnButton.IsEnabled())
			return;

		if (!m_iSelectedSpawnPointId.IsValid())
		{
			Debug.Error("Selected SpawnPointId is invalid!");
			return;
		}

		ResourceName resourcePrefab = ResourceName.Empty;
		if (m_LoadoutRequestUIHandler.GetPlayerLoadout())
			resourcePrefab = m_LoadoutRequestUIHandler.GetPlayerLoadout().GetLoadoutResource();
		else
		{
			Debug.Error("No player loadout assigned!");
			return;
		}

		m_fCurrentDeployTimeOut = DEPLOY_TIME_OUT;
		
		SCR_SpawnPointSpawnData rspData = new SCR_SpawnPointSpawnData(resourcePrefab, m_iSelectedSpawnPointId);
		if (m_PlayerMenuHandler)
			m_PlayerMenuHandler.SetLastUsedSpawnPointId(rspData.GetRplId());
		m_SpawnRequestManager.RequestSpawn(rspData);
	}

	//! Callback when player requests a faction.
	protected void OnPlayerFactionRequest(SCR_PlayerFactionAffiliationComponent component, int factionIndex)
	{
		m_wLoadingSpinner.SetVisible(true);
	}

	//! Callback when faction request receives a response.
	protected void OnPlayerFactionResponse(SCR_PlayerFactionAffiliationComponent component, int factionIndex, bool response)
	{
		if (response)
		{
			Faction assignedFaction = m_FactionManager.GetFactionByIndex(factionIndex);
			OnPlayerFactionSet(assignedFaction);
		}

		m_wLoadingSpinner.SetVisible(false);
	}

	//! Callback when player requests a loadout.
	protected void OnPlayerLoadoutRequest(SCR_PlayerLoadoutComponent component, int loadoutIndex)
	{
		m_wLoadingSpinner.SetVisible(true);
	}

	//! Callback when loadout request receives a response.
	protected void OnPlayerLoadoutResponse(SCR_PlayerLoadoutComponent component, int loadoutIndex, bool response)
	{
		if (response)
		{
			m_LoadoutRequestUIHandler.RefreshLoadoutPreview();
			m_RespawnButton.SetEnabled(true);
			m_RespawnButton.SetSupplyCost(GetLoadoutCost(component));
		}

		m_wLoadingSpinner.SetVisible(false);
		if (m_LoadoutRequestUIHandler)
			m_LoadoutRequestUIHandler.Unlock();
	}

	//----------------------------------------------------------------------------------------------
	protected void OnPlayerFactionSet(Faction assignedFaction)
	{
		if (!assignedFaction)
			return;

		if (m_LoadoutRequestUIHandler)
			m_LoadoutRequestUIHandler.ShowAvailableLoadouts(assignedFaction);
		
		if (m_GroupRequestUIHandler)
			m_GroupRequestUIHandler.ShowAvailableGroups(assignedFaction);
		
		if (m_SpawnPointRequestUIHandler)
			m_SpawnPointRequestUIHandler.ShowAvailableSpawnPoints(assignedFaction);

	}

	//! Toggle chat.
	protected void OnChatToggle()
	{
		if (!m_ChatPanel)
		{
			Widget chat = GetRootWidget().FindAnyWidget("ChatPanel");
			if (chat)
				m_ChatPanel = SCR_ChatPanel.Cast(chat.FindHandler(SCR_ChatPanel));
		}
		
		if (!m_ChatPanel || m_ChatPanel.IsOpen())
			return;

		SCR_ChatPanelManager.GetInstance().ToggleChatPanel(m_ChatPanel);
	}

	//! Opens pause menu.
	protected void OnPauseMenu()
	{
		MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PauseMenu, 0, true, false);

		PauseMenuUI pauseMenu = PauseMenuUI.Cast(menu);
		if (pauseMenu)
		{
			pauseMenu.FadeBackground(true, true);
			pauseMenu.DisableSettings();
		}
	}

	//! Callback when respawn request was sent for the player.
	protected void OnRespawnRequest(SCR_SpawnRequestComponent requestComponent)
	{
		m_bRespawnRequested = true;		
		GetRootWidget().SetEnabled(false);
		m_RespawnButton.SetEnabled(false);
		m_RespawnButton.ShowLoading(false);
		m_wLoadingSpinner.SetVisible(true);

		UpdateRespawnButton();
	}

	//! Callback when player respawn request received a response.
	protected void OnRespawnResponse(SCR_SpawnRequestComponent requestComponent, SCR_ESpawnResult response)
	{
		GetRootWidget().SetEnabled(true);
		m_wLoadingSpinner.SetVisible(false);
		if (response != SCR_ESpawnResult.OK)
		{
			m_RespawnButton.SetEnabled(true);
			m_RespawnButton.ShowLoading(true);
		}
		m_bRespawnRequested = false;
	}

	//! Sets respawn button enabled based on certain conditions.
	protected void UpdateRespawnButton()
	{
		int remainingTime = -1;
		
		if (m_ActiveRespawnTimer)
			remainingTime = m_ActiveRespawnTimer.GetPlayerRemainingTime(m_iPlayerId);
		
		bool hasGroup = true;
		if (m_GroupRequestUIHandler && m_GroupRequestUIHandler.IsEnabled())
			hasGroup = m_GroupRequestUIHandler.GetPlayerGroup();
		m_RespawnButton.SetEnabled(!m_bRespawnRequested && remainingTime <= 0 && m_fCurrentDeployTimeOut <= 0 && m_bCanRespawnAtSpawnPoint && m_PlyLoadoutComp.GetLoadout() != null && hasGroup);
	}

	protected void OpenGroupMenu()
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager || !groupManager.IsGroupMenuAllowed())
			return;
		
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.GroupMenu);
	}	

	//------------------------------------------------------------------------------------------------
	//! Opens deploy menu.
	static SCR_DeployMenuMain OpenDeployMenu()
	{
		GetGame().GetMenuManager().CloseAllMenus();
		if (!GetDeployMenu())
			GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.RespawnSuperMenu);
		
		return GetDeployMenu();
	}

	//------------------------------------------------------------------------------------------------
	//! As the name suggests, this method closes the deploy menu instance.
	static void CloseDeployMenu()
	{
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.RespawnSuperMenu);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the deploy menu instance.
	static SCR_DeployMenuMain GetDeployMenu()
	{
		return SCR_DeployMenuMain.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.RespawnSuperMenu));
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	SCR_SpawnPointRequestUIComponent GetSpawnPointRequestHandler()
	{
		return m_SpawnPointRequestUIHandler;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	OnSpawnPointRplIdSetInvoker GetOnSpawnPointRplIdSet()
	{
		if (!m_OnSpawnPointSet)
			m_OnSpawnPointSet = new OnSpawnPointRplIdSetInvoker();

		return m_OnSpawnPointSet;
	}
};

//------------------------------------------------------------------------------------------------
//! Component that handles the request respawn button.
class SCR_DeployButton : SCR_InputButtonComponent
{
	[Attribute("TextHint")]
	protected string m_sText;
	
	[Attribute("ShortcutInput")]
	protected string m_sShortcutName;
	
	[Attribute("HorizontalLayout1")]
	protected string m_sTextHolderName;
	
	protected TextWidget m_wText;
	protected Widget m_wShortcut;
	protected Widget m_wTextHolder;

	[Attribute("Spinner")]
	protected string m_sLoadingSpinner;
	
	[Attribute("Background")]
	protected string m_sBackground;
	
	protected Widget m_wLoadingSpinner;
	protected Widget m_wSupplies;
	protected Widget m_wMSARSuppliesText;
	protected Widget m_wMSARSupplies;
	protected Widget m_wBackgroundWidget;
	protected RichTextWidget m_wSuppliesText;

	protected SCR_LoadingSpinner m_LoadingSpinner;
	
	protected int m_iSupplyCost;
	protected bool m_bSuppliesEnabled;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wText = TextWidget.Cast(w.FindAnyWidget(m_sText));
		m_wShortcut = w.FindAnyWidget(m_sShortcutName);
		m_wTextHolder = w.FindAnyWidget(m_sTextHolderName);
		m_wLoadingSpinner = w.FindAnyWidget(m_sLoadingSpinner);
		m_LoadingSpinner = SCR_LoadingSpinner.Cast(m_wLoadingSpinner.FindHandler(SCR_LoadingSpinner));
		m_wSupplies = w.FindAnyWidget("w_Supplies");
		m_wBackgroundWidget = w.FindAnyWidget(m_sBackground);
		
		if (m_wBackgroundWidget)
			GetOnUpdateEnableColor().Insert(UpdateBackground);
		
		if (m_wSupplies)
			m_wSuppliesText = RichTextWidget.Cast(m_wSupplies.FindAnyWidget("SuppliesLoadoutText"));
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] enabled
	//----------------------------------------------------------------------------------------------
	void SetSuppliesEnabled(bool enabled)
	{
		m_bSuppliesEnabled = enabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] cost
	//----------------------------------------------------------------------------------------------
	void SetSupplyCost(int cost)
	{
		m_iSupplyCost = cost;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	//----------------------------------------------------------------------------------------------
	int GetSupplyCost()
	{
		return m_iSupplyCost;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set text of the button.
	void SetText(bool deployEnabled, string text, float remainingTime = -1)
	{
		if (!m_wText)
			return;
		
		if (m_wShortcut)
			m_wShortcut.SetVisible(deployEnabled && remainingTime <= 0);
		
		if (!text.IsEmpty())
		{
			if (remainingTime >= 0)
			{
				string respawnTime = SCR_FormatHelper.GetTimeFormatting(remainingTime, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);		
				m_wText.SetTextFormat(text, respawnTime);
				return;
			}

			m_wText.SetText(text);
			
			if (m_wSupplies)
				m_wSupplies.SetVisible(m_iSupplyCost > 0 && m_bSuppliesEnabled);
			
			if (m_wSuppliesText)
				m_wSuppliesText.SetText(string.ToString(m_iSupplyCost));
			return;
		}
		
		//~ Empty string given string
		if (remainingTime >= 0)
		{
			string respawnTime = SCR_FormatHelper.GetTimeFormatting(remainingTime, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);		
			m_wText.SetTextFormat("ERROR %1", respawnTime);
			return;
		}
			
		m_wText.SetText("ERROR");
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update the loading spinner widget.
	void UpdateSpinner(float timeSlice)
	{
		if (m_wLoadingSpinner && m_wLoadingSpinner.IsVisible())
			m_LoadingSpinner.Update(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! Set loading spinner widget visible.
	void ShowLoading(bool show)
	{
		m_wTextHolder.SetVisible(show);
		m_wLoadingSpinner.SetVisible(!show);
	}
	
	//------------------------------------------------------------------------------------------------
	//!  Change color of background depending on if button is enabled.
	void UpdateBackground()
	{
		Color color;
		if (!m_wRoot.IsEnabled())
			color = m_ActionDisabled;
		else
			color = m_ActionDefault;

		m_wBackgroundWidget.SetColor(color);
	}
}
