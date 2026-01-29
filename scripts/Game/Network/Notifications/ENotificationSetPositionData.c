/*!
Notification Set position data. 
Used by the notification system to know if it should set and update position data from entities linked to the notification
Always insert new entities at the bottom of the list.
*/
enum ENotificationSetPositionData
{
	NEVER_AUTO_SET_POSITION = 0,
	AUTO_SET_AND_UPDATE_POSITION = 1,
	AUTO_SET_POSITION_ONCE = 2,
	
};