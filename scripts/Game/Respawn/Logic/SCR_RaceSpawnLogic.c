//! Object responsible for handling respawn logic on the authority side.
[BaseContainerProps(category: "Respawn")]
class SCR_RaceSpawnLogic : SCR_SpawnLogic
{
	[Attribute("", uiwidget: UIWidgets.EditBox, category: "Respawn", desc: "Default faction for players to spawn with or empty if none.")]
	protected FactionKey m_sForcedFaction;

	[Attribute("", uiwidget: UIWidgets.EditBox, category: "Respawn", desc: "Default loadout for players to spawn with or empty if none")]
	protected string m_sForcedLoadout;

	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditSuccess_S(int playerId)
	{
		super.OnPlayerAuditSuccess_S(playerId);
		ExcuteInitialLoadOrSpawn_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned_S(int playerId, IEntity entity)
	{
		super.OnPlayerSpawned_S(playerId, entity);

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(entity.FindComponent(CharacterControllerComponent));
		if (!controller)
			return;

		controller.m_OnLifeStateChanged.Insert(OnPlayerLifeStateChanged_S);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerEntityCleanup_S(notnull IEntity playerEntity)
	{
		super.OnPlayerEntityCleanup_S(playerEntity);

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(playerEntity.FindComponent(CharacterControllerComponent));
		if (!controller)
			return;

		controller.m_OnLifeStateChanged.Remove(OnPlayerLifeStateChanged_S);
	}

	//------------------------------------------------------------------------------------------------
	//! Shows the same end-of-race screen as on death when the player becomes unconscious instead.
	protected void OnPlayerLifeStateChanged_S(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState, bool isJIP, SCR_CharacterControllerComponent controller)
	{
		if (newLifeState != ECharacterLifeState.INCAPACITATED || previousLifeState == ECharacterLifeState.INCAPACITATED)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(controller.GetOwner());
		if (playerId <= 0)
			return;

		OnPlayerEntityLost_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerEntityLost_S(int playerId)
	{
		super.OnPlayerEntityLost_S(playerId);

		PrintFormat("[RallyDLC] OnPlayerEntityLost_S: playerId=%1", playerId, level: LogLevel.NORMAL);

		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
		{
			Print("[RallyDLC] OnPlayerEntityLost_S: aborted, no SCR_AdditionalGameModeSettingsComponent instance", LogLevel.WARNING);
			return;
		}

		SCR_RaceCustomizationPointEntity point = settings.GetCustomizationEntity();
		if (!point)
		{
			Print("[RallyDLC] OnPlayerEntityLost_S: aborted, no customization entity set on game mode settings", LogLevel.WARNING);
			return;
		}

		string raceName = point.GetAssignedRace();
		SCR_RaceEntity race = SCR_RaceEntity.Cast(GetGame().GetWorld().FindEntityByName(raceName));
		if (!race)
		{
			PrintFormat("[RallyDLC] OnPlayerEntityLost_S: aborted, could not find race entity named '%1'", raceName, level: LogLevel.WARNING);
			return;
		}

		race.SetRaceRunning(false);
		race.ShowRaceInfoDisplay(false);

		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		PrintFormat("[RallyDLC] OnPlayerEntityLost_S: localPlayerId=%1", localPlayerId, level: LogLevel.NORMAL);

		SCR_RacerData data = race.GetRacerData(localPlayerId);
		if (!data)
		{
			PrintFormat("[RallyDLC] OnPlayerEntityLost_S: aborted, no SCR_RacerData found for localPlayerId=%1 (playerId=%2)", localPlayerId, playerId, level: LogLevel.WARNING);
			return;
		}

		//stop the timer at the moment of death, same as reaching the finish checkpoint
		if (data.GetEndTime() == 0)
		{
			ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
			TimeAndWeatherManagerEntity timeAndWeatherManager;
			if (world)
				timeAndWeatherManager = world.GetTimeAndWeatherManager();

			if (timeAndWeatherManager)
				data.SetEndTime(timeAndWeatherManager.GetEngineTime());
		}

		SCR_RallyGameOverScreen screen = SCR_RallyGameOverScreen.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.RallyDLC_GameOverScreen, 0, true, false));
		if (!screen)
		{
			Print("[RallyDLC] OnPlayerEntityLost_S: aborted, OpenMenu(RallyDLC_GameOverScreen) returned null/wrong type", LogLevel.WARNING);
			return;
		}

		Print("[RallyDLC] OnPlayerEntityLost_S: opening end screen and initializing", LogLevel.NORMAL);

		screen.SetData(data);
		screen.SetFinishedRace(race);
		screen.Init();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnFailed_S(int playerId)
	{
		super.OnPlayerSpawnFailed_S(playerId);

		const int delay = Math.RandomFloat(900, 1100);
		GetGame().GetCallqueue().CallLater(DoSpawn_S, delay, false, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DoSpawn_S(int playerId)
	{
		array<Faction> factions = {};
		GetGame().GetFactionManager().GetFactionsList(factions);

		Faction targetFaction;
		if (!GetForcedFaction(targetFaction))
			targetFaction = factions.GetRandomElement();

		GetPlayerFactionComponent_S(playerId).RequestFaction(targetFaction);

		SCR_BasePlayerLoadout targetLoadout;
		if (!GetForcedLoadout(targetLoadout))
			targetLoadout = GetGame().GetLoadoutManager().GetRandomFactionLoadout(targetFaction);

		GetPlayerLoadoutComponent_S(playerId).RequestLoadout(targetLoadout);

		Faction faction = GetPlayerFactionComponent_S(playerId).GetAffiliatedFaction();
		if (!faction)
		{
			OnPlayerSpawnFailed_S(playerId);
			return;
		}

		SCR_BasePlayerLoadout loadout = GetPlayerLoadoutComponent_S(playerId).GetLoadout();
		if (!loadout)
		{
			OnPlayerSpawnFailed_S(playerId);
			return;
		}

		SCR_SpawnPoint point = SCR_SpawnPoint.GetRandomSpawnPointForFaction(faction.GetFactionKey());
		if (!point)
		{
			OnPlayerSpawnFailed_S(playerId);
			return;
		}

		SCR_SpawnPointSpawnData data = new SCR_SpawnPointSpawnData(loadout.GetLoadoutResource(), point.GetRplId());
		if (!GetPlayerRespawnComponent_S(playerId).CanSpawn(data))
		{
			OnPlayerSpawnFailed_S(playerId);
			return;
		}

		GetPlayerRespawnComponent_S(playerId).RequestSpawn(data);
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetForcedFaction(out Faction faction)
	{
		if (m_sForcedFaction.IsEmpty() &&
			!System.GetCLIParam("autodeployFaction", m_sForcedFaction) &&
			!System.GetCLIParam("tdmf", m_sForcedFaction))
		{
			return false;
		}

		faction = GetGame().GetFactionManager().GetFactionByKey(m_sForcedFaction);
		if (!faction)
		{
			PrintFormat("Auto spawn logic did not find faction by name: %1", m_sForcedFaction, level: LogLevel.WARNING);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetForcedLoadout(out SCR_BasePlayerLoadout loadout)
	{
		if (m_sForcedLoadout.IsEmpty())
			return false;

		loadout = GetGame().GetLoadoutManager().GetLoadoutByName(m_sForcedLoadout, m_sForcedFaction);
		if (!loadout)
		{
			PrintFormat("Auto spawn logic did not find loadout by name: %1", m_sForcedLoadout, level: LogLevel.WARNING);
			return false;
		}

		return true;
	}
}
