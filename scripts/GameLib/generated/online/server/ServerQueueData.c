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
Represents the local user's active presence in a server join queue.
Provides the user's current position and allows setting up a polling callback to receive
queue status updates and leaving the queue.

Obtained through ServerJoinManagerApi::GetQueueData() after a join request
results in EApiCode::EACODE_SERVER_QUEUE_WAIT.

While in the queue, the system automatically polls the backend at regular intervals.
Each poll can result in:
 - EApiCode::EACODE_SERVER_QUEUE_WAIT — still waiting, position may have changed.
 - EApiCode::EACODE_SERVER_JOIN — approved to join, proceed with connection.

This object becomes null after the user leaves the queue, is approved for join,
or an error occurs.
*/
sealed class ServerQueueData: ServerQueueInfo
{
	private void ServerQueueData();
	private void ~ServerQueueData();

	/*!
	Returns the user's current position in the queue, starting from 0.
	Position 0 means the user is next to be admitted to the server.
	This value can also be read as how many people are ahead of the user in the queue.
	*/
	proto external int GetPosition();
	/*!
	Sets callback to receive queue polling results. While in the queue, the system
	automatically polls the server at regular intervals. Each poll triggers this callback.

	Use the callback to update your UI with the latest queue position and to detect
	when the user is approved to join or when the queue status changes.

	\note You can reuse the same callback used for ServerInfo::RequestJoin() since both
	      use similar result codes.

	API codes delivered to the callback:
	 - OnSuccess:
	   - EApiCode::EACODE_SERVER_QUEUE_WAIT — still waiting, update UI with new position.
	   - EApiCode::EACODE_SERVER_JOIN — user is approved to join the server. Proceed with connection setup.
	   - EApiCode::EACODE_SERVER_QUEUE_LEFT — user has left the queue (after calling RequestLeave).
	 - OnError:
	   - EApiCode::EACODE_ERROR_SERVER_QUEUE_EXPIRED — the queue slot expired due to inactivity.
	   - EApiCode::EACODE_ERROR_SERVER_QUEUE_NOT_FOUND — user is not in the queue.

	\warning After any result except EACODE_SERVER_QUEUE_WAIT, this ServerQueueData object
	         becomes null after the callback finishes. The data is still valid during callback processing.
	\warning Removing this callback does not cancel polling. Use RequestLeave() to leave the queue.
	\warning This callback is not preserved across script reloads. If the user is still
	         in the queue after reload, you must set the callback again.
	*/
	proto external void SetPollingCallback(BackendCallback callback);
	/*!
	Returns the currently set polling callback, or null if none is set.
	Can be used to check whether a callback is already assigned before setting a new one.
	*/
	proto external ref BackendCallback GetPollingCallback();
	/*!
	Sends a request to leave this queue. The result is delivered through the provided callback.
	\note You can reuse the polling callback if it handles EApiCode::EACODE_SERVER_QUEUE_LEFT.
	\warning After the callback finishes, this ServerQueueData object becomes null. The data is still valid during callback processing.
	*/
	proto external void RequestLeave(BackendCallback callback);
}

/*!
\}
*/
