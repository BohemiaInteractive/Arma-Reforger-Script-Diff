void SCR_HintManagerComponent_OnHint(SCR_HintUIInfo info, bool isSilent);
typedef func SCR_HintManagerComponent_OnHint;

[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "")]
class SCR_HintManagerComponentClass : SCR_BaseGameModeComponentClass
{
	[Attribute()]
	protected ref array<ref SCR_HintConditionList> m_aConditionLists;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	void InitConditionLists(IEntity owner)
	{
		for (int i, count = m_aConditionLists.Count(); i < count; i++)
		{
			m_aConditionLists[i].Init(owner);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	void ExitConditionLists(IEntity owner)
	{
		for (int i, count = m_aConditionLists.Count(); i < count; i++)
		{
			m_aConditionLists[i].Exit(owner);
		}
	}
}

class SCR_HintManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute("4")]
	protected float m_fDefaultDuration;
	
	[Attribute("1", desc: "Mark the hint as shown only if it was display for this duration (seconds).\nIf the hint duration is shorter, mark it as shown when it's hidden.")]
	protected float m_fPersistentDelay;
	
	[Attribute("0", desc: "When enabled, hints will be shown in this scenario even when they're disabled in game settings.\nUseful for tutorial scenarios.")]
	protected bool m_bIgnoreHintSettings;
	
