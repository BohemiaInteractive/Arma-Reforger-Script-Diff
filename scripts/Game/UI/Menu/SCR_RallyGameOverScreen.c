class SCR_RallyGameOverScreen : SCR_SuperMenuBase
{
	protected const float FINISH_AUDIO_FADE_DURATION = 1.5;
	protected const float ENDSCREEN_MUSIC_DELAY = 3;

	protected const string DIALOG_SCENARIO_EXIT = "scenario_exit";

	protected ref SCR_ConfigurableDialogUi m_ExitDialog;

	protected SCR_RacerData m_Data;
	protected SCR_RaceEntity m_FinishedRace;

	protected float m_fFinishFadeElapsed;
	protected bool m_bApplyFinishSoundFade;

	//------------------------------------------------------------------------------------------------
	override protected void OnMenuOpen()
	{
		super.OnMenuOpen();

		SetupNavigationButtons();
		StartAudioFadeOut();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();

		//restore volumes in case the fade was interrupted (e.g. restart/back pressed early)
		AudioSystem.SetMasterVolume(AudioSystem.SFX, 1);
		AudioSystem.SetMasterVolume(AudioSystem.Dialog, 1);
		AudioSystem.SetMasterVolume(AudioSystem.Music, 1);
		m_bApplyFinishSoundFade = false;

		GetGame().GetCallqueue().Remove(PlayEndscreenMusic);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		HandleAudioFadeOut(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	protected void StartAudioFadeOut()
	{
		m_fFinishFadeElapsed = 0;
		m_bApplyFinishSoundFade = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleAudioFadeOut(float timeSlice)
	{
		if (!m_bApplyFinishSoundFade)
			return;

		m_fFinishFadeElapsed += timeSlice;

		float t;
		if (FINISH_AUDIO_FADE_DURATION > 0)
			t = Math.Clamp(m_fFinishFadeElapsed / FINISH_AUDIO_FADE_DURATION, 0, 1);
		else
			t = 1;

		//ease-in curve so the drop is audible immediately instead of hiding in a slow linear ramp
		float volume = (1 - t) * (1 - t);

		AudioSystem.SetMasterVolume(AudioSystem.SFX, volume);
		AudioSystem.SetMasterVolume(AudioSystem.Dialog, volume);
		AudioSystem.SetMasterVolume(AudioSystem.Music, volume);

		if (t == 1)
			m_bApplyFinishSoundFade = false;
	}

	//------------------------------------------------------------------------------------------------
	void SetData(SCR_RacerData data)
	{
		m_Data = data;
	}

	//------------------------------------------------------------------------------------------------
	void Init()
	{
		HandleTimer();
		HandlePositionText();
		HandleMaxAchievedSpeed();
		HandleAverageSpeed();
		HandleSelectedVehicle();
		HandleWeather();
		HandleDayTime();
		HandleRaceInfo();
		PlayFinishSound();

		GetGame().GetCallqueue().CallLater(PlayEndscreenMusic, ENDSCREEN_MUSIC_DELAY * 1000, false);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasEarnedMedal()
	{
		if (m_Data.GetCheckpointsPassed() < m_FinishedRace.GetRaceCheckpointCount())
			return false;

		float racerTime = m_Data.GetEndTime() - m_Data.GetStartTime();
		const int medalCount = m_FinishedRace.GetMedalCount();
		for (int i = 0; i < medalCount; i++)
		{
			if (racerTime < m_FinishedRace.GetLockedMedalTime(i))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayFinishSound()
	{
		AudioHandle handler;
		if (HasEarnedMedal())
			handler = SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RALLY_ENDSCREEN_MEDAL);
		else
			handler = SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RALLY_ENDSCREEN_NOMEDAL);
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayEndscreenMusic()
	{
		//the finish-line fade drops the Music bus to 0 and leaves it there, so restore it before starting the endscreen track
		AudioSystem.SetMasterVolume(AudioSystem.Music, 1);

		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		MusicManager musicManager = world.GetMusicManager();
		if (!musicManager)
			return;

		if (HasEarnedMedal())
			musicManager.Play(SCR_SoundEvent.SOUND_MUSIC_RALLY_ENDSCREEN_MEDAL);
		else
			musicManager.Play(SCR_SoundEvent.SOUND_MUSIC_RALLY_ENDSCREEN_NO_MEDAL);
	}

	//------------------------------------------------------------------------------------------------
	void SetFinishedRace(SCR_RaceEntity race)
	{
		m_FinishedRace = race;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupNavigationButtons()
	{
		Widget restartButton = GetRootWidget().FindAnyWidget("Restart");
		if (restartButton)
		{
			SCR_InputButtonComponent restartComponent = SCR_InputButtonComponent.Cast(restartButton.FindHandler(SCR_InputButtonComponent));
			if (restartComponent)
				restartComponent.m_OnActivated.Insert(OnRestartRace);
		}

		Widget restartScenarioButton = GetRootWidget().FindAnyWidget("RestartScenario");
		if (restartScenarioButton)
		{
			SCR_InputButtonComponent restartScenarioComponent = SCR_InputButtonComponent.Cast(restartScenarioButton.FindHandler(SCR_InputButtonComponent));
			if (restartScenarioComponent)
				restartScenarioComponent.m_OnActivated.Insert(OnRestartScenario);
		}

		Widget backButton = GetRootWidget().FindAnyWidget("Back");
		if (!backButton)
			return;

		SCR_InputButtonComponent backComponent = SCR_InputButtonComponent.Cast(backButton.FindHandler(SCR_InputButtonComponent));
		if (!backComponent)
			return;

		backComponent.m_OnActivated.Insert(OnBack);
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleTimer()
	{
		RichTextWidget text = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("FinishTimeValue"));
		if (!text)
			return;

		if (m_Data.GetCheckpointsPassed() < m_FinishedRace.GetRaceCheckpointCount())
		{
			text.SetText("#AR-RallyDLC-GameOver_Failed");
			return;
		}

		text.SetTextFormat(SCR_FormatHelper.FormatTimeMSHideZeroes(m_Data.GetEndTime() - m_Data.GetStartTime(), 3));
	}

	//------------------------------------------------------------------------------------------------
	protected void HandlePositionText()
	{
		RichTextWidget text = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("ResultMedal"));
		if (!text)
			return;

		//decide which medal (if any) was won
		if (m_Data.GetCheckpointsPassed() < m_FinishedRace.GetRaceCheckpointCount())
		{
			text.SetText("#AR-RallyDLC-GameOver_Failed");
			return;
		}

		float racerTime = m_Data.GetEndTime() - m_Data.GetStartTime();
		string positionText = "#AR-RallyDLC-GameOver_NoMedal";
		float medalTime;
		const int medalCount = m_FinishedRace.GetMedalCount();
		for (int i = 0; i < medalCount; i++)
		{
			medalTime = m_FinishedRace.GetLockedMedalTime(i);
			Print(" " + racerTime + " < " + medalTime, LogLevel.NORMAL);
			if (racerTime < medalTime)
			{
				positionText = m_FinishedRace.GetMedalText(i);
				break;
			}
		}

		text.SetText(positionText);
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleMaxAchievedSpeed()
	{
		RichTextWidget text = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("MaxSpeedValue"));
		if (!text)
			return;

		float speed = m_Data.GetMaxAchievedSpeed();
		speed *= Physics.MS2KMH;

		text.SetTextFormat("%1 #AR-VehicleInfo_KMH", Math.Round(speed));
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleAverageSpeed()
	{
		RichTextWidget text = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("AverageSpeedValue"));
		if (!text)
			return;

		float speed = m_Data.GetAverageSpeed();
		speed *= Physics.MS2KMH;

		text.SetTextFormat("%1 #AR-VehicleInfo_KMH", Math.Round(speed));
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleSelectedVehicle()
	{
		RichTextWidget text = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("VehicleValue"));
		if (!text)
			return;

		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return;

		SCR_RaceCustomizationPointEntity point = settings.GetCustomizationEntity();
		if (!point)
			return;

		SCR_RallyCustomizationData customizationData = point.GetCustomizationData();
		if (!customizationData)
			return;

		text.SetText(customizationData.GetVehicleName());
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleWeather()
	{
		RichTextWidget text = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("WeatherValue"));
		if (!text)
			return;

		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return;

		SCR_RaceCustomizationPointEntity point = settings.GetCustomizationEntity();
		if (!point)
			return;

		SCR_RallyCustomizationData customizationData = point.GetCustomizationData();
		if (!customizationData)
			return;
		
		string weather;
		switch (customizationData.GetWeather())
		{
			case "Clear":
				weather = "#AR-Weather_Clear";
				break;

			case "Cloudy":
				weather = "#AR-Weather_Cloudy";
				break;

			case "Overcast":
				weather = "#AR-Weather_Overcast";
				break;

			case "Rainy":
				weather = "#AR-Weather_Rain";
				break;
		}
		
		text.SetText(weather);
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleDayTime()
	{
		RichTextWidget text = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("DayTimeValue"));
		if (!text)
			return;

		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return;

		SCR_RaceCustomizationPointEntity point = settings.GetCustomizationEntity();
		if (!point)
			return;

		SCR_RallyCustomizationData customizationData = point.GetCustomizationData();
		if (!customizationData)
			return;

		text.SetText(customizationData.GetStartingTime());
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleRaceInfo()
	{
		if (!m_FinishedRace)
			return;

		RichTextWidget raceName = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("RaceName"));
		if (!raceName)
			return;

		raceName.SetText(m_FinishedRace.GetRaceName());

		RichTextWidget raceDescription = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("RaceDescription"));
		if (!raceDescription)
			return;

		raceDescription.SetText(m_FinishedRace.GetRaceDescription());

		ImageWidget raceImage = ImageWidget.Cast(GetRootWidget().FindAnyWidget("RaceImage"));
		if (!raceImage)
			return;

		raceImage.LoadImageTexture(0, m_FinishedRace.GetRaceImage());

		RichTextWidget time1, time2, time3;

		time1 = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Time1"));
		time2 = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Time2"));
		time3 = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Time3"));

		if (!time1 || !time2 || !time3)
			return;

		if (m_FinishedRace.GetMedalCount() < 3)
		{
			Print("SCR_RallyGameOverScreen:: Not enough track records set!", LogLevel.WARNING);
			return;
		}

		//medals this run was judged by (locked at its start)
		time1.SetText(SCR_FormatHelper.FormatTimeMSHideZeroes(m_FinishedRace.GetLockedMedalTime(0), 3));
		time2.SetText(SCR_FormatHelper.FormatTimeMSHideZeroes(m_FinishedRace.GetLockedMedalTime(1), 3));
		time3.SetText(SCR_FormatHelper.FormatTimeMSHideZeroes(m_FinishedRace.GetLockedMedalTime(2), 3));

		RichTextWidget checkpointCount = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("CheckpointsCount"));
		if (!checkpointCount)
			return;

		checkpointCount.SetTextFormat("#AR-RallyDLC-RaceCheckpointsCount", m_FinishedRace.GetRaceCheckpointCount());
	}

	//------------------------------------------------------------------------------------------------
	//! Quick restart: puts the racer (alive, unconscious or dead) back at the start line of the same race
	protected void OnRestartRace(SCR_InputButtonComponent comp, string action)
	{
		SCR_RaceEntity race = m_FinishedRace;
		if (!race)
			race = SCR_RaceEntity.GetActiveRace();

		if (!race)
			return;

		// the end screen track would keep playing over the restarted race
		MusicManager musicManager;
		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			musicManager = world.GetMusicManager();

		if (musicManager)
		{
			musicManager.Stop(SCR_SoundEvent.SOUND_MUSIC_RALLY_ENDSCREEN_MEDAL);
			musicManager.Stop(SCR_SoundEvent.SOUND_MUSIC_RALLY_ENDSCREEN_NO_MEDAL);
		}

		// OnMenuClose restores the volumes and cancels the delayed end screen music
		Close();
		race.RequestRestart(SCR_PlayerController.GetLocalPlayerId());
	}

	//------------------------------------------------------------------------------------------------
	//! Full restart: reloads the whole scenario
	protected void OnRestartScenario(SCR_InputButtonComponent comp, string action)
	{
		GetGame().GetMenuManager().CloseAllMenus();
		GameStateTransitions.RequestScenarioRestart();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBack(SCR_InputButtonComponent comp, string action)
	{
		if (m_ExitDialog)
			return;

		m_ExitDialog = SCR_CommonDialogs.CreateDialog(DIALOG_SCENARIO_EXIT);
		if (!m_ExitDialog)
		{
			OnExitConfirm();
			return;
		}

		m_ExitDialog.m_OnConfirm.Insert(OnExitConfirm);
		m_ExitDialog.m_OnCancel.Insert(OnExitCancel);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnExitConfirm()
	{
		if (m_ExitDialog)
		{
			m_ExitDialog.m_OnConfirm.Remove(OnExitConfirm);
			m_ExitDialog.m_OnCancel.Remove(OnExitCancel);
			m_ExitDialog = null;
		}

		Close();
		GameStateTransitions.RequestGameplayEndTransition();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnExitCancel()
	{
		if (m_ExitDialog)
		{
			m_ExitDialog.m_OnConfirm.Remove(OnExitConfirm);
			m_ExitDialog.m_OnCancel.Remove(OnExitCancel);
			m_ExitDialog = null;
		}
	}

}
