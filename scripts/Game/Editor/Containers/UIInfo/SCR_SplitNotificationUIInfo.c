[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_SplitNotificationUIInfo : SCR_UINotificationInfo
{
	[Attribute(desc: "The right part of the split notification message. Only valid if m_bIsSplitNotification is true")]
	protected LocalizedString m_sSplitNotificationRightMessage;
	
	[Attribute("0", UIWidgets.ComboBox, "Left Notification text color", "", ParamEnumArray.FromEnum(ENotificationColor) )]
	protected ENotificationColor m_iLeftTextColor;
	
	[Attribute("0", UIWidgets.ComboBox, "Right Notification text color", "", ParamEnumArray.FromEnum(ENotificationColor) )]
	protected ENotificationColor m_iRightTextColor;
	
	[Attribute(desc: "This is for notifications that involve two entities and the two entities are friendlies. eg: Player Killed player. If Right (killed player) is friendly with Left (killer) then Left will be colored the same color. If there are only one or less entities involved this will be ignored (Supported notification classes: SCR_NotificationPlayerTargetPlayer, SCR_NotificationPlayerTargetEditableEntity, SCR_NotificationEditableEntityTargetPlayer and SCR_NotificationEditableEntityEditableEntityTarget)")]
	protected bool m_bReplaceLeftColorWithRightColorIfFactionsAreAlly;

	//------------------------------------------------------------------------------------------------
	//! Get the text to display on the right side of the notification
	//! \param text to display
	string GetSplitRightMessageText()
	{
		return m_sSplitNotificationRightMessage;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the color enum of the left text
	//! \return Color enum so the system knows what to color the left text
	ENotificationColor GetLeftTextColor()
	{
		return m_iLeftTextColor;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the color enum of the right text
	//! \return Color enum so the system knows what to color the right text
	ENotificationColor GetRightTextColor()
	{
		return m_iRightTextColor;
	}

	//------------------------------------------------------------------------------------------------
	//! This is for notifications that involve two entities and the two entities are friendlies
	//! e.g: player1 Killed player2. If player2 is friendly with player1 then player1 will be coloured the same as player2.
	//! If there are only one or less entities involved this will be ignored
	//! Supported notification classes: SCR_NotificationPlayerTargetPlayer, SCR_NotificationPlayerTargetEditableEntity, SCR_NotificationEditableEntityTargetPlayer and SCR_NotificationEditableEntityEditableEntityTarget
	//! \return whether or not it should check if factions are friendly and replace the colors if so
	bool ShouldReplaceLeftColorWithRightColorIfAlly()
	{
		return m_bReplaceLeftColorWithRightColorIfFactionsAreAlly;
	}
}
