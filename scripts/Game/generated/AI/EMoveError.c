/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

enum EMoveError
{
	OK,
	STUCK,
	STOPPED,
	UNREACHABLE,
	WAITING_ON_NAVLINK,
	ENTITY_CANT_MOVE,
	// current status prevent any movement - dead, uncon
	ENTITY_NOT_MOVABLE,
	// configuration error - missing component, navmesh etc.
	UNKNOWN,
}

/*!
\}
*/
