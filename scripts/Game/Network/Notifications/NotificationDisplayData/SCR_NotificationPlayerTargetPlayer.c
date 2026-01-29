/*!
Notification Player to Player target
Displays a notification: %1 = PlayerID name, %2 targetPlayerID
SCR_NotificationData: m_iParam1 = PlayerID, m_iParam2 = targetPlayerID
Can be used for: GM (m_iParam1) gives player (m_iParam2) photomode rights. Which allows the other GMs to go to target player position
Can be used for: KillerPlayer (m_iParam1) killed killedPlayer (m_iParam2). (ENotificationSetPositionData = SET_POSTION_ONCE)
Can be used for: GM (m_iParam1) kicks player (m_iParam2) (ENotificationSetPositionData = NEVER_AUTO_SET_POSITION)
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerTargetPlayer : SCR_NotificationDisplayData
{
	override string GetText(SCR_NotificationData data)
	{	
		int playerID, playerTargetID;
		data.GetParams(playerID, playerTargetID);
		
		string playerName, playerTargetName;
		data.GetNotificationTextEntries(playerName, playerTargetName);
		if (!GetPlayerName(playerID, playerName) || !GetPlayerName(playerTargetID, playerTargetName))
			return string.Empty;
		
		data.SetNotificationTextEntries(playerName, playerTargetName);
		return super.GetText(data);
	}
	
	override void SetPosition(SCR_NotificationData data)
	{
		if (!CanSetPosition(data))
			return;
		
		int playerID, playerTargetID;
		data.GetParams(playerID, playerTargetID);
		
		SetPositionDataEditablePlayer(playerTargetID, data);
	}
	
	override void SetFactionRelatedColor(SCR_NotificationData data)
	{
		int playerID, playerTargetID;
		data.GetParams(playerID, playerTargetID);
		data.SetFactionRelatedColor(GetFactionRelatedColorPlayer(playerTargetID, m_info.GetNotificationColor()));
		
		//Split notification sets faction colors of left and right texts taking player ids, Text color var set the faction color of the target entity
		SCR_SplitNotificationUIInfo splitNotificationUIInfo = SCR_SplitNotificationUIInfo.Cast(m_info);
		SCR_ColoredTextNotificationUIInfo coloredTextUiInfo = SCR_ColoredTextNotificationUIInfo.Cast(m_info);
		
		if (splitNotificationUIInfo)
		{
			ENotificationColor leftColor, rightColor;
			
			rightColor = GetFactionRelatedColorPlayer(playerTargetID, splitNotificationUIInfo.GetRightTextColor());
			
			//Check if left color should be the same as right color
			if (splitNotificationUIInfo.ShouldReplaceLeftColorWithRightColorIfAlly() && AreEntitiesFriendly(playerID, true, playerTargetID, true))
				leftColor = rightColor;
			else 
				leftColor = GetFactionRelatedColorPlayer(playerID, splitNotificationUIInfo.GetLeftTextColor());
			
			data.SetSplitFactionRelatedColor(leftColor, rightColor);
		}
		else if (coloredTextUiInfo)
		{
			data.SetFactionRelatedTextColor(GetFactionRelatedColorPlayer(playerTargetID, coloredTextUiInfo.GetNotificationTextColor()));
		}
	}
};