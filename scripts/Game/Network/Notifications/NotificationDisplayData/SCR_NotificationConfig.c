[BaseContainerProps(configRoot: true)]
class SCR_NotificationConfig
{
	[Attribute(desc: "Notification Color array. Each enum can have it's own color")]
	protected ref array<ref SCR_NotificationDisplayColor> m_aNotificationDisplayColor;
	
	[Attribute(desc: "List of Widgetnames for Sticky notifications.")]
	protected ref array<string> m_aStickyNotifications;
	
	[Attribute(desc: "List of notifications that can be assigned to the SCR_NotificationsComponent and be called via static functions within the same component")]
	protected ref array<ref SCR_NotificationDisplayData> m_aNotificationDisplayData;
	
	/*!
	Get notification display data
	\param[out] data All the notification display data
	\return Count of data array
	*/
	int GetNotificationData(out notnull array<ref SCR_NotificationDisplayData> data)
	{
		data.Clear();
		
		foreach (SCR_NotificationDisplayData notification: m_aNotificationDisplayData)
			data.Insert(notification);
		
		return m_aNotificationDisplayData.Count();
	}
	
	array<string> GetStickyNotifications()
	{
		return m_aStickyNotifications;
	}
	
	array<ref SCR_NotificationDisplayColor> GetNotificationDisplayColor()
	{
		return m_aNotificationDisplayColor;
	}
};