class SCR_ControllerPresetsSettingsSubmenu: SCR_SettingsSubMenuBase
{
	
	ref array<ref SCR_ControllerPreset> m_aJoystickPresets = {};
	
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
		
		EPlatform platform = System.GetPlatform();
		if (platform == EPlatform.PS4 || platform == EPlatform.PS5 || platform == EPlatform.PS5_PRO || platform == EPlatform.UNKNOWN)
		{
			HideMenuItem("Devices");
			HideMenuItem("JoystickPreset");
		} 
		else 
		{
			//joysticks are for now supported only non non-PS
			HandleJoystickDevices();
		}
		
		m_aSettingsBindings.Clear();

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
		if (!GetGame().GetInputManager().IsTrackIRConnected())
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
		
		SCR_AnalyticsApplication.GetInstance().ChangeSetting("Presets", "Controller Preset");
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
	protected void HandleJoystickDevices()
	{
		InputManager input = GetGame().GetInputManager();
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
		
		//for now we support up to 4 devices
		for (int i = 0; i <= 3 ; i++)
		{
			deviceName = input.GetJoystickProductName(i);
			if (deviceName.IsEmpty())
				continue;
			
			//handle joystic combos in here
			joystickWidget = GetGame().GetWorkspace().CreateWidgets("{D8E57BEFB1599946}UI/layouts/Menus/SettingsMenu/JoystickSettingEntry.layout", parent);
			joystickIDCombo = joystickWidget.FindAnyWidget("JoystickIDCombo");
			if (!joystickIDCombo)
				continue;
			
			component = SCR_ComboBoxComponent.Cast(joystickIDCombo.FindHandler(SCR_ComboBoxComponent));
			if (!component)
				continue;
			
			component.SetLabel(deviceName);
			component.SetCurrentItem(i);
			component.m_OnChanged.Insert(OnIndexChanged);
			
		}	
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
