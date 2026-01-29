//! Gets the Ingame time and displays it on a UI text element
class SCR_IngameClockUIComponent : MenuRootSubComponent 
{			
	[Attribute("Time", desc: "Clock needs at least the Time icon, time multiplier, time pause Icon or Time Text to function")]
	protected string m_sTimeTextName;
	
	[Attribute("DayTime_Icon", desc: "Clock needs at least the Time icon, time multiplier, time pause Icon or Time Text to function")]
	protected string m_sTimeIconName;
	
	[Attribute("Time_Multiplier", desc: "Clock needs at least the Time icon, time multiplier, time pause Icon or Time Text to function")]
	protected string m_sTimeMultiplierName;
	
	[Attribute("Time_Paused_Icon", desc: "Clock needs at least the Time icon, time multiplier, time pause Icon or Time Text to function")]
	protected string m_sTimeStoppedIconName;
	
	[Attribute("#AR-ValueUnit_Short_Multiplier", uiwidget: UIWidgets.LocaleEditBox)]
	protected string m_sTimeMultiplierFormat;
	
	[Attribute("#AR-Weather_Forecast_NoTimeProgression", uiwidget: UIWidgets.LocaleEditBox)]
	protected string m_sNoTimeProgressionHint;
	
	[Attribute("#AR-Editor_Clock_IngameTime_Name", uiwidget: UIWidgets.LocaleEditBox)]
	protected string m_sInGameTimeHint;
	
	[Attribute("1", desc: "Time until the UI will update in seconds")]
	protected float m_fUpdateFreq;
	
	[Attribute("1", desc: "If true will hide the time multiplier if one (and assigned)")]
	protected bool m_bHideTimeMultiplierIfOne;
	
	[Attribute(desc: "If the current time of day is in this Phase show moonphase instead of daytimeIcon", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EDayTimeEnums))]
	protected ref array<EDayTimeEnums> m_aShowMoonphaseAtDayTimes;

	//~ References
	protected TextWidget m_TimeText;
	protected ImageWidget m_TimeIcon;
	protected TextWidget m_TimeMultiplier;
	protected ImageWidget m_TimeStoppedIcon; //~ Icon when simulation is paused
	protected TimeAndWeatherManagerEntity m_TimeAndWeatherManagerEntity;
	protected SCR_CustomTooltipTargetEditorUIComponent m_CustomTooltipUIComponent;
	
	//======================== CLOCK UPDATE ========================\\

	//------------------------------------------------------------------------------------------------
	protected void UpdateIngameClock()
	{
		//Get time
		if (m_TimeText)
		{
			int hour, minute, sec;
			m_TimeAndWeatherManagerEntity.GetHoursMinutesSeconds(hour, minute, sec);
			m_TimeText.SetTextFormat(SCR_FormatHelper.GetTimeFormattingHoursMinutes(hour, minute));
		}
		
		if (m_TimeIcon)
		{			
			SCR_UIInfo dayTimeUIInfo;
			EDayTimeEnums dayTimePhase = m_TimeAndWeatherManagerEntity.GetCurrentDayTimeUIInfoAndPhase(dayTimeUIInfo);
			
			if (m_aShowMoonphaseAtDayTimes.Contains(dayTimePhase))
			{
				SCR_MoonPhaseUIInfo moonPhase = m_TimeAndWeatherManagerEntity.GetCurrentMoonPhaseInfoForDate();
				m_TimeIcon.SetRotation(moonPhase.GetMoonphaseImageRotation());
				moonPhase.SetIconTo(m_TimeIcon);
			}
			else 
			{
				m_TimeIcon.SetRotation(0);
				dayTimeUIInfo.SetIconTo(m_TimeIcon);
			}
		}
		
		if (m_TimeMultiplier)
		{
			if (m_TimeAndWeatherManagerEntity.GetIsDayAutoAdvanced())
			{
				float timeMultiplier = 86400 / m_TimeAndWeatherManagerEntity.GetDayDuration();	
				int TimeMultiplierInt = timeMultiplier;
				
				bool visible = !(m_bHideTimeMultiplierIfOne && (TimeMultiplierInt == 1 && (timeMultiplier - TimeMultiplierInt) < 0.1));
				m_TimeMultiplier.SetVisible(visible);
				m_TimeStoppedIcon.SetVisible(false);
				m_CustomTooltipUIComponent.GetInfo().SetDescription(m_sInGameTimeHint);
				
				if (visible)
				{
					if ((timeMultiplier - TimeMultiplierInt) != 0)
						m_TimeMultiplier.SetTextFormat(m_sTimeMultiplierFormat, timeMultiplier.ToString(-1, 1));
					else 
						m_TimeMultiplier.SetTextFormat(m_sTimeMultiplierFormat, TimeMultiplierInt);
				}
			}
			else 
			{
				// When Time progression is paused/No Time Progression Show Pause Icon hide multiplier text
				m_CustomTooltipUIComponent.GetInfo().SetDescription(m_sNoTimeProgressionHint);
				m_TimeStoppedIcon.SetVisible(true);
				m_TimeMultiplier.SetVisible(false);
			}
		}
			
	}
	
	//======================== ATTACH/DEATTACH HANDLER ========================\\

	//------------------------------------------------------------------------------------------------
	//On Init
	override void HandlerAttachedScripted(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		m_TimeText = TextWidget.Cast(w.FindAnyWidget(m_sTimeTextName));
		m_TimeIcon = ImageWidget.Cast(w.FindAnyWidget(m_sTimeIconName));
		m_TimeMultiplier = TextWidget.Cast(w.FindAnyWidget(m_sTimeMultiplierName));
		m_TimeStoppedIcon = ImageWidget.Cast(w.FindAnyWidget(m_sTimeStoppedIconName));
		
		if (!m_TimeText && !m_TimeIcon && !m_TimeMultiplier && !m_TimeStoppedIcon)
		{
			Print("'SCR_IngameClockUIComponent' is has no time time widgets and therefore won't function", LogLevel.ERROR);
			return;
		}		
		
		m_CustomTooltipUIComponent = SCR_CustomTooltipTargetEditorUIComponent.Cast(w.FindHandler(SCR_CustomTooltipTargetEditorUIComponent));
		if (!m_CustomTooltipUIComponent)
		{
			Print("'SCR_IngameClockUIComponent' is missing custom editor tooltip component", LogLevel.ERROR);
			return;
		}
		
		ChimeraWorld world = GetGame().GetWorld();
		m_TimeAndWeatherManagerEntity = world.GetTimeAndWeatherManager();
		if (!m_TimeAndWeatherManagerEntity)
		{
			Print("'SCR_IngameClockUIComponent' could not find the TimeAndWeatherManager", LogLevel.ERROR);
			return;
		}
		
		UpdateIngameClock();
		GetGame().GetCallqueue().CallLater(UpdateIngameClock, m_fUpdateFreq * 1000, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//On Destroy
	override void HandlerDeattached(Widget w)
	{
		if (m_TimeAndWeatherManagerEntity)
			GetGame().GetCallqueue().Remove(UpdateIngameClock);
		
	}
}
