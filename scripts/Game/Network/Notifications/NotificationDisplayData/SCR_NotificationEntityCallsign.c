/*!
Notification Entity (group or character) callsign
Displays a notification: %1 = Formated Callsign of entity
SCR_NotificationData: m_iParam1 = rplId to get callsign of (Can be the same or diffrent from player) 
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationEntityCallsign : SCR_NotificationEditableEntity
{
	override string GetText(SCR_NotificationData data)
	{	
		int entityID;
		data.GetParams(entityID);
		
		string callsign;
		data.GetNotificationTextEntries(callsign);
		if (!GetEntityCallsign(entityID, callsign))
			return string.Empty;
		
		data.SetNotificationTextEntries(callsign);		
		return super.GetText(data);
	}
};