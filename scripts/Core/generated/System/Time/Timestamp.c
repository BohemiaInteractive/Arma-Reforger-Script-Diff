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
Real-wall-clock timestamp with millisecond precision, independent of world time.

Internally stored as milliseconds since the Unix epoch (midnight 1970-01-01 UTC).
Use \c Duration for arithmetic and \c DateTimeView for field access (year/month/day etc.).

\warning Timestamp represent system time, for in-game usage use WorldTimestamp which works with game time and never mix them.
*/
sealed class Timestamp: handle64
{
	//! Returns the current wall-clock time.
	static proto Timestamp Now();
	//! Creates a Timestamp from a Unix epoch value (seconds since 1970-01-01 UTC); milliseconds are set to zero.
	static proto Timestamp FromEpochSeconds(int s);
	/*!
	Parses an ISO 8601 UTC string of the form \c "2026-06-19T14:30:00.000Z" into a Timestamp.
	Returns a zero Timestamp if the string cannot be parsed.
	*/
	static proto Timestamp FromIsoString(string iso8601);
	//! Returns whole seconds since the Unix epoch (1970-01-01 UTC), truncating sub-second precision.
	proto external int ToEpochSeconds();
	//! Formats the timestamp as an ISO 8601 UTC string, e.g. \c "2026-06-19T14:30:00.000Z".
	proto external string ToIsoString();
	//! Returns a DateTimeView for this timestamp interpreted in UTC.
	proto external DateTimeView AtUtc();
	//! Returns a DateTimeView for this timestamp interpreted in the local system timezone.
	proto external DateTimeView AtLocal();
	/*!
	Returns a DateTimeView for this timestamp offset by the given number of minutes from UTC.
	\param zoneOffsetMinutes Signed offset from UTC in minutes (e.g. +60 for UTC+1).
	*/
	proto external DateTimeView AtZone(int zoneOffsetMinutes);
	proto external int Compare(Timestamp other);
	proto external bool Equals(Timestamp other);
	proto external bool Greater(Timestamp other);
	proto external bool GreaterEqual(Timestamp other);
	proto external bool Less(Timestamp other);
	proto external bool LessEqual(Timestamp other);
	proto external Timestamp Plus(Duration d);
	proto external Timestamp Minus(Duration d);
	//! Returns the Duration elapsed from \p earlier to this timestamp.
	proto external Duration Since(Timestamp earlier);
}

/*!
\}
*/
