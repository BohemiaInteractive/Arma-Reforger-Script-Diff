[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationVoting : SCR_NotificationDisplayData
{
	override string GetText(SCR_NotificationData data)
	{
		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		if (!votingManager)
			return string.Empty;
		
		bool isEnd;
		EVotingType type;
		int value, winner;
		data.GetParams(isEnd, type, value, winner);
		
		//LocalizedString notificationText;
		LocalizedString name;
		SCR_UIInfo info = votingManager.GetVotingInfo(type);
		if (info)
			name = info.GetName();
		
		string valueName = votingManager.GetValueName(type, value);
		string winnerName = votingManager.GetValueName(type, winner);
		
		data.SetNotificationTextEntries(name, valueName, winnerName);
		return super.GetText(data);
	}
};