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
 5. On EACODE_ERROR_PLAYER_IS_BANNED — the join was refused because the local player is
    banned. Call GetResultData() on callback used for join to retrieve the BanEntry containing details.

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
	/*!
	Sets whether the user is ready to connect to the server. This status is sent to the backend
	as part of join and queue poll requests and controls how the backend handles the user.
	Takes effect on the next join request or queue poll. When called while in the queue,
	an immediate poll is triggered to notify the backend of the status change.

	When BUSY:
	 - Joining with a free slot available: user enters EServerJoinQueueType::UNREADY limbo instead of joining immediately (requires server to have a queue).
	 - Joining with server full: user is placed in the join queue normally.
	 - Joining with no queue on server: join is denied with EApiCode::EACODE_ERROR_SERVER_JOIN_UNREADY_DENIED.
	 - Waiting in queue and reaching position 0: user is moved to EServerJoinQueueType::UNREADY limbo.
	 - Already in EServerJoinQueueType::READY queue: user is moved to EServerJoinQueueType::UNREADY limbo.

	When READY:
	 - Free slot available: user joins immediately.
	 - Server full: user is placed in the join queue normally.
	 - Currently in EServerJoinQueueType::UNREADY limbo: user is moved to EServerJoinQueueType::READY queue on the next poll.
	 - Waiting in any other queue: queue position is preserved.
	*/
	static proto void SetUserStatus(bool isReady);
	//! Returns whether the user is currently set as ready to connect to the server.
	static proto bool IsUserReady();
}

/*!
\}
*/
