/*!
Notification which allows for displaying the name of a task and group name
Displays a notification: %1 = task name, %2 = group name
SCR_NotificationData: m_iParam1 = taskRplId, m_iParam2 = groupId
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationTaskNameGroupName : SCR_NotificationTaskName
{
	//------------------------------------------------------------------------------------------------
	override string GetText(SCR_NotificationData data)
	{
		RplId taskRplId;
		int groupID;
		data.GetParams(taskRplId, groupID);

		string taskName = GetTaskName(taskRplId);
		string groupName;

		if (!GetGroupNameFromGroupID(groupID, groupName) || taskName.IsEmpty())
			return string.Empty;

		data.SetNotificationTextEntries(taskName, groupName);
		return super.GetText(data);
	}
}
