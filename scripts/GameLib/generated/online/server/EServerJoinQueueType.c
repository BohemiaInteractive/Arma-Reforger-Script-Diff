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
Identifies which join queue a player belongs to.
Players are served in strict priority order: a player's turn comes only when they are at position 0
in their queue and all higher-priority queues are empty.
UNKNOWN is a special sentinel value with no defined priority.
*/
sealed enum EServerJoinQueueType
{
	//! Queue type was not specified or is not recognized by this version of the client.
	UNKNOWN,
	//! Synthetic client-side state: player is BUSY and either joined while a slot was available, or reached position 0 in another queue. Has a fixed expiration timer not refreshed by polling.
	UNREADY,
	//! Priority 1 (highest). Player had a slot on the server, disconnected, and is rejoining within the configured reconnect threshold. The server must first free the old slot before the player can rejoin.
	RECONNECTING,
	//! Priority 2. Player has priority access (server admin or whitelisted).
	PRIORITY,
	//! Priority 3. Player transitioned from BUSY (UNREADY) state to READY while waiting.
	READY,
	//! Priority 4 (lowest). Standard player with no special access.
	REGULAR,
}

/*!
\}
*/
