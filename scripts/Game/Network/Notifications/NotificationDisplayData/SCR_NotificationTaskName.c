/*!
Notification which allows for displaying the name of a task
Displays a notification: %1 = task name
SCR_NotificationData: m_iParam1 = taskRplId
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationTaskName : SCR_NotificationDisplayData
{
	//------------------------------------------------------------------------------------------------
	override string GetText(SCR_NotificationData data)
	{
		RplId taskRplId;
		data.GetParams(taskRplId);

		string taskName = GetTaskName(taskRplId);
		if (taskName.IsEmpty())
			return string.Empty;

		data.SetNotificationTextEntries(taskName);
		return super.GetText(data);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetTaskName(RplId taskRplId)
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

		if (!task.GetTaskData())
			return string.Empty;

		return task.GetTaskData().m_UIInfo.GetTranslatedName();
	}
}
