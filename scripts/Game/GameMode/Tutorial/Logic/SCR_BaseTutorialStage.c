[EntityEditorProps(insertable: false)]
class SCR_BaseTutorialStageClass : GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BaseTutorialStage : GenericEntity
{
	protected float m_fTimer;
	protected float m_fStageTimer;
	protected float m_fConditionCheckPeriod = 0.25;
	protected float m_fDuration;
	protected float m_fDelay;
	protected float m_fWaypointCompletionRadius = 1;
	
	protected bool m_bShowWaypoint = true;
	protected bool m_bCheckWaypoint = true;
	protected bool m_bConditionCheckOnSetDuration;
	protected bool m_bAutoShowHint = true;
	protected bool m_bInstantConditionCheck;
	protected string m_sLastFinishedEvent;
	protected float m_fStartTimestamp;
	protected string m_sHintSoundEvent;
	
	protected SCR_Waypoint m_ReachedWaypoint;
	
	protected ChimeraCharacter m_Player;
	protected ref array<SCR_Waypoint> m_aStageWaypoints;
	protected SCR_TutorialGamemodeComponent m_TutorialComponent;
	protected bool m_bFinished;
	protected Resource m_HintsConfig;
	protected SCR_TutorialStageInfo m_StageInfo;
	protected SCR_BaseTutorialCourseLogic m_Logic;
	
