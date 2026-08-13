/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_server
\{
*/

//! Type of server join queue. Different queue types have different processing priorities — a priority queue is processed before the regular queue.
sealed enum EServerJoinQueueType
{
	//! Unknown type of queue (can be unhandled type or intentionally not specified)
	UNKNOWN,
	//! Regular type of queue for all players
	REGULAR,
	//! Priority queue which is processed before REGULAR type of queue.
	PRIORITY,
}

/*!
\}
*/
