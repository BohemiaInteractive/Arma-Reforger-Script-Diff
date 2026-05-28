/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_notifications
\{
*/

sealed class NotificationInfo: Managed
{
	private void NotificationInfo();
	private void ~NotificationInfo();

	//! Can be used as a filter for listing notifications with ID greater than a specific value and to delete / mark as read.
	proto external int GetId();
	proto external bool IsMarkedAsRead();
	/*!
	Returns type of the notification.
	\note Type of the notification determines structure of the payload.
	*/
	proto external string GetType();
	//! Returns category of the notification.
	proto external string GetCategory();
	//! Returns type of the source of the notification.
	proto external string GetSourceType();
	/*!
	Returns game identity of the source user of this notification.
	\note Can be null if the source is not a user.
	*/
	proto external UUID GetSourceUserGameIdentity();
	/*!
	Expand payload of the notification.
	The payload's structure is determined by the notification's type.
	\note Given JsonApiStruct must match the expected structure of the payload.
	*/
	proto external void ExpandPayload(JsonApiStruct pPayload);
}

/*!
\}
*/
