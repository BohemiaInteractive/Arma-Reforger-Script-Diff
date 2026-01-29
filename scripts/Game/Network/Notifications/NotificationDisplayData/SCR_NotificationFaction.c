/*!
Notification Faction
Displays a notification: %1 = PlayerID name
SCR_NotificationData: m_iParam1 = PlayerID
Can be used for: Player (m_iParam1) requests something
Can be used for: Player (m_iParam1) dies (ENotificationSetPositionData = AUTO_SET_POSITION_ONCE)
Can be used for: Player (m_iParam1) pings at position(ENotificationSetPositionData = NEVER_AUTO_SET_POSITION, SCR_NotificationData.SetPosition() to ping location)
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationFaction : SCR_NotificationDisplayData
{
	override string GetText(SCR_NotificationData data)
	{	
		int factionEntityID;
		data.GetParams(factionEntityID);
		
		SCR_EditableEntityComponent factionEntity = SCR_EditableEntityComponent.Cast(Replication.FindItem(factionEntityID));
		if (!factionEntity) return string.Empty;
		
		Faction faction = factionEntity.GetFaction();
		if (!faction) return string.Empty;
		
		data.SetNotificationTextEntries(faction.GetUIInfo().GetName());		
		return super.GetText(data);
	}	
};