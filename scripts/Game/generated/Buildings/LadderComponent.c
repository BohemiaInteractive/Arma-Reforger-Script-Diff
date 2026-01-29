/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Buildings
\{
*/

class LadderComponent: GameComponent
{
	/*!
	Check if the position is above the ladder.
	\param pPositionWS Position in world space which will be checked
	\return Return true if the position is above the ladder
	*/
	proto external bool IsAbove(vector pPositionWS);
	/*!
	Enable or disable the top front entry of the ladder.
	Use when for design reasons you need to allow the associated DoorComponent's door to be entered even when closed,
	or vice-versa, prevent from being entered even when open.
	This call is synchronized in MP, and only has effect on the server.
	\param entryState Desired state.
	*/
	proto external void SetEnabledEntry(ELadderDoorState entryState);
}

/*!
\}
*/
