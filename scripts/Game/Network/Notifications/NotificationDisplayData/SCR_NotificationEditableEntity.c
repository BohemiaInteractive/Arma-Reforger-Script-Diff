/*!
Notification Editable Entity Display info
Displays a notification: %1 = EditableEntity name
SCR_NotificationData: m_iParam1 = editibleEntityID
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationEditableEntity : SCR_NotificationDisplayData
{
	[Attribute("0", desc: "If true will get Character name (first, alias and surname) if the entity is a NPC otherwise will get the entity type name eg: Rifleman")]
	protected bool m_bGetCharacterName;
	
	override string GetText(SCR_NotificationData data)
	{		
		int entityID;
		data.GetParams(entityID);
		
		string entityName;
		data.GetNotificationTextEntries(entityName);
		if (!GetEditableEntityName(entityID, entityName, m_bGetCharacterName))
			return string.Empty;
		
		data.SetNotificationTextEntries(entityName);		
		return super.GetText(data);
	}
	
	override void SetPosition(SCR_NotificationData data)
	{
		if (!CanSetPosition(data))
			return;
		
		int entityID;
		data.GetParams(entityID);
		
		SetPositionDataEditableEntity(entityID, data);
	}
	
	override void SetFactionRelatedColor(SCR_NotificationData data)
	{
		int entityID;
		data.GetParams(entityID);
		data.SetFactionRelatedColor(GetFactionRelatedColorEntity(entityID, m_info.GetNotificationColor()));
		
		SCR_ColoredTextNotificationUIInfo coloredTextUiInfo = SCR_ColoredTextNotificationUIInfo.Cast(m_info);
		
		if (coloredTextUiInfo)
			data.SetFactionRelatedTextColor(GetFactionRelatedColorEntity(entityID, coloredTextUiInfo.GetNotificationTextColor()));	
	}
};