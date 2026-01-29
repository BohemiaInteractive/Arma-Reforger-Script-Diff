class SCR_FormatHelper
{
	//------------------------------------------------------------------------------------------------
	//! Returns a float with set decimal length that will remove any ending zeroes in the decimal values
	//! \code
	//! float value = "1337.2305";
	//! Print(SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(value, 0)); // 1337
	//! Print(SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(value, 1)); // 1337.2
	//! Print(SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(value, 2)); // 1337.23
	//! Print(SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(value, 3)); // 1337.23
	//! Print(SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(value, 4)); // 1337.2305
	//! \endcode
	//! \param[in] value The float value to get the string from
	//! \param[in] lenDec Length of decimals to show. Zeroes of course are not counted
	//! return Converted float into a string with decimals without ending zeroes
	static string FloatToStringNoZeroDecimalEndings(float value, int lenDec)
	{
		return value.ToString(lenDec: lenDec).ToFloat().ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] year
	//! \param[in] month
	//! \param[in] day
	//! \param[in] hour
	//! \param[in] minute
	//! \param[in] second
	//! \return datetime in format "yyyy-mm-dd hh:ii:ss"
	static string FormatDateTime(int year, int month, int day, int hour, int minute, int second)
	{
		return string.Format("%1 %2", FormatDate(year, month, day), FormatTime(hour, minute, second));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] year can be < 100, if so 2000 will be added (86 = 2086, NOT 1986)
	//! \param[in] month
	//! \param[in] day
	//! \return date in format "yyyy-mm-dd"
	static string FormatDate(int year, int month, int day)
	{
		if (year < 100)
			year += 2000;

		return string.Format("%1-%2-%3", year.ToString(4), month.ToString(2), day.ToString(2));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] totalSeconds
	//! \return time in format "hh:ii:ss"
	static string FormatTime(int totalSeconds)
	{
		int hour, minute, second;
		SCR_DateTimeHelper.GetHourMinuteSecondFromSeconds(totalSeconds, hour, minute, second);
		return FormatTime(hour, minute, second);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] totalSeconds
	//! \param[in] precision 1 for deciseconds, 2 for centiseconds, 3 for milliseconds etc - 0 or below for automatic
	//! \return time in format "hh:ii:ss.subSeconds"
	static string FormatTimeMS(float totalSeconds, int precision = 0)
	{
		int hour, minute, second;
		SCR_DateTimeHelper.GetHourMinuteSecondFromSeconds(totalSeconds, hour, minute, second);

		if (precision < 1)
			precision = -1;

		string formattedSubSeconds = (totalSeconds - (int)totalSeconds).ToString(0, precision);
		if (totalSeconds < 0)
			formattedSubSeconds = formattedSubSeconds.Substring(3, formattedSubSeconds.Length() - 3); // removing -0.
		else
			formattedSubSeconds = formattedSubSeconds.Substring(2, formattedSubSeconds.Length() - 2); // removing 0.

		return FormatTime(hour, minute, second) + "." + formattedSubSeconds;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] hour
	//! \param[in] minute
	//! \param[in] second
	//! \return time in format "hh:ii:ss"
	static string FormatTime(int hour, int minute, int second)
	{
		return string.Format("%1:%2:%3", hour.ToString(2), minute.ToString(2), second.ToString(2));
	}

	//------------------------------------------------------------------------------------------------
	//! Depending on the flags given it will return a variation on dd:hh:mm:ss and d:h:m:s.
	//! Time variables that are -1 will always be hidden (So without days it would look: hh:mm:ss)
	//! Usage: SCR_FormatHelper.GetTimeFormatting(0, 13, 37, 0, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.MINUTES). Hide Days and Hours if 0, Never show leading 0 for minutes
	//! \param[in] days Int amount of days
	//! \param[in] hours Int amount of hours
	//! \param[in] minutes Int amount of minutes
	//! \param[in] seconds Int amount of seconds
	//! \param[in] hideEmpty flag time that needs to be hidden if zero (Example: ETimeFormatParam.DAYS hides days if 0 or less)
	//! \param[in] hideLeadingZeroes flag that hides leading zero for Hours, Minutes and Seconds (Example: ETimeFormatParam.HOURS displays h:mm:ss instead of hh:mm:ss)
	//! \return string Time ordered in days, hours, minutes and seconds (dd:hh:mm:ss)
	static string GetTimeFormatting(int days, int hours, int minutes, int seconds, ETimeFormatParam hideEmpty = 0, ETimeFormatParam hideLeadingZeroes = 0)
	{
		//Todo: Display days properly rather then dd:hh:mm:ss (Though right now it is extreamly edge case if days should be shown)
		string returnString = "";

		//To do: Decide the formatting of days. Currently it is dd:hh:mm:ss and by default days are hidden if it is 0 as it would rarely be a day or more
		//Only show days
		//if (days > -1 && ((hideEmpty & ETimeFormatParam.DAYS) && days > 0) || !(hideEmpty & ETimeFormatParam.DAYS))
		if (days > 0 && (hideEmpty & ETimeFormatParam.DAYS) || !(hideEmpty & ETimeFormatParam.DAYS))
			returnString += days.ToString();
		else
			days = -1;

		//hours (Check if should display hours and check if should display leading zero)
		if (hours > 0 && (hideEmpty & ETimeFormatParam.HOURS) || !(hideEmpty & ETimeFormatParam.HOURS))
			returnString += ReturnTimeTypeString(days, hours, ((hideLeadingZeroes & ETimeFormatParam.HOURS) == 0) + 1);
		else
			hours = -1;

		//Minutes (Check if should display Minutes and check if should display leading zero)
		if (minutes > 0 && (hideEmpty & ETimeFormatParam.MINUTES) || !(hideEmpty & ETimeFormatParam.MINUTES))
			returnString += ReturnTimeTypeString(hours, minutes, ((hideLeadingZeroes & ETimeFormatParam.MINUTES) == 0) + 1);
		else
			minutes = -1;

		//Seconds (Check if should display Seconds and check if should display leading zero)
		if (seconds > 0 && (hideEmpty & ETimeFormatParam.SECONDS) || !(hideEmpty & ETimeFormatParam.SECONDS))
			returnString += ReturnTimeTypeString(minutes, seconds, ((hideLeadingZeroes & ETimeFormatParam.SECONDS) == 0) + 1);

		return returnString;
	}

	//------------------------------------------------------------------------------------------------
	//! Always have number length 1 then check if ETimeFormatParam (for example) SECONDS is given is true (aka 1) or not (aka 0)
	//! \param[in] prevTimeTypeAmount
	//! \param[in] currentTimeTypeAmount
	//! \param[in] numberLength checks if should have leading zero
	//! \return proper formatting of TimeType
	protected static string ReturnTimeTypeString(int prevTimeTypeAmount, int currentTimeTypeAmount, int numberLength)
	{
		if (prevTimeTypeAmount > -1)
			return ":" + currentTimeTypeAmount.ToString(numberLength);
		else
			return currentTimeTypeAmount.ToString(numberLength);
	}

	//------------------------------------------------------------------------------------------------
	//! Combines GetTimeFormatting() and SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds()
	//! It is possible to hide a time varriable like days using the hide bool when it is 0 or less
	//! Usage: SCR_FormatHelper.GetTimeFormatting(totalSeconds, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.MINUTES). Hide Days and Hours if 0, Never show leading 0 for minutes
	//! \param[in] totalSeconds Int total amount of seconds to be converted to dd:hh:mm:ss
	//! \param[in] hideEmpty flag time that needs to be hidden if zero (Example: ETimeFormatParam.DAYS hides days if 0 or less)
	//! \param[in] hideLeadingZeroes flag that hides leading zero for Hours, Minutes and Seconds (Example: ETimeFormatParam.HOURS displays h:mm:ss instead of hh:mm:ss)
	//! \return string Time ordered in days, hours, minutes and seconds (dd:hh:mm:ss)
	static string GetTimeFormatting(int totalSeconds, ETimeFormatParam hideEmpty = 0, ETimeFormatParam hideLeadingZeroes = 0)
	{
		int days, hours, minutes, seconds;
		SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(totalSeconds, days, hours, minutes, seconds);
		return GetTimeFormatting(days, hours, minutes, seconds, hideEmpty, hideLeadingZeroes);
	}

	//------------------------------------------------------------------------------------------------
	//! Combines GetTimeFormatting() and SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds() while hiding seconds
	//! It is possible to hide a time varriable like days using the hide bool when it is 0 or less
	//! Usage: SCR_FormatHelper.GetTimeFormatting(totalSeconds, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.MINUTES). Hide Days and Hours if 0, Never show leading 0 for minutes
	//! \param[in] totalSeconds Int total amount of seconds to be converted to dd:hh:mm
	//! \param[in] hideEmpty flag time that needs to be hidden if zero (Example: ETimeFormatParam.DAYS hides days if 0 or less)
	//! \param[in] hideLeadingZeroes flag that hides leading zero for Hours, Minutes and Seconds (Example: ETimeFormatParam.HOURS displays h:mm instead of hh:mm)
	//! \return string Time ordered in days, hours and minutes (dd:hh:mm)
	static string GetTimeFormattingHideSeconds(int totalSeconds, ETimeFormatParam hideEmpty = 0, ETimeFormatParam hideLeadingZeroes = 0)
	{
		int days, hours, minutes, seconds;
		SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(totalSeconds, days, hours, minutes, seconds);
		return GetTimeFormatting(days, hours, minutes, -1, ETimeFormatParam.SECONDS | hideEmpty, hideLeadingZeroes);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a variant of hh:mm and h:m
	//! \param[in] hours Int amount of hours
	//! \param[in] minutes Int amount of minutes
	//! \param[in] hideEmpty flag time that needs to be hidden if zero (Example: ETimeFormatParam.HOURS hides hours if 0 or less)
	//! \param[in] hideLeadingZeroes flag that hides leading zero (Example: ETimeFormatParam.HOURS displays h:mm instead of hh:mm)
	//! \return string Time ordered in hours and minutes (hh:mm)
	static string GetTimeFormattingHoursMinutes(int hours, int minutes, ETimeFormatParam hideEmpty = 0, ETimeFormatParam hideLeadingZeroes = 0)
	{
		return GetTimeFormatting(-1, hours, minutes, -1, (ETimeFormatParam.DAYS | ETimeFormatParam.SECONDS | hideEmpty), hideLeadingZeroes);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a variant of mm:ss and m:s
	//! \param[in] minutes Int amount of minutes
	//! \param[in] seconds Int amount of seconds
	//! \param[in] hideEmpty flag time that needs to be hidden if zero (Example: ETimeFormatParam.MINUTES hides minutes if 0 or less)
	//! \param[in] hideLeadingZeroes flag that hides leading zero (Example: ETimeFormatParam.MINUTES displays m:ss instead of mm:ss)
	//! \return string Time ordered in minutes and hours (mm:ss)
	static string GetTimeFormattingMinutesSeconds(int minutes, int seconds, ETimeFormatParam hideEmpty = 0, ETimeFormatParam hideLeadingZeroes = 0)
	{
		return GetTimeFormatting(-1, -1, minutes, seconds, (ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | hideEmpty), hideLeadingZeroes);
	}

	//------------------------------------------------------------------------------------------------
	//! Helps with time formatting - returns "0 minutes", "3 minutes", but "1 minute".
	//! \param[in] amount
	//! \param[in] oneUnit
	//! \param[in] manyUnits
	protected static string GetTimeSinceEventString(int amount, string oneUnit, string manyUnits)
	{
		if (amount == 1)
			return WidgetManager.Translate(oneUnit, amount);
		else
			return WidgetManager.Translate(manyUnits, amount);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] timeDiffSeconds
	//! \return translation text of how much time has passed since event: "23 days ago", or "23 hours ago", or "23 minutes ago".
	//! When timeDiffSeconds is below 60, it returns "0 minutes ago".
	static string GetTimeSinceEventImprecise(int timeDiffSeconds)
	{
		if (timeDiffSeconds < 0)
			return string.Empty;

		int days, hours, minutes, remainingSeconds;
		SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(timeDiffSeconds, days, hours, minutes, remainingSeconds);

		if (days > 0)
			return GetTimeSinceEventString(days, "#AR-Date_DayAgo_LC", "#AR-Date_DaysAgo_LC");			// 23 days ago
		else if (hours > 0)
			return GetTimeSinceEventString(hours, "#AR-Time_HourAgo_LC", "#AR-Time_HoursAgo_LC");		// 23 hours ago
		else
			return GetTimeSinceEventString(minutes, "#AR-Time_MinuteAgo_LC", "#AR-Time_MinutesAgo_LC");	// 23 minutes ago
	}

	//------------------------------------------------------------------------------------------------
	//! Format the list of frequencies as single-line text.
	//! \param[in] frequencies List of frequencies
	//! \return Text with frequencies
	static string FormatFrequencies(notnull set<int> frequencies, set<int> highlightFrequencies = null)
	{
		if (frequencies.IsEmpty())
			return string.Empty;

		string text;
		float accurateFrequency;
		foreach (int i, int frequency : frequencies)
		{
			if (i > 0)
				text += ", ";

			accurateFrequency = frequency;
			if (highlightFrequencies && highlightFrequencies.Contains(frequency))
				text += string.Format("<color rgba='226,168,79,255'>%1</color>", accurateFrequency * 0.001); //--- ToDo: Don't hardcode color
			else
				text += (accurateFrequency * 0.001).ToString();
		}
		return text + " #AR-VON_FrequencyUnits_MHz";
	}
}
