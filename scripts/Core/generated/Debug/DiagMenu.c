/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Debug
\{
*/

/*!
\brief Diagnostic and developer menu system.
\note Check EDiagMenu enum for 'id' values
\
 */
sealed class DiagMenu
{
	private void DiagMenu();
	private void ~DiagMenu();

	/*!
	\brief Registers a new menu.
	\param id Unique menu identifier.
	\param name Menu name.
	\param parent Parent menu name.
	*/
	static proto void RegisterMenu(int id, string name, string parent);
	/*!
	\brief Registers a new item in a menu.
	\param id Unique item identifier.
	\param shortcut Shortcut key(s), e.g. "lctrl+lshift+f"; supported keys: lctrl, rctrl, lshift, rshift, shift, lalt, ralt, a-z, 0-9, num0-num9, f1-f12.
	\param name Item name.
	\param parent Parent menu name.
	\param values List of possible values for the item.
	*/
	static proto void RegisterItem(int id, string shortcut, string name, string parent, string values);
	/*!
	\brief Unregisters a menu or item by id.
	\param id Identifier of the element to unregister.
	*/
	static proto void Unregister(int id);
	/*!
	\brief Registers a boolean setting.
	\param id Unique identifier.
	\param shortcut Shortcut key(s), e.g. "lctrl+lshift+f"; supported keys: lctrl, rctrl, lshift, rshift, shift, lalt, ralt, a-z, 0-9, num0-num9, f1-f12.
	\param name Display name for the boolean item.
	\param parent Parent menu name.
	\param reverse If true, the boolean sense is inverted.
	*/
	static proto void RegisterBool(int id, string shortcut, string name, string parent, bool reverse = false);
	/*!
	\brief Registers a range-setting menu item.
	\details Range value is defined as "min,max,startValue,step". E.g. "-2, 2, 0, 0.1"
	  creates a variable in range [-2,2], starting at 0, step size 0.1.
	\param id Unique identifier.
	\param shortcut Shortcut key(s), e.g. "lctrl+lshift+f"; supported keys: lctrl, rctrl, lshift, rshift, shift, lalt, ralt, a-z, 0-9, num0-num9, f1-f12.
	\param name Display name.
	\param parent Parent menu name.
	\param valuenames String representing the range parameters.
	*/
	static proto void RegisterRange(int id, string shortcut, string name, string parent, string valuenames);
	/*!
	\brief Gets the value of a boolean item.
	\param id Boolean item identifier.
	\param reverse If true, the value is logically inverted.
	\return Boolean value (true or false).
	*/
	static proto bool GetBool(int id, bool reverse = false);
	/*!
	\brief Gets the value of an integer menu item.
	\param id Item identifier.
	\return Integer value.
	*/
	static proto int GetValue(int id);
	/*!
	\brief Sets an integer value for an item.
	\param id Item identifier.
	\param value Value to set.
	*/
	static proto void SetValue(int id, int value);
	/*!
	\brief Gets the floating-point value for a range item.
	\param id Range item identifier.
	\return Current float value.
	*/
	static proto float GetRangeValue(int id);
	/*!
	\brief Sets the floating-point value for a range item.
	\param id Range item identifier.
	\param value Value to set.
	*/
	static proto void SetRangeValue(int id, float value);
	/*!
	\warning Only for debugging.
	\brief Sets an engine value directly.
	\param id Identifier.
	\param value Value to set.
	*/
	static proto void SetEngineValue(int id, int value);
	/*!
	\brief Gets an engine integer value.
	\param id Identifier.
	\return Integer value.
	*/
	static proto int GetEngineValue(int id);
	/*!
	\brief Sets an engine float value for a range item.
	\param id Identifier.
	\param value Value to set.
	*/
	static proto void SetEngineRangeValue(int id, float value);
	/*!
	\brief Gets an engine float value for a range item.
	\param id Identifier.
	\return Float value.
	*/
	static proto float GetEngineRangeValue(int id);
	/*!
	\brief Selects a menu by its name.
	\param name Menu name.
	*/
	static proto void SelectMenuByName(string name);
	/*!
	\brief Selects a menu by its id.
	\param id Menu identifier.
	*/
	static proto void SelectMenuById(int id);
}

/*!
\}
*/
