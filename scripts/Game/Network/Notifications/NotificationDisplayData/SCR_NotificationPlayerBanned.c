/*!
Notification Player banned
Displays a notification: %1 = PlayerID name, %2 = days, %3 = hours, %4 = minutes, %5 = seconds
SCR_NotificationData: m_iParam1 = PlayerID, m_iParam2 = duration
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerBanned : SCR_NotificationPlayer
{
	override string GetText(SCR_NotificationData data)
	{	
		int playerID, kickCauseGroup, kickCauseReason, duration;
		data.GetParams(playerID, kickCauseGroup, kickCauseReason, duration);
		KickCauseCode cause = KickCauseCodeAPI.NewCode(kickCauseGroup, kickCauseReason);
		
		int days, hours, minutes, seconds;
		SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(duration, days, hours, minutes, seconds);
		
		string playerName;
		data.GetNotificationTextEntries(playerName);
		if (!GetPlayerName(playerID, playerName))
			return string.Empty;
		
		data.SetNotificationTextEntries(playerName, days.ToString(),  hours.ToString(),  minutes.ToString(),  seconds.ToString());
		return super.GetText(data);
	}	
	
};