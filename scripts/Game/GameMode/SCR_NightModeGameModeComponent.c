[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Component to handle night mode")]
class SCR_NightModeGameModeComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_NightModeGameModeComponent : SCR_BaseGameModeComponent
{
	[Attribute("0", desc: "Set true if you want the GM to be able to set global nightmode. Do not change in runtime", category: "Settings")]
	protected bool m_bAllowGlobalNightMode;
	
	[Attribute("0.1", desc: "0 means the sun is setting, 1 means the max value the sun is under (though in reality max value is around 0.5 and the sun is fully set around 0.1) this value dictates when the max nightmode EV value is reached if the sun is at the current state", params: "0.001 1", category: "Settings")]
	protected float m_fSunStateMaxNightMode;
	
	[Attribute("0", desc: "Value of EV when nightmode is NOT enabled", params: "0 12", category: "Settings")]
	protected int m_iEVValueNormalMode;
	
	[Attribute("2", desc: "Value of EV when nightmode is enabled", params: "0 12", category: "Settings")]
	protected int m_iEVValueNightMode;
	
	[Attribute("SOUND_E_TOGGLE_FLASHLIGHT", desc: "SFX when local editor nightMode is enabled/disabled", category: "Sound")]
	protected string m_sLocalEditorNightModeToggledSound;
	
	[Attribute("ManualCameraLight", desc: "Local nightmode shortcut. Toggle On and off")]
	protected string m_sToggleLocalNightModeShortcut;
	
	//~ States
	protected bool m_bLocalEditorNightModeEnabled;			//!< Can only be true if editor is open and unlimited
	protected bool m_bLocalEditorNightModeOnEditorClose;	//!< Saved local nightmode value when editor is closed to enable it again when opened
	protected bool m_bUnlimitedEditorIsOpen;
	protected bool m_bGlobalNightModeEnabled;
	protected bool m_IsPreviewingTimeOrWeather; //!< If player is locally previewing dateTime or weather and Local nightmode is enabled it will disable local nightmode
	protected float m_fNightModeAlpha = 0; ///< This is the current nightmode alpha for the transition lerp.
	
	//~ Update
	protected bool m_bUpdateEachFrame;							//!< If false it will not update each frame but instead each UPDATE_TICK_IN_SECONDS
	protected float m_fCurrentUpdateTickInSeconds;				//!< Current tick in seconds
	protected static const float UPDATE_TICK_IN_SECONDS = 1;	//!< Update speed when m_bUpdateEachFrame is false
	
	//~ Ref
	protected BaseWorld m_World;
	protected TimeAndWeatherManagerEntity m_TimeAndWeatherManager;
	protected SCR_EditorManagerEntity m_EditorManager;
	protected BaseWeatherStateTransitionManager m_WeatherStateTransitionManager;
	
	//~ Script Invokers
	protected ref ScriptInvokerBool m_OnLocalEditorNightModeEnabledChanged;
	protected ref ScriptInvokerBool m_OnGlobalNightModeEnabledChanged;
	protected ref ScriptInvokerBool m_OnNightModeEnabledChanged;
	
	//------------------------------------------------------------------------------------------------
	//~ Onframe is only active if LocalEditor and/or Global nightmode is enabled. The EV value depends on where the sun is in the sky to gratually brighten the night as the night gets darker
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		//~ If onframe is enabled but sun is not set check each second not each frame
		if (!m_bUpdateEachFrame)
		{
			m_fCurrentUpdateTickInSeconds += timeSlice;
			
			if (m_fCurrentUpdateTickInSeconds < UPDATE_TICK_IN_SECONDS)
				return;
			else 
				m_fCurrentUpdateTickInSeconds = 0;
		}
		
		//~ Get sun direction
		vector Sundirection, moondirection;
		float moonphase;
		m_TimeAndWeatherManager.GetCurrentSunMoonDirAndPhase(Sundirection, moondirection, moonphase);
		
		//~ Normalize sun direction using m_fSunStateMaxNightMode as value when the EV is at it's strongest if the sun is down
		float normalizedSun = Math.Clamp(Sundirection[1] / m_fSunStateMaxNightMode, 0, 1);
		
		//~ Check if it should update each frame if sun is at a state that the EV is set
		m_bUpdateEachFrame = (normalizedSun != 0);
		
		//~ If alpha value changed update EV otherwise do nothing to prevent the EV from updating unncessesarrly 
		if (normalizedSun != m_fNightModeAlpha)
		{			
			m_fNightModeAlpha = normalizedSun;
			m_World.AdjustCameraEV(0, Math.Lerp(m_iEVValueNormalMode, m_iEVValueNightMode, m_fNightModeAlpha));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if local editor night mode is enabled
	bool IsLocalEditorNightModeEnabled()
	{
		return m_bLocalEditorNightModeEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ToggleLocalNightModeShortcut()
	{
		EnableLocalEditorNightMode(!m_bLocalEditorNightModeEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable local editor night mode
	//! Note that enable might fail if the conditions are not met
	//! \param[in] enable To enable or not
	//! \param[in] playSound To play a sound or not
	void EnableLocalEditorNightMode(bool enable, bool playSound = true)
	{		
		if (m_bLocalEditorNightModeEnabled == enable)
			return;
		
		//~ Cannot set nightmode if editor is not unlimited
		if (enable && (!m_EditorManager || m_EditorManager.IsLimited() || !CanEnableNightMode()))
			return;
		
		m_bLocalEditorNightModeEnabled = enable;
		
		EnableNightMode(enable);
		
		if (playSound && !SCR_StringHelper.IsEmptyOrWhiteSpace(m_sLocalEditorNightModeToggledSound)) 
			SCR_UISoundEntity.SoundEvent(m_sLocalEditorNightModeToggledSound);
		
		//~ Global night mode state changed
		if (m_OnLocalEditorNightModeEnabledChanged)
			m_OnLocalEditorNightModeEnabledChanged.Invoke(enable);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if scenario allows for global night mode to be enabled
	bool IsGlobalNightModeAllowed()
	{
		return m_bAllowGlobalNightMode;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if global night mode is enabled for all players
	bool IsGlobalNightModeEnabled()
	{
		return m_bGlobalNightModeEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable global night mode
	//! Note that enable might fail if the conditions are not met
	//! \param[in] enable To enable or not
	//! \param[in] nightModeChangerPlayerID If a player ID is given then a notification is sent to all players if global night mode is enabled/disabled
	void EnableGlobalNightMode(bool enable, int nightModeChangerPlayerID = -1)
	{		
		//~ Server only and check if value can be changed and is changed
		if (m_bGlobalNightModeEnabled == enable || !IsGlobalNightModeAllowed() || !GetGameMode().IsMaster())
			return;
		
		if (enable && !CanEnableNightMode())
			return;
		
		//~ Execute enable nightmode logic
		RPC_EnableGlobalNightMode(enable, nightModeChangerPlayerID);
		Rpc(RPC_EnableGlobalNightMode, enable, nightModeChangerPlayerID);	
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_EnableGlobalNightMode(bool enable, int nightModeChangerPlayerID)
	{
		//~ Cannot change global nightmode or is already the same state as given state
		if (m_bGlobalNightModeEnabled == enable || !IsGlobalNightModeAllowed())
			return;
		
		m_bGlobalNightModeEnabled = enable;
		
		//~ Enable the nightmode
		EnableNightMode(enable);
		
		//~ Global night mode state changed
		if (m_OnGlobalNightModeEnabledChanged)
			m_OnGlobalNightModeEnabledChanged.Invoke(enable);
		
		//~ Send notification
		if (nightModeChangerPlayerID > 0)
		{
			if (enable)
				SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_GLOBAL_NIGHTMODE_ENABLED, nightModeChangerPlayerID);
			else
				SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_GLOBAL_NIGHTMODE_DISABLED, nightModeChangerPlayerID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if local editor night mode and/or global night mode is enabled
	bool IsNightModeEnabled()
	{
		return IsLocalEditorNightModeEnabled() || IsGlobalNightModeEnabled();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if general night mode can be enabled
	bool CanEnableNightMode()
	{
		return m_TimeAndWeatherManager != null;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Called by EnableLocal or EnableGlobal night mode functions which actually enables the night mode
	protected void EnableNightMode(bool enable)
	{		
		//~ If trying to enable but cannot enable if both LocalEditor and GlobalNightmode are not enable do not enable
		if (enable && (!CanEnableNightMode() || (!IsLocalEditorNightModeEnabled() || (IsLocalEditorNightModeEnabled() && !m_bUnlimitedEditorIsOpen)) && !IsGlobalNightModeEnabled()))
			return;
		//~ If trying to disable but localEditorNight mode is enabled (and unlimited editor is open) or Global Night mode is enabled than do not disable
		else if (!enable && ((IsLocalEditorNightModeEnabled() && m_bUnlimitedEditorIsOpen) || IsGlobalNightModeEnabled()))
			return;
		
		//~ On Night mode enabled start on frame
		if (IsNightModeEnabled() && !SCR_Enum.HasPartialFlag(GetEventMask(), EntityEvent.FRAME))
		{
			SetEventMask(GetOwner(), EntityEvent.FRAME);
		}
		//~ On nightmode disabled stop on frame and set EV to default
		else if (!IsNightModeEnabled())
		{
			//~ Set EV to default
			if (m_fNightModeAlpha != 0)
			{
				m_fNightModeAlpha = 0;
				m_World.AdjustCameraEV(0, m_iEVValueNormalMode);
			}
			
			//~ Stop OnFrame
			if (SCR_Enum.HasPartialFlag(GetEventMask(), EntityEvent.FRAME))
				ClearEventMask(GetOwner(), EntityEvent.FRAME);
		}
		
		//~ Reset update values
		if (enable)
		{
			m_bUpdateEachFrame = true;
			m_fCurrentUpdateTickInSeconds = 0;
		}
		
		//~ Nightmode state changed (Can be by localEditor or GlobalNightMode)
		if (m_OnNightModeEnabledChanged)
			m_OnNightModeEnabledChanged.Invoke(enable);
		
		//~ Check if previewing time or weather state and set the correct settings
		UpdatePreviewingTimeOrWeather();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorOpened()
	{
		m_bUnlimitedEditorIsOpen = !m_EditorManager.IsLimited();
		
		if (m_bUnlimitedEditorIsOpen)
			EnableLocalEditorNightMode(m_bLocalEditorNightModeOnEditorClose);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorClosed()
	{
		m_bUnlimitedEditorIsOpen = false;
		
		//~ Disable lightmode as GlobalLightmode was disabled
		if (!m_EditorManager.IsLimited())
		{
			m_bLocalEditorNightModeOnEditorClose = IsLocalEditorNightModeEnabled();
			EnableLocalEditorNightMode(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorLimitedChanged(bool isLimited)
	{
		//~ If limited disable local editor nightmode until rights are returned and the player turns it on again
		if (isLimited)
			EnableLocalEditorNightMode(false);		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdatePreviewingTimeOrWeather()
	{
		//~ Check if previewing state changed
		bool newIsPreviewing = m_WeatherStateTransitionManager.IsPreviewingWind() || m_WeatherStateTransitionManager.IsPreviewingState() || m_WeatherStateTransitionManager.IsPreviewingDateTime();
		if (m_IsPreviewingTimeOrWeather == newIsPreviewing)
			return;
		
		m_IsPreviewingTimeOrWeather = newIsPreviewing;
		
		//~ If local nightmode is enabled but global is not and previewing the scene then turn off nightmode
		if (!m_bUnlimitedEditorIsOpen || !IsLocalEditorNightModeEnabled() || IsGlobalNightModeEnabled())
			return;
		
		if (m_IsPreviewingTimeOrWeather)
		{
			//~ Pause the OnFrame
			if (SCR_Enum.HasPartialFlag(GetEventMask(), EntityEvent.FRAME))
				ClearEventMask(GetOwner(), EntityEvent.FRAME);
			
			//~ Disable nightmode
			if (m_fNightModeAlpha != 0)
			{
				m_fNightModeAlpha = 0;
				m_World.AdjustCameraEV(0, m_iEVValueNormalMode);
				
				SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_PREVIEWING_IN_NIGHTMODE);
				
				if (!SCR_StringHelper.IsEmptyOrWhiteSpace(m_sLocalEditorNightModeToggledSound)) 
					SCR_UISoundEntity.SoundEvent(m_sLocalEditorNightModeToggledSound);		
			}
		}
		else 
		{
			//~ Pause continue the on frame again
			if (!SCR_Enum.HasPartialFlag(GetEventMask(), EntityEvent.FRAME))
			{
				SetEventMask(GetOwner(), EntityEvent.FRAME);
				
				if (m_TimeAndWeatherManager.IsSunSet() && !SCR_StringHelper.IsEmptyOrWhiteSpace(m_sLocalEditorNightModeToggledSound)) 
					SCR_UISoundEntity.SoundEvent(m_sLocalEditorNightModeToggledSound);	
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnWeatherStatePreview(bool preview, string stateName)
	{
		UpdatePreviewingTimeOrWeather();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnWindPreview(bool preview, float windSpeed = -1, float windAngleDegrees = -1)
	{
		UpdatePreviewingTimeOrWeather();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDateTimePreview(bool preview, int year = -1, int month = -1, int day = -1, float timeOfTheDay = -1)
	{
		UpdatePreviewingTimeOrWeather();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return ScriptInvoker On Local editor night mode enabled changed
	ScriptInvokerBool GetOnLocalEditorNightModeEnabledChanged()
	{
		if (!m_OnLocalEditorNightModeEnabledChanged)
			m_OnLocalEditorNightModeEnabledChanged = new ScriptInvokerBool();
		
		return m_OnLocalEditorNightModeEnabledChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return ScriptInvoker On Global night mode enabled changed
	ScriptInvokerBool GetOnGlobalNightModeEnabledChanged()
	{
		if (!m_OnGlobalNightModeEnabledChanged)
			m_OnGlobalNightModeEnabledChanged = new ScriptInvokerBool();
		
		return m_OnGlobalNightModeEnabledChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return ScriptInvoker On Local Editor or Global night mode enabled changed
	ScriptInvokerBool GetOnNightModeEnabledChanged()
	{
		if (!m_OnNightModeEnabledChanged)
			m_OnNightModeEnabledChanged = new ScriptInvokerBool();
		
		return m_OnNightModeEnabledChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorManagerCreated(SCR_EditorManagerEntity editorManager)
	{
		//~ Editor Manager not yet set so that means it was obtained by editorManagerCore. Make sure to no longer listen to OnEditorManagerInitOwner
		if (!m_EditorManager)
		{
			m_EditorManager = editorManager;
			
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (core)
				core.Event_OnEditorManagerInitOwner.Remove(OnEditorManagerCreated);
		}
		
		//~ Listen to events
		m_EditorManager.GetOnOpened().Insert(OnEditorOpened);
		m_EditorManager.GetOnClosed().Insert(OnEditorClosed);
		m_EditorManager.GetOnLimitedChange().Insert(OnEditorLimitedChanged);
		
		if (m_EditorManager.IsOpened())
			OnEditorOpened();
	}	

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{		
		if (SCR_Global.IsEditMode())
			return;
		
		GetGame().GetInputManager().AddActionListener(m_sToggleLocalNightModeShortcut, EActionTrigger.DOWN, ToggleLocalNightModeShortcut);
		
		//~ Get world
		m_World = GetOwner().GetWorld();
		if (!m_World)
		{
			m_World = GetGame().GetWorld();
			if (!m_World)
				Print("'SCR_NightModeGameModeComponent' could not find World this means nightmode can never be enabled!", LogLevel.ERROR);
			
			return;
		}
		
		//~ Get time manager
		ChimeraWorld world = ChimeraWorld.CastFrom(m_World);
		if (!world)
			return;
		
		m_TimeAndWeatherManager = world.GetTimeAndWeatherManager();
		if (!m_TimeAndWeatherManager)
		{
			Print("'SCR_NightModeGameModeComponent' could not find TimeAndWeatherEntity this means nightmode can never be enabled!", LogLevel.ERROR);
			return;
		}
		
		//~ Get transition manager to make sure it can check if dateTime or weather is being previewed
		m_WeatherStateTransitionManager = m_TimeAndWeatherManager.GetTransitionManager();
		if (m_WeatherStateTransitionManager)
		{
			m_TimeAndWeatherManager.GetOnWeatherStatePreview().Insert(OnWeatherStatePreview);
			m_TimeAndWeatherManager.GetOnWindPreview().Insert(OnWindPreview);
			m_TimeAndWeatherManager.GetOnDateTimePreview().Insert(OnDateTimePreview);
		}
		
		//~ Get EditorManager
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			m_EditorManager = editorManager;
			OnEditorManagerCreated(editorManager);
		}
		else
		{
			//~ Editor manager does not exist yet (e.g., on mission start), wait for it to be created
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (core)
				core.Event_OnEditorManagerInitOwner.Insert(OnEditorManagerCreated);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		super.OnDelete(owner);
		
		if (m_EditorManager)
		{
			m_EditorManager.GetOnOpened().Remove(OnEditorOpened);
			m_EditorManager.GetOnClosed().Remove(OnEditorClosed);
			m_EditorManager.GetOnLimitedChange().Remove(OnEditorLimitedChanged);
		}
		
		if (m_TimeAndWeatherManager && m_WeatherStateTransitionManager)
		{
			m_TimeAndWeatherManager.GetOnWeatherStatePreview().Remove(OnWeatherStatePreview);
			m_TimeAndWeatherManager.GetOnWindPreview().Remove(OnWindPreview);
			m_TimeAndWeatherManager.GetOnDateTimePreview().Remove(OnDateTimePreview);
		}
		
		GetGame().GetInputManager().RemoveActionListener(m_sToggleLocalNightModeShortcut, EActionTrigger.DOWN, ToggleLocalNightModeShortcut);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
    {	
		if (IsGlobalNightModeAllowed())
        	writer.WriteBool(m_bGlobalNightModeEnabled); 
		
        return true;
    }
     
	//------------------------------------------------------------------------------------------------
    override bool RplLoad(ScriptBitReader reader)
    {
		bool globalNightModeEnabled;
		
		if (IsGlobalNightModeAllowed())
		{
			reader.ReadBool(globalNightModeEnabled);
			RPC_EnableGlobalNightMode(globalNightModeEnabled, -1);
		}
        	
        return true;
    }
}
