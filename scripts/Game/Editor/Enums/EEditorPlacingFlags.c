[EnumBitFlag()]
enum EEditorPlacingFlags
{
	CHARACTER_PLAYER,	//!< Character is placed as player
	VEHICLE_CREWED,		//!< Vehicle is placed with crew members inside
	TASK_INACTIVE,		//!< Task is placed inactive, e.g player won't see it
	VEHICLE_PASSENGER,	//!< Vehicle is placed with passenger members inside
	PLACING_ACTION,		//!< placed entity is a result of an action
}
