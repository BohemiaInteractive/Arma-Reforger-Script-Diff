/** @ingroup Editor_Entities
*/

//! Simplified action conditions.
enum EEditorCommandActionFlags
{
	WAYPOINT = 1, ///< Can place waypoint
	OBJECTIVE = 2, ///< Can place objective
	IS_QUEUE = 4,
	ATTACH = 8,
};