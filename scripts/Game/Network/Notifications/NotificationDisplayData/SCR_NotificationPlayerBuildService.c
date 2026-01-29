/*!
Notification Player when the service is build.
Displays a notification: %1 = Player name, %2 = Service name, %3 = base callsing
SCR_NotificationData: m_iParam1 = PlayerID, m_iParam2 = EditableEntityID,  m_iParam3 = BaseID
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerBuildService : SCR_NotificationDisplayData
{
	override string GetText(SCR_NotificationData data)
	{
		int playerID, entityID, callsignID;
		data.GetParams(playerID, entityID, callsignID);
		
		string playerName, entityName;
		data.GetNotificationTextEntries(playerName, entityName);
		if (!GetPlayerName(playerID, playerName) || !GetEditableEntityName(entityID, entityName, false))
			return string.Empty;

		data.SetNotificationTextEntries(playerName, entityName, GetProviderCallsign(callsignID));
		return super.GetText(data);
	}
	
	//------------------------------------------------------------------------------------------------
	private string GetProviderCallsign(int callsignID)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return string.Empty;
		
		SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
		if (!baseManager)
			return string.Empty;
		
		SCR_CampaignMilitaryBaseComponent base = baseManager.FindBaseByCallsign(callsignID);
		if (!base)
			return string.Empty;
		
		return base.GetCallsignDisplayNameOnly();
	}
};