/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System_Time
\{
*/

/*!
An immutable snapshot of a calendar date and time with timezone offset.

Obtain instances via \c Timestamp::AtUtc(), \c Timestamp::AtLocal(), \c Timestamp::AtZone(),
or the \c Of() factory for constructing from individual calendar fields.
*/
sealed class DateTimeView: handle64
{
	//! Constructs a DateTimeView from individual calendar fields and a UTC offset in minutes.
	static proto DateTimeView Of(int year, int month, int day, int hour, int minute, int second, int millis, int zoneOffsetMinutes);
	//! Returns the calendar year (e.g. 2026).
	proto external int Year();
	//! Returns the month of year in the range 1-12.
	proto external int Month();
	//! Returns the day of month in the range 1-31.
	proto external int Day();
	//! Returns the hour of day in the range 0-23.
	proto external int Hour();
	//! Returns the minute of hour in the range 0-59.
	proto external int Minute();
	//! Returns the second of minute in the range 0-59.
	proto external int Second();
	//! Returns the millisecond of second in the range 0-999.
	proto external int Millis();
	//! Returns the ISO day-of-week: 1 = Monday, 7 = Sunday.
	proto external int DayOfWeek();
	//! Returns the day of year in the range 1-366.
	proto external int DayOfYear();
	//! Returns the timezone offset from UTC in minutes (signed).
	proto external int OffsetMinutes();
	//! Converts this view back to an absolute Timestamp (UTC milliseconds since Unix epoch).
	proto external Timestamp ToTimestamp();
	//! Returns an ISO 8601 string, e.g. "2026-06-19T14:30:00.000+02:00".
	proto external string ToIsoString();
}

/*!
\}
*/
