/*!
Notification Custom group name
Displays a notification: %1 = Custom Group name (Will fall back to callsign if not group name assigned)
SCR_NotificationData: m_iParam1 = Group ID
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationGroupName : SCR_NotificationDisplayData
{
	override string GetText(SCR_NotificationData data)
	{	
		int groupID;
		data.GetParams(groupID);
		
		string groupName;
		data.GetNotificationTextEntries(groupName);
		if (!GetGroupNameFromGroupID(groupID, groupName))
			return string.Empty;
		
		data.SetNotificationTextEntries(groupName);		
		return super.GetText(data);
	}
};