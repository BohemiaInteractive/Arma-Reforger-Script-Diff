class SCR_RaceStartCheckpointClass : SCR_RaceCheckpointClass
{
}

class SCR_RaceStartCheckpoint : SCR_RaceCheckpoint
{
	//------------------------------------------------------------------------------------------------
	override void OnCheckpointReached()
	{
		super.OnCheckpointReached();
		DisableCheckpoint();

		SCR_RaceEntity race = GetRaceEntity();
		if (!race)
		{
			Print("SCR_RaceCheckpoint:: Checkpoint has no parent SCR_RaceEntity!", LogLevel.WARNING);
			return;
		}

		SCR_RaceCheckpoint nextCheckpoint = race.FindNextCheckpoint(this);
		if (!nextCheckpoint)
			return;

		nextCheckpoint.EnableCheckpoint();

		//change this to get a player who actually hit the start to start his own timer for MP
		SCR_RacerData data = race.GetRacerData(SCR_PlayerController.GetLocalPlayerId());
		if (!data)
			return;

		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity timeAndWeatherManager = world.GetTimeAndWeatherManager();
		if (!timeAndWeatherManager)
			return;

		//lock medal times once per run (circuits pass the start every lap), before a new best can be saved at the finish
		if (data.GetStartTime() == 0)
		{
			data.SetStartTime(timeAndWeatherManager.GetEngineTime());
			race.LockMedalTimes();
		}

		if (race.IsCircuit())
		{
			if (race.GetMaxLaps() > data.GetLapsDone())
				data.IncreaseLapsDone();
			else
			{
				nextCheckpoint.DisableCheckpoint();
				data.SetEndTime(timeAndWeatherManager.GetEngineTime());
			}
		}

		race.SetRaceRunning(true);

		//ghosts are single-player only; StartPlayingGhost() is itself a no-op in MP, but skip spawning
		//the ghost vehicle entirely too rather than spawn one with nothing to play back
		bool showGhost = true;
		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (settings)
		{
			SCR_RaceCustomizationPointEntity customizationPoint = settings.GetCustomizationEntity();
			if (customizationPoint)
			{
				SCR_RallyCustomizationData customizationData = customizationPoint.GetCustomizationData();
				if (customizationData)
					showGhost = customizationData.GetShowGhost();
			}
		}

		if (RplSession.Mode() == RplMode.None && showGhost)
		{
			race.StartPlayingGhost(data);
			race.SpawnGhost(data.GetVehicle());
		}

		race.RaceStarted(data.GetPlayerID());

		SCR_RaceSystem system = SCR_RaceSystem.Cast(world.FindSystem(SCR_RaceSystem));
		if (!system)
			return;

		if (!system.GetCurrentRace())
			system.SetCurrentRace(race);
	}

	//------------------------------------------------------------------------------------------------
	override int GetCheckpointID()
	{
		return SCR_RaceEntity.RACE_CHECKPOINT_START_ID;
	}
}
