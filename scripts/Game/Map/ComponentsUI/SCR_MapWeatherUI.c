class SCR_MapWeatherUI : SCR_MapUIBaseComponent
{
	[Attribute(defvalue: "1", desc: "Wind Speed Precision[m/s]", params: "0.01 inf")]
	protected float m_fWindSpeedPrecision;

	[Attribute(defvalue: "5", desc: "Wind Direction Precision[°]", params: "1 inf")]
	protected int m_iWindDirectionPrecision;

	[Attribute(defvalue: "60", desc: "Wind Data Update Cooldown[s]", params: "1 inf")]
	protected int m_iDataUpdateCooldown;

	[Attribute(defvalue: "40 0 0", desc: "Position offset of the layout from the position of the button[px]", params: "0 inf")]
	protected vector m_vPositionOffset;

	[Attribute("MapWeatherFrame", desc: "Map Weather root widget name")]
	protected string m_sMapWeatherFrame;

	protected const ResourceName ICON = "{857DD01860810AE9}UI/Textures/Editor/Attributes/Categories/Attribute_Category_Weather.edds";
	protected const ResourceName LAYOUT = "{E360A5329459869F}UI/layouts/Menus/DeployMenu/MapWeatherData.layout";
	protected const string LABEL_DIRECTION = "#AR-MapInfo_WeatherData_WindDirection";
	protected const string LABEL_SPEED = "#AR-MapInfo_WeatherData_WindSpeed";
	protected const string WIDGET_NAME_ICON = "StateIcon";
	protected const string WIDGET_NAME_TITLE = "StateTitle";
	protected const string WIDGET_NAME_TIME = "Time";
	protected const string FORMAT_HOUR = "%1:00";

	protected bool m_bVisible;
	protected bool m_bLastAutomaticWindState;
	protected float m_fLastWindSpeed;
	protected float m_fLastWindDirection;
	protected Widget m_wMapWeatherFrame;
	protected Widget m_wWeatherDataRoot;
	protected Widget m_wWindOverrideInfo;
	protected Widget m_wWeatherOverrideInfo;
	protected Widget m_wCurrentWeatherHolder;
	protected Widget m_wUpcomingWeatherHolder;
	protected TextWidget m_wCurrentWindSpeed;
	protected TextWidget m_wCurrentWindDirection;
	protected ChimeraWorld m_World;
	protected WorldTimestamp m_NextWindUpdateCooldown;
	protected SCR_MapToolEntry m_ToolMenuEntry;
	protected TimeAndWeatherManagerEntity m_WeatherManager;

	//------------------------------------------------------------------------------------------------
	protected void ToggleVisibility(SCR_ButtonBaseComponent button = null)
	{
		SetVsibilityState(!m_bVisible);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetVsibilityState(bool state)
	{
		if (!m_wWeatherDataRoot && !CreateLayout())
			return;

		m_bVisible = state;
		m_ToolMenuEntry.SetActive(m_bVisible);
		m_wWeatherDataRoot.SetVisible(m_bVisible);

		// if it is meant to be visible, then reposition the layout so it is next to the button, but not outside of the window
		if (m_bVisible)
			UpdateLayoutPosition();
	}

	//------------------------------------------------------------------------------------------------
	//! Changes the Y position of the layout to be next to the weather button
	protected void UpdateLayoutPosition()
	{
		if (!m_ToolMenuEntry || !m_ToolMenuEntry.m_ButtonComp)
			return;

		Widget toolButton = m_ToolMenuEntry.m_ButtonComp.GetRootWidget();
		if (!toolButton)
			return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		float buttonPosX, buttonPosY, buttonSizeX, buttonSizeY;
		toolButton.GetScreenPos(buttonPosX, buttonPosY);
		toolButton.GetScreenSize(buttonSizeX, buttonSizeY);
		
		FrameSlot.SetPosY(m_wWeatherDataRoot, workspace.DPIUnscale(buttonPosY - buttonSizeY + m_vPositionOffset[1]));
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		if (!m_bVisible)
			return;

		if (!m_World)
			return;

#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_UI_MAP_FORECAST_COOLDOWN))
		{
			UpdateWeatherData();
			UpdateWindData();
			return;
		}
#endif

		WorldTimestamp currentTS = m_World.GetServerTimestamp();
		if (m_NextWindUpdateCooldown && currentTS.Less(m_NextWindUpdateCooldown))
			return;

		m_NextWindUpdateCooldown = currentTS.PlusSeconds(m_iDataUpdateCooldown);
		UpdateWindData();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates the information about current wind conditions
	protected void UpdateWindData()
	{
		if (!m_wWindOverrideInfo || !m_wCurrentWindSpeed || !m_wCurrentWindDirection)
			return;

		m_bLastAutomaticWindState = m_WeatherManager.IsAutomatedWindDisabled();
		m_wWindOverrideInfo.SetVisible(m_bLastAutomaticWindState);

		m_fLastWindSpeed = Math.Round(m_WeatherManager.GetWindSpeed() / m_fWindSpeedPrecision) * m_fWindSpeedPrecision;
		m_fLastWindDirection = Math.Repeat(Math.Round(m_WeatherManager.GetWindDirection() / m_iWindDirectionPrecision) * m_iWindDirectionPrecision + 180, 360);
		m_wCurrentWindSpeed.SetTextFormat(LABEL_SPEED, m_fLastWindSpeed);
		m_wCurrentWindDirection.SetTextFormat(LABEL_DIRECTION, m_fLastWindDirection);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWeatherChanged(notnull TimeAndWeatherManagerEntity manager, int currentStateId, int nextStateId, bool transitioning)
	{
		if (transitioning)
			return;

		UpdateWeatherData();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWindOverrideDataChanged(notnull TimeAndWeatherManagerEntity manager, bool currentState, float currentSpeed, float currentDirection)
	{
		UpdateWindData();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates the information about current and upcoming weather
	void UpdateWeatherData()
	{
		if (!m_wWeatherOverrideInfo || !m_wCurrentWeatherHolder || !m_wUpcomingWeatherHolder)
			return;

		bool isAutomaticWeatherDisabled = m_WeatherManager.IsWeatherLooping();
		m_wWeatherOverrideInfo.SetVisible(isAutomaticWeatherDisabled);
		m_wUpcomingWeatherHolder.SetVisible(!isAutomaticWeatherDisabled);

		WeatherState state = m_WeatherManager.GetCurrentWeatherState();
		if (!state)
			return;

		ImageWidget stateIcon = ImageWidget.Cast(m_wCurrentWeatherHolder.FindAnyWidget(WIDGET_NAME_ICON));
		if (stateIcon)
			stateIcon.LoadImageTexture(0, state.GetIconPath());

		TextWidget stateTitle = TextWidget.Cast(m_wCurrentWeatherHolder.FindAnyWidget(WIDGET_NAME_TITLE));
		if (stateTitle)
			stateTitle.SetText(state.GetLocalizedName());

		if (isAutomaticWeatherDisabled)
			return;

		BaseWeatherStateTransitionManager transitionMgr = m_WeatherManager.GetTransitionManager();
		if (!transitionMgr)
			return;

		state = transitionMgr.GetNextState();
		if (!state)
			return;

		TextWidget time = TextWidget.Cast(m_wUpcomingWeatherHolder.FindAnyWidget(WIDGET_NAME_TIME));
		if (time)
		{
			int currentHour, hour, currentMin, min, sec;
			m_WeatherManager.GetHoursMinutesSeconds(currentHour, currentMin, sec);
			m_WeatherManager.TimeToHoursMinutesSeconds(transitionMgr.GetTimeLeftUntilNextState(), hour, min, sec);
			float roundUp = Math.Round((currentMin + min) / 60);
			hour = Math.Repeat(currentHour + hour + roundUp, 24);
			time.SetTextFormat(FORMAT_HOUR, hour.ToString(2));
		}

		stateIcon = ImageWidget.Cast(m_wUpcomingWeatherHolder.FindAnyWidget(WIDGET_NAME_ICON));
		if (stateIcon)
			stateIcon.LoadImageTexture(0, state.GetIconPath());

		stateTitle = TextWidget.Cast(m_wUpcomingWeatherHolder.FindAnyWidget(WIDGET_NAME_TITLE));
		if (stateTitle)
			stateTitle.SetText(state.GetLocalizedName());
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);

		bool previousState = m_ToolMenuEntry.IsEntryActive();
		SetVsibilityState(previousState);

		// delayed as it takes about a frame for the button to find its place, thus imidiate call would position it at 0 0 + offset
		if (previousState)
			GetGame().GetCallqueue().CallLater(UpdateLayoutPosition);

		Update(0);
		UpdateWeatherData();

		bool automaticWindState = m_WeatherManager.IsAutomatedWindDisabled();
		if (automaticWindState || m_bLastAutomaticWindState != automaticWindState)
		{
			UpdateWindData();
		}
		else
		{
			if (m_wCurrentWindSpeed)
				m_wCurrentWindSpeed.SetTextFormat(LABEL_SPEED, m_fLastWindSpeed);
	
			if (m_wCurrentWindDirection)
				m_wCurrentWindDirection.SetTextFormat(LABEL_DIRECTION, m_fLastWindDirection);
		}

		m_WeatherManager.GetOnWindOverrideDataChanged().Insert(OnWindOverrideDataChanged);
		m_WeatherManager.GetOnWeatherChanged().Insert(OnWeatherChanged);

#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_UI_MAP_FORECAST_COOLDOWN, "", "Update map weather report", "Weather");
#endif
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnMapClose(MapConfiguration config)
	{
		super.OnMapClose(config);

		m_WeatherManager.GetOnWindOverrideDataChanged().Remove(OnWindOverrideDataChanged);
		m_WeatherManager.GetOnWeatherChanged().Remove(OnWeatherChanged);

#ifdef ENABLE_DIAG
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_UI_MAP_FORECAST_COOLDOWN);
#endif
	}

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		SCR_MapToolMenuUI toolMenu = SCR_MapToolMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolMenuUI));
		if (!toolMenu)
			return;

		m_World = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!m_World)
			return;

		m_WeatherManager = m_World.GetTimeAndWeatherManager();
		if (!m_WeatherManager)
			return;

		m_ToolMenuEntry = toolMenu.RegisterToolMenuEntry(ICON, string.Empty, 20);
		m_ToolMenuEntry.m_OnClick.Insert(ToggleVisibility);
		m_ToolMenuEntry.SetEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CreateLayout()
	{
		m_wMapWeatherFrame = m_RootWidget.FindAnyWidget(m_sMapWeatherFrame);
		if (!m_wMapWeatherFrame)
			return false;

		m_wWeatherDataRoot = GetGame().GetWorkspace().CreateWidgets(LAYOUT, m_wMapWeatherFrame);
		if (!m_wWeatherDataRoot)
			return false;

		m_wCurrentWeatherHolder = m_wWeatherDataRoot.FindAnyWidget("Now");
		m_wUpcomingWeatherHolder = m_wWeatherDataRoot.FindAnyWidget("Upcoming");
		m_wWeatherOverrideInfo = m_wWeatherDataRoot.FindAnyWidget("WeatherOverrideInfo");
		m_wWindOverrideInfo = m_wWeatherDataRoot.FindAnyWidget("WindOverrideInfo");
		m_wCurrentWindDirection = TextWidget.Cast(m_wWeatherDataRoot.FindAnyWidget("Direction"));
		m_wCurrentWindSpeed = TextWidget.Cast(m_wWeatherDataRoot.FindAnyWidget("Speed"));
		return true;
	}
}
