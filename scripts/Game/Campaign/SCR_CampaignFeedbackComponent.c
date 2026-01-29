class SCR_CampaignFeedbackComponentClass : ScriptComponentClass
{
}

class SCR_CampaignFeedbackComponent : ScriptComponent
{
	[Attribute("{3EE26F4747B6E99D}Configs/Hints/Conflict/ConflictHints.conf", params: "conf class=SCR_CampaignHintStorage")]
	protected ResourceName m_sHintsConfig;

	[Attribute(defvalue: "3000", desc: "Delay before the vehicle inventory hint appears (ms)")]
	protected int m_iVehicleHintDelay;

	protected SCR_GameModeCampaign m_Campaign;

	protected SCR_PlayerController m_PlayerController;

	protected SCR_CampaignMilitaryBaseComponent m_BaseWithPlayer;

	protected AudioHandle m_PlayedRadio = AudioHandle.Invalid;

	protected vector m_vFirstSpawnPosition;

	protected SCR_MapCampaignUI m_MapCampaignUI;

	protected ref TimeContainer m_SpawnTime;

	protected ref SCR_CampaignHintStorage m_HintsConfig;

	protected ref array<int> m_aShownHints = {};
	protected ref array<int> m_aHintQueue = {};

	protected bool m_bIsPlayerInRadioRange = true;
	protected bool m_bCanShowSpawnPosition;
	protected bool m_bWasMapOpened;
	protected bool m_bIsConscious;
	protected bool m_bWasNight;
	protected bool m_bCohesionHintWasShown;

	protected float m_fNextAllowedHintTimestamp;

	protected WorldTimestamp m_fBaseWithPlayerCaptureStart;
	protected WorldTimestamp m_fBaseWithPlayerCaptureEnd;

	static const float ICON_FLASH_DURATION = 20;
	static const float ICON_FLASH_PERIOD = 0.5;

