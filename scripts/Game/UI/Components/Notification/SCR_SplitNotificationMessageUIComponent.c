class SCR_SplitNotificationMessageUIComponent : SCR_NotificationMessageUIComponent
{
	[Attribute()]
	protected string m_sSplitNotificationText;
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_NotificationData data, SCR_NotificationsLogComponent notificationLog, float fadeDelay)
	{
		super.Init(data, notificationLog, fadeDelay);
	
		TextWidget rightMessage = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sSplitNotificationText));
		if (!rightMessage)
			return;		
		
		SCR_NotificationDisplayData displayData = data.GetDisplayData();
		if (!displayData)
			return;
		
		SCR_SplitNotificationUIInfo splitUiInfo = SCR_SplitNotificationUIInfo.Cast(displayData.GetNotificationUIInfo());
		if (!splitUiInfo)
			return;

		if (splitUiInfo.GetIconPath().IsEmpty())
			Print(string.Format("SCR_SplitNotificationMessageUIComponent must have an icon assigned for %1!", typename.EnumToString(ENotification, data.GetID())), LogLevel.NORMAL);

		ENotificationColor leftColorEnum, rightColorEnum;
		displayData.GetSplitNotificationTextColors(data, leftColorEnum, rightColorEnum);
		Color textColor;
		
		//Set text colors
		TextWidget leftMessage = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sNotificationText));
		if (leftMessage)
		{
			textColor = notificationLog.GetNotificationTextColor(leftColorEnum);
			textColor.SetA(leftMessage.GetColor().A());
			leftMessage.SetColor(textColor);
		}

		textColor = notificationLog.GetNotificationTextColor(rightColorEnum);
		textColor.SetA(rightMessage.GetColor().A());
		rightMessage.SetColor(textColor);
		
		
		string rightNotificationMessage = splitUiInfo.GetSplitRightMessageText();
		string param1, param2, param3, param4, param5, param6;
		data.GetNotificationTextEntries(param1, param2, param3, param4, param5, param6);
		
		rightMessage.SetTextFormat(rightNotificationMessage, param1, param2, param3, param4, param5, param6);
	}
}
