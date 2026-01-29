/*!
Notification Player and Faction of that player
Displays a notification: %1 = PlayerID name, %2 = Faction Name of assigned faction
SCR_NotificationData: m_iParam1 = PlayerID
Can be used for: Player (m_iParam1) joined faction 'factionName'
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerAndFaction : SCR_NotificationPlayer
{
	override string GetText(SCR_NotificationData data)
	{	
		int playerID;
		data.GetParams(playerID);
		
		string playerName, factionName;
		data.GetNotificationTextEntries(playerName, factionName);
		if (!GetPlayerName(playerID, playerName))
			return string.Empty;
		
		if (factionName.IsEmpty())
		{
			if (!GetGame().GetPlayerController()) 
			return string.Empty;
		
			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			if (!factionManager) 
				return string.Empty;
			
			//Get factions using ID
			Faction playerFaction = factionManager.GetPlayerFaction(playerID);
			if (!playerFaction) 
				return string.Empty;
			
			factionName = playerFaction.GetUIInfo().GetName();
		}
		
		data.SetNotificationTextEntries(playerName, factionName);
		return super.GetText(data);
	}
	
	override void SetFactionRelatedColor(SCR_NotificationData data)
	{
		int playerID;
		data.GetParams(playerID);
		data.SetFactionRelatedColor(GetFactionRelatedColorPlayer(playerID, m_info.GetNotificationColor()));
	}
};