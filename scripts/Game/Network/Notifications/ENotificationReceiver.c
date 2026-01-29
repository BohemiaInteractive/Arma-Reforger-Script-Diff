/*!
To get who recieves the notification
*/
enum ENotificationReceiver
{
	UNKNOWN = 0,
	EVERYONE = 1,
	SPECIFIC_PLAYER = 2,
	
	PLAYER_GROUP = 10,
	
	GM_ONLY = 20,
	GM_OR_AFFECTED_PLAYER_ONLY = 21,
	
	LOCAL_ONLY = 30,
	LOCAL_GM_ONLY = 31,
	LOCAL_NON_GM_ONLY = 32,
};