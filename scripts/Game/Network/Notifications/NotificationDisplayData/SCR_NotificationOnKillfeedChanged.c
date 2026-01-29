/*!
Notification Player
Displays a notification: %1 = PlayerID name
Displays a notification: %2 = Killfeed type or Killfeed receive type
SCR_NotificationData: m_iParam1 = PlayerID
SCR_NotificationData: m_iParam2 = EKillFeedType or EKillFeedReceiveType
SCR_NotificationData: m_iParam3 = bool If Kill Feed Receive type then true else it is the Killfeed type
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationOnKillfeedChanged : SCR_NotificationPlayer
{	
	[Attribute(desc: "Which killfeed type should the notification get the names from?", uiwidget : UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EKillFeedTypeChanged))]
	protected SCR_EKillFeedTypeChanged m_eKillFeedType;
	
	override string GetText(SCR_NotificationData data)
	{	
		int playerID, killfeedType, isreceive;
		string killfeedChangedTo = "UNKNOWN";
		data.GetParams(playerID, killfeedType, isreceive);
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return super.GetText(data);
		
		SCR_NotificationSenderComponent notificationSender = SCR_NotificationSenderComponent.Cast(gameMode.FindComponent(SCR_NotificationSenderComponent));
		if (!notificationSender)
			return super.GetText(data);
		
		//~ Get the general killfeed type name
		if (m_eKillFeedType == SCR_EKillFeedTypeChanged.GLOBAL_KILLFEED_TYPE)
		{
			array<ref SCR_NotificationKillfeedTypeName> killFeedTypeNames = new array<ref SCR_NotificationKillfeedTypeName>;
			notificationSender.GetKillFeedTypeNames(killFeedTypeNames);
			
			foreach (SCR_NotificationKillfeedTypeName killfeed: killFeedTypeNames)
			{
				if (killfeed.GetKillfeedType() == killfeedType)
				{
					killfeedChangedTo = killfeed.GetName();
					break;
				}
			}
		}
		//~ Get the Receive killfeed type name
		else if (m_eKillFeedType == SCR_EKillFeedTypeChanged.RECEIVE_KILLFEED_TYPE)
		{
			array<ref SCR_NotificationKillfeedreceiveTypeName> killFeedReceiveTypeNames = new array<ref SCR_NotificationKillfeedreceiveTypeName>;
			notificationSender.GetKillFeedReceiveTypeNames(killFeedReceiveTypeNames);
			
			foreach (SCR_NotificationKillfeedreceiveTypeName killfeed: killFeedReceiveTypeNames)
			{
				if (killfeed.GetKillfeedReceiveType() == killfeedType)
				{
					killfeedChangedTo = killfeed.GetName();
					break;
				}
			}
		}
		//~ Get the friendly fire killfeed type name
		else
		{
			array<ref SCR_NotificationFriendlyFireKillfeedTypeName> friendlyFireKillFeedTypeNames = {};
			notificationSender.GetFriendlyFireKillFeedTypeNames(friendlyFireKillFeedTypeNames);
			
			foreach (SCR_NotificationFriendlyFireKillfeedTypeName killfeed: friendlyFireKillFeedTypeNames)
			{
				if (killfeed.GetFriendlyFireKillfeedType() == killfeedType)
				{
					killfeedChangedTo = killfeed.GetName();
					break;
				}
			}
		}
		
		string playerName;
		data.GetNotificationTextEntries(playerName);
		if (!GetPlayerName(playerID, playerName))
			return string.Empty;
		
		data.SetNotificationTextEntries(playerName, killfeedChangedTo);
		return super.GetText(data);
	}	
};


enum SCR_EKillFeedTypeChanged
{
	GLOBAL_KILLFEED_TYPE,
	RECEIVE_KILLFEED_TYPE,
	FRIENDLY_FIRE_KILLFEED_TYPE,
}


