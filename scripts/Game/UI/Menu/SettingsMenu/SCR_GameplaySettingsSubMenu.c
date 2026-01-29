void ScriptInvoker_GameplaySettingsSubMenu(SCR_GameplaySettingsSubMenu menu);
typedef func ScriptInvoker_GameplaySettingsSubMenu;
typedef ScriptInvokerBase<ScriptInvoker_GameplaySettingsSubMenu> ScriptInvoker_GameplaySettingsSubMenuChanged;

class SCR_GameplaySettingsSubMenu: SCR_SettingsSubMenuBase
{
	protected static const ref array<string> m_aLanguages = {
		"en_us",
		"fr_fr",
		"it_it",
		"de_de",
		"es_es",
		"cs_cz",
		"pl_pl",
		"ru_ru",
		"ja_jp",
		"ko_kr",
		"pt_br",
		"zh_cn",
		"uk_ua"
	};
	protected static const string m_sUserInterfaceSettings = "UserInterfaceSettings";

	static ref ScriptInvoker_GameplaySettingsSubMenuChanged m_OnLanguageChanged = new ScriptInvoker_GameplaySettingsSubMenuChanged();
	
	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();

		SCR_PlayerController.SetGameUserSettings();
		SCR_AimSensitivitySettings.SetAimSensitivitySettings();
		SCR_ControllerSettings.SetControllerSettings();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		m_aSettingsBindings.Clear();

		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("InputDeviceUserSettings", "MouseInvert", "MouseY"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("InputDeviceUserSettings", "GamepadInvert", "GamepadY"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_bStickyADS", "StickyADS"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_bPlatformIconNametag" , "ShowPlatformIcon" ));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_bLogitechSupport" , "LogitechLED" ));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_bStickyGadgets", "StickyGadgets"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_bAutoEquipNextPlaceableItem", "AutomaticEquipmentOfPlaceables"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_bShowRadioProtocolText", "RadioProtocolSubtitles"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_bMouseControlAircraft", "MouseControlAircraft"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_bGamepadFreelookInAircraft", "GamepadFreelookInAircraft"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_eDrivingAssistance", "DrivingAssistance"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_FieldOfViewSettings", "m_fFirstPersonFOV", "1PP"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_FieldOfViewSettings", "m_fThirdPersonFOV", "3PP"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_FieldOfViewSettings", "m_fVehicleFOV", "VehicleFOV"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_FieldOfViewSettings", "m_fFocusInADS", "FocusIntensityADS"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_FieldOfViewSettings", "m_fFocusInPIP", "FocusIntensityPIP"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_eKeyboardCollective", "KeyboardCollective"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_eGamepadCollective", "GamepadCollective"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_GameplaySettings", "m_eHotasCollective", "HotasCollective"));
		
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_AimSensitivitySettings", "m_fMouseSensitivity", "AimMouse"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_AimSensitivitySettings", "m_fStickSensitivity", "AimGamepad"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_AimSensitivitySettings", "m_fAimADS", "AimADS"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_AimSensitivitySettings", "m_fFOVInputCurveMouse", "FOVInputCurveMouse"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_AimSensitivitySettings", "m_fFOVInputCurveStick", "FOVInputCurveStick"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_AimSensitivitySettings", "m_fFOVInputCurveGyro", "FOVInputCurveGyro"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_ControllerSettings", "m_bGyroAlways", "GyroAlways"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_ControllerSettings", "m_bGyroFreelook", "GyroFreelook"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_ControllerSettings", "m_bGyroADS", "GyroADS"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_ControllerSettings", "m_fGyroSensitivity", "GyroSensitivity"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_ControllerSettings", "m_fGyroVerticalHorizontalRatio", "GyroVerticalHorizontalRatio"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_ControllerSettings", "m_eGyroDirectionYaw", "GyroDirectionYaw"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_ControllerSettings", "m_eGyroDirectionPitch", "GyroDirectionPitch"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_ControllerSettings", "m_eGyroDirectionRoll", "GyroDirectionRoll"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("InputDeviceUserSettings", "GyroAimingSpace", "GyroAimingSpace", "GyroSettings"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("InputDeviceUserSettings", "GyroAccelerationSensitivity", "GyroAcceleration", "GyroSettings"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("InputDeviceUserSettings", "GyroAccelerationSpeedMin", "GyroAccelerationSpeedMin", "GyroSettings"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("InputDeviceUserSettings", "GyroAccelerationSpeedMax", "GyroAccelerationSpeedMax", "GyroSettings"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("InputDeviceUserSettings", "GyroPrecisionSpeed", "GyroPrecisionSpeed", "GyroSettings"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingEngine("InputDeviceUserSettings", "GyroSmoothingSpeed", "GyroSmoothingSpeed", "GyroSettings"));

		LoadSettings();
		
		BindLanguage();
		BindCrossplaySettings();

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_UI_SHOW_ALL_SETTINGS, false))
			return;

#ifdef PLATFORM_CONSOLE
		// Hide unusual console settings (consoles -can- use USB KBM)
		if (!GetGame().GetHasKeyboard())
		{
			HideMenuItem("MouseY");
			HideMenuItem("AimMouse");
			HideMenuItem("MouseControlAircraft");
			HideMenuItem("FOVInputCurveMouse");
			HideMenuItem("KeyboardCollective");
			HideMenuItem("LogitechLED");	
		}
		
		HideMenuItem("HotasCollective");
		HideMenuItem("ShowPlatformIcon");
#endif

		// Hide gyro settings outside platforms that support them
		EPlatform platform = System.GetPlatform();
		if (platform != EPlatform.PS4 && platform != EPlatform.PS5 && platform != EPlatform.PS5_PRO && platform != EPlatform.UNKNOWN)
		{
			HideMenuItem("TitleGyro");

			HideMenuItem("GyroAlways");
			HideMenuItem("GyroFreelook");
			HideMenuItem("GyroADS");

			HideMenuItem("GyroSensitivity");
			HideMenuItem("FOVInputCurveGyro");
			HideMenuItem("GyroVerticalHorizontalRatio");
			HideMenuItem("GyroDirectionYaw");
			HideMenuItem("GyroDirectionPitch");
			HideMenuItem("GyroDirectionRoll");

			HideMenuItem("GyroAimingSpace");

			HideMenuItem("GyroAcceleration");
			HideMenuItem("GyroAccelerationSpeedMin");
			HideMenuItem("GyroAccelerationSpeedMax");
			HideMenuItem("GyroPrecisionSpeed");
			HideMenuItem("GyroSmoothingSpeed");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BindLanguage()
	{
		BaseContainer setting = GetGame().GetEngineUserSettings().GetModule(m_sUserInterfaceSettings);
		if (!setting)
			return;

		string currentLang;
		setting.Get("LanguageCode", currentLang);

		int i = m_aLanguages.Find(currentLang);
		if (i == -1)
			return;

		Widget w = m_wRoot.FindAnyWidget("Language");
		if (!w)
			return;

		SCR_SpinBoxComponent comp = SCR_SpinBoxComponent.Cast(w.FindHandler(SCR_SpinBoxComponent));
		if (!comp)
			return;

		comp.SetCurrentItem(i);
		comp.m_OnChanged.Insert(OnLanguageChange);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void BindCrossplaySettings()
	{
		Widget w = m_wRoot.FindAnyWidget("AllowCrossplay");
		if (!w)
			return;
		
		SCR_SpinBoxComponent comp = SCR_SpinBoxComponent.Cast(w.FindHandler(SCR_SpinBoxComponent));
		if (!comp)
			return;
		
		comp.SetCurrentItem(GetGame().IsCrossPlayEnabled());
		comp.m_OnChanged.Insert(OnCrossplayChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCrossplayChanged()
	{
		const string settingName = "AllowCrossplay";
		Widget w = m_wRoot.FindAnyWidget(settingName);
		if (!w)
			return;
		
		SCR_SpinBoxComponent comp = SCR_SpinBoxComponent.Cast(w.FindHandler(SCR_SpinBoxComponent));
		if (!comp)
			return;
		
		Print("Setting: " + comp.GetCurrentIndex());
		if (!GetGame().SetCrossPlay(comp.GetCurrentIndex()))
		{
			comp.SetCurrentItem(!comp.GetCurrentIndex());			
			Print("Setting back: " + !comp.GetCurrentIndex());
		}
		
		GetGame().UserSettingsChanged();
		
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(m_sUserInterfaceSettings, settingName);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLanguageChange(SCR_SpinBoxComponent comp, int i)
	{
		BaseContainer setting = GetGame().GetEngineUserSettings().GetModule(m_sUserInterfaceSettings);
		if (!setting)
			return;

		if (!m_aLanguages.IsIndexValid(i))
			return;

		const string settingName = "LanguageCode";
		setting.Set(settingName, m_aLanguages[i]);
		GetGame().UserSettingsChanged();
		
		m_OnLanguageChanged.Invoke(this);
		
		SCR_AnalyticsApplication.GetInstance().ChangeSetting(m_sUserInterfaceSettings, settingName);
	}
}
