/*!
Notification Editable Entity with target Editable Entity Display info
Displays a notification: %1 = EditableEntity name, %2 = target EditableEntity name
SCR_NotificationData: m_iParam1 = editibleEntityID, m_iParam2 = targetPlayerID
Can be used for AI (m_iParam1) killing player Character (m_iParam2)
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationEditableEntityTargetPlayer : SCR_NotificationPlayerTargetPlayer
{
	[Attribute("0", desc: "If true will get Character name (first, alias and surname) if the entity is a NPC otherwise will get the entity type name eg: Rifleman")]
	protected bool m_bGetCharacterName;
	
	override string GetText(SCR_NotificationData data)
	{		
		int entityID, targetPlayerID;
		data.GetParams(entityID, targetPlayerID);

		string entityName, targetPlayerName;
		data.GetNotificationTextEntries(entityName, targetPlayerName);
		if (!GetEditableEntityName(entityID, entityName, m_bGetCharacterName) || !GetPlayerName(targetPlayerID, targetPlayerName))
			return string.Empty;
		
		data.SetNotificationTextEntries(entityName, targetPlayerName);
		return super.GetText(data);
	}
	
	override void SetFactionRelatedColor(SCR_NotificationData data)
	{
		int entityID, targetPlayerID;
		data.GetParams(entityID, targetPlayerID);
		data.SetFactionRelatedColor(GetFactionRelatedColorPlayer(targetPlayerID, m_info.GetNotificationColor()));
		
		//Split notification sets faction colors of left and right texts taking player ids, Text color var set the faction color of the target entity
		SCR_SplitNotificationUIInfo splitNotificationUIInfo = SCR_SplitNotificationUIInfo.Cast(m_info);
		SCR_ColoredTextNotificationUIInfo coloredTextUiInfo = SCR_ColoredTextNotificationUIInfo.Cast(m_info);
		
		if (splitNotificationUIInfo)
		{
			ENotificationColor leftColor, rightColor;
			
			rightColor = GetFactionRelatedColorPlayer(targetPlayerID, splitNotificationUIInfo.GetRightTextColor());
			
			//Check if left color should be the same as right color
			if (splitNotificationUIInfo.ShouldReplaceLeftColorWithRightColorIfAlly() && AreEntitiesFriendly(entityID, false, targetPlayerID, true))
				leftColor = rightColor;
			else 
				leftColor = GetFactionRelatedColorEntity(entityID, splitNotificationUIInfo.GetLeftTextColor());
			
			data.SetSplitFactionRelatedColor(leftColor, rightColor);
		}
		else if (coloredTextUiInfo)
		{
			data.SetFactionRelatedTextColor(GetFactionRelatedColorPlayer(targetPlayerID, coloredTextUiInfo.GetNotificationTextColor()));	
		}
	}
};