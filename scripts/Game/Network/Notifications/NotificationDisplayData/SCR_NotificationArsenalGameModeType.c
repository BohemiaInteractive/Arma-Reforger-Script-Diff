[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationArsenalGameModeType : SCR_NotificationPlayer
{
	override string GetText(SCR_NotificationData data)
	{	
		int playerID, arsenalGameModeType;
		data.GetParams(playerID, arsenalGameModeType);
		
		string playerName;
		data.GetNotificationTextEntries(playerName);
		if (!GetPlayerName(playerID, playerName))
			return string.Empty;
		
		string arsenalGameModeTypeName = "MISSING";
		SCR_ArsenalManagerComponent arsenalManager;
		
		if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
		{
			SCR_UIInfo uiInfo = arsenalManager.GetArsenalGameModeUIInfo(arsenalGameModeType);
			if (uiInfo)
				arsenalGameModeTypeName = uiInfo.GetName();
		}
		
		data.SetNotificationTextEntries(playerName, arsenalGameModeTypeName);
		return super.GetText(data);
	}
};