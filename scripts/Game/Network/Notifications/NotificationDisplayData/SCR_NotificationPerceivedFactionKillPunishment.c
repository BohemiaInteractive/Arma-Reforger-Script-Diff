[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPerceivedFactionKillPunishment : SCR_NotificationPlayer
{
	[Attribute(desc: "Text shown in the notification when a disguise punishement type is set")]
	protected ref array<ref SCR_FactionKillPunishmentTypes> m_aNotificationTextForPunishmentType;
	
	override string GetText(SCR_NotificationData data)
	{	
		int playerID, punishmentFlags;
		data.GetParams(playerID, punishmentFlags);
		
		string playerName;
		data.GetNotificationTextEntries(playerName);
		if (!GetPlayerName(playerID, playerName))
			return string.Empty;
		
		//~ Get notification string to set which type is set
		string punishmentTypeString;
		foreach (SCR_FactionKillPunishmentTypes punishmentType : m_aNotificationTextForPunishmentType)
		{
			if (punishmentFlags != punishmentType.m_ePunishmentKillingWhileDisguised || punishmentType.m_sNotificationText.IsEmpty())
				continue;
			
			punishmentTypeString = punishmentType.m_sNotificationText;
		}
		
		if (punishmentTypeString.IsEmpty())
		{
			Print("'SCR_NotificationPerceivedFactionKillPunishment' could not find given punishment type in the notification text array. Make sure any new punishment types are added to the notification. (Or the given string is empty)", LogLevel.ERROR);
			return string.Empty;
		}
		
		data.SetNotificationTextEntries(playerName, punishmentTypeString);		
		return super.GetText(data);
	}
}

[BaseContainerProps()]
class SCR_FactionKillPunishmentTypes
{
	[Attribute("0", desc: "String will be added to the notification if all flags are valid", uiwidget: UIWidgets.Flags, enumType: SCR_EDisguisedKillingPunishment)]
	SCR_EDisguisedKillingPunishment m_ePunishmentKillingWhileDisguised;
	
	[Attribute(desc: "Text shown in the notification when the flags are set")]
	LocalizedString m_sNotificationText;
}