	protected static const int AFTER_RESPAWN_HINT_DELAY_MS = 16500;
	protected static const int DELAY_BETWEEN_HINTS_MS = 1000;
	protected static const int FEATURE_HINT_DELAY = 120000;
	protected static const int NIGHT_HINT_DELAY_MS = 10000;
	protected static const int COHESION_HINT_DELAY_MS = 20000;

	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_CampaignFeedbackComponent GetInstance()
	{
		PlayerController pc = GetGame().GetPlayerController();

		if (!pc)
			return null;

		return SCR_CampaignFeedbackComponent.Cast(pc.FindComponent(SCR_CampaignFeedbackComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Serves for updating spawn hint on map witth he given position.
	//! \param[in] enable
	//! \param[in] position
	void UpdatePlayerSpawnHint(bool show, vector position = vector.Zero)
	{
		m_bCanShowSpawnPosition = show;
		if (show)
		{
			if (m_vFirstSpawnPosition != vector.Zero)
				return; // Entity change while already spawned at some position, do not update.

			SetSpawnTime();
		}
		else
		{
			SetMapOpened(false);

			if (m_MapCampaignUI)
				m_MapCampaignUI.RemoveSpawnPositionHint();
			
			PauseHintQueue();
		}

		m_vFirstSpawnPosition = position;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] mapUi
	void SetMapCampaignUI(SCR_MapCampaignUI mapUi)
	{
		m_MapCampaignUI = mapUi;
	}

	//------------------------------------------------------------------------------------------------
	void SetSpawnTime()
	{
		ChimeraWorld world = m_PlayerController.GetWorld();
		TimeAndWeatherManagerEntity manager = world.GetTimeAndWeatherManager();

		if (manager)
				m_SpawnTime = manager.GetTime();
		else
			Print("Time And Weather manager not found", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	TimeContainer GetSpawnTime()
	{
		return m_SpawnTime;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool CanShowPlayerSpawn()
	{
		return m_bCanShowSpawnPosition;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns players position after spawning
	vector GetPlayerSpawnPos()
	{
		return m_vFirstSpawnPosition;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if map was already opened, false otherwise
	bool WasMapOpened()
	{
		return m_bWasMapOpened;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets that map was already opened by player
	//! \param[in] wasOpened
	void SetMapOpened(bool wasOpened)
	{
		m_bWasMapOpened = wasOpened;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_CampaignMilitaryBaseComponent GetBaseWithPlayer()
	{
		return m_BaseWithPlayer;
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckPlayerInsideRadioRange()
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());

		if (!fManager)
			return;

		IEntity player = SCR_PlayerController.GetLocalControlledEntity();

		if (!player)
			return;

		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		if (!faction)
			return;

		bool isInRangeNow = m_Campaign.GetBaseManager().IsEntityInFactionRadioSignal(player, faction);

		if (isInRangeNow != m_bIsPlayerInRadioRange)
		{
			m_bIsPlayerInRadioRange = isInRangeNow;

			if (isInRangeNow)
			{
				SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_RadioRangeEntered-UC", duration: 3);
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_SIGNAL_GAIN);
			}
			else
			{
				SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Campaign_RadioRangeLeft-UC", duration: 3);
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_SIGNAL_LOST);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterTasksShown()
	{
		SCR_PopUpNotification.GetInstance().HideCurrentMsg();
		GetGame().GetInputManager().RemoveActionListener("TasksOpen", EActionTrigger.DOWN, RegisterTasksShown);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] status
	void SetIsPlayerInRadioRange(bool status)
	{
		m_bIsPlayerInRadioRange = status;
	}

	//------------------------------------------------------------------------------------------------
	protected void GroupLeaderHint()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();

		if (!groupsManager)
			return;

		SCR_AIGroup group = groupsManager.GetPlayerGroup(m_PlayerController.GetPlayerId());

		if (!group)
			return;

		IEntity leader = GetGame().GetPlayerManager().GetPlayerControlledEntity(group.GetLeaderID());

		if (leader != m_PlayerController.GetControlledEntity())
			return;

		GetGame().GetCallqueue().Remove(GroupLeaderHint);
		ShowHint(EHint.GAMEPLAY_GROUPS);
	}

	//------------------------------------------------------------------------------------------------
	protected void LoneDriverHint()
	{
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());

		if (!player)
			return;

		if (!player.IsInVehicle())
			return;

		Vehicle vehicle = Vehicle.Cast(SCR_EntityHelper.GetMainParent(player));

		if (!vehicle)
			return;

		SCR_EditableVehicleComponent comp = SCR_EditableVehicleComponent.Cast(vehicle.FindComponent(SCR_EditableVehicleComponent));

		if (!comp)
			return;

		array<CompartmentAccessComponent> compartments = {};

		if (comp.GetCrew(compartments, false) > 1)
			return;

		GetGame().GetCallqueue().Remove(LoneDriverHint);

		ShowHint(EHint.CONFLICT_DRIVER);
	}

	//------------------------------------------------------------------------------------------------
	protected void TransportRequestHint()
	{
		if (m_BaseWithPlayer)
			return;

		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(m_PlayerController.GetControlledEntity());

		if (!player || player.IsInVehicle())
			return;

		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();

		if (!baseManager)
			return;

		array<SCR_MilitaryBaseComponent> bases = {};
		baseManager.GetBases(bases);
		int minDistanceSq = 500 * 500;
		vector playerPos = player.GetOrigin();
		SCR_CampaignMilitaryBaseComponent campaignBase;

		// Show this hint only if player is far enough from any base
		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

			if (!campaignBase)
				continue;

			if (vector.DistanceSqXZ(playerPos, campaignBase.GetOwner().GetOrigin()) < minDistanceSq)
				return;
		}

		minDistanceSq = 300 * 300;
		array<int> playerIds = {};
		int localPlayerId = m_PlayerController.GetPlayerId();
		GetGame().GetPlayerManager().GetPlayers(playerIds);

		// Show this hint only if player is far enough from any other living player
		foreach (int playerId : playerIds)
		{
			if (playerId == localPlayerId)
				continue;

			SCR_ChimeraCharacter otherPlayer = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId));

			if (!otherPlayer)
				continue;

			CharacterControllerComponent charControl = otherPlayer.GetCharacterController();

			if (!charControl || charControl.IsDead())
				continue;

			if (vector.DistanceSqXZ(playerPos, otherPlayer.GetOrigin()) < minDistanceSq)
				return;
		}

		GetGame().GetCallqueue().Remove(TransportRequestHint);
		ShowHint(EHint.CONFLICT_TRANSPORT_REQUEST, showMultipleTimes: true);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] base
	void BaseOutOfRangeHint(SCR_CampaignMilitaryBaseComponent base)
	{
		SCR_CampaignFaction playerFaction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		if (!playerFaction || base.GetFaction() != playerFaction)
			return;

		if (base.GetHQRadioCoverage(playerFaction) != SCR_ERadioCoverageStatus.RECEIVE)
			return;

		ShowHint(EHint.CONFLICT_NO_CONNECTION);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] config
	void OnMapOpen(MapConfiguration config)
	{
		if (GetGame().GetWorld().GetWorldTime() < FEATURE_HINT_DELAY)
			return;

		if (!config)
			return;
		
		if(config.MapEntityMode == EMapEntityMode.SPAWNSCREEN)
			return;

		switch (config.MapEntityMode)
		{
			case EMapEntityMode.COMBAT_SUPPORT_STATION:
				ShowHint(EHint.CONFLICT_COMMANDER_HQ_RADIO);
				break;

			case EMapEntityMode.OPERATIONS_STATION:
				ShowHint(EHint.CONFLICT_COMMANDER_HQ_OPERATIONS);
				break;

			case EMapEntityMode.LOGISTICS_STATION:
				ShowHint(EHint.CONFLICT_COMMANDER_HQ_LOGISTICS);
				break;

			default:
				ShowHint(EHint.CONFLICT_GROUP_ICONS);
				break;
		}

		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup playerGroup = groupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (!playerGroup)
			return;
		
		SCR_Faction playerFaction = SCR_Faction.Cast(playerGroup.GetFaction());
		if (!playerFaction)
			return;

		if (SCR_PlayerController.GetLocalPlayerId() == playerGroup.GetLeaderID() && SCR_PlayerController.GetLocalPlayerId() != playerFaction.GetCommanderId())
			ShowHint(EHint.CONFLICT_SQUAD_LEADER_COMMUNICATION);

		SCR_MapRadialUI mapContextualMenu = SCR_MapRadialUI.GetInstance();
		if (!mapContextualMenu)
			return;

		mapContextualMenu.GetOnMenuInitInvoker().Insert(OnRadialMenuAvailable);

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;

		SCR_MapCampaignUI mapCampaignUI = SCR_MapCampaignUI.Cast(mapEntity.GetMapUIComponent(SCR_MapCampaignUI));

		if (!mapCampaignUI)
			return;

		mapCampaignUI.GetOnBaseHovered().Insert(OnBaseHovered);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRadialMenuAvailable()
	{
		SCR_MapRadialUI mapMenu = SCR_MapRadialUI.GetInstance();
		if (!mapMenu)
			return;

		mapMenu.GetOnMenuInitInvoker().Insert(OnRadialMenuAvailable);
		SCR_RadialMenu m_RadialMenu = mapMenu.GetRadialController().GetRadialMenu();
		if (m_RadialMenu)
		{
			m_RadialMenu.GetOnEntryPerformed().Insert(OnEntryPerformed);
			m_RadialMenu.GetOnOpen().Insert(OnRadialMenuOpen);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnEntryPerformed(SCR_SelectionMenuEntry entry)
	{
		if (!entry)
			return;

		SCR_FactionCommanderPlayerComponent commander = SCR_FactionCommanderPlayerComponent.Cast(m_PlayerController.FindComponent(SCR_FactionCommanderPlayerComponent));
		if (!commander)
			return;

		SCR_FactionCommanderMenuEntry configForEntry = commander.GetConfigForEntry(entry);
		if (!configForEntry)
			return;

		if (configForEntry.GetShowHint())
			ShowHint(configForEntry.GetHintId());
	}

	//------------------------------------------------------------------------------------------------
	void OnMapClose(MapConfiguration config)
	{
		SCR_MapRadialUI mapMenu = SCR_MapRadialUI.GetInstance();
		if (!mapMenu)
			return;

		SCR_RadialMenu m_RadialMenu = mapMenu.GetRadialController().GetRadialMenu();
		if (m_RadialMenu)
		{
			m_RadialMenu.GetOnEntryPerformed().Remove(OnEntryPerformed);
			m_RadialMenu.GetOnOpen().Remove(OnRadialMenuOpen);
		}

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity) return;
			SCR_MapCampaignUI mapCampaignUI = SCR_MapCampaignUI.Cast(mapEntity.GetMapUIComponent(SCR_MapCampaignUI));

		if (!mapCampaignUI) return;
			mapCampaignUI.GetOnBaseHovered().Remove(OnBaseHovered)
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseHovered(SCR_CampaignMilitaryBaseComponent base)
	{
		if (base && base.GetType() == SCR_ECampaignBaseType.SOURCE_BASE)
			ShowHint(EHint.CONFLICT_SOURCE_BASE);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommanderVolunteer(int playerId)
	{
		ShowHint(EHint.CONFLICT_COMMANDER_VOTING);
		SCR_FactionCommanderVolunteerUserAction.GetOnCanBePerformed().Remove(OnCommanderVolunteer);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFactionCommanderChanged(SCR_Faction faction, int commanderPlayerId)
	{
		if (SCR_PlayerController.GetLocalPlayerId() != commanderPlayerId)
			return;

		ShowHint(EHint.CONFLICT_COMMANDER_ROLE);
		if (SCR_FactionCommanderHandlerComponent.GetInstance())
				SCR_FactionCommanderHandlerComponent.GetInstance().GetOnFactionCommanderChanged().Remove(OnFactionCommanderChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseClickedShowSupplyLimit(bool selected)
	{
		if (!selected)
			return;

		ShowHint(EHint.CONFLICT_COMMANDER_HQ_LOGISTICS_SUPPLY);
		SCR_CampaignLogisticMapUIBase.GetOnBaseSelected().Remove(OnBaseClickedShowSupplyLimit);
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshCurrentPopupMessage()
	{
		SCR_PopUpNotification popup = SCR_PopUpNotification.GetInstance();
		SCR_PopupMessage currentMsg = popup.GetCurrentMsg();

		// if is there any popup with higher priority, the seize popup will not appear.
		if (currentMsg && currentMsg.m_iPriority > SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY)
			return;

		// hide current seizing popup if is there any popup with higher priority
		if (currentMsg && (currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_YOU || currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY))
		{
			if (!IsLocalPlayerAlive() || popup.IsPresentPopupWithHigherPriority(SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY))
			{
				popup.HideCurrentMsg();
				return;
			}
		}

		if (!IsLocalPlayerAlive())
			return;

		// Player is not currently in any base, hide any relevant displayed popups
		if (!m_BaseWithPlayer)
		{
			if (currentMsg && (currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_YOU || currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY))
				popup.HideCurrentMsg();
		}
		else
		{
			SCR_CampaignFaction baseWithPlayerCapturingFaction = SCR_CampaignFaction.Cast(m_BaseWithPlayer.GetCapturingFaction());

			if (!baseWithPlayerCapturingFaction)
			{
				// Player's base is currently not being seized by anyone, hide any relevant displayed popups
				if (currentMsg && (currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_YOU || currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY))
					popup.HideCurrentMsg();
			}
			else
			{
				// If relay tower is being reconfigured, calculate the capture duration
				if (m_BaseWithPlayer.GetType() == SCR_ECampaignBaseType.RELAY)
					m_fBaseWithPlayerCaptureEnd = m_fBaseWithPlayerCaptureStart.PlusSeconds(SCR_CampaignMilitaryBaseComponent.RADIO_RECONFIGURATION_DURATION);

				// If capture timers are invalid, hide seizing popup
				if (m_fBaseWithPlayerCaptureStart == 0 || m_fBaseWithPlayerCaptureEnd == 0 && currentMsg && (currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_YOU || currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY))
				{
					popup.HideCurrentMsg();
				}
				else
				{
					if (baseWithPlayerCapturingFaction == SCR_FactionManager.SGetLocalPlayerFaction())
					{
						// Friendlies are seizing player's base
						if (currentMsg && currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY)
							popup.HideCurrentMsg();

						if (m_BaseWithPlayer.GetReconfiguredByID() != SCR_PlayerController.GetLocalPlayerId())
						{
							bool isPlayerCapturing;
							SCR_CampaignSeizingComponent seizingComp = SCR_CampaignSeizingComponent.Cast(m_BaseWithPlayer.GetOwner().FindComponent(SCR_CampaignSeizingComponent));

							if (seizingComp)
							{
								ChimeraCharacter player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());

								if (player)
								{
									CharacterControllerComponent charController = player.GetCharacterController();

									if (charController && charController.GetLifeState() != ECharacterLifeState.DEAD)
									{
										vector playerPos = player.GetOrigin();

										// Triggers calculate entity presence by bounding boxes
										// We need to calculate the player's bbox radius to match the distance to the trigger
										vector boundsMin, boundsMax;
										player.GetWorldBounds(boundsMin, boundsMax);
										float distanceDiff = vector.DistanceXZ(boundsMin, boundsMax) / 2;

										if (!player.IsInVehicle() || SCR_TerrainHelper.GetHeightAboveTerrain(playerPos) <= seizingComp.GetMaximumAltitude())
											isPlayerCapturing = vector.DistanceXZ(playerPos, m_BaseWithPlayer.GetOwner().GetOrigin()) <= (seizingComp.GetRadius() + distanceDiff);
									}
								}
							}

							string text;
							bool update;

							if (isPlayerCapturing)
							{
								if (!currentMsg || currentMsg.m_sText != "#AR-Campaign_SeizingPlayer-UC")
									update = true;

								text = "#AR-Campaign_SeizingPlayer-UC";
							}
							else
							{
								if (!currentMsg || currentMsg.m_sText != "#AR-Campaign_SeizingFriendly-UC")
									update = true;

								text = "#AR-Campaign_SeizingFriendly-UC";
							}

							if (!currentMsg || currentMsg.m_iPriority != SCR_ECampaignSeizingMessagePrio.SEIZING_YOU || update)
							{
								if (isPlayerCapturing)
									popup.PopupMsg(text, -1, prio: SCR_ECampaignSeizingMessagePrio.SEIZING_YOU, progressStart: m_fBaseWithPlayerCaptureStart, progressEnd: m_fBaseWithPlayerCaptureEnd, category: SCR_EPopupMsgFilter.TUTORIAL);
								else
									popup.PopupMsg(text, -1, "#AR-Campaign_Popup_HelpCapture", prio: SCR_ECampaignSeizingMessagePrio.SEIZING_YOU, category: SCR_EPopupMsgFilter.TUTORIAL);
							}
						}
					}
					else
					{
						// Enemies are seizing player's base
						if (currentMsg && currentMsg.m_iPriority == SCR_ECampaignSeizingMessagePrio.SEIZING_YOU)
							popup.HideCurrentMsg();

						if (!currentMsg || currentMsg.m_iPriority != SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY)
							popup.PopupMsg("#AR-Campaign_SeizingEnemy-UC", -1, prio: SCR_ECampaignSeizingMessagePrio.SEIZING_ENEMY, progressStart: m_fBaseWithPlayerCaptureStart, progressEnd: m_fBaseWithPlayerCaptureEnd, category: SCR_EPopupMsgFilter.TUTORIAL);
					}
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Hints are displayed with a delay after respawn so player has time to find their bearings
	void OnRespawn()
	{
		m_fNextAllowedHintTimestamp = 0;
		GetGame().GetCallqueue().CallLater(ProcessHintQueue, SCR_GameModeCampaign.UI_UPDATE_DELAY);	// Delay so we show the hint after the deploy menu has closed

		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup playerGroup = groupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (!playerGroup)
			return;

		if (SCR_PlayerController.GetLocalPlayerId() == playerGroup.GetLeaderID())
		{
			switch (playerGroup.GetGroupRole())
			{
				case SCR_EGroupRole.RECON:
					GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_SQUAD_LEADER_RECON, false, false);
					break;

				case SCR_EGroupRole.TRANSPORT:
				case SCR_EGroupRole.HELI_TRANSPORT:
				case SCR_EGroupRole.ENGINEER:
					GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_SQUAD_LEADER_SUPPLIES, false, false);
					break;

				case SCR_EGroupRole.ASSAULT:
				case SCR_EGroupRole.MECHANIZED:
				case SCR_EGroupRole.MACHINEGUN:
				case SCR_EGroupRole.ANTITANK:
					GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_SQUAD_LEADER_BASIC, false, false);
					break;
				case SCR_EGroupRole.MEDIC:
				case SCR_EGroupRole.MORTAR:
					GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_SQUAD_LEADER_SUPPORT, false, false);
					break;
			}
		}
		else
		{
			switch (playerGroup.GetGroupRole())
			{
				case SCR_EGroupRole.ASSAULT:
					GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_GROUPS_ASSAULT, false, false);
					break;

				case SCR_EGroupRole.MECHANIZED:
					GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_GROUPS_MECHANIZED, false, false);
					break;

				case SCR_EGroupRole.TRANSPORT:
					GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_GROUPS_TRANSPORT, false, false);
					break;

				case SCR_EGroupRole.HELI_TRANSPORT:
					GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_GROUPS_HELI_TRANSPORT, false, false);
					break;

				case SCR_EGroupRole.RECON:
					GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_GROUPS_RECON, false, false);
					break;

				case SCR_EGroupRole.MACHINEGUN:
					GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_GROUPS_MACHINEGUN, false, false);
					break;

				case SCR_EGroupRole.ANTITANK:
					GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_GROUPS_ANTITANK, false, false);
					break;

				case SCR_EGroupRole.ENGINEER:
					GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_GROUPS_ENGINEER, false, false);
					break;

				case SCR_EGroupRole.MEDIC:
					GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_GROUPS_MEDIC, false, false);
					break;

				case SCR_EGroupRole.MORTAR:
					GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_GROUPS_MORTAR, false, false);
					break;
			}
		}


		if (!m_aShownHints.Contains(EHint.CONFLICT_SERVICE_DEPOTS))
			GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_SERVICE_DEPOTS, false, false);
		else if (!m_aShownHints.Contains(EHint.CONFLICT_RESPAWN))
			GetGame().GetCallqueue().CallLater(ShowHint, AFTER_RESPAWN_HINT_DELAY_MS, false, EHint.CONFLICT_RESPAWN, false, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCheckEstablishmentArea()
	{
		ShowHint(EHint.CONFLICT_FOB_BUILDING);
		SCR_EstablishBaseTaskEntity.GetOnPlayerEnteredEstablishingArea().Remove(OnCheckEstablishmentArea);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskAssigneeAdded(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_TaskExecutorGroup groupExec = SCR_TaskExecutorGroup.Cast(executor);
		if (groupExec)
		{
			SCR_AIGroup assignedGroup = groupManager.FindGroup(groupExec.GetGroupID());
			if (!assignedGroup)
				return;

			array<int> playerIds = {};
			playerIds = assignedGroup.GetPlayerIDs();
			if (!playerIds.Contains(SCR_PlayerController.GetLocalPlayerId()))
				return;
		}
		else
		{
			SCR_TaskExecutorPlayer playerExec = SCR_TaskExecutorPlayer.Cast(executor);
			if (!playerExec)
				return;

			if (playerExec.GetPlayerID() != SCR_PlayerController.GetLocalPlayerId())
				return;
		}

		SCR_AIGroup playerGroup = groupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (!playerGroup)
			return;

		if (SCR_EstablishBaseTaskEntity.Cast(task))
		{
			if (playerGroup == SCR_EGroupRole.ENGINEER)
				ShowHint(EHint.CONFLICT_OBJECTIVES_ESTABLISH_BASE);
			else
				ShowHint(EHint.CONFLICT_FOB_OBJECTIVE);

			return;
		}

		if (SCR_PickupRequestedTaskEntity.Cast(task))
		{
			ShowHint(EHint.CONFLICT_PICKUP_ASSIGNED);
			return;
		}

		if (SCR_ReinforceRequestedTaskEntity.Cast(task))
		{
			ShowHint(EHint.CONFLICT_REINFORCE_ASSIGNMENT);
			return;
		}

		if (SCR_RearmRequestedTaskEntity.Cast(task))
		{
			ShowHint(EHint.CONFLICT_REARM_ASSIGNMENT);
			return;
		}

		if (SCR_RepairRequestedTaskEntity.Cast(task))
		{
			ShowHint(EHint.CONFLICT_REPAIR_ASSIGNMENT);
			return;
		}

		if (SCR_FireSupportRequestedTaskEntity.Cast(task))
		{
			ShowHint(EHint.CONFLICT_FIRE_SUPPORT_ASSIGNMENT);
			return;
		}

		if (SCR_ResupplyCampaignMilitaryBaseTaskEntity.Cast(task))
		{
			ShowHint(EHint.CONFLICT_RESTOCK_ASSIGNMENT);
			return;
		}

		if (SCR_AttackTaskEntity.Cast(task))
		{
			ShowHint(EHint.CONFLICT_OBJECTIVES_ATTACK);
			return;
		}

		if (SCR_SeizeCampaignMilitaryBaseTaskEntity.Cast(task))
		{
			ShowHint(EHint.CONFLICT_OBJECTIVES_SEIZE);
			return;
		}

		if (SCR_HoldCampaignMilitaryBaseTaskEntity.Cast(task))
		{
			ShowHint(EHint.CONFLICT_OBJECTIVES_HOLD);
			return;
		}

		if (SCR_ReconTaskEntity.Cast(task))
		{
			ShowHint(EHint.CONFLICT_OBJECTIVES_RECON);
			return;
		}

		if (SCR_DismantleCampaignMilitaryBaseTaskEntity.Cast(task))
		{
			ShowHint(EHint.CONFLICT_OBJECTIVES_DISMANTLE);
			return;
		}

		// TODO: once implement the hints for objectives rally, objectives clear, regroup request, restock request
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hintID
	//! \param[in] showImmediately
	//! \param[in] showMultipleTimes
	void ShowHint(EHint hintID, bool showImmediately = false, bool showMultipleTimes = false)
	{
		if (m_Campaign.IsTutorial())
			return;

		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		SCR_HintUIInfo info = m_HintsConfig.GetHintByEnum(hintID);

		if (!info)
			return;

		if (!showMultipleTimes && (m_aShownHints.Contains(hintID) || m_aHintQueue.Contains(hintID)))
			return;

		float currentTime = GetGame().GetWorld().GetWorldTime();

		if (currentTime < m_fNextAllowedHintTimestamp && !showImmediately)
		{
			m_aHintQueue.Insert(hintID);
		}
		else
		{
			m_aShownHints.Insert(hintID);
			SCR_HintManagerComponent.ShowHint(info);

			// Show the next hint in queue after this hint's duration expires
			float durationMs = 1000 * info.GetDuration();
			m_fNextAllowedHintTimestamp = currentTime + durationMs;
			GetGame().GetCallqueue().Remove(ProcessHintQueue);
			GetGame().GetCallqueue().CallLater(ProcessHintQueue, durationMs + DELAY_BETWEEN_HINTS_MS);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ProcessHintQueue()
	{
		if (m_aHintQueue.IsEmpty())
			return;

		int hintId = m_aHintQueue[0];
		m_aHintQueue.RemoveOrdered(0);

		ShowHint(hintId, showMultipleTimes: true);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void PauseHintQueue()
	{
		GetGame().GetCallqueue().Remove(ProcessHintQueue);
	}

	//------------------------------------------------------------------------------------------------
	//! Upon death, the unconscious state is no longer detectable, we need to cache it
	//! \param[in] conscious
	void OnConsciousnessChanged(bool conscious)
	{
		m_bIsConscious = conscious;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsConscious()
	{
		return m_bIsConscious;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsLocalPlayerAlive()
	{
		ChimeraCharacter player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (player)
		{
			CharacterControllerComponent charController = player.GetCharacterController();
			if (charController && charController.GetLifeState() == ECharacterLifeState.DEAD)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	//! \param[in] playerEntity
	//! \param[in] killerEntity
	//! \param[in] killer
	void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		if (instigatorContextData.GetVictimEntity() != SCR_PlayerController.GetLocalControlledEntity())
			return;

		if (m_BaseWithPlayer)
			OnBaseLeft(m_BaseWithPlayer);

		SCR_PopUpNotification.GetInstance().HideCurrentMsg();
		GetGame().GetCallqueue().Remove(ShowHint);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	//! \param[in] faction
	void OnBaseFactionChanged(notnull SCR_MilitaryBaseComponent base, Faction faction)
	{
		if (faction != SCR_FactionManager.SGetLocalPlayerFaction())
			return;

		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

		if (!campaignBase)
			return;

		if (campaignBase.GetType() == SCR_ECampaignBaseType.BASE && GetGame().GetWorld().GetWorldTime() > FEATURE_HINT_DELAY)
		{
			SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();
			array<SCR_MilitaryBaseComponent> bases = {};
			baseManager.GetBases(bases);
			int friendlyBases;

			foreach (SCR_MilitaryBaseComponent currentBase : bases)
			{
				if (currentBase.GetOwner() != faction)
					continue;

				friendlyBases++;

				if (friendlyBases == 3)
				{
					ShowHint(EHint.CONFLICT_SUPPLIES);
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	void OnBaseEntered(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		if (m_BaseWithPlayer)
			OnBaseLeft(m_BaseWithPlayer);

		m_BaseWithPlayer = base;

		SCR_CampaignSeizingComponent seizingComponent = SCR_CampaignSeizingComponent.Cast(m_BaseWithPlayer.GetOwner().FindComponent(SCR_CampaignSeizingComponent));

		if (seizingComponent)
		{
			OnSeizingTimerChange(seizingComponent.GetSeizingStartTimestamp(), seizingComponent.GetSeizingEndTimestamp());
			seizingComponent.GetOnTimerChange().Remove(OnSeizingTimerChange);
			seizingComponent.GetOnTimerChange().Insert(OnSeizingTimerChange);
		}
		else
		{
			OnSeizingTimerChange(null, null);
		}

		SCR_CampaignFaction playerFaction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		if (m_BaseWithPlayer.GetFaction() != playerFaction)
		{
			// Entering an enemy base
			if (playerFaction && m_BaseWithPlayer.IsHQRadioTrafficPossible(playerFaction))
			{
				SCR_TaskExecutor executor = SCR_TaskExecutor.FromPlayerID(m_PlayerController.GetPlayerId());

				if (executor)
				{
					array<SCR_Task> availableTasks = {};
					SCR_TaskSystem.GetInstance().GetTasksVisibleFor(availableTasks, executor);
					SCR_SeizeCampaignMilitaryBaseTaskEntity campaignTask;

					foreach (SCR_Task task : availableTasks)
					{
						campaignTask = SCR_SeizeCampaignMilitaryBaseTaskEntity.Cast(task);

						if (!campaignTask || campaignTask.GetMilitaryBase() != m_BaseWithPlayer || campaignTask.IsTaskAssignedTo(executor))
							continue;

						// Entering an enemy base within radio signal reach while not having its seize task assigned
						ShowHint(EHint.CONFLICT_VOLUNTEERING);
						break;
					}
				}

				// Entering an enemy base within radio signal reach
				if (m_BaseWithPlayer.GetType() == SCR_ECampaignBaseType.RELAY)
					ShowHint(EHint.CONFLICT_TOWER_SEIZING);
				else
					ShowHint(EHint.CONFLICT_BASE_SEIZING);
			}
		}
		else
		{
			// Entering a friendly base
			ChimeraCharacter player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());

			if (player && player.IsInVehicle())
			{
				// Entering a friendly base in a vehicle
				if (m_BaseWithPlayer.GetType() == SCR_ECampaignBaseType.BASE && !m_BaseWithPlayer.IsHQ())
					ShowHint(EHint.CONFLICT_SUPPLY_RUNS);
			}

			// Entering a friendly base with an armory
			if (GetGame().GetWorld().GetWorldTime() > SCR_GameModeCampaign.BACKEND_DELAY && m_BaseWithPlayer.GetServiceDelegateByType(SCR_EServicePointType.ARMORY))
			{
				ShowHint(EHint.CONFLICT_LOADOUTS);
				ShowHint(EHint.GAMEPLAY_RADIO_RESPAWN);
			}

			// Entering a friendly base covered by enemy radio signal
			bool covered;

			if (SCR_FactionManager.SGetLocalPlayerFaction() == m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
				covered = base.IsHQRadioTrafficPossible(m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR));
			else
				covered = base.IsHQRadioTrafficPossible(m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));

			if (covered)
				ShowHint(EHint.CONFLICT_DEFENDING_BASES);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] base
	void OnBaseLeft(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		SCR_CampaignSeizingComponent seizingComponent = SCR_CampaignSeizingComponent.Cast(base.GetOwner().FindComponent(SCR_CampaignSeizingComponent));

		if (seizingComponent)
			seizingComponent.GetOnTimerChange().Remove(OnSeizingTimerChange);

		if (m_BaseWithPlayer == base)
			m_BaseWithPlayer = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMatchSituationChanged()
	{
		Faction winner = GetGame().GetFactionManager().GetFactionByIndex(m_Campaign.GetWinningFactionId());

		if (!winner || winner == SCR_FactionManager.SGetLocalPlayerFaction())
			return;

		m_Campaign.GetOnMatchSituationChanged().Remove(OnMatchSituationChanged);
		ShowHint(EHint.CONFLICT_LOSING);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] start
	//! \param[in] end
	void OnSeizingTimerChange(WorldTimestamp start, WorldTimestamp end)
	{
		m_fBaseWithPlayerCaptureStart = start;
		m_fBaseWithPlayerCaptureEnd = end;
		SCR_PopUpNotification.GetInstance().ChangeProgressBarFinish(end);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] msgID
	//! \param[in] playerID
	//! \param[in] factionID
	void MobileAssemblyFeedback(SCR_EMobileAssemblyStatus msgID, int playerID, int factionID)
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());

		if (!fManager)
			return;

		Faction localPlayerFaction = SCR_FactionManager.SGetLocalPlayerFaction();

		if (fManager.GetFactionIndex(localPlayerFaction) != factionID)
			return;

		const int duration = 6; // TODO: check for good const usage
		LocalizedString text;
		LocalizedString text2;
		string playerName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID);

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();

		if (mapEntity && mapEntity.IsOpen())
			m_MapCampaignUI.InitMobileAssembly(localPlayerFaction.GetFactionKey(), msgID == SCR_EMobileAssemblyStatus.DEPLOYED);

		switch (msgID)
		{
			case SCR_EMobileAssemblyStatus.DEPLOYED:
			{
				text = "#AR-Campaign_MobileAssemblyDeployed-UC";
				text2 = "#AR-Campaign_MobileAssemblyPlayerName";
				break;
			}

			case SCR_EMobileAssemblyStatus.DISMANTLED:
			{
				text = "#AR-Campaign_MobileAssemblyDismantled-UC";
				text2 = "#AR-Campaign_MobileAssemblyPlayerName";
				Faction reconfigurerFaction = SCR_FactionManager.SGetPlayerFaction(playerID);

				if (reconfigurerFaction && localPlayerFaction != reconfigurerFaction)
					playerName = reconfigurerFaction.GetFactionName();

				break;
			}

			case SCR_EMobileAssemblyStatus.DESTROYED:
			{
				text = "#AR-Campaign_MobileAssemblyDestroyed-UC";
				break;
			}
		}

		if (text != string.Empty)
		{
			if (text2 != string.Empty && playerName != string.Empty)
				SCR_PopUpNotification.GetInstance().PopupMsg(text, duration, text2: text2, text2param1: playerName, prio: SCR_ECampaignPopupPriority.MHQ);
			else
				SCR_PopUpNotification.GetInstance().PopupMsg(text, duration);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] msg
	//! \param[in] factionId
	//! \param[in] baseCallsign
	//! \param[in] callerCallsignCompany
	//! \param[in] callerCallsignPlatoon
	//! \param[in] callerCallsignSquad
	//! \param[in] callerCallsignCharacter
	//! \param[in] calledCallsignCompany
	//! \param[in] calledCallsignPlatoon
	//! \param[in] calledCallsignSquad
	//! \param[in] calledCallsignCharacter
	//! \param[in] param
	//! \param[in] seed
	//! \param[in] quality
	void PlayRadioMsg(SCR_ERadioMsg msg, int factionId, int baseCallsign, int callerCallsignCompany, int callerCallsignPlatoon, int callerCallsignSquad, int callerCallsignCharacter, int calledCallsignCompany, int calledCallsignPlatoon, int calledCallsignSquad, int calledCallsignCharacter, int param, float seed, float quality)
	{
		if (m_Campaign.IsTutorial())
			return;

		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());

		if (!pc)
			return;

		IEntity player = pc.GetMainEntity();

		if (!player)
			return;

		SCR_HQRadioSoundEntity radioSoundEntity = SCR_HQRadioSoundEntity.GetInstance();
		if (!radioSoundEntity)
			return;

		SimpleSoundComponent soundComp = radioSoundEntity.GetSimpleSoundComponent();
		if (!soundComp)
			return;

		int signalBase = soundComp.GetSignalIndex("Base");
		int signalCompanyCaller = soundComp.GetSignalIndex("CompanyCaller");
		int signalCompanyCalled = soundComp.GetSignalIndex("CompanyCalled");
		int signalPlatoonCaller = soundComp.GetSignalIndex("PlatoonCaller");
		int signalPlatoonCalled = soundComp.GetSignalIndex("PlatoonCalled");
		int signalSquadCaller = soundComp.GetSignalIndex("SquadCaller");
		int signalSquadCalled = soundComp.GetSignalIndex("SquadCalled");
		int signalSoldierCaller = soundComp.GetSignalIndex("SoldierCaller");
		int signalSoldierCalled = soundComp.GetSignalIndex("SoldierCalled");
		int signalSeed = soundComp.GetSignalIndex("Seed");
		int signalQuality = soundComp.GetSignalIndex("TransmissionQuality");

		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(factionId));
		SCR_CampaignMilitaryBaseComponent base = m_Campaign.GetBaseManager().FindBaseByCallsign(baseCallsign);

		if (base)
		{
			SCR_MilitaryBaseCallsign callsignInfo;

			if (faction == m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR))
				callsignInfo = faction.GetBaseCallsignByIndex(base.GetCallsign());
			else
				callsignInfo = faction.GetBaseCallsignByIndex(base.GetCallsign(), m_Campaign.GetCallsignOffset());

			if (callsignInfo)
				soundComp.SetSignalValue(signalBase, callsignInfo.GetSignalIndex());
		}

		if (callerCallsignCompany != SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX)
		{
			soundComp.SetSignalValue(signalCompanyCaller, callerCallsignCompany);
			soundComp.SetSignalValue(signalPlatoonCaller, callerCallsignPlatoon);
			soundComp.SetSignalValue(signalSquadCaller, callerCallsignSquad);
			soundComp.SetSignalValue(signalSoldierCaller, callerCallsignCharacter);
		}

		if (calledCallsignCompany != SCR_CampaignMilitaryBaseComponent.INVALID_PLAYER_INDEX)
		{
			soundComp.SetSignalValue(signalCompanyCalled, calledCallsignCompany);
			soundComp.SetSignalValue(signalPlatoonCalled, calledCallsignPlatoon);
			soundComp.SetSignalValue(signalSquadCalled, calledCallsignSquad);
			soundComp.SetSignalValue(signalSoldierCalled, calledCallsignCharacter);
		}

		soundComp.SetSignalValue(signalSeed, seed);
		soundComp.SetSignalValue(signalQuality, quality);

		string msgName;
		LocalizedString text;
		LocalizedString text2;
		string param1;
		string text2param1;
		string text2param2;
		int duration = SCR_PopUpNotification.DEFAULT_DURATION;
		SCR_ECampaignPopupPriority prio = SCR_ECampaignPopupPriority.DEFAULT;
		string sound;
		bool mustSetFactionKey = true;
		bool mustSetFactionIdentityVoice = false;

		switch (msg)
		{
			case SCR_ERadioMsg.SEIZED_MAIN:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_MOB;
				break;
			}

			case SCR_ERadioMsg.SEIZED_MAJOR:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_FOB;
				break;
			}

			case SCR_ERadioMsg.SEIZED_SMALL:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_COP;
				break;
			}

			case SCR_ERadioMsg.DEMOTION_RENEGADE:
			{
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				msgName = SCR_SoundEvent.SOUND_HQC_M_RENEGADE_COMMANDER;
				text = "#AR-Campaign_Demotion-UC";
				text2 = "#AR-Rank_Renegade";
				break;
			}

			case SCR_ERadioMsg.DEMOTION:
			{
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				msgName = SCR_SoundEvent.SOUND_HQC_M_DEMOTE_COMMANDER;
				text = "#AR-Campaign_Demotion-UC";
				SCR_CampaignFaction f = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

				if (f)
					text2 = f.GetRankNameUpperCase(param);

				break;
			}

			case SCR_ERadioMsg.PROMOTION_PRIVATE:
			{
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				msgName = SCR_SoundEvent.SOUND_HQC_M_PPRIVATE_COMMANDER;
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestPrivate";
				break;
			}

			case SCR_ERadioMsg.PROMOTION_CORPORAL:
			{
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				msgName = SCR_SoundEvent.SOUND_HQC_M_PCORPORAL_COMMANDER;
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestCorporal";
				break;
			}

			case SCR_ERadioMsg.PROMOTION_SERGEANT:
			{
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				msgName = SCR_SoundEvent.SOUND_HQC_M_PSERGEANT_COMMANDER;
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestSergeant";
				break;
			}

			case SCR_ERadioMsg.PROMOTION_LIEUTENANT:
			{
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				msgName = SCR_SoundEvent.SOUND_HQC_M_PLIEUTENANT_COMMANDER;
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestLieutenant";
				break;
			}

			case SCR_ERadioMsg.PROMOTION_CAPTAIN:
			{
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				msgName = SCR_SoundEvent.SOUND_HQC_M_PCAPTAIN_COMMANDER;
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestCaptain";
				break;
			}

			case SCR_ERadioMsg.PROMOTION_MAJOR:
			{
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				msgName = SCR_SoundEvent.SOUND_HQC_M_PMAJOR_COMMANDER;
				text = "#AR-Campaign_Promotion-UC";
				text2 = "#AR-Rank_WestMajor";
				break;
			}

			case SCR_ERadioMsg.VICTORY:
			{
				SCR_CampaignFaction f = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(param));

				if (!f || f != SCR_FactionManager.SGetLocalPlayerFaction())
					return;

				msgName = SCR_SoundEvent.SOUND_HQC_M_VICTORY_COMMANDER;
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				break;
			}

			case SCR_ERadioMsg.WINNING:
			{
				msgName = SCR_SoundEvent.SOUND_HQC_M_WINNING_COMMANDER;
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				break;
			}

			case SCR_ERadioMsg.LOSING:
			{
				msgName = SCR_SoundEvent.SOUND_HQC_M_LOSING_COMMANDER;
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				break;
			}

			case SCR_ERadioMsg.DEFEAT:
			{
				SCR_CampaignFaction f = SCR_CampaignFaction.Cast(fManager.GetFactionByIndex(param));

				if (!f || f == SCR_FactionManager.SGetLocalPlayerFaction())
					return;

				msgName = SCR_SoundEvent.SOUND_HQC_M_DEFEAT_COMMANDER;
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				break;
			}

			case SCR_ERadioMsg.RELAY:
			{
				msgName = SCR_SoundEvent.SOUND_SL_RRD;
				break;
			}

			case SCR_ERadioMsg.REQUEST_EVAC:
			{
				msgName = SCR_SoundEvent.SOUND_SL_ERT;
				break;
			}

			case SCR_ERadioMsg.REQUEST_FUEL:
			{
				msgName = SCR_SoundEvent.SOUND_SL_RRT;
				break;
			}

			case SCR_ERadioMsg.SUPPLIES:
			{
				msgName = SCR_SoundEvent.SOUND_SL_SDD;
				break;
			}

			case SCR_ERadioMsg.REQUEST_REINFORCEMENTS:
			{
				msgName = SCR_SoundEvent.SOUND_SL_REI;
				break;
			}

			case SCR_ERadioMsg.REQUEST_TRANSPORT:
			{
				msgName = SCR_SoundEvent.SOUND_SL_TRT;
				break;
			}

			case SCR_ERadioMsg.CONFIRM:
			{
				msgName = SCR_SoundEvent.SOUND_SL_CSR;
				break;
			}

			case SCR_ERadioMsg.TASK_ASSIGN_SEIZE:
			{
				msgName = SCR_SoundEvent.SOUND_SL_SRT;
				break;
			}

			case SCR_ERadioMsg.TASK_UNASSIGN_REFUEL:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_RRU;
				break;
			}

			case SCR_ERadioMsg.TASK_UNASSIGN_TRANSPORT:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_TRU;
				break;
			}

			case SCR_ERadioMsg.TASK_UNASSIGN_EVAC:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_ETU;
				break;
			}

			case SCR_ERadioMsg.TASK_CANCEL_REQUEST:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_RCR;
				break;
			}

			case SCR_ERadioMsg.TASK_ASSIST:
			{
				msgName = SCR_SoundEvent.SOUND_SL_CHR;
				break;
			}

			case SCR_ERadioMsg.BASE_LOST:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_BAL;
				break;
			}

			case SCR_ERadioMsg.RELAY_LOST:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_RRL;
				break;
			}

			case SCR_ERadioMsg.BASE_UNDER_ATTACK:
			{
				if (!base)
					return;

				if (base.GetType() == SCR_ECampaignBaseType.BASE)
				{
					msgName = SCR_SoundEvent.SOUND_HQC_M_BASEUNDERATTACK_COMMANDER;
					mustSetFactionKey = false;
					mustSetFactionIdentityVoice = true;
				}

				text = "#AR-Campaign_BaseUnderAttack-UC";

				if (base.GetType() == SCR_ECampaignBaseType.RELAY)
					text2 = "%1";
				else
					text2 = "%1 (%2)";

				text2param1 = base.GetBaseName();
				text2param2 = base.GetCallsignDisplayName();

				if (m_BaseWithPlayer == base)
					sound = SCR_SoundEvent.SOUND_SIREN;

				prio = SCR_ECampaignPopupPriority.BASE_UNDER_ATTACK;
				duration = 11;
				break;
			}

			case SCR_ERadioMsg.BUILT_ARMORY:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_BAA;
				break;
			}

			case SCR_ERadioMsg.BUILT_FUEL:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_BFA;
				break;
			}

			case SCR_ERadioMsg.BUILT_REPAIR:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_BRA;
				break;
			}

