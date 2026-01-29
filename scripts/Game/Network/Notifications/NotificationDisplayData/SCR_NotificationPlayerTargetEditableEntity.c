/*!
Notification Player with target Editable Entity Display info
Displays a notification: %1 = Player name, %2 = EditableEntity name
SCR_NotificationData: m_iParam1 = PlayerID, m_iParam2 = editibleEntityID
Can also be used for Player pinging a target entity (m_iParam1 pinging playerID, m_iParam2 = target entityID)
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerTargetEditableEntity : SCR_NotificationDisplayData
{
	[Attribute("0", desc: "If true will get target Character name (first, alias and surname) if the entity is a NPC otherwise will get the entity type name eg: Rifleman")]
	protected bool m_bGetTargetCharacterName;
	
	override string GetText(SCR_NotificationData data)
	{
		int playerID, targetID;
		data.GetParams(playerID, targetID);
		
		string playerName, entityName;
		data.GetNotificationTextEntries(playerName, entityName);
		if (!GetPlayerName(playerID, playerName) || !GetEditableEntityName(targetID, entityName, m_bGetTargetCharacterName))
			return string.Empty;
		
		data.SetNotificationTextEntries(playerName, entityName);
		return super.GetText(data);
	}
	
	override void SetPosition(SCR_NotificationData data)
	{
		if (!CanSetPosition(data))
			return;
		
		int playerID, targetID;
		data.GetParams(playerID, targetID);
		SetPositionDataEditableEntity(targetID, data);
	}

	override void SetFactionRelatedColor(SCR_NotificationData data)
	{	
		int playerID, entityID;
		data.GetParams(playerID, entityID);
		data.SetFactionRelatedColor(GetFactionRelatedColorEntity(entityID, m_info.GetNotificationColor()));
	
		//Split notification sets faction colors of left and right texts taking player ids, Text color var set the faction color of the target entity
		SCR_SplitNotificationUIInfo splitNotificationUIInfo = SCR_SplitNotificationUIInfo.Cast(m_info);
		SCR_ColoredTextNotificationUIInfo coloredTextUiInfo = SCR_ColoredTextNotificationUIInfo.Cast(m_info);
		
		if (splitNotificationUIInfo)
		{
			ENotificationColor leftColor, rightColor;
			
			rightColor = GetFactionRelatedColorEntity(entityID, splitNotificationUIInfo.GetRightTextColor());
			
			//Check if left color should be the same as right color
			if (splitNotificationUIInfo.ShouldReplaceLeftColorWithRightColorIfAlly() && AreEntitiesFriendly(playerID, true, entityID, false))
				leftColor = rightColor;
			else 
				leftColor = GetFactionRelatedColorPlayer(playerID, splitNotificationUIInfo.GetLeftTextColor());
			
			data.SetSplitFactionRelatedColor(leftColor, rightColor);
		}
		else if (coloredTextUiInfo)
		{
			data.SetFactionRelatedTextColor(GetFactionRelatedColorEntity(entityID, coloredTextUiInfo.GetNotificationTextColor()));	
		}
			
	}
};