class SCR_RallyRaceCustomizationSubmenu : SCR_SubMenuBase
{
	protected SCR_RaceCustomizationPointEntity m_CustomizationPoint;
	protected TimeAndWeatherManagerEntity m_TimeWeatherManager;

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return;

		m_CustomizationPoint = settings.GetCustomizationEntity();
		if (!m_CustomizationPoint)
			return;

		ChimeraWorld world = GetGame().GetWorld();

		m_TimeWeatherManager = world.GetTimeAndWeatherManager();
		if (!m_TimeWeatherManager)
			return;

		Widget widget = m_wRoot.FindAnyWidget("Weather");
		if (!widget)
			return;

		SCR_SpinBoxComponent component = SCR_SpinBoxComponent.Cast(widget.FindHandler(SCR_SpinBoxComponent));
		if (!component)
			return;

		component.m_OnChanged.Insert(OnWeatherChanged);

		int weatherIndex = m_TimeWeatherManager.GetCurrentWeatherState().GetStateID();
		component.SetCurrentItem(weatherIndex);
		OnWeatherChanged(component, weatherIndex);

		widget = m_wRoot.FindAnyWidget("Time");
		if (!widget)
			return;

		SCR_SliderComponent sliderComp = SCR_SliderComponent.Cast(widget.FindHandler(SCR_SliderComponent));
		if (!sliderComp)
			return;

		sliderComp.m_OnChanged.Insert(OnTimeChanged);

		TimeContainer currentTime = m_TimeWeatherManager.GetTime();

		sliderComp.SetValue(currentTime.m_iHours);
		OnTimeChanged(sliderComp, currentTime.m_iHours);

		widget = m_wRoot.FindAnyWidget("Ghost");
		if (!widget)
			return;

		SCR_SpinBoxComponent ghostComponent = SCR_SpinBoxComponent.Cast(widget.FindHandler(SCR_SpinBoxComponent));
		if (!ghostComponent)
			return;

		ghostComponent.m_OnChanged.Insert(OnGhostChanged);

		bool showGhost = true;
		SCR_RallyCustomizationData customizationData = m_CustomizationPoint.GetCustomizationData();
		if (customizationData)
			showGhost = customizationData.GetShowGhost();

		ghostComponent.SetCurrentItem(!showGhost);
		OnGhostChanged(ghostComponent, !showGhost);

		SetupRaceInfo();

		super.OnTabCreate(menuRoot, buttonsLayout, index);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupRaceInfo()
	{
		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return;

		SCR_RaceCustomizationPointEntity point = settings.GetCustomizationEntity();
		if (!point)
			return;

		SCR_RaceEntity race = SCR_RaceEntity.Cast(GetGame().GetWorld().FindEntityByName(point.GetAssignedRace()));
		if (!race)
			return;

		RichTextWidget raceName = RichTextWidget.Cast(m_wRoot.FindAnyWidget("RaceName"));
		if (!raceName)
			return;

		raceName.SetText(race.GetRaceName());

		RichTextWidget raceDescription = RichTextWidget.Cast(m_wRoot.FindAnyWidget("RaceDescription"));
		if (!raceDescription)
			return;

		raceDescription.SetText(race.GetRaceDescription());

		ImageWidget raceImage = ImageWidget.Cast(m_wRoot.FindAnyWidget("RaceImage"));
		if (!raceImage)
			return;

		raceImage.LoadImageTexture(0, race.GetRaceImage());

		int imageWidth, imageHeight;
		raceImage.GetImageSize(0, imageWidth, imageHeight);
		raceImage.SetSize(imageWidth, imageHeight);

		RichTextWidget time1, time2, time3;

		time1 = RichTextWidget.Cast(m_wRoot.FindAnyWidget("Time1"));
		time2 = RichTextWidget.Cast(m_wRoot.FindAnyWidget("Time2"));
		time3 = RichTextWidget.Cast(m_wRoot.FindAnyWidget("Time3"));

		if (!time1 || !time2 || !time3)
			return;

		if (race.GetMedalCount() < 3)
		{
			Print("SCR_RallyCustomizationMenu:: Not enough track records set!", LogLevel.WARNING);
			return;
		}

		//medals for the next run, based on the ghost the player will race against
		const float ghostFinishTime = race.GetGhostFinishTime();
		time1.SetText(SCR_FormatHelper.FormatTimeMSHideZeroes(race.GetMedalTimeForGhost(0, ghostFinishTime), 2));
		time2.SetText(SCR_FormatHelper.FormatTimeMSHideZeroes(race.GetMedalTimeForGhost(1, ghostFinishTime), 2));
		time3.SetText(SCR_FormatHelper.FormatTimeMSHideZeroes(race.GetMedalTimeForGhost(2, ghostFinishTime), 2));

		RichTextWidget checkpointCount = RichTextWidget.Cast(m_wRoot.FindAnyWidget("CheckpointsCount"));
		if (!checkpointCount)
			return;

		checkpointCount.SetTextFormat("#AR-RallyDLC-RaceCheckpointsCount", race.GetRaceCheckpointCount());

	}

	//------------------------------------------------------------------------------------------------
	protected void OnWeatherChanged(SCR_SpinBoxComponent comp, int index)
	{
		string weather;
		switch (index)
		{
			case 0:
				weather = "Clear";
				break;

			case 1:
				weather = "Cloudy";
				break;

			case 2:
				weather = "Overcast";
				break;

			case 3:
				weather = "Rainy";
				break;
		}

		m_TimeWeatherManager.ForceWeatherTo(false, weather);

		SCR_RallyCustomizationData customizationData = m_CustomizationPoint.GetCustomizationData();
		if (customizationData)
			customizationData.SetWeather(weather);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTimeChanged(SCR_SliderComponent comp, float value)
	{
		int hours = Math.Round(value);

		TimeContainer time = new TimeContainer(hours, 0, 0);
		m_TimeWeatherManager.SetTime(time);
		SCR_TimeWindowVisibilityManager.s_OnGameTimeChanged.Invoke();

		SCR_RallyCustomizationData customizationData = m_CustomizationPoint.GetCustomizationData();
		if (customizationData)
			customizationData.SetStartingTime(string.Format("%1:00", hours));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGhostChanged(SCR_SpinBoxComponent comp, int index)
	{
		bool showGhost = index == 0;

		SCR_RallyCustomizationData customizationData = m_CustomizationPoint.GetCustomizationData();
		if (customizationData)
			customizationData.SetShowGhost(showGhost);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTabShow()
	{
		m_CustomizationPoint.MoveCamera(SCR_ERallyCustomizationCameraPosition.RACE);

		Widget weatherSpinbox = m_wRoot.FindAnyWidget("Weather");
		if (weatherSpinbox)
			GetGame().GetCallqueue().CallLater(GetGame().GetWorkspace().SetFocusedWidget, 0, false, weatherSpinbox, false);

		super.OnTabShow();
	}
}
