class SCR_RaceTimeInfo : SCR_InfoDisplayExtended
{
	protected ref SCR_RaceTimerWidgets m_Widgets;

	protected TimeAndWeatherManagerEntity m_TimeWeatherManager;
	protected ChimeraWorld m_World;

	protected SCR_RaceEntity m_Race;
	protected SCR_RacerData m_Data;

	//------------------------------------------------------------------------------------------------
	override void Show(bool show, float speed = UIConstants.FADE_RATE_INSTANT, EAnimationCurve curve = EAnimationCurve.LINEAR)
	{
		super.Show(show, speed, curve);

		CheckpointsUpdated(null)
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);

		if (!m_wRoot)
			return;

		m_Widgets = new SCR_RaceTimerWidgets();
		m_Widgets.Init(m_wRoot);

		SetContentVisibility(false);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		super.DisplayStopDraw(owner);

		m_wRoot = null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayUpdate(IEntity owner, float timeSlice)
	{
		super.DisplayUpdate(owner, timeSlice);

		if (!m_bShown)
			return;

		if (!m_Data && !FetchRacerData())
			return;

		if (m_Data.GetStartTime() == 0)
		{
			m_Widgets.m_wTimer.SetText(SCR_FormatHelper.FormatTimeMSHideZeroes(0, 2));
			return;
		}

		float raceTime;
		if (m_Data.GetEndTime() == 0)
			raceTime = m_TimeWeatherManager.GetEngineTime() - m_Data.GetStartTime();
		else
			raceTime = m_Data.GetEndTime() - m_Data.GetStartTime();

		m_Widgets.m_wTimer.SetText(SCR_FormatHelper.FormatTimeMSHideZeroes(raceTime, 2));
	}

	//------------------------------------------------------------------------------------------------
	protected bool FetchWorldData()
	{
		m_World = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!m_World)
			return false;

		m_TimeWeatherManager = m_World.GetTimeAndWeatherManager();
		if (!m_TimeWeatherManager)
			return false;

		if (m_Race)
			return true;

		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return false;

		SCR_RaceCustomizationPointEntity point = settings.GetCustomizationEntity();
		if (!point)
			return false;

		m_Race = SCR_RaceEntity.Cast(m_World.FindEntityByName(point.GetAssignedRace()));
		return m_Race != null;
	}

	//------------------------------------------------------------------------------------------------
	protected bool FetchRacerData()
	{
		if (!m_Race && !FetchWorldData())
			return false;

		m_Data = m_Race.GetRacerData(SCR_PlayerController.GetLocalPlayerId());
		if (!m_Data)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckpointsUpdated(IEntity checkpoint)
	{
		if (!m_Data && !FetchRacerData())
			return;

		if (!m_Race.IsCircuit())
		{
			m_Widgets.m_wCheckpoints.SetTextFormat("#AR-Campaign_BaseRespawnsAmount", m_Data.GetCheckpointsPassed(), m_Race.GetRaceCheckpointCount());
			m_Widgets.m_wTitleCheckpoints.SetTextFormat("#AR-RallyDLC-RaceCheckpoints");
		}
		else
		{
			m_Widgets.m_wCheckpoints.SetTextFormat("#AR-Campaign_BaseRespawnsAmount", m_Data.GetLapsDone(), m_Race.GetMaxLaps());
			m_Widgets.m_wTitleCheckpoints.SetTextFormat("#AR-RallyDLC-HUD_Laps");
		}
		
		SCR_PopUpNotification popup = SCR_PopUpNotification.GetInstance();
		if (!popup)
			return;
		
		popup.HideCurrentMsg();

		if (checkpoint)
			m_Widgets.m_wCheckpoints.SetColor(Color.Green);
		
		GetGame().GetCallqueue().CallLater(ResetCheckpointsColor, 500);
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetCheckpointsColor()
	{
		m_Widgets.m_wCheckpoints.SetColor(Color.White);
	}

	//------------------------------------------------------------------------------------------------
	void SetContentVisibility(bool isVisible)
	{
		if (!m_Widgets || !m_Widgets.m_wSizeLayoutContent)
			return;

		m_Widgets.m_wSizeLayoutContent.SetVisible(isVisible);

		CheckpointsUpdated(null);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCheckpointMissed(IEntity checkpoint)
	{
		SCR_PopUpNotification popup = SCR_PopUpNotification.GetInstance();
		if (!popup)
			return;
		
		popup.PopupMsg("#AR-RallyDLC-HUD_MissedCheckpoint", -1);
		
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_RaceTimeInfo()
	{
		if (!m_Race && !FetchWorldData())
			return;

		m_Race.GetOnCheckpointsUpdated().Insert(CheckpointsUpdated);
		m_Race.GetOnCheckpointMissed().Insert(OnCheckpointMissed);
	}
}
