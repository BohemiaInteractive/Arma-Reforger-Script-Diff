/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Debug
\{
*/

//! Check EDiagMenu enum for 'id' values
sealed class DiagMenu
{
	private void DiagMenu();
	private void ~DiagMenu();

	static proto void RegisterMenu(int id, string name, string parent);
	static proto void RegisterItem(int id, string shortcut, string name, string parent, string values);
	static proto void Unregister(int id);
	static proto void RegisterBool(int id, string shortcut, string name, string parent, bool reverse = false);
	/*!
	Range value, which is defined in format "min,max,startValue,step". For example,
	"-2, 2, 0, 0.1" creates variable with value in range [-2,2], starting at 0 and
	changing in 0.1 increments.
	*/
	static proto void RegisterRange(int id, string shortcut, string name, string parent, string valuenames);
	static proto bool GetBool(int id, bool reverse = false);
	static proto int GetValue(int id);
	static proto void SetValue(int id, int value);
	static proto float GetRangeValue(int id);
	static proto void SetRangeValue(int id, float value);
	//! \warning Only for debugging.
	static proto void SetEngineValue(int id, int value);
	static proto int GetEngineValue(int id);
	static proto void SetEngineRangeValue(int id, float value);
	static proto float GetEngineRangeValue(int id);
	static proto void SelectMenuByName(string name);
	static proto void SelectMenuById(int id);
}

/*!
\}
*/
