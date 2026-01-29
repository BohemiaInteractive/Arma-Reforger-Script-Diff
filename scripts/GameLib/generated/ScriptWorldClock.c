/*
===========================================
Do not modify, this script is generated
===========================================
*/

class ScriptWorldClock
{
	private void ScriptWorldClock();
	private void ~ScriptWorldClock();

	/*!
	Sets current time of the day as <0.0, 24.0> hours.
	Can only be set by the authority (server, singleplayer...).
	*/
	proto external void SetTimeOfDay(float hours24);
	/*!
	Returns current time of the day as <0.0, 24.0> hours.
	*/
	proto external float GetTimeOfDay();
	//! Sets the month in 1...12 range.
	proto external void SetMonth(int month_1_12);
	//! Gets the month in 1...12 range.
	proto external int GetMonth();
	//! Sets the year (0 = 0AD).
	proto external void SetYear(int year);
	//! Gets the year (0 = 0AD).
	proto external int GetYear();
	//! Sets the day of the month in 1...31 range.
	proto external void SetDay(int day_1_31);
	//! Gets the day of the month in 1...31 range.
	proto external int GetDay();
	//! Sets the day of the month in 1...371 range.
	proto external void SetDayInYear(int dayInYear_1_371);
	//! Gets the day of the month in 1...371 range.
	proto external int GetDayInYear();
}
