/*!
Notification Player and entity (group or character) callsign
Displays a notification: %1 = PlayerID name, %2 = Formated Callsign of entity
SCR_NotificationData: m_iParam1 = PlayerID, m_iParam2 = rplId to get callsign of (Can be the same or diffrent from player) 
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerAndEntityCallsign : SCR_NotificationPlayer
{
	override string GetText(SCR_NotificationData data)
	{	
		int playerID, entityID;
		data.GetParams(playerID, entityID);
		
		string playerName, callsign;
		data.GetNotificationTextEntries(playerName, callsign);
		if (!GetPlayerName(playerID, playerName) || !GetEntityCallsign(entityID, callsign))
			return string.Empty;
		
		data.SetNotificationTextEntries(playerName, callsign);		
		return super.GetText(data);
	}
};
