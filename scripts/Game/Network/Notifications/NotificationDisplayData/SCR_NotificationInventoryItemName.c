/*!
Notification which allows for displaying the name of an inventory item
Displays a notification: %1 = item name
SCR_NotificationData: m_iParam1 = itemRplId
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationInventoryItemName : SCR_NotificationDisplayData
{
	override string GetText(SCR_NotificationData data)
	{		
		RplId itemReplicationId;
		data.GetParams(itemReplicationId);
		
		string inventoryItemName;
		data.GetNotificationTextEntries(inventoryItemName);
		
		if (!GetInventoryItemName(itemReplicationId, inventoryItemName))
			return string.Empty;
		
		data.SetNotificationTextEntries(inventoryItemName);		
		return super.GetText(data);
	}
};