			case SCR_ERadioMsg.BUILT_SUPPLY:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_SCB;
				break;
			}

			case SCR_ERadioMsg.BUILT_VEHICLES_LIGHT:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_LCB;
				break;
			}

			case SCR_ERadioMsg.BUILT_VEHICLES_HEAVY:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_VCB;
				break;
			}

			case SCR_ERadioMsg.BUILT_BARRACKS:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_BCB;
				break;
			}

			case SCR_ERadioMsg.BUILT_ANTENNA:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_ACB;
				break;
			}

			case SCR_ERadioMsg.BUILT_FIELD_HOSPITAL:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_HCB;
				break;
			}

			case SCR_ERadioMsg.BUILT_HELIPAD:
			{
				msgName = SCR_SoundEvent.SOUND_HQ_BHA;
				break;
			}

			case SCR_ERadioMsg.DESTROYED_ARMORY:
			{
				if (!base)
					return;

				msgName = SCR_SoundEvent.SOUND_HQ_BAD;
				text = "#AR-Campaign_Building_Destroyed-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_Armory-UC";
				duration = 5;
				break;
			}

			case SCR_ERadioMsg.DESTROYED_FUEL:
			{
				if (!base)
					return;

				msgName = SCR_SoundEvent.SOUND_HQ_BFD;
				text = "#AR-Campaign_Building_Destroyed-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_FuelDepot-UC";
				duration = 5;
				break;
			}

			case SCR_ERadioMsg.DESTROYED_REPAIR:
			{
				if (!base)
					return;

				msgName = SCR_SoundEvent.SOUND_HQ_BRD;
				text = "#AR-Campaign_Building_Destroyed-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_RepairDepot-UC";
				duration = 5;
				break;
			}

			case SCR_ERadioMsg.REPAIRED_ARMORY:
			{
				if (!base)
					return;

				msgName = SCR_SoundEvent.SOUND_HQ_BAR;
				text = "#AR-Campaign_Building_Available-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_Armory-UC";
				duration = 5;
				break;
			}

			case SCR_ERadioMsg.REPAIRED_FUEL:
			{
				if (!base)
					return;

				msgName = SCR_SoundEvent.SOUND_HQ_BFR;
				text = "#AR-Campaign_Building_Available-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_FuelDepot-UC";
				duration = 5;
				break;
			}

			case SCR_ERadioMsg.REPAIRED_REPAIR:
			{
				if (!base)
					return;

				msgName = SCR_SoundEvent.SOUND_HQ_BRR;
				text = "#AR-Campaign_Building_Available-UC";
				text2 = base.GetBaseName();
				param1 = "#AR-Campaign_Building_RepairDepot-UC";
				duration = 5;
				break;
			}

			case SCR_ERadioMsg.COMMANDER_ARRIVAL:
			{
				msgName = SCR_SoundEvent.SOUND_HQC_ARRIVAL_HQCBATTALION;
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				break;
			}

			case SCR_ERadioMsg.COMMANDER_RESIGNATION:
			{
				msgName = SCR_SoundEvent.SOUND_HQC_RESIGNATION_HQCBATTALION;
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				break;
			}

			case SCR_ERadioMsg.COMMANDER_DISRUPTION:
			{
				msgName = SCR_SoundEvent.SOUND_HQC_DISRUPTION_HQCBATTALION;
				mustSetFactionKey = false;
				mustSetFactionIdentityVoice = true;
				break;
			}
		}

		bool isFriendly = faction == fManager.GetPlayerFaction(pc.GetPlayerId());

		if (!msgName.IsEmpty())
		{
			if (mustSetFactionKey)
				msgName = msgName + "_" + faction.GetFactionKey();
			
			if (mustSetFactionIdentityVoice)
			{
				int signalIdentityVoice = soundComp.GetSignalIndex("IdentityVoice");
				soundComp.SetSignalValue(signalIdentityVoice, faction.GetIndentityVoiceSignal());
			}

			BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_AudioSettings");
			bool announcerEnabled = true; //enabled so everything is in the default state if we dont find settings
			if (settings)
			{
				settings.Get("m_bHQAnnouncer", announcerEnabled);
			}
			
			if (announcerEnabled)
				radioSoundEntity.PlayRadioSound(msgName);
		}

		if (isFriendly && (!text.IsEmpty() || !text2.IsEmpty()))
			SCR_PopUpNotification.GetInstance().PopupMsg(text, duration, text2: text2, prio: prio, param1: param1, sound: sound, text2param1: text2param1, text2param2: text2param2);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnXPChanged(int totalXP, SCR_EXPRewards rewardID, int XP, bool volunteer, bool profileUsed, int skillLevel)
	{
		if (rewardID == SCR_EXPRewards.ENEMY_KILL || rewardID == SCR_EXPRewards.ENEMY_KILL_VEH)
		{
			GetGame().GetCallqueue().CallLater(ShowHint, 30000 + Math.RandomIntInclusive(0, 30000), false, EHint.CONFLICT_ELIMINATING_ENEMIES, false, false);
		}

		if (XP > 0)
		{
			if (rewardID != SCR_EXPRewards.ENEMY_KILL && rewardID != SCR_EXPRewards.ENEMY_KILL_VEH)
				ShowHint(EHint.CONFLICT_PROMOTIONS);

			if (rewardID == SCR_EXPRewards.VETERANCY)
				ShowHint(EHint.CONFLICT_VETERANCY);
		}

		PrintFormat("Local player XP change: %1, event: %2, current XP: %3", XP, SCR_Enum.GetEnumName(SCR_EXPRewards, rewardID), totalXP, level: LogLevel.VERBOSE);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerJoinedGroup(SCR_AIGroup group, int playerID)
	{
		if (playerID != SCR_PlayerController.GetLocalPlayerId())
			return;

		switch (group.GetGroupRole())
		{
			case SCR_EGroupRole.MORTAR:
			case SCR_EGroupRole.ANTITANK:
			case SCR_EGroupRole.MACHINEGUN:
			case SCR_EGroupRole.ENGINEER:
			case SCR_EGroupRole.MEDIC:
			case SCR_EGroupRole.RECON:
				ShowHint(EHint.CONFLICT_GROUPS_SUPPORT);
				break;

			case SCR_EGroupRole.TRANSPORT:
			case SCR_EGroupRole.MECHANIZED:
				ShowHint(EHint.CONFLICT_GROUPS_LOGISTIC);
				break;

			default:
				return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupMenuShow()
	{
		ShowHint(EHint.CONFLICT_GROUPS_INTRO);

		SCR_GroupMenu.GetOnGroupMenuShown().Remove(OnGroupMenuShow);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskSelected(SCR_Task task)
	{
		if (!task)
			return;
		
		if (task.IsInherited(SCR_PickupRequestedTaskEntity) ||
			task.IsInherited(SCR_ReinforceRequestedTaskEntity) ||
			task.IsInherited(SCR_RearmRequestedTaskEntity) ||
			task.IsInherited(SCR_RepairRequestedTaskEntity) ||
			task.IsInherited(SCR_FireSupportRequestedTaskEntity) ||
			task.IsInherited(SCR_HoldCampaignMilitaryBaseTaskEntity) ||
			task.IsInherited(SCR_ResupplyCampaignMilitaryBaseTaskEntity))
		{
			ShowHint(EHint.CONFLICT_SUPPORT_REQUESTS_BASICS);
			return;
		}

		if (task.IsInherited(SCR_AttackTaskEntity) ||
			task.IsInherited(SCR_SeizeCampaignMilitaryBaseTaskEntity) ||
			task.IsInherited(SCR_HoldCampaignMilitaryBaseTaskEntity) ||
			task.IsInherited(SCR_ReconTaskEntity) ||
			task.IsInherited(SCR_DismantleCampaignMilitaryBaseTaskEntity))
		{
			SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
			if (!groupManager)
				return;

			SCR_AIGroup playerGroup = groupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
			if (!playerGroup)
				return;
			
			SCR_Faction playerFaction = SCR_Faction.Cast(playerGroup.GetFaction());
			if (!playerFaction)
				return;

			if (SCR_PlayerController.GetLocalPlayerId() == playerGroup.GetLeaderID() && SCR_PlayerController.GetLocalPlayerId() != playerFaction.GetCommanderId())
				ShowHint(EHint.CONFLICT_OBJECTIVES_SQUAD_LEADER);
			else
				ShowHint(EHint.CONFLICT_OBJECTIVES_BASICS);

			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called when local player enters any vehicle compartment.
	//! \param[in] playerCharacter The player character entity
	//! \param[in] compartmentEntit The compartment entity entered
	protected void OnVehicleEnter(ChimeraCharacter playerCharacter, IEntity compartmentEntit)
	{
		if (playerCharacter != SCR_PlayerController.GetLocalControlledEntity())
			return;

		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup playerGroup = groupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (!playerGroup)
			return;

		if (SCR_PlayerController.GetLocalPlayerId() == playerGroup.GetLeaderID())
		{
			switch (playerGroup.GetGroupRole())
			{
				case SCR_EGroupRole.TRANSPORT:
				case SCR_EGroupRole.HELI_TRANSPORT:
				case SCR_EGroupRole.ENGINEER:
					ShowHint(EHint.CONFLICT_SQUAD_LEADER_LOGISTIC_FOB);
					break;
			}
		}
		else
		{
			GetGame().GetCallqueue().CallLater(ShowHint, m_iVehicleHintDelay, false, EHint.GAMEPLAY_VEHICLE_INVENTORY, false, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Registers a callback for vehicle enter events for the local player.
	protected void RegisterVehicleListener()
	{
		GetGame().GetCallqueue().CallLater(RegisterVehicleEnter, 100, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterVehicleEnter()
	{
		ChimeraCharacter player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!player)
			return;

		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(player.FindComponent(SCR_CompartmentAccessComponent));
		if (!accessComp)
		{
			GetGame().GetCallqueue().Remove(RegisterVehicleEnter);
			return;
		}

		accessComp.GetOnPlayerCompartmentEnter().Insert(OnVehicleEnter);
		GetGame().GetCallqueue().Remove(RegisterVehicleEnter);
	}

	//------------------------------------------------------------------------------------------------
	protected void UnregisterVehicleEnter()
	{
		ChimeraCharacter player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!player)
			return;

		SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast(player.FindComponent(SCR_CompartmentAccessComponent));
		if (!accessComp)
			return;

		accessComp.GetOnPlayerCompartmentEnter().Remove(OnVehicleEnter);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskStateChanged(SCR_Task task, SCR_ETaskState newState)
	{
		if (newState != SCR_ETaskState.COMPLETED)
			return;

		array<int> assigneeIDs = task.GetTaskAssigneePlayerIDs();
		int myPlayerId = SCR_PlayerController.GetLocalPlayerId();
		if (assigneeIDs.Contains(myPlayerId))
		{
 			ShowHint(EHint.CONFLICT_OBJECTIVES_ADVANCED);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRadialMenuOpen()
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup playerGroup = groupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (!playerGroup)
			return;

		if (SCR_PlayerController.GetLocalPlayerId() == playerGroup.GetLeaderID())
		{
			ShowHint(EHint.CONFLICT_SUPPORT_REQUESTS_SQUAD_LEADER);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnJournalOpened(bool newVisibility)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup playerGroup = groupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (!playerGroup)
			return;

		if (SCR_PlayerController.GetLocalPlayerId() == playerGroup.GetLeaderID())
		{
			ShowHint(EHint.CONFLICT_SQUAD_LEADER_ADVANCED);
		}

		ShowHint(EHint.CONFLICT_TASK_RELEVANCE);
	}

	//------------------------------------------------------------------------------------------------
	protected void NightHint()
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;
		
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager)
			return;

		float now = timeManager.GetTimeOfTheDay();

		if (timeManager.IsNightHour(now) && !m_bWasNight)
			ShowHint(EHint.CONFLICT_NIGHT_PLAY);

		m_bWasNight = timeManager.IsNightHour(now);

		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (hintManager && hintManager.WasShown(EHint.CONFLICT_NIGHT_PLAY))
			GetGame().GetCallqueue().Remove(NightHint);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnArsenalOpen()
	{
		ShowHint(EHint.CONFLICT_MSAR);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemSlotHover(SCR_InventorySlotUI slot)
	{
		if (!slot)
			return;

		if (slot.IsInherited(SCR_InventorySlotWeaponSlotsUI))
			// CallLater (with delay) is part of the intended design: it prevents showing the hint too aggressively
			GetGame().GetCallqueue().CallLater(ShowHint, 3000, false, EHint.GAMEPLAY_WEAPON_INSPECTION, false, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void TryRegisterInventoryHover()
	{
		SCR_InventoryMenuUI menu = SCR_InventoryMenuUI.GetInventoryMenu();
		if (menu)
		{
			menu.GetOnItemHover().Insert(OnItemSlotHover);
			GetGame().GetCallqueue().Remove(TryRegisterInventoryHover);
		}
		else
		{
			GetGame().GetCallqueue().CallLater(TryRegisterInventoryHover, 500, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckSquadCohesionHint()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		SCR_AIGroup group = groupsManager.GetPlayerGroup(m_PlayerController.GetPlayerId());
		if (!group)
			return;

		if (group.GetLeaderID() != m_PlayerController.GetPlayerId())
			return;

		SCR_AIGroupCohesionComponent cohesionComp = SCR_AIGroupCohesionComponent.Cast(group.FindComponent(SCR_AIGroupCohesionComponent));
		if (!cohesionComp)
			return;

		array<int> playersInCohesion = {};
		cohesionComp.GetPlayersInCohesion(playersInCohesion);

		int total = group.GetPlayerCount();
		if (total < 2)
			return;

		float ratio = playersInCohesion.Count() / total;
		if (ratio >= 0.6)
		{
			if (!m_bCohesionHintWasShown)
			{
				ShowHint(EHint.CONFLICT_SQUAD_LEADER_COHESION);
				m_bCohesionHintWasShown = true;
			}
		}
		else
		{
			m_bCohesionHintWasShown = false;
		}

		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (hintManager && hintManager.WasShown(EHint.CONFLICT_SQUAD_LEADER_COHESION))
		{
			GetGame().GetCallqueue().Remove(CheckSquadCohesionHint);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ProcessEvents(bool activate)
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		GetGame().GetCallqueue().Remove(CheckPlayerInsideRadioRange);
		GetGame().GetCallqueue().Remove(RefreshCurrentPopupMessage);
		GetGame().GetCallqueue().Remove(GroupLeaderHint);
		GetGame().GetCallqueue().Remove(LoneDriverHint);
		GetGame().GetCallqueue().Remove(TransportRequestHint);
		GetGame().GetCallqueue().Remove(NightHint);
		GetGame().GetCallqueue().Remove(CheckSquadCohesionHint);

		if (activate)
		{
			GetGame().GetCallqueue().CallLater(CheckPlayerInsideRadioRange, 3000, true);
			GetGame().GetCallqueue().CallLater(RefreshCurrentPopupMessage, 500, true);
			GetGame().GetCallqueue().CallLater(GroupLeaderHint, FEATURE_HINT_DELAY, true);
			GetGame().GetCallqueue().CallLater(LoneDriverHint, FEATURE_HINT_DELAY, true);
			GetGame().GetCallqueue().CallLater(TransportRequestHint, FEATURE_HINT_DELAY, true);
			GetGame().GetCallqueue().CallLater(NightHint, NIGHT_HINT_DELAY_MS, true);
			GetGame().GetCallqueue().CallLater(CheckSquadCohesionHint, COHESION_HINT_DELAY_MS, true);

			GetGame().GetInputManager().AddActionListener("TasksOpen", EActionTrigger.DOWN, RegisterTasksShown);
			SCR_TaskManagerUIComponent taskUIManager = SCR_TaskManagerUIComponent.GetInstance();
			if (taskUIManager)
				taskUIManager.GetOnTaskHUDVisible().Insert(ShowVolunteerHint);

			SCR_MilitaryBaseSystem.GetInstance().GetOnBaseFactionChanged().Insert(OnBaseFactionChanged);

			if (m_Campaign)
			{
				m_Campaign.GetOnPlayerKilled().Insert(OnPlayerKilled);
				m_Campaign.GetBaseManager().GetOnLocalPlayerEnteredBase().Insert(OnBaseEntered);
				m_Campaign.GetBaseManager().GetOnLocalPlayerLeftBase().Insert(OnBaseLeft);
				m_Campaign.GetBaseManager().GetOnSignalChanged().Insert(BaseOutOfRangeHint);
				m_Campaign.GetOnMatchSituationChanged().Insert(OnMatchSituationChanged);
			}

			SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerXPHandlerComponent));

			if (comp)
				comp.GetOnXPChanged().Insert(OnXPChanged);

			SCR_ResourcePlayerControllerInventoryComponent inventoryComp = SCR_ResourcePlayerControllerInventoryComponent.Cast(m_PlayerController.FindComponent(SCR_ResourcePlayerControllerInventoryComponent));

			if (inventoryComp)
				inventoryComp.GetOnPlayerInteraction().Insert(OnPlayerSuppliesInteraction);

			SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
			SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
			SCR_FactionCommanderVolunteerUserAction.GetOnCanBePerformed().Insert(OnCommanderVolunteer);
			if (SCR_FactionCommanderHandlerComponent.GetInstance())
				SCR_FactionCommanderHandlerComponent.GetInstance().GetOnFactionCommanderChanged().Insert(OnFactionCommanderChanged);

			SCR_CampaignLogisticMapUIBase.GetOnBaseSelected().Insert(OnBaseClickedShowSupplyLimit);
			SCR_EstablishBaseTaskEntity.GetOnPlayerEnteredEstablishingArea().Insert(OnCheckEstablishmentArea);
			SCR_Task.GetOnTaskAssigneeAdded().Insert(OnTaskAssigneeAdded);
			SCR_AIGroup.GetOnPlayerAdded().Insert(OnPlayerJoinedGroup);
			SCR_GroupMenu.GetOnGroupMenuShown().Insert(OnGroupMenuShow);
			SCR_TaskManagerUIComponent m_TaskManager = SCR_TaskManagerUIComponent.GetInstance();
			if (m_TaskManager)
				m_TaskManager.GetOnTaskSelected().Insert(OnTaskSelected);

			RegisterVehicleListener();

			SCR_Task.GetOnTaskStateChanged().Insert(OnTaskStateChanged);
			SCR_TaskManagerUIComponent taskManager = SCR_TaskManagerUIComponent.GetInstance();
			if (taskManager)
				taskManager.GetOnTaskHUDVisible().Insert(OnJournalOpened);

			SCR_InventoryStorageBaseUI.GetOnArsenalEnter().Insert(OnArsenalOpen);
			TryRegisterInventoryHover();
		}
		else
		{
			GetGame().GetInputManager().RemoveActionListener("TasksOpen", EActionTrigger.DOWN, RegisterTasksShown);
			SCR_TaskManagerUIComponent taskUIManager = SCR_TaskManagerUIComponent.GetInstance();
			if (taskUIManager)
				taskUIManager.GetOnTaskHUDVisible().Remove(ShowVolunteerHint);

			SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();

			if (baseManager)
				baseManager.GetOnBaseFactionChanged().Remove(OnBaseFactionChanged);

			if (m_Campaign)
			{
				m_Campaign.GetOnPlayerKilled().Remove(OnPlayerKilled);
				m_Campaign.GetOnMatchSituationChanged().Remove(OnMatchSituationChanged);

				SCR_CampaignMilitaryBaseManager campaignBaseManager = m_Campaign.GetBaseManager();

				if (campaignBaseManager)
				{
					campaignBaseManager.GetOnLocalPlayerEnteredBase().Remove(OnBaseEntered);
					campaignBaseManager.GetOnLocalPlayerLeftBase().Remove(OnBaseLeft);
					campaignBaseManager.GetOnSignalChanged().Remove(BaseOutOfRangeHint);
				}
			}

			if (m_PlayerController)
			{
				SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerXPHandlerComponent));

				if (comp)
					comp.GetOnXPChanged().Remove(OnXPChanged);

				SCR_ResourcePlayerControllerInventoryComponent inventoryComp = SCR_ResourcePlayerControllerInventoryComponent.Cast(m_PlayerController.FindComponent(SCR_ResourcePlayerControllerInventoryComponent));

				if (inventoryComp)
					inventoryComp.GetOnPlayerInteraction().Remove(OnPlayerSuppliesInteraction);
			}

			SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
			SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);
			SCR_FactionCommanderVolunteerUserAction.GetOnCanBePerformed().Remove(OnCommanderVolunteer);
			if (SCR_FactionCommanderHandlerComponent.GetInstance())
				SCR_FactionCommanderHandlerComponent.GetInstance().GetOnFactionCommanderChanged().Remove(OnFactionCommanderChanged);

			SCR_CampaignLogisticMapUIBase.GetOnBaseSelected().Remove(OnBaseClickedShowSupplyLimit);
			SCR_EstablishBaseTaskEntity.GetOnPlayerEnteredEstablishingArea().Remove(OnCheckEstablishmentArea);
			SCR_Task.GetOnTaskAssigneeAdded().Remove(OnTaskAssigneeAdded);
			SCR_AIGroup.GetOnPlayerAdded().Remove(OnPlayerJoinedGroup);
			SCR_GroupMenu.GetOnGroupMenuShown().Remove(OnGroupMenuShow);
			SCR_TaskManagerUIComponent m_TaskManager = SCR_TaskManagerUIComponent.GetInstance();
			if (m_TaskManager)
				m_TaskManager.GetOnTaskSelected().Remove(OnTaskSelected);

			UnregisterVehicleEnter();

			SCR_Task.GetOnTaskStateChanged().Remove(OnTaskStateChanged);
			SCR_TaskManagerUIComponent taskManager = SCR_TaskManagerUIComponent.GetInstance();
			if (taskManager)
				taskManager.GetOnTaskHUDVisible().Remove(OnJournalOpened);

			SCR_InventoryStorageBaseUI.GetOnArsenalEnter().Remove(OnArsenalOpen);
			SCR_InventoryMenuUI menu = SCR_InventoryMenuUI.GetInventoryMenu();
			if (menu)
				menu.GetOnItemHover().Remove(OnItemSlotHover);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] changing
	//! \param[in] becameOwner
	void OnOwnershipChanged(bool changing, bool becameOwner)
	{
		if (changing)
			return;

		ProcessEvents(becameOwner);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowVolunteerHint(bool newVisibility)
	{
		if (GetGame().GetWorld().GetWorldTime() < FEATURE_HINT_DELAY)
			return;

		SCR_TaskManagerUIComponent taskUIManager = SCR_TaskManagerUIComponent.GetInstance();
		if (taskUIManager)
			taskUIManager.GetOnTaskHUDVisible().Remove(ShowVolunteerHint);

		ShowHint(EHint.CONFLICT_VOLUNTEERING);
		ShowHint(EHint.CONFLICT_PRIMARY_OBJECTIVES);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] interactionType
	//! \param[in] playerController
	//! \param[in] resourceComponentFrom
	//! \param[in] resourceComponentTo
	//! \param[in] resourceType
	//! \param[in] resourceValue
	void OnPlayerSuppliesInteraction(EResourcePlayerInteractionType interactionType, PlayerController playerController, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue)
	{
		if (interactionType != EResourcePlayerInteractionType.VEHICLE_UNLOAD)
			return;

		SCR_ResourcePlayerControllerInventoryComponent comp = SCR_ResourcePlayerControllerInventoryComponent.Cast(m_PlayerController.FindComponent(SCR_ResourcePlayerControllerInventoryComponent));

		if (comp)
			comp.GetOnPlayerInteraction().Remove(OnPlayerSuppliesInteraction);

		ShowHint(EHint.CONFLICT_BUILDING);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!m_PlayerController)
			return;

		m_Campaign = SCR_GameModeCampaign.GetInstance();

		m_PlayerController.GetOnOwnershipChangedInvoker().Insert(OnOwnershipChanged);

		RplComponent rpl = RplComponent.Cast(m_PlayerController.FindComponent(RplComponent));

		if (rpl && rpl.IsOwner())
			ProcessEvents(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!GetGame().InPlayMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);

		m_PlayerController = SCR_PlayerController.Cast(owner);

		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		//Parse & register hints list
		Resource container = BaseContainerTools.LoadContainer(m_sHintsConfig);
		m_HintsConfig = SCR_CampaignHintStorage.Cast(BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer()));
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CampaignFeedbackComponent()
	{
		ProcessEvents(false);
	}
}

//! Popup message priorities sorted from lowest to highest
enum SCR_ECampaignPopupPriority
{
	DEFAULT,
	SUPPLIES_HANDLED,
	TASK_AVAILABLE,
	RELAY_DETECTED,
	TASK_PROGRESS,
	TASK_DONE,
	BASE_LOST,
	MHQ,
	BASE_UNDER_ATTACK,
	RESPAWN,
	MATCH_END
}

enum SCR_ECampaignSeizingMessagePrio
{
	NONE,
	SEIZING_YOU = 998,
	SEIZING_ENEMY = 999
}
