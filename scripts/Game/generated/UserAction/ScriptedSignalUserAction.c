/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UserAction
\{
*/

//! This action will take care of the synchronization of the signal value
class ScriptedSignalUserAction: ScriptedUserAction
{
	proto external float GetMinimumValue();
	proto external float GetMaximumValue();
	proto external float GetCurrentValue();
	proto external void SetSignalValue(float newValue);

	// callbacks

	event override float GetActionProgressScript(float fProgress, float timeSlice) { return GetCurrentValue() - GetMinimumValue(); };
}

/*!
\}
*/
