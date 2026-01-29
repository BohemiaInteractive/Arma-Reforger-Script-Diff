/*!
Notification Player and number
Displays a notification: %1 = Editable Entity name, %2 = given int converted to float devided by m_iNumberDivider
SCR_NotificationData: m_iParam1 = editibleEntityID
SCR_NotificationData: m_iParam2 = number
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationEditableEntityAndNumber : SCR_NotificationDisplayData
{
	[Attribute("0", desc: "If true will get Character name (first, alias and surname) if the entity is a NPC otherwise will get the entity type name eg: Rifleman")]
	protected bool m_bGetCharacterName;
	
	[Attribute(defvalue: "1", params: "1 1000")]
	protected int m_iNumberDivider;
	
	override string GetText(SCR_NotificationData data)
	{	
		int entityID, param1, param2, param3, param4;
		data.GetParams(entityID, param1, param2, param3, param4);
		
		float display1 = param1;
		display1 = param1 / m_iNumberDivider;
		
		float display2 = param2;
		display2 = param2 / m_iNumberDivider;
		
		float display3 = param3;
		display3 = param3 / m_iNumberDivider;
		
		float display4 = param4;
		display4 = param4 / m_iNumberDivider;
		
		string entityName;
		data.GetNotificationTextEntries(entityName);
		if (!GetEditableEntityName(entityID, entityName, m_bGetCharacterName))
			return string.Empty;
		
		data.SetNotificationTextEntries(entityName, display1.ToString(), display2.ToString(), display3.ToString(), display4.ToString());
		return super.GetText(data);
	}
};