/*!
Notification which allows for displaying the creation text of a task
Displays a notification: %1 = task creation text
SCR_NotificationData: m_iParam1 = taskNotification, m_iParam2 = factionId, m_iParam3 = baseCallsign, m_iParam4 = callerGroupId
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationTaskCreationText : SCR_NotificationDisplayData
{
	//------------------------------------------------------------------------------------------------
	override string GetText(SCR_NotificationData data)
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

		SCR_ETaskNotification taskNotification;
		int factionId;
		int baseCallsign;
		int callerGroupId;
		data.GetParams(taskNotification, factionId, baseCallsign, callerGroupId);

		Faction faction = factionManager.GetFactionByIndex(factionId);
		if (!faction)
			return string.Empty;

		SCR_MinorRequestedTaskNotificationData requestedNotificationData = SCR_MinorRequestedTaskNotificationData.Cast(taskNotificationManager.GetTaskNotificationData(taskNotification, SCR_ETaskNotificationMsg.MINOR_CREATION));
		if (requestedNotificationData)
			return requestedNotificationData.GetCreationText(callerGroupId);

		SCR_MinorTaskNotificationData notificationData = SCR_MinorTaskNotificationData.Cast(taskNotificationManager.GetTaskNotificationData(taskNotification, SCR_ETaskNotificationMsg.MINOR_CREATION));
		if (!notificationData)
			return string.Empty;

		SCR_CampaignMilitaryBaseComponent base = campaign.GetBaseManager().FindBaseByCallsign(baseCallsign);
		if (base)
			return WidgetManager.Translate(notificationData.m_sText, base.GetFormattedBaseNameWithCallsign(faction));

		return notificationData.m_sText;
	}
}
