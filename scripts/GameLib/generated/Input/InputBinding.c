/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Input
\{
*/

/*!
\brief Manages input key bindings, presets, contexts, and device calibration.
Provides functions for creating, resetting, querying, modifying, and calibrating input bindings for various device types and action presets.

\code
InputBinding binding = GetGame().GetInputManager().CreateUserBinding();
array<string> bindings = {};
binding.GetBindings("CamForward", bindings);
Print(bindings);
\endcode
*/
class InputBinding
{
	private void InputBinding();

	/*!
	\brief Saves all current bindings and settings to persistent storage.
	*/
	proto external void Save();
	/*!
	\brief Resets the binding for the specified action to its default value.
	\param actionName Action name to reset.
	\param deviceType (Optional) Input device type. Defaults to INVALID.
	\param preset     (Optional) Preset name. Defaults to empty string.
	*/
	proto external void ResetDefault(string actionName, EInputDeviceType deviceType = EInputDeviceType.INVALID, string preset = string.Empty);
	/*!
	\brief Checks if the binding for an action is currently set to its default value.
	\param actionName Action name to check.
	\param deviceType (Optional) Input device type. Defaults to KEYBOARD.
	\param preset     (Optional) Preset name. Defaults to empty string.
	\return True if binding is default, false otherwise.
	 */
	proto external bool IsDefault(string actionName, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty);
	/*!
	\brief Creates a new user-defined binding for the given action.
	\param actionName Name of the action.
	\param deviceType (Optional) Device type. Defaults to KEYBOARD.
	\param preset     (Optional) Preset name. Defaults to empty string.
	\return True if binding was created successfully, false otherwise.
	 */
	proto external bool CreateUserBinding(string actionName, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty);
	/*!
	\brief Gets the number of bindings for the given action and device type.
	\param actionName Name of the action.
	\param deviceType (Optional) Device type. Defaults to KEYBOARD.
	\param preset     (Optional) Preset name. Defaults to empty string.
	\return Number of bindings.
	 */
	proto external int GetBindingsCount(string actionName, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty);
	/*!
	\brief Removes a specific binding by its index for the given action, device, and preset.
	\param actionName  Target action name.
	\param deviceType  Target device type.
	\param preset      Target preset.
	\param keyBindIndex Index of the binding to be removed.
	 */
	proto external void RemoveBinding(string actionName, EInputDeviceType deviceType, string preset, int keyBindIndex);
	/*!
	\brief Adds a new binding for the specified action and preset.
	\param actionName Name of the action.
	\param preset     Name of the preset.
	\param keyBinding Keybinding string to add.
	\param filterName (Optional) Filter name to use. Defaults to empty string.
	 */
	proto external void AddBinding(string actionName, string preset, string keyBinding, string filterName = string.Empty);
	/*!
	\brief Inserts a key combination into the binding list.
	\param actionName   Name of the action.
	\param preset       Preset name.
	\param keyBinding   Keybinding string to insert.
	\param filterName   Name of the filter.
	\param keyBindIndex Index of the binding to modify.
	\param comboIndex   (Optional) Index to insert the combo. Defaults to -1 (append).
	 */
	proto external void InsertCombo(string actionName, string preset, string keyBinding, string filterName, int keyBindIndex, int comboIndex = -1);
	/*!
	\brief Sets the filter for a user binding.
	\param actionName   Name of the action.
	\param deviceType   Device type.
	\param preset       Preset name.
	\param keyBindIndex Index of the binding.
	\param filterName   Name of the filter to assign.
	\return True if successful, false otherwise.
	 */
	proto external bool SetFilter(string actionName, EInputDeviceType deviceType, string preset, int keyBindIndex, string filterName);
	/*!
	\brief Gets the filter name for the specified binding.
	\param actionName   Name of the action.
	\param deviceType   Device type.
	\param preset       Preset name.
	\param keyBindIndex Index of the binding.
	\return Filter name as string.
	 */
	proto external string GetFilter(string actionName, EInputDeviceType deviceType, string preset, int keyBindIndex);
	/*!
	\brief Starts capturing user input for binding purposes.

	After calling this method, InputBinding is listening for activated inputs. After input are pressed, capturing ends and state is returned back to EInputBindingCaptureState.IDLE \see GetCaptureState and caputer inputs are added as user key binding.

	\see CancelCapture
	\see SaveCapture

	\param actionName   Name of the action.
	\param deviceType   (Optional) Device type. Defaults to KEYBOARD.
	\param preset       (Optional) Preset name. Defaults to empty string.
	\param bAppend      (Optional) If true, appends new binding. Defaults to false.
	\param eAxleCapture (Optional) Method for capturing axes. Defaults to HALF_AXLE.
	 */
	proto external void StartCapture(string actionName, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty, bool bAppend = false, EInputBindingAxleCapture eAxleCapture = EInputBindingAxleCapture.HALF_AXLE);
	/*!
	\brief Cancels the current binding capture process.
	\see StartCapture
	 */
	proto external void CancelCapture();
	/*!
	\brief Immediately finish running input capture and save results.
	Additional key bindings can be added using additionalKeyBindings array.

	\see StartCapture

	\param additionalKeyBindings (Optional) Array of additional key bindings to save.
	 */
	proto external void SaveCapture(array<string> additionalKeyBindings = null);
	/*!
	\brief Returns the current state of an active binding capture operation.
	\see StartCapture

	\return Current capture state.
	 */
	proto external EInputBindingCaptureState GetCaptureState();
	/*!
	\brief Retrieves the custom binding configurations.
	\param[out] customConfigs Output array for custom configs.
	*/
	proto external void GetCustomConfigs(out notnull array<ResourceName> customConfigs);
	/*!
	\brief Sets custom configuration for key bindings.
	\param customConfigs Input array of custom configs.
	*/
	proto external void SetCustomConfigs(notnull array<ResourceName> customConfigs);
	/*!
	\brief Finds and returns the container for the specified context.
	\param contextName Name of the context to search for.
	\return BaseContainer for the context, or null if not found.
	*/
	proto external BaseContainer FindContext(string contextName);
	/*!
	\brief Finds and returns the container for the specified action.
	\param actionName Name of the action to search for.
	\return BaseContainer for the action, or null if not found.
	*/
	proto external BaseContainer FindAction(string actionName);
	/*!
	\brief Starts calibration process for the specified input device type.

	\see StopCalibration
	\see GetCalibrationStatus
	\see GetCalibrationData

	\param deviceType Device type to calibrate.
	*/
	proto external void StartCalibration(EInputDeviceType deviceType);
	/*!
	\brief Stops any running calibration process.
	*/
	proto external void StopCalibration();
	/*!
	\brief Gets the current status of the calibration process.
	\return Calibration status.
	*/
	proto external EInputBindingCalibrationStatus GetCalibrationStatus();
	/*!
	\brief Gets the min and max calibration values for the given data type.
	\param dataType Calibration data type.
	\param[out] min Output vector for minimum calibration.
	\param[out] max Output vector for maximum calibration.
	\return True if data is valid, false otherwise.
	*/
	proto external bool GetCalibrationData(EInputBindingCalibrationData dataType, out vector min, out vector max);
	/*!
	\brief Checks for key binding conflicts for a given action.

	Identifies if the specified action's key bindings conflict with other actions on the given input device type and preset.

	\param actionName                Name of the action to check.
	\param[out] keyBindIndices       Output array of indices of the conflicting key bindings.
	\param[out] conflictedActions    Output array of conflicted action names.
	\param[out] conflictedActionPresets Output array of conflicted action presets.
	\param deviceType                (Optional) Input device type to check. Defaults to KEYBOARD.
	\param preset                    (Optional) Name of the preset to check. Defaults to empty string.
	\return True if conflicts found, false otherwise.
	*/
	proto external bool GetConflicts(string actionName, out notnull array<int> keyBindIndices, out notnull array<string> conflictedActions, out notnull array<string> conflictedActionPresets, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string preset = string.Empty);
	/*!
	\brief Retrieves input bindings for a specified action and device type.

	Finds and collects input bindings matching the given action name, device type, and optional preset.

	\param actionName    Name of the input action.
	\param bindings      Array to store found binding strings.
	\param deviceType    Enum specifying the source device type.
	\param sPreset       Name of the binding preset to use.
	\param uiInputNames      (Optional) Whether to use user-friendly names. Defaults to true.
	\return True if bindings were found, false otherwise.
	\code
	InputBinding binding = GetGame().GetInputManager().CreateUserBinding();
	array<string> bindings = {};
	binding.GetBindings("CamForward", bindings, EInputDeviceType.KEYBOARD, string.Empty, false);
	foreach(string id: bindings)
	{
	 Print("KeyCode: " + binding.GetKeyCode(id));
	}

	binding.GetBindings("CamForward", bindings, EInputDeviceType.KEYBOARD, string.Empty, true);
	foreach(string name: bindings)
	{
	 Print("UI friendly key name: " + name);
	}
	\endcode
	*/
	proto external bool GetBindings(string actionName, out notnull array<string> bindings, EInputDeviceType deviceType = EInputDeviceType.KEYBOARD, string sPreset = string.Empty, bool uiInputNames = true);
	/*!
	\brief Retrieves all available input contexts.

	Populates an array with the names of all defined input contexts.

	\param[out] contextNames Output array to be populated with context names.
	*/
	proto external void GetContexts(out array<string> contextNames);
	/*!
	\brief Gets all preset names for a given action.

	Fills an array with the names of all presets in given action.

	\param actionName      Name of the input action.
	\param[out] presetNames Output array to hold the preset names.
	*/
	proto external void GetPresets(string actionName, out array<string> presetNames);
	/*!
	\brief Resolves an input name to its corresponding key code.

	Returns the KeyCode corresponding to the specified input name.

	\param inputName Name of the input/key.
	\return KeyCode for the given input name.
	*/
	proto external KeyCode GetKeyCode(string inputName);
}

/*!
\}
*/
