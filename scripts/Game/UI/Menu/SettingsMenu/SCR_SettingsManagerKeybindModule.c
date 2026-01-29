//------------------------------------------------------------------------------------------------
class SCR_SettingsManagerKeybindModule : SCR_SettingsManagerModuleBase
{
	protected const string GAMEPAD_PRESET_PREFIX = "gamepad:";
	protected const string PRIMARY_PRESET_PREFIX = "";
	
	protected ref InputBinding m_Binding;
	
	protected ref SCR_KeyBindingMenuConfig m_KeybindConfig;

	// Duplicate configs mayy be found as active preset for multiple combo boxes.
	// Configs linked must be unique for each list to prevent it.
	protected ref SCR_ControllerPresetsConfig m_ControllerPresetsConfig;
	protected ref SCR_ControllerPresetsConfig m_JoystickPresetsConfig;

	protected static const ResourceName KEY_BINDING_CONFIG = "{4EE7794C9A3F11EF}Configs/System/keyBindingMenu.conf";
	protected static const ResourceName CONTROLLER_PRESETS_CONFIG = "{27780DD27C5E97CF}Configs/System/ControlSchemes/Gamepad/ControllerPresets.conf";
	protected static const ResourceName JOYSTICK_0_PRESETS_CONFIG = "{2823E147630F55BF}Configs/System/ControlSchemes/Joystick/Joystick0Presets.conf";
	protected static const ResourceName JOYSTICK_1_PRESETS_CONFIG = "{810AEDDD191484A0}Configs/System/ControlSchemes/Joystick/Joystick1Presets.conf";

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_SettingsManagerKeybindModule()
	{
		m_KeybindConfig = SCR_KeyBindingMenuConfig.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(KEY_BINDING_CONFIG, true));
		m_ControllerPresetsConfig = SCR_ControllerPresetsConfig.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(CONTROLLER_PRESETS_CONFIG, true));
		m_JoystickPresetsConfig = SCR_ControllerPresetsConfig.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(JOYSTICK_0_PRESETS_CONFIG, true));

		SetModuleType(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING);
		m_Binding = GetGame().GetInputManager().CreateUserBinding();
		if (!m_Binding)
			Print("SCR_SettingsManagerKeybindModule: InputBindings were not created!", LogLevel.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetActionBindCount(string actionName, string actionPreset = string.Empty, EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{
		if (!m_Binding)
			return 0;
	
		return m_Binding.GetBindingsCount(actionName, device, actionPreset);
	}
	
	//------------------------------------------------------------------------------------------------
	InputBinding GetInputBindings()
	{
		return m_Binding;
	}
	
	//------------------------------------------------------------------------------------------------
	void DeleteActionBindByIndex(string actionName, int keybindIndex, string actionPreset = "", EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{
		if (!m_Binding)
			return;
		
		if (m_Binding.IsDefault(actionName, device, actionPreset))
		{
			// if it is default binding first create user binding
			m_Binding.CreateUserBinding(actionName, device, actionPreset);
			m_Binding.Save();
		}
		
		m_Binding.RemoveBinding(actionName, device, actionPreset, keybindIndex);
		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	void StartCaptureForAction(string actionName, string actionPreset, EInputDeviceType device = EInputDeviceType.KEYBOARD, bool append = false)
	{
		if (!m_Binding)
			return;

		m_Binding.StartCapture(actionName, device, actionPreset, append);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFilterForActionByIndex(string actionName, string actionPreset, int filterIndex, int keybindIndex, SCR_EActionPrefixType prefixType, EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{
		if (!m_Binding)
			return;
		
		array<ref SCR_KeyBindingFilter> filters = GetFilters(prefixType);
		if (!filters)
			return;
		
		SCR_KeyBindingFilter filter = filters.Get(filterIndex);
		if (!filter)
			return;
		
		if (m_Binding.IsDefault(actionName, device, actionPreset))
		{
			// if it is default binding first create user binding
			m_Binding.CreateUserBinding(actionName, device, actionPreset);
			m_Binding.Save();
		}
		
		m_Binding.SetFilter(actionName, device, actionPreset, keybindIndex, filter.m_sFilterString);
		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	void AddKeybindToActionByIndex(string actionName, string preset, int bindIndex, string filterName = string.Empty)
	{
		if (!m_Binding)
			return;
		
		array<ref SCR_KeyBindingBind> binds = GetCustomBinds();
		if (!binds)
			return;
		
		SCR_KeyBindingBind bind = binds.Get(bindIndex);
		if (!bind)
			return;
		
		m_Binding.AddBinding(actionName, preset, bind.m_sBindString, filterName);
		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_KeyBindingFilter> GetFilters(SCR_EActionPrefixType filterType)
	{
		if (!m_KeybindConfig)
			return null;
		
		array <ref SCR_KeyBindingFilter> foundFilters = {};
		foreach (SCR_KeyBindingFilter filter : m_KeybindConfig.m_aInputFilters)
		{
			if (filterType == SCR_EActionPrefixType.AMBIGUOUS || filter.GetFilterType() == filterType)
				foundFilters.Insert(filter);
		}
		
		return foundFilters;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_KeyBindingBind> GetCustomBinds()
	{
		if (!m_KeybindConfig)
			return null;
		
		return m_KeybindConfig.m_aInputBinds;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_KeyBindingCombo> GetCustomComboKeys()
	{
		if (!m_KeybindConfig)
			return null;
		
		return m_KeybindConfig.m_aComboKeys;
	}
	
	//------------------------------------------------------------------------------------------------
	//! returns false if the action has no conflicts (no other action in keybind settings with same bind), otherwise returns those actions in confirmedConflicts
	bool IsActionConflicted(string actionName, notnull out array<SCR_KeyBindingEntry> confirmedConflicts, int bindIndex, string preset = string.Empty)
	{
		if (!m_Binding || actionName.IsEmpty())
			return false;
		
		confirmedConflicts.Clear();
		
		array<int> indices = {};
		array<string> conflicts = {};
		array<string> presets = {};
		m_Binding.GetConflicts(actionName, indices, conflicts, presets, EInputDeviceType.KEYBOARD, preset);
		
		if (conflicts.IsEmpty())
			return false;
		
		array<SCR_KeyBindingEntry> foundActions = {};
		
		//compare the actions against those configures in keybinding config, and save only those, to avoid actions for UI etc
		foreach (int index, string action : conflicts)
		{
			if (indices[index] != bindIndex)
				continue;
			
			foreach (SCR_KeyBindingCategory category : m_KeybindConfig.m_KeyBindingCategories)
			{
				foreach (SCR_KeyBindingEntry entry : category.m_KeyBindingEntries)
				{
					if (entry.m_sActionName == "separator" || entry.m_sActionName != action || entry.m_sPreset != presets[index]) 
						continue;
					
					foundActions.Insert(entry);
				}
			}
		}
				
		confirmedConflicts.Copy(foundActions);
		
		return !foundActions.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	void UnbindAction(string actionName, string preset = "", EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{	
		m_Binding.StartCapture(actionName, device, preset, false);
		m_Binding.SaveCapture();
		m_Binding.Save();
		SCR_AnalyticsApplication.GetInstance().ChangeKeybind(actionName, preset);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddComboToActionByIndex(string actionName, string preset, int comboIndex, int keybindIndex, string filterName = string.Empty, int comboPosition = 0)
	{
		if (!m_Binding)
			return;
		
		array<ref SCR_KeyBindingCombo> combos = GetCustomComboKeys();
		if (!combos)
			return;
		
		SCR_KeyBindingCombo combo = combos.Get(comboIndex);
		if (!combo)
			return;
		
		m_Binding.InsertCombo(actionName, preset, combo.m_sComboString, filterName, keybindIndex, comboPosition);
		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetAction(string actionName, string preset = "", EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{
		if (!m_Binding)
			return;

		bool reset;
		switch (device)
		{
			case EInputDeviceType.KEYBOARD:
			case EInputDeviceType.MOUSE:
			{
				if (!m_Binding.IsDefault(actionName, EInputDeviceType.KEYBOARD, preset))
				{
					m_Binding.ResetDefault(actionName, EInputDeviceType.KEYBOARD, preset);
					reset = true;
				}

				if (!m_Binding.IsDefault(actionName, EInputDeviceType.MOUSE, preset))
				{
					m_Binding.ResetDefault(actionName, EInputDeviceType.MOUSE, preset);
					reset = true;
				}

				break;
			}
			case EInputDeviceType.GAMEPAD:
			{
				if (!m_Binding.IsDefault(actionName, EInputDeviceType.GAMEPAD, preset))
				{
					m_Binding.ResetDefault(actionName, EInputDeviceType.GAMEPAD, preset);
					reset = true;
				}

				break;
			}
		}
		
		if (!reset)
			return;

		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetAllActions(EInputDeviceType device = EInputDeviceType.KEYBOARD)
	{
		array<string> contexts = {};
		m_Binding.GetContexts(contexts);
		string finalPreset;
		string devicePrefix;
		string actionName;

		foreach (SCR_KeyBindingCategory category: m_KeybindConfig.m_KeyBindingCategories)
		{
			foreach (SCR_KeyBindingEntry entry: category.m_KeyBindingEntries)
			{
				if (device == EInputDeviceType.GAMEPAD)
					devicePrefix = GetGamepadPresetPrefix();
				else 
					devicePrefix = GetPrimaryPresetPrefix();
				
				if (device == EInputDeviceType.GAMEPAD && !entry.m_sPresetGamepadOptional.IsEmpty())
					finalPreset = devicePrefix + entry.m_sPresetGamepadOptional;
				else if (!entry.m_sPreset.IsEmpty())
					finalPreset = devicePrefix + entry.m_sPreset;
				
				if (device == EInputDeviceType.GAMEPAD && !entry.m_sActionNameGamepadOptional.IsEmpty())
					actionName = entry.m_sActionNameGamepadOptional;
				else
					actionName = entry.m_sActionName;

				ResetAction(actionName, finalPreset, device);
			}
		}
		
		m_Binding.Save();
		SCR_AnalyticsApplication.GetInstance().ResetAllKeybinds();
	}
	
	//------------------------------------------------------------------------------------------------
	string GetGamepadPresetPrefix()
	{
		return GAMEPAD_PRESET_PREFIX;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetPrimaryPresetPrefix()
	{
		return PRIMARY_PRESET_PREFIX;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get all preset lists defined in this module
	//! \param[out] controllerPresets
	//! \param[out] joystick0Presets
	//! \param[out] joystick1Presets
	void GetControllerPresets(notnull out array<ref SCR_ControllerPreset> controllerPresets)
	{
		if (m_ControllerPresetsConfig)
			controllerPresets = m_ControllerPresetsConfig.GetPresets();
		else
			Print("SCR_SettingsManagerKeybindModule: Controller presets not present, check init process!", LogLevel.ERROR);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Get all preset lists defined for joysticks
	//todo: add so this returns presets loaded from profile folder
	void GetJoystickPresets(notnull out array<ref SCR_ControllerPreset> joystickPresets)
	{
		if (m_JoystickPresetsConfig)
			joystickPresets = m_JoystickPresetsConfig.GetPresets();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Combines selected presets and feeds it to SetCustomConfigs
	//! \param[in] presetIndex
	void SelectControllerPresets(int presetIndex = -1)
	{
		if (!m_Binding)
			return;

		array<ref SCR_ControllerPreset> controllerPresets = {};
		GetControllerPresets(controllerPresets);

		//empty preset config means we set it to default
		array<ResourceName> presets = {};
		SCR_ControllerPreset preset;
		int index;

		// Controller
		if (controllerPresets.IsIndexValid(presetIndex))
			index = presetIndex;
		else
			index = GetActivePresetIndex(controllerPresets);

		if (controllerPresets.IsIndexValid(index))
		{
			preset = controllerPresets.Get(index);
			if (preset && !preset.GetResourceName().IsEmpty())
				presets.Insert(preset.GetResourceName());
		}

		m_Binding.SetCustomConfigs(presets);
		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Combines selected presets and feeds it to SetCustomConfigs
	//! \param[in] presetIndex
	void SelectJoystickPreset(int presetIndex = -1)
	{
		if (!m_Binding)
			return;

		array<ref SCR_ControllerPreset> joystickPresets = {};
		GetJoystickPresets(joystickPresets);

		//empty preset config means we set it to default
		array<ResourceName> presets = {};
		SCR_ControllerPreset preset;
		int index;

		// Controller
		if (joystickPresets.IsIndexValid(presetIndex))
			index = presetIndex;
		else
			index = GetActivePresetIndex(joystickPresets);

		if (joystickPresets.IsIndexValid(index))
		{
			preset = joystickPresets.Get(index);
			if (preset && !preset.GetResourceName().IsEmpty())
				presets.Insert(preset.GetResourceName());
		}

		m_Binding.SetCustomConfigs(presets);
		m_Binding.Save();
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Combines selected presets and feeds it to SetCustomConfigs
	//! \param[in] presetIndex
	void SelectJoystickPresetPath(string path)
	{
		if (!m_Binding)
			return;

		array<ref SCR_ControllerPreset> joystickPresets = {};
		GetJoystickPresets(joystickPresets);

		//empty preset config means we set it to default
		array<ResourceName> presets = {};
		presets.Insert(path);

		m_Binding.SetCustomConfigs(presets);
		m_Binding.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find index of active preset from provided controller preset list
	//! \param[in] presets
	//! \return
	int GetActivePresetIndex(notnull array<ref SCR_ControllerPreset> presets)
	{
		if (!m_Binding)
			return -1;
		
		array<ResourceName> setPresets = {};
		m_Binding.GetCustomConfigs(setPresets);
		string path;
		foreach (int i, SCR_ControllerPreset preset : presets)
		{
			if (setPresets.Contains(preset.GetResourceName()))
				return i;
		}
		
		return -1;
	}
}