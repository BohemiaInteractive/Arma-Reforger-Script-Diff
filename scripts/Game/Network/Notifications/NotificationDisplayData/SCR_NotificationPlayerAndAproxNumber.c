/*!
Notification Player name and aprox number
Displays a specific text if the given number is between given min and max
SCR_NotificationData: m_iParam1 = PlayerId, m_iParam2 to m_iParam6 number to return the string in the m_aApproxDisplayContainers that passes the approx text
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerAndAproxNumber : SCR_NotificationAproxNumber
{
	//------------------------------------------------------------------------------------------------
	override string GetText(SCR_NotificationData data)
	{	
		int playerID;
		data.GetParams(playerID);
		
		string playerName;
		data.GetNotificationTextEntries(playerName);
		if (!GetPlayerName(playerID, playerName))
			return string.Empty;
		
		data.SetNotificationTextEntries(playerName);		
	
		return super.GetText(data);
	}
}