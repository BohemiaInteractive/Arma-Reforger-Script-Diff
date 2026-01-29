/*!
Notification which allows for displaying the name of a task
Displays a notification: %1 = task name
SCR_NotificationData: m_iParam1 = taskNotification, m_iParam2 = taskNotificationMsg, m_iParam3 = factionId, m_iParam4 = baseCallsign, m_iParam5 = grid
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationTaskName : SCR_NotificationDisplayData
{
	//------------------------------------------------------------------------------------------------
	override string GetText(SCR_NotificationData data)
	{
		SCR_ETaskNotification taskNotification;
		SCR_ETaskNotificationMsg taskNotificationMsg;
		int factionId;
		int baseCallsign;
		int grid;
		data.GetParams(taskNotification, taskNotificationMsg, factionId, baseCallsign, grid);

		string taskName = GetTaskName(taskNotification, taskNotificationMsg, factionId, baseCallsign, grid);
		if (taskName.IsEmpty())
			return string.Empty;

		data.SetNotificationTextEntries(taskName);
		return super.GetText(data);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetTaskName(SCR_ETaskNotification taskNotification, SCR_ETaskNotificationMsg taskNotificationMsg, int factionId, int baseCallsign, int grid)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return string.Empty;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return string.Empty;

		SCR_TaskNotificationManagerComponent taskNotificationManager = SCR_TaskNotificationManagerComponent.GetInstance();
		if (!taskNotificationManager)
			return string.Empty;

		Faction faction = factionManager.GetFactionByIndex(factionId);
		if (!faction)
			return string.Empty;

		SCR_TaskNotificationConfig config = taskNotificationManager.GetTaskNotificationConfig(taskNotification);
		if (!config)
			return string.Empty;

		SCR_CampaignMilitaryBaseComponent base = campaign.GetBaseManager().FindBaseByCallsign(baseCallsign);
		if (base)
			return WidgetManager.Translate(config.m_sTaskName, base.GetFormattedBaseNameWithCallsign(faction));

		if (grid >= 0)
			return taskNotificationManager.GetTaskNameWithGrid(config.m_sTaskName, grid);

		return config.m_sTaskName;
	}
}