	protected bool m_bIsShown;
	protected float m_fDurationOverride;
	protected SCR_HintUIInfo m_LatestHint;
	protected BaseContainer m_SettingsContainer;
	protected ref SCR_HintSettings m_Settings;
	protected ref SCR_HintUIInfo m_CustomHint; //--- Strong reference to hint info created in run-time from texts
	protected ref set<EHint> m_aSessionShownHints = new set<EHint>();
	protected ref ScriptInvokerBase<SCR_HintManagerComponent_OnHint> m_OnHintShow = new ScriptInvokerBase<SCR_HintManagerComponent_OnHint>();
	protected ref ScriptInvokerBase<SCR_HintManagerComponent_OnHint> m_OnHintHide = new ScriptInvokerBase<SCR_HintManagerComponent_OnHint>();
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Public functions
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	//! Show hint based on existing configuration.
	//! \param[in] info Hint UI info
	//! \param[in] isSilent True to show the hint without any sound effect
	//! \param[in] ignoreShown When true, the hint will be shown even if it was shown previously
	//! \return True if the hint was shown
	bool Show(SCR_HintUIInfo info, bool isSilent = false, bool ignoreShown = false)
	{
		//--- Ignore if hints are disabled in gameplay settings (not for sequence hints, because they're triggered manually)
		if ((!CanShow() || (!ignoreShown && WasShown(info))) && !info.IsInSequence())
			return false;
		
		//--- Ignore if the new hint has lower priority than the current one
		if (m_bIsShown && m_LatestHint)
		{
			if (info.GetPriority() < m_LatestHint.GetPriority())
				return false;
		}
		
		//--- Check if timer is visible, if yes, set the timestamp of start.
		
		if (info.IsTimerVisible())
		{
			info.SetTimeStamp();
		}
		
		//--- Always silent when refreshing the same hint
		isSilent |= m_bIsShown && info == m_LatestHint;
		
		//--- Hide current hint (need to call associated event)
		Hide();
		
		//--- Set new current hint
		m_bIsShown = true;
		m_LatestHint = info;
		
		//--- Call an event for GUI systems which will actually show the hint (no GUI here!)
		m_OnHintShow.Invoke(info, isSilent);
		
		//--- Set duration (only when the value is *not* negative, which means endless hint)
		float duration;
		if (m_fDurationOverride != 0)
			duration = m_fDurationOverride;
		else
			duration = info.GetDuration();
		
		GetGame().GetCallqueue().Remove(Hide);
		if (duration >= 0)
		{
			//--- Use default duration when the value is 0
			if (duration == 0)
				duration = m_fDefaultDuration;
			
			GetGame().GetCallqueue().CallLater(Hide, duration * 1000, false, info);
		}
		
		//--- Mark the hint as shown after a delay (multiple hints may be triggered on the same frame, delay will make sure only the actually shown one will be saved)
		GetGame().GetCallqueue().Remove(SetShown);
		int type = info.GetType();
		if (type > 0)
		{
			//--- Make sure the delay is shorter than duration
			float delay = m_fPersistentDelay;
			if (duration > 0)
				delay = Math.Min(delay, duration);
			
			GetGame().GetCallqueue().CallLater(SetShown, delay * 1000, false, info);
		}
		
		//info.Log("SCR_HintManagerComponent.Show: ");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Show hint made of custom texts.
	//!
	//! *************************************************************************************************
	//! ## WARNING! ##
	//!
	//! Use only for quick debugging.
	//! For legit use, please configure the hint as SCR_UIInfo attribute on your entity/component/config!
	//! That will allow you to set all hint properties, as well as to localize it using LocParserPlugin.
	//!
	//! *************************************************************************************************
	//!
	//! \param[in] description Hint text
	//! \param[in] name Hint title
	//! \param[in] duration For how long should the hint be shown (in seconds)
	//! \param[in] isSilent True to show the hint without any sound effect
	//! \param[in] type Hint type. When defined, the hint will be shown only once and never again.
	//! \return True if the hint was shown
	//!
	bool ShowCustom(string description, string name = string.Empty, float duration = 0, bool isSilent = false, EHint type = EHint.UNDEFINED, EFieldManualEntryId fieldManualEntry = EFieldManualEntryId.NONE, bool isTimerVisible = false)
	{
		m_CustomHint = SCR_HintUIInfo.CreateInfo(description, name, duration, type, fieldManualEntry, isTimerVisible);
		return Show(m_CustomHint, isSilent);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool ClearHint()
	{
		if(m_LatestHint)
		{
			m_LatestHint = null;
			return true;
		}
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Repeat previously shown hint.
	//! \param[in] isSilent
	//! \return True if the hint was shown
	bool Repeat(bool isSilent = false)
	{
		if (m_LatestHint)
			return Show(m_LatestHint, isSilent, true);
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Silently refresh currently shown hint.
	//! \return True if the hint was refreshed
	bool Refresh()
	{
		if (m_bIsShown)
			return Show(m_LatestHint, true, true);
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Hide currently shown hint.
	//! \param[in] info When defined, clear only this hint. If other hint is shown, do nothing.
	//! \return True if a hint was cleared
	bool Hide(SCR_HintUIInfo info = null)
	{
		//--- Nothing to clear
		if (!m_bIsShown)
			return false;
		
		//--- Check if it's the hint passed in a param
		if (info && info != m_LatestHint)
			return false;
		
		GetGame().GetCallqueue().Remove(Hide);
		
		m_bIsShown = false;
		m_OnHintHide.Invoke(m_LatestHint, false);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Toggle hint. Hide it if it's shown, and open it again if it's hidden.
	void Toggle()
	{
		if (m_Settings && !m_Settings.AreHintsEnabled())
			return;
		
		if (IsShown())
			Hide();
		else
			Repeat();
	}

	//------------------------------------------------------------------------------------------------
	//! Open context to currently shown hint.
	void OpenContext()
	{
		if (!IsShown() || !m_LatestHint || (m_Settings && !m_Settings.AreHintsEnabled()))
			return;
		
		EFieldManualEntryId link = m_LatestHint.GetFieldManualLink();
		if (link != EFieldManualEntryId.NONE)
			SCR_FieldManualUI.Open(link);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the most recent hint.
	//! \return Hint UI info
	SCR_HintUIInfo GetLatestHint()
	{
		return m_LatestHint;
	}

	//------------------------------------------------------------------------------------------------
	//! Get thecurrently shown hint.
	//! \return Hint UI info
	SCR_HintUIInfo GetCurrentHint()
	{
		if (m_bIsShown)
			return m_LatestHint;
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if a hint is shown at this moment.
	//! \return True when shown
	bool IsShown()
	{
		return m_bIsShown;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if hints are enabled in gameplay settings.
	//! \return True when enabled
	bool CanShow()
	{
		return m_bIgnoreHintSettings || !m_Settings || m_Settings.AreHintsEnabled();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if hint type was shown previously (even in previous game sessions; the information is stored persistently).
	//! \param[in] hint Hint type
	//! \param[in] limit How many times can the hint be shown
	//! \return True when shown
	bool WasShown(EHint hint, int limit = 1)
	{
		return hint > 0 //--- Is type defined (hints without type are never remembered)?
				&& (
					(m_Settings && (m_Settings.GetCount(hint) < 0 || m_Settings.GetCount(hint) >= limit) //--- Can the hint be shown again? How many times was the hint shown across instances?
					|| m_aSessionShownHints.Contains(hint)) //--- Was the hint shown in this instance?
				)
				&& !DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_HINT_IGNORE_SHOWN); //--- Is debug mode suppressing this check?
	}

	//------------------------------------------------------------------------------------------------
	//! Check if hint info was shown previously (even in previous game sessions; the information is stored persistently).
	//! \return True when shown
	bool WasShown(SCR_HintUIInfo info)
	{
		return info && WasShown(info.GetType(), info.GetShowLimit());
	}

	//------------------------------------------------------------------------------------------------
	//! Override hint duration.
	//! \param[in] duration. When 0, override is reset.
	void SetDurationOverride(float duration)
	{
		m_fDurationOverride = duration;
	}
	//------------------------------------------------------------------------------------------------
	//! Get override of hint duration.
	//! \return Duration. When 0, no override is active.
	float GetDurationOverride()
	{
		return m_fDurationOverride;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the event called when a hint is shown.
	//! \return Script invoker
	ScriptInvokerBase<SCR_HintManagerComponent_OnHint> GetOnHintShow()
	{
		return m_OnHintShow;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the event called when a hint is hidden.
	//! \return Script invoker
	ScriptInvokerBase<SCR_HintManagerComponent_OnHint> GetOnHintHide()
	{
		return m_OnHintHide;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Static functions
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	//! Get instance of the hint manager.
	//! \return Hint manager
	static SCR_HintManagerComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			return SCR_HintManagerComponent.Cast(gameMode.FindComponent(SCR_HintManagerComponent));
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Show hint based on existing configuration.
	//! \param[in] info Hint UI info
	//! \param[in] isSilent True to show the hint without any sound effect
	//! \param[in] ignoreShown When true, the hint will be shown even if it was shown previously
	//! \return True if the hint was shown
	static bool ShowHint(SCR_HintUIInfo info, bool isSilent = false, bool ignoreShown = false)
	{
		SCR_HintManagerComponent hintManager = GetInstance();
		if (hintManager)
			return hintManager.Show(info, isSilent, ignoreShown);
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Show hint made of custom texts.
	//!
	//! *************************************************************************************************
	//! ## WARNING! ##
	//!
	//! Use only for quick debugging.
	//! For legit use, please configure the hint as SCR_UIInfo attribute on your entity/component/config!
	//! That will allow you to set all hint properties, as well as to localize it using LocParserPlugin.
	//!
	//! *************************************************************************************************
	//!
	//! \param[in] description Hint text
	//! \param[in] name Hint title
	//! \param[in] duration For how long should the hint be shown (in seconds)
	//! \param[in] isSilent True to show the hint without any sound effect
	//! \return True if the hint was shown
	//!
	static bool ShowCustomHint(string description, string name = string.Empty, float duration = 0, bool isSilent = false, EFieldManualEntryId fieldManualEntry = EFieldManualEntryId.NONE, bool isTimerVisible = false)
	{
		SCR_HintManagerComponent hintManager = GetInstance();
		if (hintManager)
			return hintManager.ShowCustom(description, name, duration, isSilent, EHint.UNDEFINED, fieldManualEntry, isTimerVisible);
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Clear the last used hint so it no longer shows.
	static bool ClearLatestHint()
	{
		SCR_HintManagerComponent hintManager = GetInstance();
		if (hintManager)
			return hintManager.ClearHint();
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Repeat previously shown hint.
	//! \param[in] isSilent
	//! \return True if the hint was shown
	static bool RepeatHint(bool isSilent = false)
	{
		SCR_HintManagerComponent hintManager = GetInstance();
		if (hintManager)
			return hintManager.Repeat(isSilent);
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Clear currently shown hint.
	//! \param[in] info When defined, clear only this hint. If other hint is shown, do nothing.
	//! \return True if a hint was cleared
	static bool HideHint(SCR_HintUIInfo info = null)
	{
		SCR_HintManagerComponent hintManager = GetInstance();
		if (hintManager)
			return hintManager.Hide(info);
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if a hint is shown at this moment.
	//! \return True when shown
	static bool IsHintShown()
	{
		SCR_HintManagerComponent hintManager = GetInstance();
		if (hintManager)
			return hintManager.IsShown();
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if hints are enabled in gameplay settings.
	//! \return True when enabled
	static bool CanShowHints()
	{
		SCR_HintManagerComponent hintManager = GetInstance();
		if (hintManager)
			return hintManager.CanShow();
		else
			return false;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Protected functions
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void LoadSettings()
	{
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager)
			return;
		
		BaseContainer interfaceSettings = GetGame().GetGameUserSettings().GetModule(hudManager.GetInterfaceSettingsClass());
		if (!interfaceSettings)
			return;
		
		bool state;
		interfaceSettings.Get("m_bShowHints", state);

		m_SettingsContainer = GetGame().GetGameUserSettings().GetModule("SCR_HintSettings");
		m_Settings.SetHintsEnabled(state);
		m_Settings.LoadShownHints(m_SettingsContainer);
		
		if (m_bIsShown && !m_Settings.AreHintsEnabled())
			Hide();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetShown(SCR_HintUIInfo info)
	{
		if (!info || WasShown(info) || DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_HINT_IGNORE_SHOWN))
			return;
		
		EHint type = info.GetType();
		m_aSessionShownHints.Insert(type);
		int count = m_Settings.AddCount(type);
		m_Settings.SaveShownHints(m_SettingsContainer);
		
		Print(string.Format("Hint %1 = %2 saved persistently, count = %3.", typename.EnumToString(EHint, type), type, count), LogLevel.VERBOSE);
	}

	//------------------------------------------------------------------------------------------------
	void DontShowAgainCurrent()
	{
		if (!m_Settings || !m_LatestHint)
			return;

		EHint hintType = m_LatestHint.GetType();
		if (hintType <= 0)
			return;

		m_Settings.DontShowAgain(hintType, m_SettingsContainer);
		Hide();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default functions
	////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (System.IsConsoleApp())
			return;
		
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_HINT_IGNORE_SHOWN, "", "Ignore hint persistency", "UI");
		
		if (SCR_Global.IsEditMode(owner))
			return;
		
		//--- Call only for the owner, not server (disabled, called before ownership is transferred)
		//RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		//if (rplComponent && !rplComponent.IsOwner())
		//	return;
		
		SCR_HintManagerComponentClass componentPrefab = SCR_HintManagerComponentClass.Cast(GetComponentData(owner));
		componentPrefab.InitConditionLists(owner);
		
		m_Settings = new SCR_HintSettings();

		GetGame().GetOnHUDManagerChanged().Insert(LoadSettings);
		GetGame().OnUserSettingsChangedInvoker().Insert(LoadSettings);
		
		GetGame().GetInputManager().AddActionListener("HintToggle", EActionTrigger.DOWN, Toggle);
		GetGame().GetInputManager().AddActionListener("HintContext", EActionTrigger.DOWN, OpenContext);
		GetGame().GetInputManager().AddActionListener("HintDismiss", EActionTrigger.DOWN, DontShowAgainCurrent);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{		
		if (System.IsConsoleApp())
			return;
		
		DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_HINT_IGNORE_SHOWN);
		
		SCR_HintManagerComponentClass componentPrefab = SCR_HintManagerComponentClass.Cast(GetComponentData(owner));
		componentPrefab.ExitConditionLists(owner);

		GetGame().GetOnHUDManagerChanged().Remove(LoadSettings);
		GetGame().OnUserSettingsChangedInvoker().Remove(LoadSettings);

		GetGame().GetInputManager().RemoveActionListener("HintToggle", EActionTrigger.DOWN, Toggle);
		GetGame().GetInputManager().RemoveActionListener("HintContext", EActionTrigger.DOWN, OpenContext);
		GetGame().GetInputManager().RemoveActionListener("HintDismiss", EActionTrigger.DOWN, DontShowAgainCurrent);
	}
}
