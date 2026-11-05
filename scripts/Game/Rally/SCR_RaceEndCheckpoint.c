class SCR_RaceEndCheckpointClass : SCR_RaceCheckpointClass
{
}

class SCR_RaceEndCheckpoint : SCR_RaceCheckpoint
{
	[Attribute(uiwidget: UIWidgets.None)] // unused - remove?
	protected bool m_bCircleRace;

	//------------------------------------------------------------------------------------------------
	override protected void OnCheckpointReached()
	{
		super.OnCheckpointReached();

		DisableCheckpoint();

		SCR_RaceEntity race = GetRaceEntity();
		if (!race)
		{
			Print("SCR_RaceCheckpoint:: Checkpoint has no parent SCR_RaceEntity!", LogLevel.WARNING);
			return;
		}

		SCR_RacerData data = race.GetRacerData(SCR_PlayerController.GetLocalPlayerId());
		if (!data)
			return;

		ArmaReforgerScripted game = GetGame();
		ChimeraWorld world = ChimeraWorld.CastFrom(game.GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity timeAndWeatherManager = world.GetTimeAndWeatherManager();
		if (!timeAndWeatherManager)
			return;

		data.SetEndTime(timeAndWeatherManager.GetEngineTime());
		race.SetRaceRunning(false);
		data.FinishWritingGhostData();
		race.ShowRaceInfoDisplay(false);
		race.RaceFinished();

		SCR_RallyGameOverScreen screen = SCR_RallyGameOverScreen.Cast(game.GetMenuManager().OpenMenu(ChimeraMenuPreset.RallyDLC_GameOverScreen, 0, true, false));
		if (!screen)
			return;

		screen.SetData(data);
		screen.SetFinishedRace(race);
		screen.Init();

		//has to happen later otherwise held input will override it
		game.GetCallqueue().CallLater(BrakeVehicle, 100, false, data);
	}

	//------------------------------------------------------------------------------------------------
	protected void BrakeVehicle(SCR_RacerData data)
	{
		CarControllerComponent controller = CarControllerComponent.Cast(data.GetVehicle().GetVehicleController());
		if (controller)
			controller.SetPersistentHandBrake(true);
	}

	//------------------------------------------------------------------------------------------------
	override int GetCheckpointID()
	{
		return SCR_RaceEntity.RACE_CHECKPOINT_END_ID;
	}
}
