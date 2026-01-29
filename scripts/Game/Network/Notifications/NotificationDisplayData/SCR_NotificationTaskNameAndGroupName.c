/*!
Notification which allows for displaying the name of a task and group name
Displays a notification: %1 = task name, %2 = group name
SCR_NotificationData: m_iParam1 = taskNotification, m_iParam2 = taskNotificationMsg, m_iParam3 = factionId, m_iParam4 = baseCallsign, m_iParam5 = groupID
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationTaskNameGroupName : SCR_NotificationTaskName
{
	//------------------------------------------------------------------------------------------------
	override string GetText(SCR_NotificationData data)
	{
		SCR_ETaskNotification taskNotification;
		SCR_ETaskNotificationMsg taskNotificationMsg;
		int factionId;
		int baseCallsign;
		int groupID;
		data.GetParams(taskNotification, taskNotificationMsg, factionId, baseCallsign, groupID);

		string taskName = GetTaskName(taskNotification, taskNotificationMsg, factionId, baseCallsign, 0);
		string groupName;

		if (!GetGroupNameFromGroupID(groupID, groupName) || taskName.IsEmpty())
			return string.Empty;

		data.SetNotificationTextEntries(taskName, groupName);
		return super.GetText(data);
	}
}
