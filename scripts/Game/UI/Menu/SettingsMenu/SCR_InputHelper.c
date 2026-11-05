class SCR_InputHelper
{
	static const string INPUT_CONTAINER_SOURCE_COMBO_TYPE = "InputSourceCombo";
	static const string INPUT_CONTAINER_FILTER_NAME = "Filter";
	static const string INPUT_CONTAINER_MULTIPLIER_NAME = "Multiplier";
	static const string INPUT_CONTAINER_OFFSET_NAME = "Offset";
	static const string INPUT_CONTAINER_INVERT_NAME = "UseInvert";
	static const string INPUT_CONTAINER_INPUT_SOURCE_NAME = "InputSource";
	static const string INPUT_CONTAINER_SOURCES_NAME = "Sources";
	static const string INPUT_CONTAINER_FILTER_PRESET_NAME = "FilterPreset";
	static const string INPUT_CONTAINER_INPUT_NAME = "Input";
	static const string INPUT_CONTAINER_THRESHOLD_NAME = "Threshold";
	static const string INPUT_CONTAINER_HOLD_DURATION_NAME = "HoldDuration";
	static const string INPUT_CONTAINER_CLICK_DURATION_NAME = "ClickDuration";
	static const string INPUT_DEVICE_CONTAINER_DEVICE_NAME = "DeviceNameContains";
	static const string INPUT_DEVICE_CONTAINER_DEVICE_SLOT_NAME = "DeviceSlotNumber";
	static const string INPUT_FILTER_VALUE_TYPE_NAME = "InputFilterValue";
	static const string DEVICE_ID_DISPLAY_FORMAT = "%1 %2";

	static const LocalizedString DEVICE_ID_DISPLAY_TEXT = "#AR-Settings_Joysticks_DeviceId";

	//------------------------------------------------------------------------------------------------
	//! Finds and returns input binding container
	//! \param[in] actionName for which input will be fetched
	//! \param[in] actionPreset filter preset which is used by the searched binding
	//! \param[in] inputId id of the binding which should be fetched, if inputId is below 0, then entire InputSource container is returned
	//! \param[in] binding cached version of user bindings handler
	//! \return container with matching binding, and if such is not found then null. In case of combo inputs it will return first valid InputSourceValue container
	static BaseContainer GetInputBinding(string actionName, string actionPreset, int inputId, InputBinding binding = null)
	{
		if (!binding)
		{
			binding = GetGame().GetInputManager().CreateUserBinding();
			if (!binding)
				return null;
		}

		BaseContainer actionContaier = binding.FindAction(actionName);
		if (!actionContaier)
			return null;

		BaseContainer inputSrc = actionContaier.GetObject(INPUT_CONTAINER_INPUT_SOURCE_NAME);
		if (!inputSrc)
			return null;

		if (inputId < 0)
			return inputSrc;

		BaseContainerList sources = inputSrc.GetObjectArray(INPUT_CONTAINER_SOURCES_NAME);
		if (!sources)
			return null;

		int count = sources.Count();
		if (count < 1)
			return null;

		string filter, input;
		BaseContainer source;
		BaseContainerList comboSources;
		BaseContainer comboSource;
		int matchingInputs = -1;
		for (int i; i < count; i++)
		{
			source = sources.Get(i);
			source.Get(INPUT_CONTAINER_FILTER_PRESET_NAME, filter);
			if (filter != actionPreset)
				continue;

			if (source.GetClassName() == INPUT_CONTAINER_SOURCE_COMBO_TYPE)
			{
				comboSources = source.GetObjectArray(INPUT_CONTAINER_SOURCES_NAME);
				if (!comboSources)
					continue;

				for (int j, comboCount = comboSources.Count(); j < comboCount; j++)
				{
					comboSource = comboSources.Get(j);
					if (!comboSource.Get(INPUT_CONTAINER_INPUT_NAME, input))
						continue;

					if (input.IsEmpty())
						continue;

					source = comboSource;
					break; // we dont support combos between different devices, thus first valid input binding is fine
				}
			}
			else
			{
				source.Get(INPUT_CONTAINER_INPUT_NAME, input);
				if (input.IsEmpty())
					continue;
			}

			matchingInputs++;
			if (matchingInputs == inputId)
				return source;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Translates device type to its prefix used for input bindings
	//! \param[in] device for which prefixes will be fetched
	//! \return array of strings that used by bindings of such devices
	static array<string> GetDeviceString(EInputDeviceType device)
	{
		switch (device)
		{
			case EInputDeviceType.GAMEPAD:
				return {UIConstants.DEVICE_GAMEPAD};

			case EInputDeviceType.JOYSTICK:
				return {UIConstants.DEVICE_JOYSTICK};

			case EInputDeviceType.TRACK_IR:
				return {UIConstants.DEVICE_TRACK_IR};
		}

		return {UIConstants.DEVICE_MOUSE, UIConstants.DEVICE_KEYBOARD};
	}
}