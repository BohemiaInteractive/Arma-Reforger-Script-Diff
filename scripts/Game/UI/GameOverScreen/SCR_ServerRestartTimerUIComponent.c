class SCR_ServerRestartTimerUIComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("#AR-Endscreen_RestartTimer_Format-UC", desc: "Restart timer format %1 is the time")]
	protected LocalizedString m_sRestartTimerFormat;
	
	[Attribute(desc: "Widget name of restart timer text. Leave empty to take root as the TextWidget")]
	protected string m_sRestartTimerName;
	
	[Attribute("1", desc: "Countdown sfx will play if true")]
	protected bool m_bEnableCountDownSfx;
	
	[Attribute("1", desc: "Start timer on init, note that it will be hidden if the game has not ended or it is not set up correctly")]
	protected bool m_bStartOnInit;
	
	//~ Timer values
	protected float m_fAutoReloadDelay = -1;
	protected WorldTimestamp m_GameEndTimeStamp = null;
	protected int m_iPreviousRestartTime;
	
	//~ References
	protected ChimeraWorld m_World;
	protected TextWidget m_wRestartTimer;
	
	//! Refresh time of timer (in milliseconds)
	protected const int UPDATE_TIME = 100;
	
	//------------------------------------------------------------------------------------------------
	//! return If the restart timer is set up correctly
	bool IsValid()
	{
		if (!m_wRestartTimer)
			Init();
		
		return m_fAutoReloadDelay > 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Init()
	{
		if (!m_sRestartTimerName.IsEmpty())
			m_wRestartTimer = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sRestartTimerName));
		else
			m_wRestartTimer = TextWidget.Cast(m_wRoot);
		
		if (!m_wRestartTimer)
			return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		m_World = GetGame().GetWorld();
		if (!m_World)
			return;
		
		m_fAutoReloadDelay = gameMode.GetAutoReloadDelay();
		m_GameEndTimeStamp = gameMode.GetGameEndTimeStamp();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Start the timer 
	//! param[in] checkIfValid If true checks if the timer is valid (aka all systems and UI are there and the game has ended) This is done on init as well
	void StartTimer(bool checkIfValid = true)
	{
		if (checkIfValid && !IsValid())
			return;
		
		UpdateTimer();
		GetGame().GetCallqueue().CallLater(UpdateTimer, UPDATE_TIME, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Stop the timer
	void StopTimer()
	{
		if (m_fAutoReloadDelay <= 0)
			return;
		
		GetGame().GetCallqueue().Remove(UpdateTimer);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateTimer()
	{
		if (!m_World || !m_wRestartTimer || !m_wRoot.IsVisible())
			return;
		
		//~ Call function which knows everythime the time changes a second with current restart time left
		OnTimeUpdate(Math.Clamp(m_fAutoReloadDelay - ((m_World.GetLocalTimestamp().DiffMilliseconds(m_GameEndTimeStamp)) * 0.001), 0, float.MAX));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTimeUpdate(float currentRestartTimer)
	{
		//~ Value did not change
		if ((int)currentRestartTimer == m_iPreviousRestartTime)
			return;

		//~ Set current timer as previous timer
		m_iPreviousRestartTime = currentRestartTimer;
				
		//~ Update the timer widget
		string timerText = string.Format("<color rgba='%1'>%2</color>",  UIColors.FormatColor(GUIColors.ENABLED), SCR_FormatHelper.GetTimeFormatting(currentRestartTimer, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS));
		m_wRestartTimer.SetTextFormat(m_sRestartTimerFormat, timerText);
		
		//~ Countdown SFX
		if (m_bEnableCountDownSfx)
		{
			if ((int)currentRestartTimer > 0)
			{
				SCR_UISoundEntity.SetSignalValueStr("countdownValue", currentRestartTimer);
				SCR_UISoundEntity.SetSignalValueStr("maxCountdownValue", m_fAutoReloadDelay);					
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RESPAWN_COUNTDOWN);
			}
			else 
			{
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RESPAWN_COUNTDOWN_END);
			}			
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		if (SCR_Global.IsEditMode())
			return;
		
		Init();
		
		if (!IsValid())
			w.SetVisible(false);
		else if (m_bStartOnInit)
			StartTimer(false);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_fAutoReloadDelay <= 0)
			return;
		
		GetGame().GetCallqueue().Remove(UpdateTimer);
	}
}
