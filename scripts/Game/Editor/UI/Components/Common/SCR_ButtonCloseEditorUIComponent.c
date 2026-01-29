class SCR_ButtonCloseEditorUIComponent: ScriptedWidgetComponent
{
	[Attribute()]
	protected string m_sButtonCloseName;
	
	[Attribute()]
	protected string m_sButtonPlayerName;
	
	[Attribute()]
	protected string m_sButtonRespawnMenuName;
	
	[Attribute()]
	protected string m_sButtonRespawnMenuDisabledName;
	
	[Attribute("Toolbar_CaptureButton")]
	protected string m_sButtonCapture;
	
	protected Widget m_ButtonClose;
	protected Widget m_ButtonPlayer;
	protected Widget m_ButtonRespawnMenu;
	protected Widget m_ButtonRespawnMenuDisabled;
	protected Widget m_wButtonCapture;
	
	//---- REFACTOR NOTE START: States could be enums
	
	protected const int BUTTON_CLOSE = 0;
	protected const int BUTTON_PLAYER = 1;
	protected const int BUTTON_RESPAWN_MENU = 2;
	protected const int BUTTON_RESPAWN_MENU_DISABLED = 3;
	
	//------------------------------------------------------------------------------------------------
	protected int GetButtonIndex()
	{
		//--- Player avatar exists - 'Play' button
		IEntity localPlayer = SCR_PlayerController.GetLocalMainEntity();
		if (localPlayer && !localPlayer.IsDeleted())
		{
			DamageManagerComponent damageManager = DamageManagerComponent.Cast(localPlayer.FindComponent(DamageManagerComponent));
			if (!damageManager)
				return BUTTON_PLAYER;
			
			if (damageManager.GetState() != EDamageState.DESTROYED)
				return BUTTON_PLAYER;
		}
		
		//--- 'Return to respawn menu' button based on available spawn points
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
		{
			int spawnPointCount;
			Faction playerFaction = factionManager.GetPlayerFaction(SCR_PlayerController.GetLocalPlayerId());
			if (playerFaction)
			{
				//--- Player has faction - return its spawn points
				string playerFactionKey;
				playerFactionKey = playerFaction.GetFactionKey();
				spawnPointCount = SCR_SpawnPoint.GetSpawnPointCountForFaction(playerFactionKey)
			}
			else
			{
				//--- Player has no faction - return all spawn points
				spawnPointCount = SCR_SpawnPoint.CountSpawnPoints();
			}
			
			if (spawnPointCount == 0)
				return BUTTON_RESPAWN_MENU_DISABLED;
			else
				return BUTTON_RESPAWN_MENU;
		}
		
		//--- Default - plain 'Close' button
		return BUTTON_CLOSE;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Refresh()
	{
		int showButton = GetButtonIndex();
		
		m_ButtonClose.SetVisible(showButton == BUTTON_CLOSE);
		m_ButtonPlayer.SetVisible(showButton == BUTTON_PLAYER);
		m_ButtonRespawnMenu.SetVisible(showButton == BUTTON_RESPAWN_MENU);
		m_ButtonRespawnMenuDisabled.SetVisible(showButton == BUTTON_RESPAWN_MENU_DISABLED);
		
		if (m_wButtonCapture)
		{
			SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
			if (!editorManager)
				return;
			
			EEditorMode mode = editorManager.GetCurrentMode();
			m_wButtonCapture.SetVisible(mode == EEditorMode.PHOTO_SAVE);
		}
	}
	
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		Refresh();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawnedOrDeleted(int playerId, IEntity player)
	{
		Refresh();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			GetGame().GetCallqueue().CallLater(editorManager.Close, 1, false, false); //--- Cannot trigger operation which closes UI from that UI's handler
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		m_ButtonPlayer = w.FindAnyWidget(m_sButtonPlayerName);
		m_ButtonRespawnMenu = w.FindAnyWidget(m_sButtonRespawnMenuName);
		m_ButtonRespawnMenuDisabled = w.FindAnyWidget(m_sButtonRespawnMenuDisabledName);
		m_ButtonClose = w.FindAnyWidget(m_sButtonCloseName);
		m_wButtonCapture = w.FindAnyWidget(m_sButtonCapture);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawnedOrDeleted);
			gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
			gameMode.GetOnPlayerDeleted().Insert(OnPlayerSpawnedOrDeleted);
		}
		
		SCR_SpawnPoint.Event_OnSpawnPointCountChanged.Insert(Refresh);
		
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerSpawned().Remove(OnPlayerSpawnedOrDeleted);
			gameMode.GetOnPlayerKilled().Remove(OnPlayerKilled);
			gameMode.GetOnPlayerDeleted().Remove(OnPlayerSpawnedOrDeleted);
		}
		
		SCR_SpawnPoint.Event_OnSpawnPointCountChanged.Remove(Refresh);
	}
}
