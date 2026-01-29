/** @ingroup Editor_UI
*/

//! Editor cursor type. Values must match cursor IDs in gproj!
enum EEditorCursor
{
	DEFAULT = 0,
	FOCUS = 1, ///< Hovering over editable entity
	PLACE = 2, ///< Placing a new entity
	TRANSFORM = 3, ///< Transforming an entity
	TRANSFORM_DISABLED = 4, ///< Transforming, but confirming is not possible (e.g., cursor is on the sky)
	WAITING = 5, ///< Waiting for response from server (e.g., when placing was initiated)
	MOVE_CAMERA = 6, ///< Rotating camera
	TRANSFORM_SNAP = 7, ///< Edited entity is over compatible entity
	TRANSFORM_SNAP_DISABLED = 8, ///< Edited entity is over incompatible entity
	ROTATE = 9, ///< Rotating an entity
	WAYPOINT = 10, ///< Placing a waypoint
	OBJECTIVE = 11, ///< Placing an objective
	TRANSFORM_GEOMETRY = 13, ///< Transform over geometry
	PLACE_GEOMETRY = 14, ///< Transform over geometry
};