	//------------------------------------------------------------------------------------------------
	SCR_TutorialStageInfo GetStageInfo()
	{
		return m_StageInfo;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDuration()
	{
		return GetGame().GetWorld().GetWorldTime() - m_fStartTimestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateMarkerPath(string pathEntityName)
	{
		IEntity dotEntity = GetGame().GetWorld().FindEntityByName(pathEntityName);
		if (!dotEntity)
			return;
		
		CreateMarkerCustom(dotEntity, SCR_EScenarioFrameworkMarkerCustom.DOT, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
		
		dotEntity = dotEntity.GetChildren();
		while (dotEntity)
		{
			CreateMarkerCustom(dotEntity, SCR_EScenarioFrameworkMarkerCustom.DOT, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
			dotEntity = dotEntity.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_MapMarkerBase CreateMarkerCustom(string entityName, SCR_EScenarioFrameworkMarkerCustom icon, SCR_EScenarioFrameworkMarkerCustomColor color, string name = string.Empty)
	{
		IEntity ent = GetGame().GetWorld().FindEntityByName(entityName);
		if (!ent)
			return null;
		
		return CreateMarkerCustom(ent, icon, color, name);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_MapMarkerBase CreateMarkerCustom(notnull IEntity entity, SCR_EScenarioFrameworkMarkerCustom icon, SCR_EScenarioFrameworkMarkerCustomColor color, string name = string.Empty)
	{
		SCR_MapMarkerManagerComponent mapMarkerMgr = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		
		if (!mapMarkerMgr)
			return null;
	
		SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
		marker.SetType(SCR_EMapMarkerType.PLACED_CUSTOM);
		marker.SetIconEntry(icon);
		marker.SetColorEntry(color);
		marker.SetCustomText(name);
		
		RegisterMarker(marker, entity);
		return marker;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_MapMarkerBase CreateMarkerMilitary(string entityName, SCR_EScenarioFrameworkMarkerCustom icon, SCR_EScenarioFrameworkMarkerCustomColor color, string name = string.Empty)
	{
		IEntity ent = GetGame().GetWorld().FindEntityByName(entityName);
		if (!ent)
			return null;
	
		return CreateMarkerCustom(ent, icon, color, name);
	}
		
	
	//------------------------------------------------------------------------------------------------
	protected SCR_MapMarkerBase CreateMarkerMilitary(notnull IEntity entity, EMilitarySymbolIdentity factionIdentity, EMilitarySymbolDimension dimension, EMilitarySymbolIcon typeFlags, string name = string.Empty)
	{
		SCR_MapMarkerManagerComponent mapMarkerMgr = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		
		if (!mapMarkerMgr)
			return null;
	
		SCR_MapMarkerBase marker = mapMarkerMgr.PrepareMilitaryMarker(factionIdentity, dimension, typeFlags);
		marker.SetCustomText(name);
		
		RegisterMarker(marker, entity);
		return marker;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RegisterMarker(notnull SCR_MapMarkerBase marker, notnull IEntity entity)
	{
		SCR_MapMarkerManagerComponent mapMarkerMgr = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		
		if (!mapMarkerMgr)
			return;

		vector worldPos = entity.GetOrigin();
		marker.SetWorldPos(worldPos[0], worldPos[2]);
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
		{
			Faction faction = factionManager.GetFactionByKey("US");
			if (faction)
				marker.AddMarkerFactionFlags(factionManager.GetFactionIndex(faction));
		}
		
		m_TutorialComponent.RegisterMarker(marker);
		mapMarkerMgr.InsertStaticMarker(marker, false, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DeleteMarker(SCR_MapMarkerBase marker)
	{
		if (!marker)
			return;
		
		if (!GetGame().GetGameMode())
			return;
		
		SCR_MapMarkerManagerComponent mapMarkerMgr = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		
		if (!mapMarkerMgr)
			return;
		
		mapMarkerMgr.RemoveStaticMarker(marker);
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetQuickslotIndexByPrefab(ResourceName prefabName)
	{
		if (!m_Player)
			return -1;
		
		SCR_CharacterInventoryStorageComponent comp = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		if (!comp)
			return -1;

		array<ref SCR_QuickslotBaseContainer> baseContainers = comp.GetQuickSlotItems();
		SCR_QuickslotEntityContainer entityContainer;
		IEntity item;
				
		foreach (int index, SCR_QuickslotBaseContainer baseContainer : baseContainers)
		{
			entityContainer = SCR_QuickslotEntityContainer.Cast(baseContainer);
			
			if (!entityContainer)
				continue;
			
			item = entityContainer.GetEntity();

			if (!item || item.GetPrefabData().GetPrefabName() != prefabName)
				continue;

			return index;
		}

		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowHint(int index = 0)
	{
		if (!m_StageInfo)
			return;
		
		SCR_HintUIInfo hintInfo = m_StageInfo.GetHint(index);

		if (hintInfo && SCR_HintManagerComponent.GetInstance().GetCurrentHint() != hintInfo)
		{
			hintInfo.SetPersistent(true);
			SCR_HintManagerComponent.ShowHint(hintInfo);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Setup()
	{

	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetIsFinished()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void Reset()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInputDeviceChanged(bool switchedToKeyboard)
	{
		if (switchedToKeyboard)
			HintKeyboard(true);
		else
			HintGamepad(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVoiceoverEventFinishedBase(string eventName)
	{
		m_sLastFinishedEvent = eventName;
		m_bInstantConditionCheck = true;

		if (!eventName.IsEmpty() && eventName == m_sHintSoundEvent)
		{
			GetGame().GetCallqueue().Remove(ShowHint);
			ShowHint();
		}
		
		OnVoiceoverEventFinished(eventName);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVoiceoverEventFinished(string eventName)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HintGamepad(bool isSilent = false)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HintKeyboard(bool isSilent = false)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedPopup(string text = "", string subtitle = "", float duration = SCR_PopUpNotification.DEFAULT_DURATION, string param1 = "", string param2 = "", string subtitleParam1 = "", string subtitleParam2 = "")
	{
		SCR_PopUpNotification.GetInstance().PopupMsg(text, duration, text2: subtitle, param1: param1, param2: param2, text2param1: subtitleParam1, text2param2: subtitleParam2, category: SCR_EPopupMsgFilter.TUTORIAL);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterWaypoint(IEntity waypointEntity)
	{
		SCR_WaypointDisplay display = m_TutorialComponent.GetWaypointDisplay();
		SCR_Waypoint waypoint;
		if (display)
		{
			waypoint = display.FindWaypointByEntity(waypointEntity);
			display.DeleteWaypoint(waypoint);
		}
		
		if (waypoint && m_aStageWaypoints)
			m_aStageWaypoints.RemoveItem(waypoint);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterWaypoint(string waypointName)
	{
		IEntity entity = GetGame().GetWorld().FindEntityByName(waypointName);
		if (entity)
			UnregisterWaypoint(entity);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_Waypoint RegisterWaypoint(string entityName, string title = string.Empty, string icon = "MISC")
	{
		IEntity entity = GetGame().GetWorld().FindEntityByName(entityName);
		if (entity)
			return RegisterWaypoint(entity, title, icon);
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_Waypoint RegisterWaypoint(notnull IEntity entity, string title = string.Empty, string icon = "MISC")
	{
		SCR_WaypointDisplay display = m_TutorialComponent.GetWaypointDisplay();
		if (!display)
			return null;
		
		//Don't register same waypoint twice. At least for now...
		if (display.FindWaypointByEntity(entity))
			return null;
		
		if (!m_aStageWaypoints)
			m_aStageWaypoints = {};
		
		SCR_Waypoint wp = display.CreateWaypoint(entity);
		if (!wp)
			return null;
		
		wp.m_wTitle.SetText(title);
		wp.SetIconImage(icon, true);
		
		m_aStageWaypoints.Insert(wp);
		
		return wp;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_Waypoint GetWaypoint(int index = 0)
	{
		if (!m_aStageWaypoints || m_aStageWaypoints.IsEmpty() || !m_aStageWaypoints.IsIndexValid(index))
			return null;
		
		return m_aStageWaypoints[index];
	}
	
	//------------------------------------------------------------------------------------------------
	void OnStructureBuilt(SCR_CampaignMilitaryBaseComponent base, IEntity structure)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsBuildingModeOpen()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		
		if (!core)
			return false;
		
		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		
		if (!editorManager)
		    return false;
		
		SCR_EditorModeEntity modeEntity = editorManager.FindModeEntity(EEditorMode.BUILDING);
		
		if (!modeEntity)
		    return false;
		
		return modeEntity.IsOpened();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_fTimer += timeSlice;
		m_fStageTimer += timeSlice;
		bool conditionComplete;
		bool waypointReached = true;
		
		// Check if the stage is already complete and we're just waiting for end delay timer
		if (m_bFinished)
		{
			if (m_fTimer > m_fDelay)
				OnStageFinished();
			
			return;
		}
		
		if (!m_Player)
			return; // If this returns something is horribly fundamentally wrong with the game.
		
		// If duration is set up, simply check the timer
		// Otherwise periodically check the ending condition
		if (m_fDuration != 0)	
		{
			if (m_fTimer > m_fDuration)
				if (m_bConditionCheckOnSetDuration)
					conditionComplete = GetIsFinished();
				else
					conditionComplete = true;
		}
		else if (m_bInstantConditionCheck || m_fTimer > m_fConditionCheckPeriod)
		{
			m_fTimer = 0;
			m_bInstantConditionCheck = false;
			conditionComplete = GetIsFinished();
			
			// If player is required to reach the waypoint, check its distance
			//TODO: Once we start to generalize most common stages, like move, remove this and rather set m_bCheckWaypoint on demand individually
			if (m_aStageWaypoints && !m_aStageWaypoints.IsEmpty())
			{
				if (m_bCheckWaypoint && m_fWaypointCompletionRadius != 0)
					waypointReached = CheckWaypoints();
			}
		}
		
		// Stage was finished
		// If a delay is set up, just prepare the timer
		if (conditionComplete && waypointReached)
		{
			if (m_fDelay == 0)
			{
				OnStageFinished();
			}
			else
			{
				m_bFinished = true;
				m_bShowWaypoint = false;
				m_fTimer = 0;
			}
		}

	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnStageFinished()
	{
		GetGame().GetCallqueue().Remove(ShowHint);
		
		if (m_TutorialComponent)
			m_TutorialComponent.FinishStage();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CheckWaypoints()
	{
		if (!m_aStageWaypoints || m_aStageWaypoints.IsEmpty())
			return true;
		
		foreach (SCR_Waypoint wp : m_aStageWaypoints)
		{
			if (vector.DistanceSq(m_Player.GetOrigin(), wp.GetPosition()) > (Math.Pow(m_fWaypointCompletionRadius, 2)))
				continue;
		
			m_ReachedWaypoint = wp;
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void Init(SCR_TutorialStageInfo stageInfo)
	{
		Print(stageInfo.GetClassName());
		
		m_TutorialComponent = SCR_TutorialGamemodeComponent.GetInstance();
		
		if (!m_TutorialComponent)
			return;
		
		m_Player = m_TutorialComponent.GetPlayer();
		m_StageInfo = stageInfo;
		m_sLastFinishedEvent = string.Empty;
		m_fStartTimestamp = GetGame().GetWorld().GetWorldTime();
		m_Logic = m_TutorialComponent.GetActiveConfig().GetCourseLogic();
		m_sHintSoundEvent = stageInfo.GetHintSoundEvent();
		m_fConditionCheckPeriod = stageInfo.GetConditionCheckPeriod();
		m_fDuration = stageInfo.GetDuration();
		m_fDelay = stageInfo.GetDelay();
		m_fWaypointCompletionRadius = stageInfo.GetWaypointCompletionRadius();
		m_bShowWaypoint = stageInfo.ShowWaypoint();
		m_bCheckWaypoint = stageInfo.CheckWaypoint();
		m_bConditionCheckOnSetDuration = stageInfo.ConditionCheckOnSetDuration();
		m_bAutoShowHint = stageInfo.AutoShowHint();

		SCR_VoiceoverSystem.GetInstance().GetOnFinishedEvent().Insert(OnVoiceoverEventFinishedBase);
		Setup();
		
		if (m_bAutoShowHint)
			ShowHint();
		
		if (!m_sHintSoundEvent.IsEmpty() && m_Player)
			GetGame().GetCallqueue().CallLater(ShowHint, stageInfo.GetFailsafeHintTimeout() * 1000, false, 0);
		
		SetEventMask(EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_BaseTutorialStage()
	{
		SCR_VoiceoverSystem.GetInstance().GetOnFinishedEvent().Remove(OnVoiceoverEventFinishedBase);
		
		if (m_aStageWaypoints && !m_aStageWaypoints.IsEmpty())
		{
			SCR_WaypointDisplay display = m_TutorialComponent.GetWaypointDisplay();
			if (!display)
				return;
			
			for (int i = m_aStageWaypoints.Count()-1; i >= 0; i--)
			{
				display.DeleteWaypoint(m_aStageWaypoints[i]);
				m_aStageWaypoints.Remove(i);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void PlayNarrativeCharacterStage(string characterName, int stage)
	{
		// Delayed call used globally because of an issue with event triggering from anims
		// The issue is that some sentences would not get played when triggered directly from callback OnVoiceoverEventFinished
		GetGame().GetCallqueue().CallLater(m_TutorialComponent.PlayNarrativeCharacterStageDelayed, 100, false, characterName, stage);
	}

	//------------------------------------------------------------------------------------------------
	void SetPermanentNarrativeStage(string characterName, int stage)
	{
		IEntity entity = GetGame().GetWorld().FindEntityByName(characterName);
		
		if (!entity)
			return;
		
		SCR_NarrativeComponent narrative = SCR_NarrativeComponent.Cast(entity.FindComponent(SCR_NarrativeComponent));
		
		if (!narrative)
			return;
		
		narrative.SetPermanentNarrativeStage(stage);
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetSuppliesInInventory()
	{
		SCR_InventoryStorageManagerComponent inventory = m_TutorialComponent.GetPlayerInventory();
		if (!inventory)
			return 0;
		
		int supplies = 0;

		array<IEntity> foundItems = {};		
		inventory.FindItemsWithComponents(foundItems, {SCR_ResourceComponent}, EStoragePurpose.PURPOSE_ANY);

		foreach (IEntity item : foundItems)
		{
			const SCR_ResourceComponent resourceComp = SCR_ResourceComponent.Cast(item.FindComponent(SCR_ResourceComponent));
			const SCR_ResourceContainer container = resourceComp.GetContainer(EResourceType.SUPPLIES);
			if (container)
				supplies += container.GetResourceValue();
		}

		return supplies;
	}

	//------------------------------------------------------------------------------------------------
	void FinishStage()
	{
		OnStageFinished();
	}
};