/*!
Notification Player, distance and location
Displays a notification: %1 = PlayerID name, %2 = distance
SCR_NotificationData: m_iParam1 = PlayerID
SCR_NotificationData: m_iParam2 = distance
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationPlayerAndLocation : SCR_NotificationPlayer
{
	[Attribute(defvalue: "1", params: "1 1000", desc: "Notifications can only send int's thus if you want to send a float you will need to send all values the amount of values you want after the dot. eg: 23.05 = * 100 to get 2 values after the dot. You will need to make sure this attribute is the same amount as you multiplied it with")]
	protected int m_iNumberDivider;
	
	override string GetText(SCR_NotificationData data)
	{	
		int playerID, param2;
		data.GetParams(playerID, param2);
		
		string playerName;
		data.GetNotificationTextEntries(playerName);
		if (!GetPlayerName(playerID, playerName))
			return string.Empty;
		
		float distance = param2;
		distance = param2 / m_iNumberDivider;
		
		vector position;
		data.GetPosition(position);
	
		string location;
		
		//~ Get nearest location
		SCR_EditableEntityComponent nearestLocation;
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			nearestLocation = core.FindNearestEntity(position, EEditableEntityType.COMMENT, EEditableEntityFlag.LOCAL);
		
		//~ Set nearest location name or position as location name if no nearest found
		if (nearestLocation)
			location = nearestLocation.GetDisplayName();
		else
			location = SCR_MapEntity.GetGridLabel(position);
		
		data.SetNotificationTextEntries(playerName, distance.ToString(), location);
		return super.GetText(data);
	}
};
