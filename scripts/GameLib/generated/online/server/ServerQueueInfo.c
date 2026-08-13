/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_server
\{
*/

/*!
Provides general information about a server join queue such as its type, current size,
capacity, and average wait time per position.
This data is available regardless of whether the local user is currently in the queue.
\note Obtain queue info through ServerInfo::GetQueueInfoByType().
*/
sealed class ServerQueueInfo
{
	private void ServerQueueInfo();
	private void ~ServerQueueInfo();

	//! Returns the server this queue belongs to.
	proto external ServerInfo GetServer();
	//! Returns the type of this queue.
	proto external EServerJoinQueueType GetType();
	//! Returns how many players are currently in the queue.
	proto external int GetSize();
	//! Returns the maximum number of players that can be in this queue.
	proto external int GetMaxSize();
	/*!
	Returns the average wait time in seconds to advance by one position in the queue.
	\return Average seconds per position, or -1 if there is insufficient data.
	*/
	proto external int GetAvgSecsPerPosition();
}

/*!
\}
*/
