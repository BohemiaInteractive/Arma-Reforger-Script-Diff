[EntityEditorProps(category: "GameScripted/FastTravel", description: "Handles client > server communication for Fast travel in Conflict. Should be attached to PlayerController.")]
class SCR_CampaignFastTravelComponentClass : SCR_FastTravelComponentClass
{
	[Attribute("50", params: "0 inf 1")]
	protected int m_iSupplyCost;

	[Attribute("100", desc: "Tolerance when player identifies their location in the map (meters)", params: "0 inf 1")]
	protected int m_iPickupRadius;

	[Attribute("200", desc: "If there are enemies closer that this distance to the player, the request will fail", params: "0 inf 1")]
	protected int m_iNoEnemyRadius;

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetSupplyCost()
	{
		return m_iSupplyCost;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetPickupRadius()
	{
		return m_iPickupRadius;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetNoEnemyRadius()
	{
		return m_iNoEnemyRadius;
	}
}

class SCR_CampaignFastTravelComponent : SCR_FastTravelComponent
{
	protected bool m_bSelectionInProgress;
	protected bool m_bEnemiesNearby;

	protected SCR_MapRadialUI m_MapContextualMenu;

	protected SCR_SelectionMenuCategoryEntry m_MainEntry;

	protected SCR_CampaignFaction m_PlayerFaction;

	protected ref map<SCR_SelectionMenuEntry, SCR_DeployableSpawnPoint> m_mMenuEntries = new map<SCR_SelectionMenuEntry, SCR_DeployableSpawnPoint>();

	//------------------------------------------------------------------------------------------------
	override protected bool ServerSanityCheck(notnull IEntity target)
	{
		if (!super.ServerSanityCheck(target))
			return false;

		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(m_PlayerController.GetControlledEntity());

		if (!player)
			return false;

		if (!m_PlayerFaction)
			m_PlayerFaction = SCR_CampaignFaction.Cast(player.GetFaction());

		SCR_CampaignFastTravelComponentClass componentData = SCR_CampaignFastTravelComponentClass.Cast(GetComponentData(GetOwner()));

		if (!componentData)
			return false;

		m_bEnemiesNearby = false;
		GetGame().GetWorld().QueryEntitiesBySphere(player.GetOrigin(), componentData.GetNoEnemyRadius(), ProcessEntity, FilterEntity, EQueryEntitiesFlags.DYNAMIC | EQueryEntitiesFlags.WITH_OBJECT);

		if (m_bEnemiesNearby)
			SCR_NotificationsComponent.SendToPlayer(m_PlayerController.GetPlayerId(), ENotification.FASTTRAVEL_ENEMIES_NEARBY);

		return !m_bEnemiesNearby;
	}


	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CheckSpawnpointAvailability(RplId spawnpointId)
	{
		SCR_RestrictedDeployableSpawnPoint spawnpoint = SCR_RestrictedDeployableSpawnPoint.Cast(GetEntityByDestinationId(spawnpointId));

		if (!spawnpoint)
			return;

		if (spawnpoint.GetRespawnCount() >= spawnpoint.GetMaxRespawns())
			return;

		Rpc(RpcDo_EnableDestination, spawnpointId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_EnableDestination(RplId spawnpointId)
	{
		SCR_DeployableSpawnPoint spawnpoint = SCR_DeployableSpawnPoint.Cast(GetEntityByDestinationId(spawnpointId));

		if (!spawnpoint)
			return;

		SCR_SelectionMenuEntry entry = SCR_MapHelper<SCR_SelectionMenuEntry, SCR_DeployableSpawnPoint>.GetKeyByValue(m_mMenuEntries,spawnpoint);
		
		if (!entry)
			return;
		
		entry.SetName("#AR-MapMarker_DeployedRadio");
		entry.Enable(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Reads the cooldown value from player's rank
	override protected int GetCooldown()
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());

		if (!fManager)
			return m_iCooldown;

		IEntity player = m_PlayerController.GetControlledEntity();

		if (!player)
			return m_iCooldown;

		return fManager.GetRankFastTravelCooldown(SCR_CharacterRankComponent.GetCharacterRank(player));
	}

	//------------------------------------------------------------------------------------------------
	protected bool FilterEntity(IEntity ent)
	{
		return SCR_ChimeraCharacter.Cast(ent) != null;
	}

	//------------------------------------------------------------------------------------------------
	protected bool ProcessEntity(IEntity ent)
	{
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(ent);
		Faction entityFaction = char.GetFaction();

		if (!entityFaction.IsFactionEnemy(m_PlayerFaction))
			return true;

		if (char.GetCharacterController().IsDead())
			return true;

		m_bEnemiesNearby = true;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] config
	void OnMapOpen(MapConfiguration config)
	{
		if (!m_MapContextualMenu)
			m_MapContextualMenu = SCR_MapRadialUI.GetInstance();

		if (!m_MapContextualMenu)
			return;

		m_MapContextualMenu.GetOnMenuInitInvoker().Insert(SetupMapRadialMenuEntries);
		m_MapContextualMenu.GetOnEntryPerformedInvoker().Insert(OnMapFastTravelRequested);
		m_MapContextualMenu.GetOnEntrySelectedInvoker().Insert(CenterMapOnDestination);

		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] config
	void OnMapClose(MapConfiguration config)
	{
		SCR_SpawnPoint.Event_SpawnPointAdded.Remove(OnSpawnpointAdded);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Remove(OnSpawnpointRemoved);

		SCR_MapEntity.GetOnSelection().Remove(SendPlayerCoords);
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);

		GetGame().GetCallqueue().Remove(RefreshShownCooldown);

		SCR_HintManagerComponent.HideHint();

		if (m_bSelectionInProgress)
			SCR_NotificationsComponent.SendLocal(ENotification.FASTTRAVEL_PLAYER_LOCATION_CANCELLED);

		ToggleDestinationSelection(false);

		if (!m_MapContextualMenu)
			return;

		m_MapContextualMenu.GetOnMenuInitInvoker().Remove(SetupMapRadialMenuEntries);
		m_MapContextualMenu.GetOnEntryPerformedInvoker().Remove(OnMapFastTravelRequested);
		m_MapContextualMenu.GetOnEntrySelectedInvoker().Remove(CenterMapOnDestination);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapFastTravelRequested(SCR_SelectionMenuEntry element, float[] worldPos)
	{
		SCR_DeployableSpawnPoint spawnpoint = m_mMenuEntries.Get(element);

		if (!spawnpoint)
			return;

		SCR_CampaignFeedbackComponent feedbackComp = SCR_CampaignFeedbackComponent.GetInstance();

		if (feedbackComp)
			feedbackComp.ShowHint(EHint.CONFLICT_TRANSPORT_PICKUP, true, true);

		ToggleDestinationSelection(true);
		SetDestination(FindDestinationId(spawnpoint), string.Empty);

		SCR_MapEntity.GetOnSelection().Remove(SendPlayerCoords);
		SCR_MapEntity.GetOnSelection().Insert(SendPlayerCoords);
	}

	//------------------------------------------------------------------------------------------------
	protected void SendPlayerCoords(vector coords)
	{
		SCR_MapEntity.GetOnSelection().Remove(SendPlayerCoords);
		SCR_HintManagerComponent.HideHint();

		ToggleDestinationSelection(false);

		if (!m_PlayerController)
			return;

		IEntity player = m_PlayerController.GetControlledEntity();

		if (!player)
			return;

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();

		if (!mapEntity)
			return;

		SCR_CampaignFastTravelComponentClass componentData = SCR_CampaignFastTravelComponentClass.Cast(GetComponentData(GetOwner()));

		if (!componentData)
			return;

		if (player && mapEntity.IsOpen())
		{
			SCR_GadgetManagerComponent comp = SCR_GadgetManagerComponent.GetGadgetManager(player);

			if (comp)
				comp.RemoveHeldGadget();
		}

		float x, y;
		mapEntity.ScreenToWorld(coords[0], coords[2], x, y);
		coords[0] = x;
		coords[2] = y;

		if (vector.DistanceXZ(player.GetOrigin(), coords) > componentData.GetPickupRadius())
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.ACTION_FAILED);
			SCR_NotificationsComponent.SendLocal(ENotification.FASTTRAVEL_PLAYER_LOCATION_WRONG);
			return;
		}

		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_MAP_CLICK_POINT_ON);
		FastTravel();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupMapRadialMenuEntries()
	{
		m_mMenuEntries.Clear();

		if (!m_MapContextualMenu || !m_PlayerController)
			return;

		// Parent for fast travel radial menu entries
		m_MainEntry = m_MapContextualMenu.AddRadialCategory("#AR-Tasks_TitleTransport");
		m_MainEntry.SetIconFromDeafaultImageSet("terrainIcon");
		ComposeDestinationList();
		bool available = RefreshMainEntry();
		
		if (!available)
			return;

		SCR_SpawnPoint.Event_SpawnPointAdded.Insert(OnSpawnpointAdded);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Insert(OnSpawnpointRemoved);

		if (m_mMenuEntries.IsEmpty())
		{
			m_MainEntry.Enable(false);
			return;
		}

		RefreshDestinationList();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool RefreshMainEntry()
	{
		ChimeraWorld world = GetGame().GetWorld();
		WorldTimestamp timestamp = world.GetServerTimestamp();

		// Disable fast travel entry if its cooldown is still in progress
		bool cooldownDone = !m_fNextTravelAvailableAt || timestamp.GreaterEqual(m_fNextTravelAvailableAt);
		m_MainEntry.SetName("#AR-Tasks_TitleTransport");
		m_MainEntry.Enable(cooldownDone);

		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(m_PlayerController.GetControlledEntity());

		if (!player)
			return false;

		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (player.IsInVehicle())
		{
			m_MainEntry.Enable(false);
			m_MainEntry.SetName(string.Empty);
			return false;
		}

		// Disable fast travel entry if player is renegade
		if (fManager && fManager.IsRankRenegade(SCR_CharacterRankComponent.GetCharacterRank(player)))
		{
			m_MainEntry.Enable(false);
			m_MainEntry.SetName("#AR-Rank_Renegade");
			return false;
		}

		// Disable fast travel entry if player is carrying a radio backpack
		EquipedLoadoutStorageComponent loadoutStorage = EquipedLoadoutStorageComponent.Cast(player.FindComponent(EquipedLoadoutStorageComponent));

		if (loadoutStorage)
		{
			IEntity backpack = loadoutStorage.GetClothFromArea(LoadoutBackpackArea);
			
			if (backpack && BaseRadioComponent.Cast(backpack.FindComponent(BaseRadioComponent)))
			{
				m_MainEntry.Enable(false);
				m_MainEntry.SetName("#AR-ArsenalItemType_RadioBackpack_Name");
				return false;
			}
		}

		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(player);

		if (resourceComponent)
		{
			SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.SELF, EResourceType.SUPPLIES);

			if (consumer)
			{
				//Print(consumer.GetAggregatedResourceValue());
			}
		}

		if (campaign && !campaign.GetBaseManager().IsEntityInFactionRadioSignal(player, player.GetFaction()))
		{
			m_MainEntry.Enable(false);
			m_MainEntry.SetName("#AR-Campaign_NoRadioSignal");
			return false;
		}

		if (!cooldownDone)
			GetGame().GetCallqueue().CallLater(RefreshShownCooldown, SCR_GameModeCampaign.UI_UPDATE_DELAY, true);

		return cooldownDone;
	}

	//------------------------------------------------------------------------------------------------
	protected void CenterMapOnDestination(SCR_SelectionMenuEntry entry, int id)
	{
		if (!entry)
			return;

		SCR_DeployableSpawnPoint spawnpoint = m_mMenuEntries.Get(entry);

		if (!spawnpoint)
			return;

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();

		if (!mapEntity || !mapEntity.IsOpen())
			return;

		vector spawnpointPos = spawnpoint.GetOrigin();
		float x, y;
		mapEntity.WorldToScreen(spawnpointPos[0], spawnpointPos[2], x, y);
		mapEntity.PanSmooth(x, y);
	}

	//------------------------------------------------------------------------------------------------
	protected void ComposeDestinationList()
	{
		array<SCR_SpawnPoint> spawnpoints = SCR_SpawnPoint.GetSpawnPoints();
		SCR_RestrictedDeployableSpawnPoint radioSpawnpoint;
		int playerId = m_PlayerController.GetPlayerId();

		foreach (SCR_SpawnPoint spawnpoint : spawnpoints)
		{
			radioSpawnpoint = SCR_RestrictedDeployableSpawnPoint.Cast(spawnpoint);

			if (!radioSpawnpoint || !radioSpawnpoint.IsSpawnPointVisibleForPlayer(playerId))
				continue;

			SCR_SelectionMenuEntry newEntry = m_MapContextualMenu.AddRadialEntry("#AR-DeployableSpawnPoints_MapIconName", m_MainEntry);
			m_mMenuEntries.Set(newEntry, radioSpawnpoint);
			newEntry.SetIconFromDeafaultImageSet("VON_radio");
			newEntry.SetName("#AR-DeployMenu_DisabledSpawnPointReason_OutOfRespawns");
			newEntry.Enable(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshDestinationList()
	{
		SCR_DeployableSpawnPoint radioSpawnpoint;

		for (int i = 0, count = m_mMenuEntries.Count(); i < count; i++)
		{
			radioSpawnpoint = m_mMenuEntries.GetElement(i);

			if (!radioSpawnpoint)
				continue;

			Rpc(RpcAsk_CheckSpawnpointAvailability, FindDestinationId(radioSpawnpoint));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshShownCooldown()
	{
		ChimeraWorld world = GetGame().GetWorld();
		WorldTimestamp timestamp = world.GetServerTimestamp();
		Widget radialRoot = m_MapContextualMenu.GetRadialDisplay().GetRootWidget();
		SCR_SelectionMenuEntry currentEntry = m_MapContextualMenu.GetRadialController().GetRadialMenu().GetSelectionEntry();

		if (!m_fNextTravelAvailableAt || timestamp.GreaterEqual(m_fNextTravelAvailableAt))
		{
			// Cooldown finished, re-enable fast travel
			GetGame().GetCallqueue().Remove(RefreshShownCooldown);
			RefreshDestinationList();
			RefreshMainEntry();
		}
		else
		{
			// Update timer
			m_MainEntry.Enable(false);
			m_MainEntry.SetName(SCR_FormatHelper.GetTimeFormatting(m_fNextTravelAvailableAt.DiffMilliseconds(timestamp) * 0.001, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS));

			if (currentEntry == m_MainEntry)
				m_MainEntry.SetNameTo(TextWidget.Cast(radialRoot.FindAnyWidget("InfoName")));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSpawnpointRemoved(SCR_SpawnPoint spawnpoint)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSpawnpointAdded(SCR_SpawnPoint spawnpoint)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleDestinationSelection(bool enable)
	{
		bool updateCursor = (m_bSelectionInProgress != enable);
		m_bSelectionInProgress = enable;

		if (!updateCursor)
			return;

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();

		if (!mapEntity)
			return;

		SCR_MapCursorModule module = SCR_MapCursorModule.Cast(mapEntity.GetMapModule(SCR_MapCursorModule));

		if (!module)
			return;

		module.ToggleFastTravelDestinationSelection(enable);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] id
	//! \return
	bool IsDestinationReachable(RplId id)
	{
		IEntity destination = GetEntityByDestinationId(id);

		if (!destination)
			return false;

		return !ChimeraWorldUtils.TryGetWaterSurfaceSimple(GetGame().GetWorld(), CalculateDestinationVector(destination));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] destination
	//! \return
	bool IsDestinationReachable(vector destination)
	{
		if (destination == vector.Zero)
			return false;

		return !ChimeraWorldUtils.TryGetWaterSurfaceSimple(GetGame().GetWorld(), CalculateDestinationVector(destination));
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (m_PlayerController.GetPlayerId() != SCR_PlayerController.GetLocalPlayerId())
			return;

		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
	}
}
