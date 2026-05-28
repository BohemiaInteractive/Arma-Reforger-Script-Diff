class SCR_DateTimeHelper
{
	//------------------------------------------------------------------------------------------------
	//! \return local datetime in format "yyyy-mm-dd hh:ii:ss"
	static string GetDateTimeLocal()
	{
		int year, month, day, hour, minute, second;
		System.GetYearMonthDay(year, month, day);
		System.GetHourMinuteSecond(hour, minute, second);
		return SCR_FormatHelper.FormatDateTime(year, month, day, hour, minute, second);
	}

	//------------------------------------------------------------------------------------------------
	//! \return UTC datetime in format "yyyy-mm-dd hh:ii:ss"
	static string GetDateTimeUTC()
	{
		int year, month, day, hour, minute, second;
		System.GetYearMonthDayUTC(year, month, day);
		System.GetHourMinuteSecondUTC(hour, minute, second);
		return SCR_FormatHelper.FormatDateTime(year, month, day, hour, minute, second);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the day's number (e.g 1989-01-01 = 1, 1989-12-31 = 365, 2000-12-31 = 366)
	//! \important Only works with Gregorian calendar, does NOT deal with e.g year 1582 shenanigans!
	//! \param[in] year
	//! \param[in] month
	//! \param[in] day
	//! \return in range 1..365 (normal year) or 366 (leap year) or -1 on error (wrong input values)
	static int GetDayNumber(int year, int month, int day)
	{
		if (month < 1 || month > 12 || day < 1 || day > 31)
			return -1;

		int monthDays[12] = {
			31, 28, 31, 30, 31, 30,
			31, 31, 30, 31, 30, 31,
		};

		if (month > 2 && IsLeapYear(year))
			monthDays[1] = 29;

		if (day > monthDays[month - 1])
			return -1;

		int result;
		for (int i; i < month - 1; ++i)
		{
			result += monthDays[i];
		}

		result += day;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] year
	//! \return true on leap year, false otherwise
	static bool IsLeapYear(int year)
	{
		return year % 400 == 0 || (year % 100 != 0 && year % 4 == 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns translated date expression, either in short or verbose style
	//! \param[in] day number
	//! \param[in] month number from 1 to 12 (NOT 0-based)
	//! \param[in] year number
	//! \param[in] verbose bool
	//! \return translated date expression
	static string GetDateString(int day, int month, int year, bool verbose = true)
	{
		if (verbose)
			return WidgetManager.Translate("#AR-Date_Format_MonthFull", day, SCR_DateTimeHelper.GetMonthString(month), year);
		else
			return WidgetManager.Translate("#AR-Date_Format", day, month, year);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns translation key for the month.
	//! \param[in] month month number from 1 to 12 (NOT 0-based)
	//! \param[in] standalone is the month name going to be displayed as standalone string or part of the date
	//! \return month's translation key or empty string on wrong parameter
	static string GetMonthString(int month, bool standalone = false)
	{
		if (standalone)
		{
			switch (month)
			{
				case 1: return "#AR-Date_January_Standalone";
				case 2: return "#AR-Date_February_Standalone";
				case 3: return "#AR-Date_March_Standalone";
				case 4: return "#AR-Date_April_Standalone";
				case 5: return "#AR-Date_May_Standalone";
				case 6: return "#AR-Date_June_Standalone";
				case 7: return "#AR-Date_July_Standalone";
				case 8: return "#AR-Date_August_Standalone";
				case 9: return "#AR-Date_September_Standalone";
				case 10: return "#AR-Date_October_Standalone";
				case 11: return "#AR-Date_November_Standalone";
				case 12: return "#AR-Date_December_Standalone";
			}
		}
		else
		{
			switch (month)
			{
				case 1: return "#AR-Date_January";
				case 2: return "#AR-Date_February";
				case 3: return "#AR-Date_March";
				case 4: return "#AR-Date_April";
				case 5: return "#AR-Date_May";
				case 6: return "#AR-Date_June";
				case 7: return "#AR-Date_July";
				case 8: return "#AR-Date_August";
				case 9: return "#AR-Date_September";
				case 10: return "#AR-Date_October";
				case 11: return "#AR-Date_November";
				case 12: return "#AR-Date_December";
			}
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns translation key for the abbrev. month.
	//! \param[in] month number (from 1 to 12, NOT 0-based)
	//! \return month's short translation key or empty string on wrong parameter
	static string GetAbbreviatedMonthString(int month)
	{
		switch (month)
		{
			case 1: return "#AR-Date_January_StandaloneShort";
			case 2: return "#AR-Date_February_StandaloneShort";
			case 3: return "#AR-Date_March_StandaloneShort";
			case 4: return "#AR-Date_April_StandaloneShort";
			case 5: return "#AR-Date_May_StandaloneShort";
			case 6: return "#AR-Date_June_StandaloneShort";
			case 7: return "#AR-Date_July_StandaloneShort";
			case 8: return "#AR-Date_August_StandaloneShort";
			case 9: return "#AR-Date_September_StandaloneShort";
			case 10: return "#AR-Date_October_StandaloneShort";
			case 11: return "#AR-Date_November_StandaloneShort";
			case 12: return "#AR-Date_December_StandaloneShort";
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] month 1..12
	//! \param[in] year
	//! \return the maximum value the day can take for a specific month or -1 on wrong input
	static int GetMaxDayFromMonth(int month, int year = -1)
	{
		if (month < 1 || month > 12)
			return -1;

		// February specific
		if (year > 0 && month == 2)
		{
			if (IsLeapYear(year))
				return 29;
			else
				return 28;
		}

		if (month == 2)
			return 28;
		else if (month == 4 || month == 6 || month == 9 || month == 11)
			return 30;
		else
			return 31;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] hour0
	//! \param[in] minute0
	//! \param[in] second0
	//! \param[in] hour1
	//! \param[in] minute1
	//! \param[in] second1
	//! \param[out] hour
	//! \param[out] minute
	//! \param[out] second
	//! \return the absolute difference in seconds
	static int GetTimeDifference(int hour0, int minute0, int second0, int hour1, int minute1, int second1, out int hour = 0, out int minute = 0, out int second = 0)
	{
		int time0 = GetSecondsFromHourMinuteSecond(hour0, minute0, second0);
		int time1 = GetSecondsFromHourMinuteSecond(hour1, minute1, second1);

		int result = time0 - time1;
		if (result < 0)
			result = -result;

		GetHourMinuteSecondFromSeconds(result, hour, minute, second);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] hour0
	//! \param[in] minute0
	//! \param[in] second0
	//! \param[in] hour1
	//! \param[in] minute1
	//! \param[in] second1
	//! \return total absolute difference in hh:ii:ss format
	static string GetTimeDifferenceFormatted(int hour0, int minute0, int second0, int hour1, int minute1, int second1)
	{
		int hour, minute, second;
		GetTimeDifference(hour0, minute0, second0, hour1, minute1, second1, hour, minute, second);
		return SCR_FormatHelper.FormatTime(hour, minute, second);
	}

	//------------------------------------------------------------------------------------------------
	//! \return local time in format "hh:ii:ss"
	static string GetTimeLocal()
	{
		int hour, minute, second;
		System.GetHourMinuteSecond(hour, minute, second);
		return SCR_FormatHelper.FormatTime(hour, minute, second);
	}

	//------------------------------------------------------------------------------------------------
	//! \return UTC time in format "hh:ii:ss"
	static string GetTimeUTC()
	{
		int hour, minute, second;
		System.GetHourMinuteSecondUTC(hour, minute, second);
		return SCR_FormatHelper.FormatTime(hour, minute, second);
	}

	//------------------------------------------------------------------------------------------------
	//! Splits given seconds to days, hours, minutes and remaining seconds
	//! \param[in] totalSeconds total seconds to be converted - absolute value will be used
	//! \param[out] outDays Returns amount of Days
	//! \param[out] outHours Returns amount of Hours 0..23
	//! \param[out] outMinutes Returns amount of Minutes 0..59
	//! \param[out] outSeconds Returns remaining amount of Seconds 0..59
	static void GetDayHourMinuteSecondFromSeconds(int totalSeconds, out int outDays, out int outHours, out int outMinutes, out int outSeconds)
	{
		if (totalSeconds < 0)
			totalSeconds = -totalSeconds;

		outDays = totalSeconds / 86400;
		outHours = (totalSeconds % 86400) / 3600;
		outMinutes = (totalSeconds % 3600) / 60;
		outSeconds = totalSeconds % 60;
	}

	//------------------------------------------------------------------------------------------------
	//! Splits given seconds to hours, minutes and remaining seconds
	//! \param[in] totalSeconds total seconds to be converted - absolute value will be used
	//! \param[out] outHours Returns amount of Hours 0..x (can be above 24)
	//! \param[out] outMinutes Returns amount of Minutes 0..59
	//! \param[out] outSeconds Returns remaining amount of Seconds 0..59
	static void GetHourMinuteSecondFromSeconds(int totalSeconds, out int outHours, out int outMinutes, out int outSeconds)
	{
		if (totalSeconds < 0)
			totalSeconds = -totalSeconds;

		outHours = totalSeconds / 3600;
		outMinutes = (totalSeconds % 3600) / 60;
		outSeconds = totalSeconds % 60;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] hour
	//! \param[in] minute
	//! \param[in] second
	//! \return seconds
	static int GetSecondsFromHourMinuteSecond(int hour = 0, int minute = 0, int second = 0)
	{
		return hour * 3600 + minute * 60 + second;
	}

	//------------------------------------------------------------------------------------------------
	//! Grabs all given values and convert it to minutes.
	//! \param[in] year Years to convert into minutes. Note that max years is around 4000
	//! \param[in] month Months to convert into minutes
	//! \param[in] day Days to convert into minutes
	//! \param[in] hour Hours to convert into minutes
	//! \param[in] minutes To add to the total minutes
	static int ConvertDateIntoMinutes(int year = 0, int month = 0, int day = 0, int hour = 0, int minutes = 0)
	{
		return (year * 525600) + (month * 43800) + (day * 1440) + (hour * 60) + minutes;
	}

	//------------------------------------------------------------------------------------------------
	//! Grabs the given total minutes and converts it into years, months, days, hours and minutes
	//! \param[in] totalDateMinutes total minutes to be converted - absolute value will be used
	//! \param[out] year amount of years 0..x
	//! \param[out] month amount of months 0..12
	//! \param[out] day amount of days 0..31
	//! \param[out] hour amount of hours 0..23
	//! \param[out] minutes amount of minutes 0..59
	static void ConvertMinutesIntoDate(int totalDateMinutes, out int year, out int month, out int day, out int hour, out int minutes)
	{
		if (totalDateMinutes < 0)
			totalDateMinutes = -totalDateMinutes;

		year = totalDateMinutes / 525600;
		totalDateMinutes -= year * 525600;

		month = totalDateMinutes / 43800;
		totalDateMinutes -= month * 43800;

		day = totalDateMinutes / 1440;
		totalDateMinutes -= day * 1440;

		hour = totalDateMinutes / 60;
		totalDateMinutes -= hour * 60;

		minutes = totalDateMinutes;
	}
}
