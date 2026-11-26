class SCR_ControllerPresetsSettingsSubmenu: SCR_SettingsSubMenuBase
{
	ref array<ref SCR_ControllerPreset> m_aJoystickPresets = {};

	protected const ResourceName JOYSTICK_ID_COMBO_LAYOUT = "{D8E57BEFB1599946}UI/layouts/Menus/SettingsMenu/JoystickSettingEntry.layout";
	protected const ResourceName JSEPARATOR_WIDGET = "{D379EAA437ED9CED}UI/layouts/Menus/PauseMenu/PauseMenuSeparator.layout";

	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();

		SCR_HeadTrackingSettings.SetHeadTrackingSettings();
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		m_aSettingsBindings.Clear();

		HandleJoystickDevices();
		HandleControllerDeadZones();

		// HEAD TRACKING
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_bHeadTrackingFreelook", "HeadTrackingFreelook"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_bHeadTrackingADS", "HeadTrackingADS"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingSensitivity", "HeadTrackingSensitivity"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingADSDeadzone", "HeadTrackingADSDeadzone"));

		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_eHeadTrackingLean", "HeadTrackingLean"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingLeanSensitivity", "HeadTrackingLeanSensitivity"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingLeanDeadzone", "HeadTrackingLeanDeadzone"));
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_HeadTrackingSettings", "m_fHeadTrackingLeanYawLimit", "HeadTrackingLeanYawLimit"));

		LoadSettings();

		// PRESETS
		array<ref SCR_ControllerPreset> controllerPresets = {};
		array<ref SCR_ControllerPreset> joystick0Presets = {};

		SCR_SettingsManagerKeybindModule keybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (keybindModule)
			keybindModule.GetControllerPresets(controllerPresets);

		SCR_ComboBoxComponent combo;
		combo = BindControllerPresets("Presets", controllerPresets);
		if (combo)
			combo.m_OnChanged.Insert(SelectControllerPreset);

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_UI_SHOW_ALL_SETTINGS, false))
			return;

		// Show only if there is TrackIR connected
		if (!GetGame().GetInputManager().GetTrackIRDeviceHandler().IsTrackIRConnected())
		{
			HideMenuItem("TitleHeadTracking");

			HideMenuItem("HeadTrackingFreelook");
			HideMenuItem("HeadTrackingADS");
			HideMenuItem("HeadTrackingSensitivity");
			HideMenuItem("HeadTrackingADSDeadzone");

			HideMenuItem("HeadTrackingLean");
			HideMenuItem("HeadTrackingLeanSensitivity");
			HideMenuItem("HeadTrackingLeanDeadzone");
			HideMenuItem("HeadTrackingLeanYawLimit");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] widgetName
	//! \param[in] presets
	//! \return
	SCR_ComboBoxComponent BindControllerPresets(string widgetName, notnull array<ref SCR_ControllerPreset> presets)
	{
		Widget presetsWidget = m_wRoot.FindAnyWidget(widgetName);
		if (!presetsWidget)
			return null;

		SCR_SettingsManagerKeybindModule keybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (!keybindModule)
			return null;

		SCR_ComboBoxComponent comboComp = SCR_ComboBoxComponent.Cast(presetsWidget.FindHandler(SCR_ComboBoxComponent));
		if (!comboComp)
			return null;

		foreach (SCR_ControllerPreset preset : presets)
		{
			comboComp.AddItem(preset.GetDisplayName());
		}

		comboComp.SetCurrentItem(keybindModule.GetActivePresetIndex(presets));
		return comboComp;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] comp
	//! \param[in] index
	void SelectControllerPreset(SCR_ComboBoxComponent comp, int index)
	{
		SCR_SettingsManagerKeybindModule keybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (keybindModule)
			keybindModule.SelectControllerPresets(presetIndex: index);
		
		SCR_AnalyticsApplication analytics = SCR_AnalyticsApplication.GetInstance();
		if (analytics)
			analytics.ChangeSetting("Presets", "Controller Preset");
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] comp
	//! \param[in] index
	void SelectJoystickPreset(SCR_ComboBoxComponent comp, int index)
	{
		SCR_SettingsManagerKeybindModule keybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (!keybindModule)
			return;
		
		SCR_ControllerPreset preset = m_aJoystickPresets.Get(index);
		if (!preset)
			return;
		
		keybindModule.SelectJoystickPresetPath(preset.GetResourceName());
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleControllerDeadZones()
	{
		Widget parent = m_wRoot.FindAnyWidget("TitleControllers");
		if (!parent)
			return;

		BaseContainer gamepadProfile = SCR_InputHelper.GetGamepadProfileContainer();
		if (!gamepadProfile)
		{
			HideMenuItem("TitleControllers");
			return;
		}

		// Left thumb horizontal axis
		BaseContainer entry = gamepadProfile.GetObject("LeftThumbHorizontal");
		float deadZoneValue;
		entry.Get("DeadZone", deadZoneValue);
		InitializeAxisDeadZoneSlider(parent, "ControllerCenterDeadzoneLeftStickX", deadZoneValue).Insert(OnLeftXCenterDeadZoneSliderChangedFinal);

		TVectorArray curve = SCR_InputHelper.GetInputCurve(entry);
		deadZoneValue = 0;
		if (curve && !curve.IsEmpty())
			deadZoneValue = 1 - curve[curve.Count() - 1][0];

		InitializeAxisDeadZoneSlider(parent, "ControllerEdgeDeadzoneLeftStickX", deadZoneValue).Insert(OnLeftXEdgeDeadZoneSliderChangedFinal);


		// Left thumb vertical axis
		entry = gamepadProfile.GetObject("LeftThumbVertical");
		entry.Get("DeadZone", deadZoneValue);
		InitializeAxisDeadZoneSlider(parent, "ControllerCenterDeadzoneLeftStickY", deadZoneValue).Insert(OnLeftYCenterDeadZoneSliderChangedFinal);
		
		curve = SCR_InputHelper.GetInputCurve(entry);
		deadZoneValue = 0;
		if (curve && !curve.IsEmpty())
			deadZoneValue = 1 - curve[curve.Count() - 1][0];

		InitializeAxisDeadZoneSlider(parent, "ControllerEdgeDeadzoneLeftStickY", deadZoneValue).Insert(OnLeftYEdgeDeadZoneSliderChangedFinal);


		// Right thumb horizontal axis
		entry = gamepadProfile.GetObject("RightThumbHorizontal");
		entry.Get("DeadZone", deadZoneValue);
		InitializeAxisDeadZoneSlider(parent, "ControllerCenterDeadzoneRightStickX", deadZoneValue).Insert(OnRightXCenterDeadZoneSliderChangedFinal);
		
		curve = SCR_InputHelper.GetInputCurve(entry);
		deadZoneValue = 0;
		if (curve && !curve.IsEmpty())
			deadZoneValue = 1 - curve[curve.Count() - 1][0];

		InitializeAxisDeadZoneSlider(parent, "ControllerEdgeDeadzoneRightStickX", deadZoneValue).Insert(OnRightXEdgeDeadZoneSliderChangedFinal);


		// Right thumb vertical axis
		entry = gamepadProfile.GetObject("RightThumbVertical");
		entry.Get("DeadZone", deadZoneValue);
		InitializeAxisDeadZoneSlider(parent, "ControllerCenterDeadzoneRightStickY", deadZoneValue).Insert(OnRightYCenterDeadZoneSliderChangedFinal);

		curve = SCR_InputHelper.GetInputCurve(entry);
		deadZoneValue = 0;
		if (curve && !curve.IsEmpty())
			deadZoneValue = 1 - curve[curve.Count() - 1][0];

		InitializeAxisDeadZoneSlider(parent, "ControllerEdgeDeadzoneRightStickY", deadZoneValue).Insert(OnRightYEdgeDeadZoneSliderChangedFinal);
	}

	//------------------------------------------------------------------------------------------------
	protected ScriptInvoker InitializeAxisDeadZoneSlider(notnull Widget parent, string widgetName, float sliderValue)
	{
		Widget entryWidget = parent.FindAnyWidget(widgetName);
		SCR_SliderComponent entryComp = SCR_SliderComponent.Cast(entryWidget.FindHandler(SCR_SliderComponent));
		entryComp.SetValue(sliderValue, false);
		SCR_AxisCurveGraphDescriptionComponent graphComp = SCR_AxisCurveGraphDescriptionComponent.Cast(entryWidget.FindHandler(SCR_AxisCurveGraphDescriptionComponent));
		if (graphComp)
			graphComp.Init(entryWidget);

		return entryComp.GetOnChangedFinal();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLeftXCenterDeadZoneSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		BaseContainer profileContainer;
		if (!SetAxisDeadZone("LeftThumbHorizontal", value, profileContainer))
			return;

		if (profileContainer)
			UpdateDesriptionGraph(comp, profileContainer);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLeftYCenterDeadZoneSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		BaseContainer profileContainer;
		if (!SetAxisDeadZone("LeftThumbVertical", value, profileContainer))
			return;

		if (profileContainer)
			UpdateDesriptionGraph(comp, profileContainer);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRightXCenterDeadZoneSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		BaseContainer profileContainer;
		if (!SetAxisDeadZone("RightThumbHorizontal", value, profileContainer))
			return;

		if (profileContainer)
			UpdateDesriptionGraph(comp, profileContainer);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRightYCenterDeadZoneSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		BaseContainer profileContainer;
		if (!SetAxisDeadZone("RightThumbVertical", value, profileContainer))
			return;

		if (profileContainer)
			UpdateDesriptionGraph(comp, profileContainer);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDesriptionGraph(notnull SCR_SliderComponent comp, notnull BaseContainer profileContainer)
	{
		SCR_AxisCurveGraphDescriptionComponent graphDrawingComp = SCR_AxisCurveGraphDescriptionComponent.Cast(comp.GetRootWidget().FindHandler(SCR_AxisCurveGraphDescriptionComponent));
		if (!graphDrawingComp)
			return;

		graphDrawingComp.RequestGraphUpdate(profileContainer);
	}

	//------------------------------------------------------------------------------------------------
	protected bool SetAxisDeadZone(string profileName, float deadZoneValue, out BaseContainer profileContainer = null)
	{
		BaseContainer gamepadProfile = SCR_InputHelper.GetGamepadProfileContainer();
		if (!gamepadProfile)
		{
			HideMenuItem("TitleControllers");
			return false;
		}

		deadZoneValue = Math.Min(0.999, deadZoneValue);
		profileContainer = gamepadProfile.GetObject(profileName);

		float oldValue;
		profileContainer.Get("DeadZone", oldValue);
		if (oldValue == deadZoneValue)
			return false;

		profileContainer.Set("DeadZone", deadZoneValue);
		gamepadProfile.SetObject(profileName, profileContainer);
		SCR_BaseContainerTools.CleanContainerData(gamepadProfile);
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLeftYEdgeDeadZoneSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		BaseContainer profileContainer;
		if (!SetCurveDeadZone("LeftThumbVertical", value, profileContainer))
			return;

		if (profileContainer)
			UpdateDesriptionGraph(comp, profileContainer);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLeftXEdgeDeadZoneSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		BaseContainer profileContainer;
		if (!SetCurveDeadZone("LeftThumbHorizontal", value, profileContainer))
			return;

		if (profileContainer)
			UpdateDesriptionGraph(comp, profileContainer);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRightYEdgeDeadZoneSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		BaseContainer profileContainer;
		if (!SetCurveDeadZone("RightThumbVertical", value, profileContainer))
			return;

		if (profileContainer)
			UpdateDesriptionGraph(comp, profileContainer);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRightXEdgeDeadZoneSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		BaseContainer profileContainer;
		if (!SetCurveDeadZone("RightThumbHorizontal", value, profileContainer))
			return;

		if (profileContainer)
			UpdateDesriptionGraph(comp, profileContainer);
	}

	//------------------------------------------------------------------------------------------------
	protected bool SetCurveDeadZone(string profileName, float deadZoneValue, out BaseContainer profileContainer = null)
	{
		BaseContainer gamepadProfile = SCR_InputHelper.GetGamepadProfileContainer();
		if (!gamepadProfile)
		{
			HideMenuItem("TitleControllers");
			return false;
		}

		profileContainer = gamepadProfile.GetObject(profileName);
		if (!profileContainer)
			return false;

		if (!SCR_InputHelper.SetCurveDeadZone(profileContainer, deadZoneValue))
			return false;

		gamepadProfile.SetObject(profileName, profileContainer);
		SCR_BaseContainerTools.CleanContainerData(gamepadProfile);
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleJoystickDevices()
	{
		InputManager inputManager = GetGame().GetInputManager();
		Widget parent = m_wRoot.FindAnyWidget("Content");
		string deviceName;
		Widget joystickWidget;
		Widget joystickIDCombo;
		SCR_ComboBoxComponent component;
		if (!parent)
			return;

		Widget joystickPreset = m_wRoot.FindAnyWidget("JoystickPreset");
		if (!joystickPreset)
			return;
		
		component = SCR_ComboBoxComponent.Cast(joystickPreset.FindHandler(SCR_ComboBoxComponent));
		if (!component)
			return;
		
		SetupJoystickPresets(component);
		int numberOfDevices;

		const bool isConsole = !SCR_SystemHelper.IsPC();
		map<string, int> previouslySavedDevicesMap = GetSavedDevices();
		bool deviceState[4];

		const JoystickDeviceHandler joystickHandler = inputManager.GetJoystickDeviceHandler();
		//for now we support up to 4 devices
		for (int i = 0; i <= 3 ; i++)
		{
			deviceState[i] = joystickHandler.IsJoystickConnected(i);
			if (!deviceState[i])
				continue;

			if (isConsole)
			{
				numberOfDevices++;
				continue; // JoystickDeviceHandler.GetJoystickProductName is not supported on consoles thus we cannot uniquely identify them, and because of that we only care about the number of connected devices
			}

			deviceName = joystickHandler.GetJoystickProductName(i);
			if (deviceName.IsEmpty())
				continue;
			
			//handle joystic combos in here
			joystickWidget = GetGame().GetWorkspace().CreateWidgets(JOYSTICK_ID_COMBO_LAYOUT, parent);
			joystickIDCombo = joystickWidget.FindAnyWidget("JoystickIDCombo");
			if (!joystickIDCombo)
				continue;
			
			component = SCR_ComboBoxComponent.Cast(joystickIDCombo.FindHandler(SCR_ComboBoxComponent));
			if (!component)
				continue;
			
			numberOfDevices++;
			component.SetLabel(deviceName);
			int savedId;
			if (!previouslySavedDevicesMap.Find(deviceName, savedId))
				savedId = i;

			component.SetCurrentItem(savedId);
			component.m_OnChanged.Insert(OnIndexChanged);
		}	

		if (numberOfDevices < 1)
		{
			HideMenuItem("Devices");
			HideMenuItem("JoystickPreset");
			return;
		}

		if (isConsole)
		{
			HideMenuItem("Devices");
			return;
		}

		BaseContainer controllerSettings = GetGame().GetGameUserSettings().GetModule("SCR_ControllerSettings");
		if (!controllerSettings)
			return;

		GetGame().GetWorkspace().CreateWidgets(JSEPARATOR_WIDGET, parent);
		joystickWidget = GetGame().GetWorkspace().CreateWidgets("{4B5AE6E64037FFB4}UI/layouts/WidgetLibrary/ComboBox/WLib_ComboBox.layout", parent);
		if (!joystickWidget)
			return;

		joystickWidget.SetName("FFBDeviceId");
		component = SCR_ComboBoxComponent.Cast(joystickWidget.FindHandler(SCR_ComboBoxComponent));
		if (!component)
			return;

		component.SetLabel("#AR-Settings_Controllers_ForceFeedbackDevice");
		foreach (int id, bool devicePresenceState : deviceState)
		{
			string entryName = id.ToString();
			if (isConsole && !devicePresenceState)
				entryName = string.Format("%1 (%2)", id, "#AR-GamepadRemovalDialog_Header");

			component.AddItem(entryName, true);
		}

		int savedDeviceId;
		int sessionDeviceId = GamepadEffect.GetDefaultJoystickDeviceIndex();
		if (controllerSettings.Get("m_iForceFeedbackDeviceId", savedDeviceId) && savedDeviceId != sessionDeviceId)
			GamepadEffect.SetDefaultJoystickDeviceIndex(savedDeviceId);

		component.SetCurrentItem(savedDeviceId, false, false, false);
		m_aSettingsBindings.Insert(new SCR_SettingBindingGameplay("SCR_ControllerSettings", "m_iForceFeedbackDeviceId", "FFBDeviceId"));
		component.m_OnChanged.Insert(OnForceFeedbackIdChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnForceFeedbackIdChanged(SCR_ComboBoxComponent component, int index)
	{
		GamepadEffect.SetDefaultJoystickDeviceIndex(index);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnIndexChanged(SCR_ComboBoxComponent component, int index)
	{
		//this is safe to do because we do not translate device names, we use exact strings from the devices so we can rely on it
		string deviceName = component.GetLabel().GetText();
		if (deviceName.IsEmpty())
			return;
		
		BaseContainer inputDeviceSettings = GetGame().GetEngineUserSettings().GetModule("InputDeviceUserSettings");
		BaseContainer joystickSetting = inputDeviceSettings.GetObject("JoystickSettings");
		if (!joystickSetting)
			return;
		
		BaseContainerList rules = joystickSetting.SetObjectArray("DeviceOrderRules");
		if (!rules)
			return;
		
		BaseContainer container;				
		for (int i; i < rules.Count(); i++)
		{
			BaseContainer foundContainer = rules.Get(i);
			string name;
			if (foundContainer.Get("DeviceNameContains", name) && name == deviceName)
			{
				container = foundContainer;
				break;
			}
		} 
		
		if (!container)
		{	
			Resource holder = BaseContainerTools.CreateContainer("JoystickOrderRule");
			if (holder)
				container = holder.GetResource().ToBaseContainer();
			
			rules.Insert(container);
		}
		
		container.Set("DeviceNameContains", deviceName);
		container.Set("DeviceSlotNumber", index);
		
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
	}
	
	//------------------------------------------------------------------------------------------------
	protected map<string, int> GetSavedDevices()
	{
		map<string, int> savedDevices = new map<string, int>();
		BaseContainer inputDeviceSettings = GetGame().GetEngineUserSettings().GetModule("InputDeviceUserSettings");
		BaseContainer joystickSetting = inputDeviceSettings.GetObject("JoystickSettings");
		if (!joystickSetting)
			return savedDevices;

		BaseContainerList rules = joystickSetting.SetObjectArray("DeviceOrderRules");
		if (!rules)
			return savedDevices;

		BaseContainer container;
		for (int i; i < rules.Count(); i++)
		{
			container = rules.Get(i);
			string name;
			if (!container.Get("DeviceNameContains", name))
				continue;

			int deviceSlotId;
			if (!container.Get("DeviceSlotNumber", deviceSlotId))
				continue;

			savedDevices.Set(name, deviceSlotId);
		}

		return savedDevices;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] comp
	protected void SetupJoystickPresets(SCR_ComboBoxComponent component)
	{
		SCR_SettingsManagerKeybindModule keybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (!keybindModule)
			return;
		
		m_aJoystickPresets.Clear();
		keybindModule.GetJoystickPresets(m_aJoystickPresets);
		
		// load custom configs from profile folder
		array<string> userConfigs = {};
		FileIO.FindFiles(userConfigs.Insert, "$profile:.save/settings/customInputConfigs", ".conf");
		foreach(string config : userConfigs)
		{
			bool found = false;
			foreach (SCR_ControllerPreset preset : m_aJoystickPresets)
			{
				if (preset.GetResourceName() == config)
				{
					found = true;
					break;
				}
			}
			
			if (found)
				continue;
			
			SCR_ControllerPreset customPreset = new SCR_ControllerPreset();
			customPreset.SetResourceName(config);
			customPreset.SetDisplayName(FilePath.StripPath(config));
			m_aJoystickPresets.Insert(customPreset);
		}
		
		foreach (SCR_ControllerPreset preset : m_aJoystickPresets)
		{
			component.AddItem(preset.GetDisplayName());
		}
		
		component.SetCurrentItem(keybindModule.GetActivePresetIndex(m_aJoystickPresets));
		component.m_OnChanged.Insert(SelectJoystickPreset);
	}
}
