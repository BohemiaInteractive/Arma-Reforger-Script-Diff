[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_VotingUIInfo : SCR_UIInfo
{
	[Attribute(desc: "Text used when starting a vote about something.")]
	protected LocalizedString m_sStartVotingName;
	
	[Attribute("", desc: "Text used when canceling your vote about something.")]
	protected LocalizedString m_sCancelVotingName;
	
	[Attribute("", desc: "Text used when currently abstaining but want to revote to yes. Leave empty to use normal vote string")]
	protected LocalizedString m_sRevoteName;
	
	[Attribute("", desc: "When a vote is in progress and the player has not yet voted or abstained.")]
	protected LocalizedString m_sAbstainVoteName;
	
	[Attribute(desc: "Text shown consistently when the vote is active. %1 is player that is voted for")]
	protected LocalizedString m_sStickyNotificationText;
	
	[Attribute(desc: "Text shown with the name of the author of the vote")]
	protected LocalizedString m_sVoteAuthorText;
	
	[Attribute(ENotification.UNKNOWN.ToString(), desc: "The Start notification id to which the voting type is linked. Leave UNKNOWN to not have a notification", uiwidget: UIWidgets.SearchComboBox, enumType: ENotification)]
	protected ENotification m_iStartNotificationId;
	
	[Attribute(ENotification.UNKNOWN.ToString(), desc: "The Succeed notification id to which the voting type is linked. Leave UNKNOWN to not have a notification", uiwidget: UIWidgets.SearchComboBox, enumType: ENotification)]
	protected ENotification m_iSucceededNotificationId;
	
	[Attribute(ENotification.UNKNOWN.ToString(), desc: "The Fail notification id to which the voting type is linked. Leave UNKNOWN to not have a notification", uiwidget: UIWidgets.SearchComboBox, enumType: ENotification)]
	protected ENotification m_iFailedNotificationId;

	[Attribute(ENotification.UNKNOWN.ToString(), desc: "Notification that is meant to be shown as an alternative text for the result of the vote", uiwidget: UIWidgets.SearchComboBox, enumType: ENotification)]
	protected ENotification m_iAlternativeNotificationId;
	
	[Attribute(ENotification.UNKNOWN.ToString(), desc: "Local notification send when player casts vote. Leave UNKNOWN to not have a notification", uiwidget: UIWidgets.SearchComboBox, enumType: ENotification)]
	protected ENotification m_eVoteCastedLocalNotification;	
	
	[Attribute(ENotification.UNKNOWN.ToString(), desc: "Local notification send when player abstains from voting. Leave UNKNOWN to not have a notification", uiwidget: UIWidgets.SearchComboBox, enumType: ENotification)]
	protected ENotification m_eVoteAbstainedLocalNotification;
	
	//------------------------------------------------------------------------------------------------
	//! \return Name used when starting a new vote.
	string GetStartVotingName()
	{
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(m_sStartVotingName))
			return m_sStartVotingName;
		else
			return "ERROR MISSING START VOTE IN UIINFO";
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Name used when withdrawing a vote.
	string GetCancelVotingName()
	{
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(m_sCancelVotingName))
			return m_sCancelVotingName;
		else
			return "ERROR MISSING CANCEL VOTE IN UIINFO";
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Name used when abstaining voting
	string GetRevoteName()
	{
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(m_sRevoteName))
			return m_sRevoteName;
		else
			return GetName();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Name used when abstaining voting
	string GetAbstainVoteName()
	{
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(m_sAbstainVoteName))
			return m_sAbstainVoteName;
		else
			return "ERROR MISSING ABSTAIN VOTE IN UIINFO";
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get sticky notification text shown in the notification log when the specific voting type is active.
	//! \return string stricky notification text
	string GetStickyNotificationText()
	{
		return m_sStickyNotificationText;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return text that is meant to filled with the name of the author after translation
	LocalizedString GetAuthorText()
	{
		return m_sVoteAuthorText;
	}

	//------------------------------------------------------------------------------------------------
	//! Get Start notification ID which is send to the Notification Component to show the specific notification when the voting starts
	//! \return ENotification Notification ID of Start voting
	ENotification GetVotingStartNotification()
	{
		return m_iStartNotificationId;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Get Succeed notification ID which is send to the Notification Component to show the specific notification when voting ends and succeeds
	//! \return ENotification Notification ID of successful voting
	ENotification GetVotingSucceedNotification()
	{
		return m_iSucceededNotificationId;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Get Fail notification ID which is send to the Notification Component to show the specific notification when voting ends and fails
	//! \return ENotification Notification ID of failed voting
	ENotification GetVotingFailNotification()
	{
		return m_iFailedNotificationId;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return ENotification notification ID that is meant to be used for alternative resolution of the vote
	ENotification GetVotingAlternativeNotification()
	{
		return m_iAlternativeNotificationId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Local Vote notification ID which is send to the Notification Component to show the specific notification when a player locally casts a vote
	//! \return Notification ID
	ENotification GetLocalVoteCastNotification()
	{
		return m_eVoteCastedLocalNotification;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Local Vote notification ID which is send to the Notification Component to show the specific notification when a player locally abstains from voting
	//! \return Notification ID
	ENotification GetLocalVoteAbstainedNotification()
	{
		return m_eVoteAbstainedLocalNotification;
	}
}
