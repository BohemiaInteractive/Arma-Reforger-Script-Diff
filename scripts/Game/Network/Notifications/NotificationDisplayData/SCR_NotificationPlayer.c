/*!
Notification Player
Displays a notification: %1 = PlayerID name
SCR_NotificationData: m_iParam1 = PlayerID
Can be used for: Player (m_iParam1) requests something
Can be used for: Player (m_iParam1) dies (ENotificationSetPositionData = AUTO_SET_POSITION_ONCE)
Can be used for: Player (m_iParam1) pings at position(ENotificationSetPositionData = NEVER_AUTO_SET_POSITION, SCR_NotificationData.SetPosition() to ping location)
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayer : SCR_NotificationDisplayData
{
	override string GetText(SCR_NotificationData data)
	{	
		string textEntry1, textEntry2, textEntry3, textEntry4, textEntry5, textEntry6;
		data.GetNotificationTextEntries(textEntry1, textEntry2, textEntry3, textEntry4, textEntry5, textEntry6);
		
		int playerID;
		data.GetParams(playerID);
		
		string playerName;
		data.GetNotificationTextEntries(playerName);
		if (!GetPlayerName(playerID, playerName))
			return string.Empty;
		
		data.SetNotificationTextEntries(playerName);		
		
		return super.GetText(data);
	}
		
	override void SetPosition(SCR_NotificationData data)
	{		
		if (!CanSetPosition(data))
			return;
		
		int playerID;
		data.GetParams(playerID);
		SetPositionDataEditablePlayer(playerID, data);
	}
	
	override void SetFactionRelatedColor(SCR_NotificationData data)
	{
		int playerID;
		data.GetParams(playerID);
		data.SetFactionRelatedColor(GetFactionRelatedColorPlayer(playerID, m_info.GetNotificationColor()));
		
		SCR_ColoredTextNotificationUIInfo coloredTextUiInfo = SCR_ColoredTextNotificationUIInfo.Cast(m_info);

		if (coloredTextUiInfo)
			data.SetFactionRelatedTextColor(GetFactionRelatedColorPlayer(playerID, coloredTextUiInfo.GetNotificationTextColor()));	
	}
	
};
