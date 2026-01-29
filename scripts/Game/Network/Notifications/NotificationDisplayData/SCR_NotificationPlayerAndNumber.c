/*!
Notification Player and number
Displays a notification: %1 = PlayerID name, %2 = given int converted to float devided by m_iNumberDivider
SCR_NotificationData: m_iParam1 = PlayerID
SCR_NotificationData: m_iParam2 = Number
Can be used for: GM (m_iParam1) set respawn time to (m_iParam2)
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerAndNumber : SCR_NotificationPlayer
{
	[Attribute(defvalue: "1", params: "1 1000", desc: "Notifications can only send int's thus if you want to send a float you will need to send all values the amount of values you want after the dot. eg: 23.05 = * 100 to get 2 values after the dot. You will need to make sure this attribute is the same amount as you multiplied it with")]
	protected int m_iNumberDivider;

	override string GetText(SCR_NotificationData data)
	{	
		int playerID, param2, param3, param4, param5, param6;
		data.GetParams(playerID, param2, param3, param4, param5, param6);
		
		float display2 = param2;
		display2 = param2 / m_iNumberDivider;
		
		float display3 = param3;
		display3 = param3 / m_iNumberDivider;
		
		float display4 = param4;
		display4 = param4 / m_iNumberDivider;
		
		float display5 = param5;
		display5 = param5 / m_iNumberDivider;
		
		float display6 = param6;
		display5 = param6 / m_iNumberDivider;
		
		string playerName;
		data.GetNotificationTextEntries(playerName);
		if (!GetPlayerName(playerID, playerName))
			return string.Empty;
		
		data.SetNotificationTextEntries(playerName, display2.ToString(), display3.ToString(), display4.ToString(), display5.ToString(), display6.ToString());
		return super.GetText(data);
	}
};