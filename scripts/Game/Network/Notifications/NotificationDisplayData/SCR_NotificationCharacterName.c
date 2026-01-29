/*!
Notification to get character names
Displays a notification: %1 FirstName, %2 Alias, %3 Surname, %4 formatting
SCR_NotificationData: m_iParam1 = editibleEntityID
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationCharacterName : SCR_NotificationDisplayData
{
	override string GetText(SCR_NotificationData data)
	{		
		int entityID;
		data.GetParams(entityID);
		
		string format, firstname, alias, surname;
		
		if (!GetCharacterName(entityID, format, firstname, alias, surname))
		{
			string entityName;
			data.GetNotificationTextEntries(entityName);
			if (!GetEditableEntityName(entityID, entityName, false))
				return string.Empty;
			
			data.SetNotificationTextEntries(entityName);	
			Print("'SCR_NotificationCharacterName' entity could not be found or has no 'SCR_CharacterIdentityComponent'", LogLevel.ERROR);
		}
		else 
		{
			data.SetNotificationTextEntries(firstname, alias, surname, format);	
		}
		
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