/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_UserSessionData
\{
*/

sealed class UserSessionApi
{
	private void UserSessionApi();
	private void ~UserSessionApi();

	/*!
	Returns own session data obtained when the session was begun.
	\note Can return nullptr if the session has not been begun yet.
	*/
	static proto UserSessionData GetOwnSession();
	/*!
	Returns the batch of user session data that was requested.
	\note This should be called after UserSessionDataApi.RequestBatchPresence()
	*/
	static proto void GetSessionBatch(out notnull array<UserSessionData> outData);
	/*!
	Request online presence for the given list of users.
	\param callback Script callback where you will receive the result.
	\param userIds List of users to return presence for.
	\note If a user has no presence record, the corresponding entry status will be NOT_FOUND.
	*/
	static proto void RequestBatchSessionData(notnull BackendCallback callback, notnull array<UUID> userIds);
}

/*!
\}
*/
