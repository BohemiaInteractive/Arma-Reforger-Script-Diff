/*!
Notification Player kicked or banned
SCR_NotificationData: m_iParam1 = PlayerID, m_iParam2 = kick reason, m_iParam3 = duration

if Kicked: 
Displays a notification: %1 = PlayerID name, %2 = kick reason

If Temp banned:
Displays a notification: %1 = PlayerID name, %2 = days, %3 = hours, %4 = minutes, %5 = seconds, %6 = ban reason

if banned:
Displays a notification: %1 = PlayerID name, %2 = ban reason

if Banned: 
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerKicked : SCR_NotificationPlayer
{	
	[Attribute("{D3BFEE28E7D5B6A1}Configs/ServerBrowser/KickDialogs.conf", params: "conf")]
	protected ref SCR_ConfigurableDialogUiPresets m_PlayerKickReasonsConfig;
	
	[Attribute("#AR-ServerBrowser_SessionError", desc: "If kick reason is unknown use this string", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sUnknownKickReason;
	
	override string GetText(SCR_NotificationData data)
	{	
		int playerID, kickCauseGroup, kickCauseReason, duration;
		data.GetParams(playerID, kickCauseGroup, kickCauseReason, duration);
		KickCauseCode kickCode = KickCauseCodeAPI.NewCode(kickCauseGroup, kickCauseReason);
		
		string playerName;
		data.GetNotificationTextEntries(playerName);
		if (!GetPlayerName(playerID, playerName))
			return string.Empty;
		
		string kickReasonText = m_sUnknownKickReason;
		SCR_ConfigurableDialogUiPreset preset;
		
		string groupId, reasonId;
		
		//~ Using the config display the kick reason (Same as displayed to players in MainMenu on kick/Ban). If it could not find the message it will send the m_sUnknownKickReason instead
		if (m_PlayerKickReasonsConfig)
		{
			KickCauseGroup2 groupInt;
			int reasonInt;
			
			if (GetGame().GetFullKickReason(kickCode, groupInt, reasonInt, groupId, reasonId))
			{
				preset = m_PlayerKickReasonsConfig.FindPreset(groupId + "_" + reasonId);
				if (preset && !preset.m_sMessage.IsEmpty())
					kickReasonText = preset.m_sMessage;
			}
		}
		else
		{
			Print("SCR_NotificationPlayerKicked (kick or ban) does not have the m_PlayerKickReasonsConfig assigned with a SCR_ConfigurableDialogUiPresets config prefab!", LogLevel.ERROR);
		}	
		
		//~ Player was kicked (0 duration) or banned (-1 duration)
		if (duration <= 0)
		{
			data.SetNotificationTextEntries(playerName, kickReasonText);
		}
		//~ Higher then 0 so Player was temp banned
		else
		{
			int days, hours, minutes, seconds;
			SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(duration, days, hours, minutes, seconds);
			
			data.SetNotificationTextEntries(playerName, days.ToString(),  hours.ToString(),  minutes.ToString(),  seconds.ToString(), kickReasonText);
		}

		return super.GetText(data);
	}	
};