/*!
Notification Player and group name
Displays a notification: %1 = PlayerID name, %2 = Custom Group name (Will fall back to callsign if not group name assigned)
SCR_NotificationData: m_iParam1 = PlayerID, m_iParam2 = Group ID
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerAndGroupName : SCR_NotificationPlayer
{
	override string GetText(SCR_NotificationData data)
	{	
		int playerID, groupID;
		data.GetParams(playerID, groupID);
		
		string playerName, groupName;
		data.GetNotificationTextEntries(playerName, groupName);
		if (!GetPlayerName(playerID, playerName) || !GetGroupNameFromGroupID(groupID, groupName))
			return string.Empty;
		
		data.SetNotificationTextEntries(playerName, groupName);		
		return super.GetText(data);
	}
};