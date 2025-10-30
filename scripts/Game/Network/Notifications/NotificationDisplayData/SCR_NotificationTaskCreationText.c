/*!
Notification which allows for displaying the creation text of a task
Displays a notification: %1 = task creation text
SCR_NotificationData: m_iParam1 = taskRplId
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationTaskCreationText : SCR_NotificationDisplayData
{
	//------------------------------------------------------------------------------------------------
	override string GetText(SCR_NotificationData data)
	{
		RplId taskRplId;
		data.GetParams(taskRplId);
		string creationText = GetTaskCreationText(taskRplId);
		if (creationText.IsEmpty())
			return string.Empty;

		data.SetNotificationTextEntries(creationText);
		return super.GetText(data);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetTaskCreationText(RplId taskRplId)
	{
		RplComponent taskRplcComp = RplComponent.Cast(Replication.FindItem(taskRplId));
		if (!taskRplcComp)
			return string.Empty;

		IEntity taskEntity = taskRplcComp.GetEntity();
		if (!taskEntity)
			return string.Empty;

		SCR_Task task = SCR_Task.Cast(taskEntity);
		if (!task)
			return string.Empty;

		SCR_TaskNotificationComponent notificationComp = SCR_TaskNotificationComponent.Cast(task.FindComponent(SCR_TaskNotificationComponent));
		if (!notificationComp)
			return string.Empty;

		return notificationComp.GetCreationText();
	}
}
