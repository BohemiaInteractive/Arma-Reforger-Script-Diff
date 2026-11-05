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
Signed duration with millisecond precision.

Use the factory methods (\c OfMillis, \c OfSeconds, etc.) to create values,
and the arithmetic methods (\c Plus, \c Minus, \c MultipliedBy, \c DividedBy) to combine them.
*/
sealed class Duration: handle64
{
	//! Creates a Duration from a millisecond count.
	static proto Duration OfMillis(int ms);
	//! Creates a Duration from a whole-second count.
	static proto Duration OfSeconds(int s);
	//! Creates a Duration from a whole-minute count.
	static proto Duration OfMinutes(int m);
	//! Creates a Duration from a whole-hour count.
	static proto Duration OfHours(int h);
	//! Creates a Duration from a whole-day count (1 day = 24 hours).
	static proto Duration OfDays(int d);
	//! Returns the Duration elapsed from \p a to \p b (may be negative).
	static proto Duration Between(Timestamp a, Timestamp b);
	//! Hours component of the duration for human-readable formatting (e.g. "1h 2m 5s").
	proto external int HoursPart();
	//! Minutes component (0-59) of the duration for human-readable formatting.
	proto external int MinutesPart();
	//! Seconds component (0-59) of the duration for human-readable formatting.
	proto external int SecondsPart();
	//! Milliseconds component (0-999) of the duration for human-readable formatting.
	proto external int MillisPart();
	//! Returns an ISO 8601 duration string, e.g. \c "PT1H2M5S".
	proto external string ToIsoString();
	//! Returns \c true when the duration is exactly zero.
	proto external bool IsZero();
	//! Returns \c true when the duration is negative.
	proto external bool IsNegative();
	//! Returns the absolute (non-negative) value of this duration.
	proto external Duration Abs();
	//! Returns this duration with its sign flipped.
	proto external Duration Negated();
	//! Returns -1 if other is greater, 0 if both are equals, 1 if other is less
	proto external int CompareTo(Duration other);
	proto external bool Equals(Duration other);
	proto external Duration Plus(Duration other);
	proto external Duration Minus(Duration other);
	proto external Duration MultipliedBy(int n);
	proto external Duration DividedBy(int n);
	/*! Formats this duration using a NodaTime-style DurationPattern.

	Tokens (repetition controls zero-padding width, e.g. \c hh pads to 2 digits,
	\c MoMo pads months to 2 digits):
	- \c Y  / \c y  - years    (uppercase: cascade/total, lowercase: partial component)
	- \c Mo / \c mo - months
	- \c D  / \c d  - days
	- \c H  / \c h  - hours
	- \c M  / \c m  - minutes
	- \c S  / \c s  - seconds
	- \c F  / \c f  - milliseconds

	Exactly one unit must be uppercase - the "cascade point". Every unit larger
	than it is folded into it; smaller units appear as partials. Patterns are
	invalid if a larger unit appears as a partial, or if the partial chain leaves
	a gap between the total and the last partial specified. A leading minus sign
	is prepended automatically for negative durations. Any character not listed
	above is emitted literally.

	\code
	  // 2 days, 6 hours, 30 minutes, 45 seconds
	  duration.ToDurationString("D:hh:mm:ss"); // "2:06:30:45"
	  duration.ToDurationString("H:mm:ss");    // "54:30:45"
	  duration.ToDurationString("M:ss");       // "3270:45"
	  duration.ToDurationString("S");          // "196245"
	\endcode
	*/
	proto external string ToDurationString(string pattern);
	/*! Splits this duration into calendar-style period components.
	A year is treated as 365 days and a month as 30 days; for negative durations
	every component is negated so the parts sum back to the original value.
	*/
	proto external void ToPeriod(out int years, out int months, out int days, out int hours, out int minutes, out int seconds, out int milliseconds);
}

/*!
\}
*/
