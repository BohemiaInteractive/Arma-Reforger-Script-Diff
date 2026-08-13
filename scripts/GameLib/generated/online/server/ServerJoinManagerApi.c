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
Provides access to server join and queue state.

When a join is requested via ServerInfo::RequestJoin(), the server may either
approve the join immediately (EApiCode::EACODE_SERVER_JOIN) or place the user
in a queue (EApiCode::EACODE_SERVER_QUEUE_WAIT).

Typical workflow:
 1. Call ServerInfo::RequestJoin() with a BackendCallback.
 2. On EACODE_SERVER_JOIN — request a game state transition to connect to the server.
 3. On EACODE_SERVER_QUEUE_WAIT — retrieve GetQueueData() and set up a polling callback
    via ServerQueueData::SetPollingCallback() to receive queue updates.
 4. When polling returns EACODE_SERVER_JOIN — request a game state transition to connect.

Use GetRequestedServer(), GetJoiningServer(), GetPreviousServer() to track
which server the user is interacting with at each stage.
*/
sealed class ServerJoinManagerApi
{
	private void ServerJoinManagerApi();
	private void ~ServerJoinManagerApi();

	/*!
	Returns the server that a join request is currently in progress for, or null if no request is active.
	\note Once the join request completes (with any result), this returns null
	      and the server moves to GetPreviousServer().
	*/
	static proto ServerInfo GetRequestedServer();
	/*!
	Returns the server that the user has been approved to join, or null.
	This is set after receiving EApiCode::EACODE_SERVER_JOIN from a join request or queue poll
	and is available inside the callback that delivers that result.
	\warning This becomes null once the connection parameters are consumed for the actual server connection.
	*/
	static proto ServerInfo GetJoiningServer();
	/*!
	Returns the last server the user attempted to join or was queued for, or null if no join has been attempted.
	This is updated whenever a join request completes, a queue poll finishes (except while still waiting),
	or after leaving a queue.
	*/
	static proto ServerInfo GetPreviousServer();
	/*!
	Returns the active queue data if the user is currently waiting in a server join queue, or null.
	\warning This becomes null after the user is approved for join, leaves the queue, or an error occurs.
	*/
	static proto ServerQueueData GetQueueData();
}

/*!
\}
*/
