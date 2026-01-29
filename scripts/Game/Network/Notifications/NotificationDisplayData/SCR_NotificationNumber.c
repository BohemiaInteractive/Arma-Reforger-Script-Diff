/*!
Notification nr
Displays a notification: %1 = given int converted to float devided by 
SCR_NotificationData: m_iParam1 to m_iParam2 number to show in notification
The m_iNumberDivider allows for floats as this system only works with ints. So Giving the param float value * x and dividing x here again
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationNumber : SCR_NotificationDisplayData
{
	[Attribute(defvalue: "1", params: "1 1000", desc: "The m_iNumberDivider allows for floats as this system only works with ints. So Giving the param float value * x and dividing x here again")]
	protected int m_iNumberDivider;

	override string GetText(SCR_NotificationData data)
	{	
		int param1, param2, param3, param4, param5;
		data.GetParams(param1, param2, param3, param4, param5);
		
		float display1 = param1;
		display1 = param1 / m_iNumberDivider;
		
		float display2 = param2;
		display2 = param2 / m_iNumberDivider;
		
		float display3 = param3;
		display3 = param3 / m_iNumberDivider;
		
		float display4 = param4;
		display4 = param4 / m_iNumberDivider;
		
		float display5 = param5;
		display5 = param5 / m_iNumberDivider;
		
		data.SetNotificationTextEntries(display1.ToString(), display2.ToString(), display3.ToString(), display4.ToString(), display5.ToString());
		return super.GetText(data);
	}
};